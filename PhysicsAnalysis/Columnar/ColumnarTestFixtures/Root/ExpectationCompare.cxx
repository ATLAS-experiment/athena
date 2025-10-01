/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarTestFixtures/ExpectationCompare.h>

#include <boost/core/demangle.hpp>
#include <gtest/gtest.h>

#include <iomanip>
#include <span>
#include <string>

//
// method implementations
//

namespace columnar
{
  namespace TestUtils
  {
    namespace
    {
      template<typename T> void checkExpectationTyped (const std::string& columnName, std::span<const T> output, std::span<const T> expectation)
      {
        SCOPED_TRACE (columnName);
        EXPECT_EQ (output.size(), expectation.size());
        for (std::size_t index = 0; index != std::min (output.size(), expectation.size()); ++ index)
        {
          SCOPED_TRACE (index);
          if constexpr (std::is_floating_point_v<T>)
            EXPECT_NEAR (output[index], expectation[index], 1e-6);
          else
            EXPECT_EQ (output[index], expectation[index]);
        }
      }
    }

    void checkExpectation (const std::string& columnName, const std::type_info& outputType, std::size_t outputSize, const void *outputData, const std::type_info& expectationType, std::size_t expectationSize, const void *expectationData)
    {
      SCOPED_TRACE (columnName);
      ASSERT_EQ (outputType, expectationType);
      if (outputType == typeid(float))
        checkExpectationTyped (columnName, std::span<const float> (static_cast<const float*> (outputData), outputSize), std::span<const float> (static_cast<const float*> (expectationData), expectationSize));
      else if (outputType == typeid(char))
        checkExpectationTyped (columnName, std::span<const char> (static_cast<const char*> (outputData), outputSize), std::span<const char> (static_cast<const char*> (expectationData), expectationSize));
      else if (outputType == typeid(int))
        checkExpectationTyped (columnName, std::span<const int> (static_cast<const int*> (outputData), outputSize), std::span<const int> (static_cast<const int*> (expectationData), expectationSize));
      else if (outputType == typeid(std::uint8_t))
        checkExpectationTyped (columnName, std::span<const std::uint8_t> (static_cast<const std::uint8_t*> (outputData), outputSize), std::span<const std::uint8_t> (static_cast<const std::uint8_t*> (expectationData), expectationSize));
      else if (outputType == typeid(std::uint16_t))
        checkExpectationTyped (columnName, std::span<const std::uint16_t> (static_cast<const std::uint16_t*> (outputData), outputSize), std::span<const std::uint16_t> (static_cast<const std::uint16_t*> (expectationData), expectationSize));
      else if (outputType == typeid(std::uint32_t))
        checkExpectationTyped (columnName, std::span<const std::uint32_t> (static_cast<const std::uint32_t*> (outputData), outputSize), std::span<const std::uint32_t> (static_cast<const std::uint32_t*> (expectationData), expectationSize));
      else if (outputType == typeid(std::uint64_t))
        checkExpectationTyped (columnName, std::span<const std::uint64_t> (static_cast<const std::uint64_t*> (outputData), outputSize), std::span<const std::uint64_t> (static_cast<const std::uint64_t*> (expectationData), expectationSize));
      else if (outputType == typeid(std::size_t))
        checkExpectationTyped (columnName, std::span<const std::size_t> (static_cast<const std::size_t*> (outputData), outputSize), std::span<const std::size_t> (static_cast<const std::size_t*> (expectationData), expectationSize));
      else
        throw std::logic_error ("received unsupported type " + boost::core::demangle(outputType.name()) + " for column compare, cast value or extend test handler to support it");
    }

    namespace
    {
      template<typename T> void printExpectedOutputTyped (const std::string& columnName, const std::span<const T>& output)
      {
        std::cout << "    columnMap.setExpectation (\"" << columnName << "\", {";
        for (std::size_t index = 0; index != output.size(); ++ index)
        {
          if (index != 0)
            std::cout << ", ";
          if constexpr (std::is_floating_point_v<T>){
            auto ss = std::cout.precision();
            std::cout << std::setprecision (8) << output[index];
            std::cout.precision(ss); //restore ostream state
          } else if constexpr (std::is_same_v<T,char>){
            std::cout << int (output[index]);
          } else {
            std::cout << output[index];
          }
        }
        std::cout << "});" << std::endl;
      }
    }

    void printExpectedOutput (const std::string& columnName, const std::type_info& outputType, std::size_t outputSize, const void *outputData)
    {
      if (outputType == typeid(float))
        printExpectedOutputTyped (columnName, std::span<const float> (static_cast<const float*> (outputData), outputSize));
      else if (outputType == typeid(char))
        printExpectedOutputTyped (columnName, std::span<const char> (static_cast<const char*> (outputData), outputSize));
      else if (outputType == typeid(int))
        printExpectedOutputTyped (columnName, std::span<const int> (static_cast<const int*> (outputData), outputSize));
      else if (outputType == typeid(std::uint8_t))
        printExpectedOutputTyped (columnName, std::span<const std::uint8_t> (static_cast<const std::uint8_t*> (outputData), outputSize));
      else if (outputType == typeid(std::uint16_t))
        printExpectedOutputTyped (columnName, std::span<const std::uint16_t> (static_cast<const std::uint16_t*> (outputData), outputSize));
      else if (outputType == typeid(std::uint32_t))
        printExpectedOutputTyped (columnName, std::span<const std::uint32_t> (static_cast<const std::uint32_t*> (outputData), outputSize));
      else if (outputType == typeid(std::uint64_t))
        printExpectedOutputTyped (columnName, std::span<const std::uint64_t> (static_cast<const std::uint64_t*> (outputData), outputSize));
      else if (outputType == typeid(std::size_t))
        printExpectedOutputTyped (columnName, std::span<const std::size_t> (static_cast<const std::size_t*> (outputData), outputSize));
      else
        throw std::logic_error ("received unsupported type " + boost::core::demangle(outputType.name()) + " for column printout, cast value or extend test handler to support it");
    }
  }
}
