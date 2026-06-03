/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
#ifndef CXXUTILS_TRANSPARENT_STRING_HASH_H
#define CXXUTILS_TRANSPARENT_STRING_HASH_H

#include <cstddef>
#include <functional> // std::hash
#include <string>
#include <string_view>

namespace CxxUtils {

  /**
   * @brief Transparent hash functor for string-like keys.
   *
   * This hash functor allows an associative container with
   * `std::string` keys to be searched using other string-like types, such as
   * `std::string_view` or `const char*`, without first constructing a temporary
   * `std::string`.
   *
   * The nested `is_transparent` type marks this functor as a transparent
   * hash functor, allowing heterogeneous lookup in containers such as
   * `std::unordered_map`.
   *
   * Example:
   *
   * @code
   * #include "CxxUtils/transparent_string_hash.h"
   *
   * #include <functional>
   * #include <string>
   * #include <string_view>
   * #include <unordered_map>
   *
   * using CLID = unsigned int;
   *
   * using NameMap = std::unordered_map<
   *   std::string,
   *   CLID,
   *   CxxUtils::TransparentStringHash,
   *   std::equal_to<> >;
   *
   * NameMap nameMap;
   * nameMap.emplace("MyClass", 123);
   *
   * const std::string key = "MyClass";
   * const std::string_view view = "MyClass";
   * const char* cstr = "MyClass";
   *
   * auto fromString = nameMap.find(key);
   * auto fromStringView = nameMap.find(view);
   * auto fromCString = nameMap.find(cstr);
   * @endcode
   *
   * @warning The `const char*` overload expects a non-null, null-terminated
   * string. For character buffers that are not null-terminated, use
   * `std::string_view`.
   */
  struct TransparentStringHash{
    using is_transparent = void;

    /**
     * @brief Hash a string view.
     * @param s String view to hash.
     * @return Hash value for @p s.
     */
    std::size_t
    operator()(std::string_view s) const{
      return std::hash<std::string_view>{}(s);
    }

    /**
     * @brief Hash a string.
     * @param s String to hash.
     * @return Hash value for @p s.
     */
    std::size_t
    operator()(const std::string& s) const{
      return (*this)(std::string_view{s});
    }

    /**
     * @brief Hash a C string.
     * @param s Null-terminated C string to hash.
     * @return Hash value for @p s.
     */
    std::size_t
    operator()(const char* s) const{
      return (*this)(std::string_view{s});
    }
  };

} // namespace CxxUtils

#endif