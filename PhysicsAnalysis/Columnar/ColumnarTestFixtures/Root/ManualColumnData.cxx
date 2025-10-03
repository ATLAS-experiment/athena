/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarTestFixtures/ManualColumnData.h>

#include <cstdint>
#include <stdexcept>

//
// method implementations
//

namespace columnar
{
  namespace TestUtils
  {
    namespace
    {
      template<typename T>
      T castGetAny (const std::string& columnName, const std::any& value)
      {
        if (value.type() == typeid(float))
          return std::any_cast<float> (value);
        if (value.type() == typeid(double))
          return std::any_cast<double> (value);
        if (value.type() == typeid(char))
          return std::any_cast<char> (value);
        if (value.type() == typeid(int))
          return std::any_cast<int> (value);
        if (value.type() == typeid(unsigned))
          return std::any_cast<unsigned> (value);
        if (value.type() == typeid(std::size_t))
          return std::any_cast<std::size_t> (value);
        throw std::logic_error (columnName + ": received unsupported input type " + boost::core::demangle(value.type().name()) + ", cast value or extend test handler to support it");
      }
      template<typename T>
      std::vector<T> castGetAnyVector (const std::string& columnName, const std::vector<std::any>& value)
      {
        std::vector<T> result;
        result.reserve (value.size());
        for (auto& v : value)
          result.push_back (castGetAny<T> (columnName, v));
        return result;
      }
      template<typename T>
      std::shared_ptr<void> castGetAnyColumn (const std::string& columnName, const std::vector<std::any>& value)
      {
        auto vec = std::make_shared<std::vector<T>> (castGetAnyVector<T> (columnName, value));
        return std::shared_ptr<void> (vec, vec->data());
      }
    }



    ManualColumnData ::
    ManualColumnData (std::vector<std::any>&& data)
      : m_data (std::move (data))
    {
    }



    void ManualColumnData ::
    configureType (const std::string& columnName, const std::type_info& type)
    {
      // I could check whether the type matches an already configured
      // type, but by not doing so I allow the user to reset the column
      // data if it got overwritten by the tool.

      if (type == typeid(float))
      {
        m_column = castGetAnyColumn<float> (columnName, m_data);
      } else if (type == typeid(char))
      {
        m_column = castGetAnyColumn<char> (columnName, m_data);
      } else if (type == typeid(int))
      {
        m_column = castGetAnyColumn<int> (columnName, m_data);
      } else if (type == typeid(std::uint8_t))
      {
        m_column = castGetAnyColumn<std::uint8_t> (columnName, m_data);
      } else if (type == typeid(std::uint16_t))
      {
        m_column = castGetAnyColumn<std::uint16_t> (columnName, m_data);
      } else if (type == typeid(std::uint32_t))
      {
        m_column = castGetAnyColumn<std::uint32_t> (columnName, m_data);
      } else if (type == typeid(std::uint64_t))
      {
        m_column = castGetAnyColumn<std::uint64_t> (columnName, m_data);
      } else
        throw std::runtime_error (
          columnName + ": column has unsupported type " +
          boost::core::demangle(type.name()) +
          ", extend test handler to support it");
      m_type = &type;
    }
  }
}