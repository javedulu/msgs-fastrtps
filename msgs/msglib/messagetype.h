#ifndef _PUB_SUB_TYPE_H_
#define _PUB_SUB_TYPE_H_

#include <fastrtps/TopicDataType.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>
#include "demangle.hxx"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

template <typename messageType>
class PubSubType : public eprosima::fastrtps::TopicDataType
{
    public:
        typedef messageType type;
    
        PubSubType();
        virtual ~PubSubType();
        
        bool serialize(void *data, 
            eprosima::fastrtps::rtps::SerializedPayload_t *payload);
        bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data);
            std::function<uint32_t()> getSerializedSizeProvider(void* data);
        bool getKey(void *data, 
            eprosima::fastrtps::rtps::InstanceHandle_t *ihandle, bool force_md5);
        void* createData();
        void deleteData(void * data);
    public:
        MD5 m_md5;
        unsigned char* m_keyBuffer;
};

template <typename messageType>
inline PubSubType<messageType>::PubSubType()
{
    setName(TypeName<messageType>::Get());
    m_typeSize = (uint32_t)messageType::getMaxCdrSerializedSize() + 4 /*encapsulation*/;
    m_isGetKeyDefined = messageType::isKeyDefined();
    m_keyBuffer = (unsigned char*)malloc(messageType::getKeyMaxCdrSerializedSize()>16 ? messageType::getKeyMaxCdrSerializedSize() : 16);
}

template <typename messageType>
inline PubSubType<messageType>::~PubSubType()
{
    if(m_keyBuffer!=nullptr)
        free(m_keyBuffer);
}

template <typename messageType>
inline bool PubSubType<messageType>::serialize(void *data, SerializedPayload_t *payload) {
    messageType *p_type = (messageType*) data;
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload->data, payload->max_size); // Object that manages the raw buffer.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
    // Serialize encapsulation
    ser.serialize_encapsulation();

    try
    {
        p_type->serialize(ser); // Serialize the object:
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    payload->length = (uint32_t)ser.getSerializedDataLength(); 	//Get the serialized length
    return true;
}

template <typename messageType>
bool PubSubType<messageType>::deserialize(SerializedPayload_t* payload, void* data) {
    messageType* p_type = (messageType*) data; 	//Convert DATA to pointer of your type
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length); 	// Object that manages the raw buffer.
    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.
    // Deserialize encapsulation.
    deser.read_encapsulation();
    payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    try
    {
        p_type->deserialize(deser); //Deserialize the object:
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    return true;
}

template <typename messageType>
std::function<uint32_t()> PubSubType<messageType>::getSerializedSizeProvider(void* data) {
    return [data]() -> uint32_t {
        return (uint32_t)type::getCdrSerializedSize(*static_cast<messageType*>(data)) + 4 /*encapsulation*/;
    };
}

template <typename messageType>
void* PubSubType<messageType>::createData() {
    return (void*)new messageType();
}

template <typename messageType>
void PubSubType<messageType>::deleteData(void* data) {
    delete((messageType*)data);
}

template <typename messageType>
bool PubSubType<messageType>::getKey(void *data, InstanceHandle_t* handle, bool force_md5) {
    if(!m_isGetKeyDefined)
        return false;
    messageType* p_type = (messageType*) data;
    eprosima::fastcdr::FastBuffer fastbuffer((char*)m_keyBuffer,messageType::getKeyMaxCdrSerializedSize()); 	// Object that manages the raw buffer.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS); 	// Object that serializes the data.
    p_type->serializeKey(ser);
    if(force_md5 || messageType::getKeyMaxCdrSerializedSize()>16)	{
        m_md5.init();
        m_md5.update(m_keyBuffer,(unsigned int)ser.getSerializedDataLength());
        m_md5.finalize();
        for(uint8_t i = 0;i<16;++i)    	{
            handle->value[i] = m_md5.digest[i];
        }
    }
    else    {
        for(uint8_t i = 0;i<16;++i)    	{
            handle->value[i] = m_keyBuffer[i];
        }
    }
    return true;
}


#endif //_PUB_SUB_TYPE_H_


