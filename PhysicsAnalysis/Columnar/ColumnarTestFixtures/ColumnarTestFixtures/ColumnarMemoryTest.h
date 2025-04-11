/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES__COLUMNAR_MEMORY_TEST_H
#define COLUMNAR_TEST_FIXTURES__COLUMNAR_MEMORY_TEST_H

#include <gtest/gtest.h>

#include <AsgTools/AsgTool.h>
#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarInterfaces/IColumnarTool.h>
#include <ColumnarToolWrapper/ColumnarToolHelpers.h>
#include <ColumnarToolWrapper/ColumnarToolWrapper.h>
#include <PATInterfaces/ISystematicsTool.h>
#include <PATInterfaces/SystematicsUtil.h>
#include <span>

namespace columnar
{
  struct ColumnarMemoryTest : public testing::Test
  {
    ColumnarMemoryTest ();

    /// \brief make a unique tool name to be used in unit tests
    std::string makeUniqueName ();

    /// @brief check whether we have the right mode
    static bool checkMode ();

    class ColumnarTestToolHandle;
    struct ColumnMapType;
  };



  /// @brief a handle to a columnar tool for running tests
  ///
  /// This used to be shared with the python bindings, but there are
  /// sufficient differences between testing and python bindings to
  /// split the two.

  class ColumnarMemoryTest::ColumnarTestToolHandle final
  {
    /// Public Members
    /// ==============

  public:

    explicit ColumnarTestToolHandle (asg::AsgTool& val_tool);

    /// rename the columns the tool uses
    void renameContainers (const std::vector<std::pair<std::string,std::string>>& renames);

    /// initialize the tool
    void initialize ();

    /// set the tool to apply the given systematic variation
    void applySystematicVariation (const std::string& sysName);

    /// get the expected column info
    [[nodiscard]] std::vector<ColumnInfo> getColumnInfo () const;

    /// get the expected column names
    std::vector<std::string> getColumnNames () const;

    /// get the recommended systematics
    std::vector<std::string> getRecommendedSystematics () const;

    /// get the tool wrapper
    [[nodiscard]] const ColumnarToolWrapper& getToolWrapper () const;

    /// get the contained tool
    [[nodiscard]] IColumnarTool* getTool ();



    /// Private Members
    /// ===============

  private:

    IColumnarTool* m_tool = nullptr;
    CP::ISystematicsTool* m_systTool = nullptr;

    std::shared_ptr<const ColumnarToolWrapper> m_toolWrapper;
  };



  struct ColumnarMemoryTest::ColumnMapType final
  {
    ColumnMapType (ColumnarTestToolHandle& val_toolHandle);

    void addColumn (const std::string& name, std::vector<std::any> data);

    void setExpectation (const std::string& name, const std::vector<std::any> & values);

    /// @brief add the columns we have to the tool
    void connectColumnsToTool ();

    void call ();

    void checkExpectations ();

  private:

    template<typename T> T extractAny (const std::string& columnName, const std::any& value)
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
      throw std::logic_error ("column " + columnName + " received unsupported input " + value.type().name() + ", cast value or extend test handler to support it");
    }

    template<typename T> void addInputTyped (const std::string& name, const std::vector<std::any>& data)
    {
      std::vector<T> typedData;
      for (auto& value : data)
        typedData.emplace_back (extractAny<T> (name, value));
      m_inputs.emplace (name, std::move (typedData));
    }

    template<typename T> void addTypedColumn (const std::string& name, std::vector<T> data)
    {
      auto column = m_columnMap.find (name);
      if (column == m_columnMap.end())
        throw std::runtime_error ("adding unknown column: " + name);
      if (m_inputs.contains (name))
        throw std::runtime_error ("column added twice: " + name);
      if (column->second.type != &typeid(T))
        throw std::runtime_error ("column " + name + " has wrong type: " + column->second.type->name());
      m_inputs.emplace (name, std::move (data));
    }

    template<typename T> void addExpectationTyped (const std::string& name, std::vector<std::any> data)
    {
      std::vector<T> typedData;
      for (auto& value : data)
        typedData.emplace_back (extractAny<T> (name, value));
      m_expectations.emplace (name, std::move (typedData));
    }

    ColumnarOffsetType columnSize (const std::string& name);

    template<typename T> std::span<const T> getOutputColumn (const std::string& name)
    {
      auto info = m_columnMap.find (name);
      if (info == m_columnMap.end())
        throw std::runtime_error ("output column not found: " + name);
      auto iter = m_activeColumns.find (name);
      if (iter == m_activeColumns.end())
        throw std::runtime_error ("output column not set: " + name);
      if (!std::holds_alternative<std::vector<T>> (iter->second))
        throw std::runtime_error ("output column has wrong type: " + name);
      return std::span<const T> (std::get<std::vector<T>> (iter->second));
    }

    template<typename T> void checkExpectationTyped (const std::string& columnName)
    {
      auto outputIter = m_activeColumns.find (columnName);
      if (outputIter == m_activeColumns.end())
        throw std::runtime_error ("output column not set: " + columnName);
      if (!std::holds_alternative<std::vector<T>> (outputIter->second))
        throw std::runtime_error ("output column has wrong type: " + columnName);
      auto& output = std::get<std::vector<T>> (outputIter->second);

      auto expectationIter = m_expectations.find (columnName);
      if (expectationIter == m_expectations.end())
        throw std::runtime_error ("output column not found: " + columnName);
      if (!std::holds_alternative<std::vector<T>> (expectationIter->second))
        throw std::runtime_error ("output column has wrong type: " + columnName);
      auto& expectation = std::get<std::vector<T>> (expectationIter->second);

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
      std::cout << "    m_columnMap.setExpectation (\"" << columnName << "\", {";
      for (std::size_t index = 0; index != expectation.size(); ++ index)
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

    ColumnarTestToolHandle* m_toolHandle = nullptr;

    std::unique_ptr<ColumnarToolWrapperData> m_columnData;

    std::unordered_map<std::string,const ColumnInfo> m_columnMap;

    std::unordered_map<std::string, std::variant<std::vector<float>,std::vector<char>,std::vector<int>,std::vector<std::uint8_t>,std::vector<std::uint16_t>,std::vector<std::uint32_t>,std::vector<std::uint64_t>>> m_inputs;
    std::unordered_map<std::string, std::variant<std::vector<float>,std::vector<char>,std::vector<int>,std::vector<std::uint8_t>,std::vector<std::uint16_t>,std::vector<std::uint32_t>,std::vector<std::uint64_t>>> m_activeColumns;
    std::unordered_map<std::string, std::variant<std::vector<float>,std::vector<char>,std::vector<int>,std::vector<std::uint8_t>,std::vector<std::uint16_t>,std::vector<std::uint32_t>,std::vector<std::uint64_t>>> m_expectations;
  };
}

#endif
