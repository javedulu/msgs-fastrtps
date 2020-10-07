#ifndef _TRANSMIT_H_
#define _TRANSMIT_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/utils/IPLocator.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>


#include <thread>
#include <chrono>
#include <typeinfo>
#include <vector>
#include <memory>

#include "demangle.hxx"
#include "messagetype.h"

class PubListener : public eprosima::fastdds::dds::DataWriterListener
{   
    public:
        PubListener() :
            matched_(0), 
            first_connected_(false)
            {

            }

        ~PubListener() override {}

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        int matched_ = 0;
        bool first_connected_ = false;
} ;

template <typename messageType>
class Transmitter
{
    public: 
        Transmitter(std::string topicname);
        virtual ~Transmitter();
        bool init();
        void run();
        void send(std::shared_ptr<messageType> t);
        bool init(
            const std::string& wan_ip,    
            unsigned short port,    
            bool use_tls,    
            const std::vector<std::string>& whitelist
        );
    private:
        eprosima::fastdds::dds::DomainParticipant* participant_;
        eprosima::fastdds::dds::Publisher* publisher_;
        eprosima::fastdds::dds::Topic* topic_;
        eprosima::fastdds::dds::DataWriter* writer_;
        eprosima::fastdds::dds::TypeSupport type_;
        PubListener listener_;
    private:   
        std::string topicname_;
        int domainID = 16;
        bool waitForListener = true;
        std::shared_ptr<messageType> msg_;
};

template <typename messageType>
inline Transmitter<messageType>::Transmitter(std::string topicname)
        : participant_(nullptr)
        , publisher_(nullptr)
        , topic_(nullptr)
        , writer_(nullptr)
        , type_(new PubSubType<messageType>())
        , topicname_(topicname)
        
{
}

template <typename messageType>
inline Transmitter<messageType>::~Transmitter()
{   
    if (writer_ != nullptr)
    {
        publisher_->delete_datawriter(writer_);
    }   
    if (publisher_ != nullptr)
    {   
        participant_->delete_publisher(publisher_);
    }   
    if (topic_ != nullptr)
    {   
        participant_->delete_topic(topic_);
    }
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
}       

template <typename messageType>
inline bool Transmitter<messageType>::init()
{
    msg_ = std::make_shared<messageType>();

    //CREATE THE PARTICIPANT
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = eprosima::fastrtps::Duration_t(1,0);
    pqos.name("Domain_pub");
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

    //CREATE THE PUBLISHER
    publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        return false;
    }

    std::cout<<"Creating topic with name "<< topicname_ << " @ " << TypeName<messageType>::Get() << std::endl;

    //CREATE THE TOPIC
    topic_ = participant_->create_topic(topicname_, TypeName<messageType>::Get() , eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    eprosima::fastdds::dds::DataWriterQos wqos; 
    wqos.history().kind = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 2;
    wqos.resource_limits().max_samples = 5; 
    wqos.resource_limits().allocated_samples = 5;
    wqos.reliable_writer_qos().times.heartbeatPeriod.seconds = 2;
    wqos.reliable_writer_qos().times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;
    wqos.reliability().kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
    wqos.publish_mode().kind = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;
    
    writer_ = publisher_->create_datawriter(topic_, wqos, &listener_);
    if (writer_ == nullptr)
    {
        return false;
    }

    std::cout<<"Created Data Transmit with Topic [RTPS Simple Discovery]"<< topicname_.c_str() << " & Type " << TypeName<messageType>::Get() << std::endl;

    return true;
}

template <typename messageType>
inline bool Transmitter<messageType>::init(
        const std::string& wan_ip,
        unsigned short port,
        bool use_tls,
        const std::vector<std::string>& whitelist)
{
    //CREATE THE PARTICIPANT
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = eprosima::fastrtps::Duration_t(1,0);
    pqos.name("Domain_pub");

    pqos.transport().use_builtin_transports = false;
    std::shared_ptr<eprosima::fastdds::rtps::TCPv4TransportDescriptor> descriptor = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
    
    for (std::string ip : whitelist)
    {   
        descriptor->interfaceWhiteList.push_back(ip);
        std::cout << "Whitelisted " << ip << std::endl;
    }
    
    if (use_tls)
    {   
        std::cerr<<"TLS Options still not supported"<<std::endl;
#if 0
        using TLSOptions = TCPTransportDescriptor::TLSConfig::TLSOptions;
        descriptor->apply_security = true;
        descriptor->tls_config.password = "test";
        descriptor->tls_config.cert_chain_file = "server.pem";
        descriptor->tls_config.private_key_file = "server.pem";
        descriptor->tls_config.tmp_dh_file = "dh2048.pem";
        descriptor->tls_config.add_option(TLSOptions::DEFAULT_WORKAROUNDS);
        descriptor->tls_config.add_option(TLSOptions::SINGLE_DH_USE);
        descriptor->tls_config.add_option(TLSOptions::NO_SSLV2);
#endif 
    }
    
    descriptor->wait_for_tcp_negotiation = false;
    descriptor->sendBufferSize = 8912896; // 8.5mb 
    descriptor->receiveBufferSize = 8912896; // 8.5mb
    
    if (!wan_ip.empty())
    {   
        descriptor->set_WAN_address(wan_ip);
        std::cout << wan_ip << ":" << port << std::endl;
    }
    descriptor->add_listener_port(port);
    pqos.transport().user_transports.push_back(descriptor);
    
    participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(domainID, pqos);
    
    if (participant_ == nullptr)
    {   
        return false;
    }
    
    //REGISTER THE TYPE
    type_.register_type(participant_);
    
    //CREATE THE PUBLISHER
    publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
    
    if (publisher_ == nullptr)
    {   
        return false;
    }
    
    //CREATE THE TOPIC
    topic_ = participant_->create_topic(topicname_, TypeName<messageType>::Get() , eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    
    if (topic_ == nullptr)
    {   
        return false;
    }
    
    //CREATE THE DATAWRITER
    eprosima::fastdds::dds::DataWriterQos wqos; 
    wqos.history().kind = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS;
    wqos.history().depth = 1;
    wqos.resource_limits().max_samples = 5; 
    wqos.resource_limits().allocated_samples = 5;
    wqos.reliable_writer_qos().times.heartbeatPeriod.seconds = 2;
    wqos.reliable_writer_qos().times.heartbeatPeriod.nanosec = 200 * 1000 * 1000;
    wqos.reliability().kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
    wqos.publish_mode().kind = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;

    writer_ = publisher_->create_datawriter(topic_, wqos, &listener_);
    
    if (writer_ == nullptr)
    {   
        return false;
    }

    std::cout<<"Created Data Transmit with Topic [TCP Mode] "<< topicname_.c_str() << " & Type " << TypeName<messageType>::Get() << std::endl;
    
    return true;
}


void PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        first_connected_ = true;
        std::cout << "DataWriter matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "DataWriter unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

template <typename messageType>
inline void Transmitter<messageType>::run()
{
    std::cout << "DataWriter waiting for DataReaders." << std::endl;
    while(listener_.matched_ == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250)); // Sleep 250 ms
    }

    // Publication code

    /* Initialize your structure here */

    int msgsent = 0;
    char ch = 'y';
    do
    {
        if(ch == 'y')
        {
            writer_->write(msg_.get());
            ++msgsent;
            std::cout << "Sending sample, count=" << msgsent << ", send another sample?(y-yes,n-stop): ";
        }
        else if(ch == 'n')
        {
            std::cout << "Stopping execution " << std::endl;
            break;
        }
        else
        {
            std::cout << "Command " << ch << " not recognized, please enter \"y/n\":";
        }
    } while(std::cin >> ch);
}

template <typename messageType>
inline void Transmitter<messageType>::send(std::shared_ptr<messageType> t)
{
    // This need's to be refined to not send if subscriber is not found.
    while(listener_.matched_ == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250)); // Sleep for 250ms
    }
    if (listener_.first_connected_ || !waitForListener || listener_.matched_ > 0)
    {
        std::cout<<"Writer invoked .. "<<std::endl;
        writer_->write(t.get());
    }
}

#endif //_TRANSMIT_H_
