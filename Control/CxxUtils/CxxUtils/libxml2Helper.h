/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <libxml/parser.h>
#include <string>
#include <string_view>
#include <charconv>
#include <type_traits>
#include <stdexcept>
#include "CxxUtils/checker_macros.h"

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

//Note you will need to include the package LibXml2 in your package to use these methods.

namespace CxxUtils{

template <typename T>
[[nodiscard]] T GetXmlAttr(xmlNodePtr node, const char* attrName, const T &defaultValue = T{})
    noexcept(std::is_arithmetic_v<T>)          // ← conditional on type
{
    // 1. Grab the raw property
    xmlChar* raw = xmlGetProp(node, BAD_CAST attrName);
    if (!raw) {
        return defaultValue;
    }

    // Convert to a string_view for easy handling without extra copies
    std::string_view sv(reinterpret_cast<const char*>(raw));
    T result = defaultValue;

    // 2. Handle conversion based on type
    if constexpr (std::is_same_v<T, std::string>) {
        result = std::string(sv);
    } else if constexpr (std::is_arithmetic_v<T>) {
        // std::from_chars requires a pointer range [first, last)
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
        
        // If parsing fails (e.g. non-numeric string), result remains defaultValue
        if (ec != std::errc{}) {
            result = defaultValue;
        }
    } else
    {
        static_assert(false, "Type not implemented");
    }

    // 3. The most important part: Free the libxml2 memory!
    xmlFree(raw);
    
    return result;
}

template <typename T>
void GetXmlAttrIfThere(xmlNodePtr node, const char* attrName, T &value)
{
    // 1. Grab the raw property
    xmlChar* raw = xmlGetProp(node, BAD_CAST attrName);
    if (!raw) {
        return;
    }

    // Convert to a string_view for easy handling without extra copies
    std::string_view sv(reinterpret_cast<const char*>(raw));

    // 2. Handle conversion based on type
    if constexpr (std::is_same_v<T, std::string>) {
        value = std::string(sv);
    } else if constexpr (std::is_arithmetic_v<T>) {
        // std::from_chars requires a pointer range [first, last)
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
        if (ec != std::errc{}){
            xmlFree(raw);
            throw std::runtime_error("Cannot read value");
        }
    } else
    {
        static_assert(false, "Type not implemented");
    }

    // 3. The most important part: Free the libxml2 memory!
    xmlFree(raw);
    
}

template <typename X, typename COL>
void AddXmlToCollectionMap(xmlNodePtr node, const char* attrName, COL &collection)
{
    // 1. Grab the raw property
    xmlChar* raw = xmlGetProp(node, BAD_CAST attrName);
    if (!raw) {
        return;
    }

    // Convert to a string_view for easy handling without extra copies
    std::string_view sv(reinterpret_cast<const char*>(raw));
    X result{};
    // 2. Handle conversion based on type
    if constexpr (std::is_same_v<X, std::string>) {
        result = std::string(sv);
    } else if constexpr (std::is_arithmetic_v<X>) {
        // std::from_chars requires a pointer range [first, last)
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
        if (ec != std::errc{}){
            xmlFree(raw);
            throw std::runtime_error("Cannot read value");
        }
    } else
    {
        static_assert(false, "Type not implemented");
    }

    // 3. The most important part: Free the libxml2 memory!
    xmlFree(raw);
    collection.emplace(attrName, std::move(result));
}

template <typename X, typename COL>
void AddXmlToCollection(xmlNodePtr node, const char* attrName, COL &collection)
{
    // 1. Grab the raw property
    xmlChar* raw = xmlGetProp(node, BAD_CAST attrName);
    if (!raw) {
        return;
    }

    // Convert to a string_view for easy handling without extra copies
    std::string_view sv(reinterpret_cast<const char*>(raw));
    X result{};
    // 2. Handle conversion based on type
    if constexpr (std::is_same_v<X, std::string>) {
        result = std::string(sv);
    } else if constexpr (std::is_arithmetic_v<X>) {
        // std::from_chars requires a pointer range [first, last)
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
        if (ec != std::errc{}){
            xmlFree(raw);
            throw std::runtime_error("Cannot read value");
        }
    } else
    {
        static_assert(false, "Type not implemented");
    }

    // 3. The most important part: Free the libxml2 memory!
    xmlFree(raw);
    collection.emplace_back(std::move(result));
}

};
