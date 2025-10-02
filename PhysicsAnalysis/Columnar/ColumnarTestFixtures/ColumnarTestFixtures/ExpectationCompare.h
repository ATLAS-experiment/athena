/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES_EXPECTATION_COMPARE_H
#define COLUMNAR_TEST_FIXTURES_EXPECTATION_COMPARE_H

#include <string>
#include <typeinfo>

namespace columnar
{
  namespace TestUtils
  {
    void checkExpectation (const std::string& columnName, const std::type_info& outputType, std::size_t outputSize, const void *outputData, const std::type_info& expectationType, std::size_t expectationSize, const void *expectationData);

    void printExpectedOutput (const std::string& columnName, const std::type_info& outputType, std::size_t outputSize, const void *outputData);
  }
}

#endif
