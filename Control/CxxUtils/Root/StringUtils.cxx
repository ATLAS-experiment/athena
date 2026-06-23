/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <CxxUtils/StringUtils.h>
#include <CxxUtils/StringUtilsTemplates.h>

#include <limits>
#include <algorithm>


namespace CxxUtils {
    std::vector<std::string> tokenize(std::string_view str,
                                      std::string_view delimiters) {
                                    
       return tokenize<std::string, std::string_view>(str, delimiters);
    }

    std::vector<std::string> tokenize(std::string_view str,
                                      char delimiter) {                             
        return tokenize<std::string, char>(str, delimiter);
    }

    std::vector<double> tokenizeDouble(std::string_view str, std::string_view delimiters) {
        return tokenize<double, std::string_view>(str, delimiters);
    }

    std::string_view trimWhiteSpaces(std::string_view str) noexcept {
        auto is_not_space = [](unsigned char c) { return !std::isspace(c); };
        
        auto start = std::find_if(str.begin(), str.end(), is_not_space);
        auto end = std::find_if(str.rbegin(), str.rend(), is_not_space).base();
    
        return (start < end) ? std::string_view(start, end) : std::string_view{};
    }

    std::vector<int> tokenizeInt(std::string_view str, std::string_view delimiters) {
        return tokenize<int, std::string_view>(str, delimiters);
    }

    int atoi(std::string_view str) { 
        int result{std::numeric_limits<int>::max()};
        convertToNumber(str, result);
        return result;
    }

    double atof(std::string_view str) {       
        double result{std::numeric_limits<double>::max()};
        convertToNumber(str, result);
        return result;
    }
}
