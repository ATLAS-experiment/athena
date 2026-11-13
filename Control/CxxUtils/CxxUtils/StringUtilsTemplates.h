/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CXXUTILS_STRINGUTILSTEMPLATES_H
#define CXXUTILS_STRINGUTILSTEMPLATES_H

#include <string>
#include <string_view>
#include <vector>
#include <concepts>
#include <CxxUtils/StringUtils.h>
#include <stdexcept>
#include <type_traits>
#include <span>
#include <cstring>

namespace CxxUtils {

    template <typename T>
    concept Numeric = std::integral<T> || std::floating_point<T>;
    
    template <Numeric dType, bool silenceEmpty = true> 
    void convertToNumber(std::string_view str, dType& number) {
        str = trimWhiteSpaces(str);
        if constexpr(silenceEmpty){
           if (str.empty()) {
               number = 0;
               return;
           }
        }
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), number);
    
        if (ec != std::errc{}) {
            throw std::runtime_error("CxxUtils::convertToNumber() - Invalid numeric string: " + std::string(str));
        }
    }

    // A generic tokenizer that returns views
    template <typename T = std::string, typename X = std::string_view>
    std::vector<T> tokenize(std::string_view str, X delimiters) {
        std::vector<T> tokens;
        size_t lastPos = str.find_first_not_of(delimiters, 0);
        size_t pos = str.find_first_of(delimiters, lastPos);
    
        while (lastPos != std::string_view::npos) {
            std::string_view token = str.substr(lastPos, pos - lastPos);
            
            if constexpr (std::is_same_v<T, std::string_view>) {
                tokens.push_back(token);
            } else if constexpr (std::is_same_v<T, std::string>) {
                tokens.emplace_back(token);
            } else {
                // This handles the int/double cases via your existing convertToNumber
                T value;
                convertToNumber(token, value);
                tokens.push_back(value);
            }
    
            lastPos = str.find_first_not_of(delimiters, pos);
            pos = str.find_first_of(delimiters, lastPos);
        }
        return tokens;
    }


    namespace detail{

        /**
         * @brief Computes the exact character length of a given argument at compile time or runtime.
         *
         * This helper function inspects the type of the argument (`T`) and determines its 
         * serialized length without constructing temporary string objects:
         * - Single `char` values contribute 1 byte.
         * - Raw character arrays and string literals exclude the null-terminator.
         * - Types featuring a static or member `stringSize()` method (such as `Guid`) 
         *   return their fixed string size trait.
         * - All other string-like types use `std::string_view`.
         *
         * @tparam T The type of the argument.
         * @param arg The argument value or reference.
         * @return constexpr size_t The exact character length of the argument.
         */
        template<typename T>
        constexpr size_t get_arg_size(const T& arg) noexcept {
            using DecayT = std::decay_t<T>;
            if constexpr (std::is_same_v<DecayT, char>) {
                return 1;
            }
            else if constexpr (std::is_array_v<std::remove_reference_t<T>>) {
                using CharType = std::remove_cv_t<std::remove_extent_t<std::remove_reference_t<T>>>;
                if constexpr (std::is_same_v<CharType, char>) {
                    return sizeof(std::remove_reference_t<T>) - 1;
                } else {
                    return std::string_view(arg, sizeof(std::remove_reference_t<T>) / sizeof(CharType) - 1).size();
                }
            }
            // 1. Handle types with a static stringSize() trait (e.g., Guid)
            else if constexpr (requires { DecayT::stringSize(); }) {
                return DecayT::stringSize();
            }
            // 2. Handle types with a member stringSize() method
            else if constexpr (requires { arg.stringSize(); }) {
                return arg.stringSize();
            }
            else {
                return std::string_view(arg).size();
            }
        }
        

        /**
         * @brief Variadic helper that writes and serializes multiple arguments directly 
         *        into a raw target character buffer.
         *
         * Using fold expressions and `if constexpr` routing, this function serializes 
         * each argument directly into the target memory location (`p`) and advances the pointer:
         * - Single characters are assigned and incremented.
         * - Raw arrays and string literals are copied via `std::memcpy`.
         * - Custom objects with sizing traits (like `Guid`) write directly into a bounded 
         *   `std::span` buffer, avoiding any intermediate heap allocations.
         * - General string types fall back to standard `std::string_view` block copying.
         *
         * @tparam Args Variadic parameter pack representing the elements to append.
         * @param p Reference to the current write pointer in the destination buffer (advanced in-place).
         * @param args The items to append.
         */
        template<typename... Args>
        constexpr void append_to(char*& p, Args&&... args) {
            [[maybe_unused]] auto write_arg = [&p](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                using RawT = std::remove_reference_t<decltype(arg)>;
                
                if constexpr (std::is_same_v<T, char>) {
                    *p++ = arg;
                } 
                else if constexpr (std::is_array_v<RawT>) {
                    using CharType = std::remove_cv_t<std::remove_extent_t<RawT>>;
                    if constexpr (std::is_same_v<CharType, char>) {
                        constexpr size_t N = sizeof(RawT) - 1;
                        std::memcpy(p, arg, N);
                        p += N;
                    } else {
                        std::string_view sv(arg, sizeof(RawT) / sizeof(CharType) - 1);
                        std::memcpy(p, sv.data(), sv.size());
                        p += sv.size();
                    }
                }
                // Serialize types with static stringSize() directly into the span buffer
                else if constexpr (requires { T::stringSize(); }) {
                    constexpr size_t N = T::stringSize();
                    arg.toString(std::span<char, N>(p, N));
                    p += N;
                }
                // Serialize types with member stringSize() directly into the span buffer
                else if constexpr (requires { arg.stringSize(); }) {
                    const size_t N = arg.stringSize();
                    arg.toString(std::span<char>(p, N));
                    p += N;
                }
                else {
                    std::string_view sv = arg;
                    std::memcpy(p, sv.data(), sv.size());
                    p += sv.size();
                }
            };
            (write_arg(std::forward<Args>(args)), ...);
        }
    };
   /**
    * @brief Concatenates multiple string-like, character, or custom formatted 
    *        objects into a single std::string with zero intermediate allocations.
    *
    * This function computes the exact required buffer size upfront using fold 
    * expressions and `get_arg_size`, then writes directly into a pre-allocated 
    * string buffer using `std::string::resize_and_overwrite`. This avoids the 
    * performance overhead of temporary string copies and multiple reallocations.
    *
    * Supported argument types automatically handled by the function include:
    * - Single `char` values
    * - Raw character arrays and string literals (size computed without null-terminators)
    * - `std::string_view` and `std::string`
    * - Custom types with compile-time or runtime string sizing traits (e.g., `Guid`)
    *
    * Use this function when to concatenate quickly, can be prefered over std::format
    * for simple cases.
    * 
    * @tparam Args Variadic template types of the elements to concatenate.
    * @param args The components (labels, IDs, text segments) to join together.
    * @return std::string The final concatenated string.
    *
    * @example
    * std::string result = CxxUtils::concat(
    *     LABEL_DB, m_dbID, ']', 
    *     LABEL_CNT, m_cntID, ']', 
    *     m_auxString
    * );
    */
    template<typename... Args>
    constexpr std::string strcat(Args&&... args) {
        static_assert(sizeof...(Args) > 0, "strcat() requires at least one argument.");
        const size_t total_size = ( ... + detail::get_arg_size(args));
    
        std::string res;
        res.resize_and_overwrite(total_size, [&](char* buf, size_t) {
            char* p = buf;
            detail::append_to(p, std::forward<Args>(args)...);
            return static_cast<size_t>(p - buf);
        });
        return res;
    }
}

#endif
