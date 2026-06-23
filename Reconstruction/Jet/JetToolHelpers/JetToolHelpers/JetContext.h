/*
 * @file JetContext.h
 * @brief a class for storing arbitrary event data.
 * @date 2022-06-01
 *
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 *
 */
 
#ifndef JETTOOLHELPERS_JETCONTEXT_H
#define JETTOOLHELPERS_JETCONTEXT_H

#include <unordered_map>
#include <stdexcept>
#include <variant>
#include <type_traits>

namespace JetHelper {

    /// Class JetContext
    /// Designed to read AOD information related to the event, N vertices, Ntracks, mu etc ...

class JetContext {
    public:

        template <typename T> bool setValue(const std::string& name, const T value, bool allowOverwrite = false);
        template <typename T> void getValue(const std::string& name, T& value) const;
        template <typename T> T getValue(const std::string& name) const;

        bool isAvailable(const std::string& name) const {
            return m_dict_.find(name) != m_dict_.end();
        };

    private:
        std::unordered_map<std::string, std::variant<int, float>> m_dict_;
};

template <typename T> void JetContext::getValue(const std::string& name, T& value) const {
    if(isAvailable(name))
        value = std::get<T>(m_dict_.at(name));
    else
        throw std::invalid_argument(std::string("Key Error : ") + name + std::string(" not found in JetContext."));
}

template <typename T> T JetContext::getValue(const std::string& name) const {
    T value;
    getValue(name, value);
    return value;
}

template <typename T> bool JetContext::setValue(const std::string& name, const T value, bool allowOverwrite) {
    if(( !allowOverwrite && isAvailable(name)) ) return false;
        
    if constexpr (!std::is_same<T, int>::value && !std::is_same<T, float>::value) {
        static_assert(std::is_same<T, double>::value, "Unsupported type provided, please use integers or doubles.");
            m_dict_.insert_or_assign(name, (float) value);
    } else {
        m_dict_.insert_or_assign(name, value);         // insert returns pair with iterator and return code
    }
    return true;                                     // if true => insertion, if false => assignement.
}
} // namespace JetHelper
#endif

