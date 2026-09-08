/*
 * @file JetContext.h
 * @brief a class for storing arbitrary event data.
 * @date 2022-06-01
 *
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 *
 */
 
#ifndef JETTOOLHELPERS_JETCONTEXT_H
#define JETTOOLHELPERS_JETCONTEXT_H

#include <unordered_map>
#include <stdexcept>
#include <variant>
#include <type_traits>
#include <string>
#include <string_view>
#include <format>
#include "CxxUtils/transparent_string_hash.h"

namespace JetHelper {

    /// Class JetContext
    /// Designed to read AOD information related to the event, N vertices, Ntracks, mu etc ...

class JetContext {
    public:

        template <typename T> bool setValue(std::string_view name, const T value, bool allowOverwrite = false);
        template <typename T> void getValue(std::string_view name, T& value) const;
        template <typename T> T getValue(std::string_view name) const;

        bool isAvailable(std::string_view name) const {
            return m_dict_.find(name) != m_dict_.end();
        };

    private:
        std::unordered_map<std::string, std::variant<int, float>, 
          CxxUtils::TransparentStringHash,std::equal_to<> > m_dict_;
};

template <typename T> void JetContext::getValue(std::string_view name, T& value) const {
    //don't effectively 'find' twice
    auto it = m_dict_.find(name);
    if(it != m_dict_.end())
        value = std::get<T>(it->second);
    else
        throw std::invalid_argument(std::format("Key Error : {} not found in JetContext.",  name ));
}

template <typename T> T JetContext::getValue(std::string_view name) const {
    T value;
    getValue(name, value);
    return value;
}

template <typename T> bool JetContext::setValue(std::string_view name, const T value, bool allowOverwrite) {
    if(( !allowOverwrite && isAvailable(name)) ) return false;
        
    if constexpr (!std::is_same<T, int>::value && !std::is_same<T, float>::value) {
        static_assert(std::is_same<T, double>::value, "Unsupported type provided, please use integers or doubles.");
        m_dict_.insert_or_assign(std::string{name}, (float) value);
    } else {
        m_dict_.insert_or_assign(std::string{name}, value);         // insert returns pair with iterator and return code
    }
    return true;
}
} // namespace JetHelper
#endif

