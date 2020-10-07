#ifndef _MSG_CONFIGURATION_H_
#define _MSG_CONFIGURATION_H_

#include "enum.h"

enum class eConnectionMode 
{
    TCP_RTPS = 0,
    UDP_RTPS,
    SHARED_MEMORY
};

template<>
inline std::vector<std::pair<std::string,eConnectionMode>>enum_map()
{
    return {
        {"TCP_RTPS", eConnectionMode::TCP_RTPS},
        {"UDP_RTPS", eConnectionMode::UDP_RTPS},
        {"SHARED_MEMORY", eConnectionMode::SHARED_MEMORY}
    };  
}

#endif //_MSG_CONFIGURATION_H_