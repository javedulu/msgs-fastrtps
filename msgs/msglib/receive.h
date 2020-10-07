#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>

#include "demangle.hxx"
#include "messagetype.h"
#include <functional>


template<typename messageType>
class SubListener: public eprosima::fastdds::dds::DataReaderListener
{
    public:
        SubListener() : matched(0) , samples(0) 
        {
            msg_ = std::make_shared<messageType>();
        }
        ~SubListener() override { };
    public:
        void on_data_available( 
            eprosima::fastdds::dds::DataReader* reader) override;
        void on_subscription_matched( 
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;
        void on_sample_rejected(
            eprosima::fastdds::dds::DataReader *reader,
            const eprosima::fastdds::dds::SampleRejectedStatus& status) override;
        void on_sample_lost(
            eprosima::fastdds::dds::DataReader* reader, 
            const eprosima::fastdds::dds::SampleLostStatus& status) override;
    public:
        int matched = 0;
        uint32_t samples = 0;
        std::function<void(messageType)> fp = nullptr;
        std::shared_ptr<messageType> msg_;
};

template< typename messageType > 
class Receiver {
public:
    Receiver(std::string topicname);
    virtual ~Receiver();
    bool init();
    void run();
    void receive(std::function<void(messageType)> x);
    bool init(
        const std::string& wan_ip,
        unsigned short port,
        bool use_tls,
        const std::vector<std::string>& whitelist);
private:
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataReader* reader_;
    eprosima::fastdds::dds::TypeSupport type_;
    SubListener<messageType> listener_;
private:
    std::string topicname_;
    int domainID = 16;
};


template <typename messageType>
inline Receiver<messageType>::Receiver(std::string topicname)
        : participant_(nullptr)
        , subscriber_(nullptr)
        , topic_(nullptr)
        , reader_(nullptr)
        , type_(new PubSubType<messageType>())
        , topicname_(topicname)

{

}

template <typename messageType>
inline Receiver<messageType>::~Receiver()
{
    if (reader_ != nullptr)
    {   
        subscriber_->delete_datareader(reader_);
    }   
    if (topic_ != nullptr)
    {   
        participant_->delete_topic(topic_);
    }   
    if (subscriber_ != nullptr)
    {   
        participant_->delete_subscriber(subscriber_);
    }   
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
}


template <typename messageType>
inline bool Receiver<messageType>::init()
{
    //CREATE THE PARTICIPANT
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.name("Domain_sub");
    std::shared_ptr<eprosima::fastdds::rtps::UDPv4TransportDescriptor> udpv4descriptor = 
                                    std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
    udpv4descriptor->sendBufferSize = 65536;
    udpv4descriptor->receiveBufferSize = 65536;
    //TODO: To add initial_peer list , white list and wan address similar to TCP
    pqos.transport().user_transports.push_back(udpv4descriptor);
    pqos.transport().use_builtin_transports = false; //Custom UDP
    participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(domainID, pqos);
    if (participant_ == nullptr)
    {   
        return false;
    }   

    //REGISTER THE TYPE
    type_.register_type(participant_);

    //CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {   
        return false;
    }   

    //CREATE THE TOPIC
    topic_ = participant_->create_topic(
            topicname_,
            TypeName<messageType>::Get(),
            eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {   
        return false;
    }   

    //CREATE THE READER
    eprosima::fastdds::dds::DataReaderQos rqos = eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);
    if (reader_ == nullptr)
    {   
        return false;
    }   
    std::cout<<"Created Data Receiver with Topic [RTPS Simple Discovery Mode] "<< topicname_.c_str() << " & Type: " << TypeName<messageType>::Get() << std::endl;

    return true;
}

template <typename messageType>
inline bool Receiver<messageType>::init(
        const std::string& wan_ip,
        unsigned short port,
        bool use_tls,
        const std::vector<std::string>& whitelist)
{
    //CREATE THE PARTICIPANT
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    int32_t kind = LOCATOR_KIND_TCPv4;

    Locator_t initial_peer_locator;
    initial_peer_locator.kind = kind;

    std::shared_ptr<eprosima::fastdds::rtps::TCPv4TransportDescriptor> tcpv4descriptor = 
                                                std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();

    for (std::string ip : whitelist)
    {   
        tcpv4descriptor->interfaceWhiteList.push_back(ip);
        std::cout << "Whitelisted " << ip << std::endl;
    }   

    if (!wan_ip.empty())
    {   
        IPLocator::setIPv4(initial_peer_locator, wan_ip);
        std::cout << wan_ip << ":" << port << std::endl;
    }   
    else
    {   
        IPLocator::setIPv4(initial_peer_locator, "127.0.0.1");
    }   
    initial_peer_locator.port = port;
    pqos.wire_protocol().builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's meta channel

    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    //pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(5, 0);
    pqos.name("Domain_sub");

    pqos.transport().use_builtin_transports = false;

    if (use_tls)
    {   
        std::cerr<<"TLS Options still not supported"<<std::endl;
#if 0
        using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
        using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
        tcpv4descriptor->apply_security = true;
        tcpv4descriptor->tls_config.password = "test";
        tcpv4descriptor->tls_config.verify_file = "ca.pem";
        tcpv4descriptor->tls_config.verify_mode = TLSVerifyMode::VERIFY_PEER;
        tcpv4descriptor->tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
#endif 
    }   

    tcpv4descriptor->wait_for_tcp_negotiation = false;
    pqos.transport().user_transports.push_back(tcpv4descriptor);

    participant_ =eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(domainID, pqos);

    if (participant_ == nullptr)
    {   
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_);

    //CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    topic_ = participant_->create_topic(topicname_, TypeName<messageType>::Get() , eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);


    if (topic_ == nullptr)
    {
        return false;
    }

    //CREATE THE DATAREADER
    eprosima::fastdds::dds::DataReaderQos rqos;
    rqos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
    rqos.history().depth = 2;
    rqos.resource_limits().max_samples = 5;
    rqos.resource_limits().allocated_samples = 5;
    rqos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    rqos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);

    if (reader_ == nullptr)
    {
        return false;
    }

    std::cout<<"Created Data Receiver with Topic [TCP Mode] "<< topicname_.c_str() << " & Type " << TypeName<messageType>::Get() << std::endl;

    return true;
}

template <typename messageType>
inline void SubListener<messageType>::on_subscription_matched(
        eprosima::fastdds::dds::DataReader*,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched = info.total_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched = info.total_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

/*

void HelloWorldSubscriber::SubListener::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    if (reader->take_next_sample(hello_.get(), &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.instance_state == eprosima::fastdds::dds::ALIVE)
        {
            samples_++;
            const size_t data_size = hello_->data().size();
            // Print your structure data here.
            std::cout << "Message " << hello_->message() << " " << hello_->index()
                      << " RECEIVED With " << data_size << "(bytes) of Data. DataEnd = "
                      << (char*)&hello_->data()[data_size - 9] << std::endl;
        }
    }
}

*/

template <typename messageType>
inline void SubListener<messageType>::on_data_available(
        eprosima::fastdds::dds::DataReader* reader)
{
    eprosima::fastdds::dds::SampleInfo info;
    if (reader->take_next_sample((void *)(msg_.get()), &info) == eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
    {
        if (info.instance_state == eprosima::fastdds::dds::ALIVE)
        {
            // Print your structure data here.
            // or invoke the function pointer here .
            ++samples;
            std::cout << "Sample received, count=" << samples << std::endl;
            /*if (this->fp)
                this->fp(st);
            */
        }
    }
}

template <typename messageType>
inline void SubListener<messageType>::on_sample_rejected(
        eprosima::fastdds::dds::DataReader *reader,
        const eprosima::fastdds::dds::SampleRejectedStatus& status)
{

    std::cout << "Sample Rejected, count=" << status.last_reason << std::endl;
}

template <typename messageType>
inline void SubListener<messageType>::on_sample_lost(
            eprosima::fastdds::dds::DataReader* reader, 
            const eprosima::fastdds::dds::SampleLostStatus& status)
{

    std::cout << "Sample Lost , count=" << status.total_count << std::endl;
}

template <typename messageType>
inline void Receiver<messageType>::run()
{
    std::cout << "Waiting for Data, press Enter to stop the DataReader. "<<std::endl;
    std::cin.ignore();
    std::cout << "Shutting down the Subscriber." << std::endl;
}

template<typename messageType>
inline void Receiver<messageType>::receive(std::function<void(messageType)> x)
{
    this->listener_.fp = x;
}

#endif //_RECEIVER_H_
