#ifndef _DEMANGLE_TYPE_INFO_H_
#define _DEMANGLE_TYPE_INFO_H_

#pragma once

#include <iostream>
#include <cstring>

#ifdef __GNUG__

#include <cstdlib>
#include <memory>
#include <cxxabi.h>

inline std::string demangle(const char *name)
{
    int status = -4; // some arbitrary value to eliminate the compiler warning
        // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    switch (status) {
        case 0:
            return res.get();
        case -1:
            std::cerr << "[Demangle : ] A memory allocation failure occurred.";
            break;
        case -2:
            std::cerr << "[Demangle :] name is not a valid name under the C++ ABI mangling rules.";
            break;
        case -3:
            std::cerr << "[Demangle :] One of the arguments is invalid.";
            break;
        default:
            std::cerr << "[Demangle :] Unknown error.";
            break;
    }

    return(status==0) ? res.get() : name;
}

#else
// does nothing if not g++
inline std::string demangle(const char* name) {
    return name;
}

#endif //__GNUG__
template <typename T>
struct TypeName
{
    static const char* Get()
    {
        //demangle returns a local string converting to c_str make's it invalid
        return strdup(demangle(typeid(T).name()).c_str());
    }
};

#endif //_DEMANGLE_TYPE_INFO_H_