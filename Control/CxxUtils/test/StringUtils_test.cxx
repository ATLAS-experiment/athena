/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#undef NDEBUG


#include "CxxUtils/StringUtils.h"
#include <string>
#include <iostream>

int main() {
    /// Test conversion of simple integers
    std::string intStr{"42"};
    if (CxxUtils::atoi(intStr) != 42) {
        std::println (std::cerr, "String conversion of '{}' failed. Expected 42 but got {}",
                      intStr, CxxUtils::atoi(intStr));
        return 1;
    }
    std::string negIntStr{"-42"};
    if (CxxUtils::atoi(negIntStr) != -42) {
        std::println (std::cerr, "String conversion of '{}' failed. Expected -42 but got {}",
                      negIntStr, CxxUtils::atoi(negIntStr));
        return 1;
    }
    std::string strWspace{"42 "};
    if (CxxUtils::atoi(strWspace) != 42) {
        std::println (std::cerr, "String conversion of '{}' failed. Expected 42 but got {}",
                      strWspace, CxxUtils::atoi(strWspace));
        return 1;
    }
    std::string strWspace1{" 42"};
    if (CxxUtils::atoi(strWspace1) != 42) {
        std::println (std::cerr, "String conversion of '{}' failed. Expected 42 but got {}",
                      strWspace1, CxxUtils::atoi(strWspace1));
        return 1;
    }

    /// check the conversion of floating point numbers
    std::string floatStr{"42.66"};
    if (CxxUtils::atoi(floatStr) != 42) {
        std::println (std::cerr, "String conversion of '{}' failed. Expected 42 but got {}",
                      floatStr, CxxUtils::atoi(floatStr));
        return 1;
    }
    if (std::abs(CxxUtils::atof(floatStr) - 42.66) > std::numeric_limits<float>::epsilon()) {
        std::println (std::cerr, "String conversion of '{}' failed. Expected 42.66 but got {}",
                      floatStr, CxxUtils::atof(floatStr));
        return 1;        
    }
    /// check an empty string
    std::string emptyStr{};
    if (CxxUtils::atoi(emptyStr) || CxxUtils::atof(emptyStr)){
        std::println (std::cerr, "Empty strings should be converted to zero. Instead function returned {} ^ {}",
                      CxxUtils::atoi(emptyStr), CxxUtils::atof(emptyStr));
        return 1;
    }
    /// Check the scientific notation
    std::string scientNote{"42.e-3"};
    if ( std::abs(CxxUtils::atof(scientNote) - 42.e-3) > std::numeric_limits<float>::epsilon()) {
        std::println (std::cerr, "String conversion of {} failed. Expected 42.e-3 but got {}",
                      scientNote, CxxUtils::atof(scientNote));
    }
    std::string scientNote1{"43.2e-3"};
    if ( std::abs(CxxUtils::atof(scientNote1) - 43.2e-3) > std::numeric_limits<float>::epsilon()) {
        std::println (std::cerr, "String conversion of {} failed. Expected 43.2e-3 but got {}",
                      scientNote1, CxxUtils::atof(scientNote1));
    }
    /// Let's check the string list
    std::vector<std::string> items{"Forklift","RitterKokusnuss", "Train","Cake", "Tortoise", "PolarBear"};
    std::string itemStr{};
    for (const std::string& it : items) itemStr+=it+";";
    std::vector<std::string> splitList = CxxUtils::tokenize(itemStr, ";");
    if (splitList.size() != items.size()) {
        std::println (std::cerr, "The list {} is expected to be split into 6 tokens. But got {} elements.",
                      itemStr, splitList.size());
        for (const std::string& el : splitList) {
           std::println (std::cerr, " *** {}", el);
        }
        return 1;
    }
    for (size_t i = 0 ; i < items.size(); ++i){
        if (items[i] != splitList[i]) {
            std::println (std::cerr, "The {}-th element is converted wrongly. Expected '{}' got '{}'",
                          i, items[i], splitList[i]);
            return 1;
        }
    }
    return 0;
}