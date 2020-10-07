#ifndef _ENUM_H_
#define _ENUM_H_

#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <iostream>
#include <cstdio>
#include <typeinfo>

/* Returns an mapping from name to enum type */
template <typename EnumT> std::vector<std::pair<std::string, EnumT>> enum_map();
//
template <typename EnumT>
inline std::vector<std::string> enum_names()
{
    std::vector<std::string>v;
    for (const auto &p : enum_map<EnumT>())
        v.push_back(p.first);
    return v;
}
//
template <typename EnumT>
inline EnumT str2enum(const std::string& v)
{
    for (const auto&p : enum_map<EnumT>())
        if (p.first == v)
            return p.second;
    char buf[256]; snprintf(buf,sizeof(buf), "Enum: Couldn't match enum with name : <%s> : %s \n", typeid(EnumT).name(), v.c_str());
    throw std::runtime_error(buf);
    return static_cast<EnumT>(-1);
}
//
template<typename EnumT>
inline std::string enum2str(const EnumT v)
{
    for (const auto&p : enum_map<EnumT>())
        if (p.second == v)
            return p.first;
    throw std::runtime_error("ENUM: Couldn't convert enum to string , is map() defined ?");
    return "INVALID";
}
//
template <typename EnumT>
inline bool isvalid(const std::string& v)
{
    for (const auto&p : enum_map<EnumT>())
        if (p.first == v)
            return true;
    return false;
}



#endif //_ENUM_H_