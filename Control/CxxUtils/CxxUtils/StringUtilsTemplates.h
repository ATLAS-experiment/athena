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

}

#endif