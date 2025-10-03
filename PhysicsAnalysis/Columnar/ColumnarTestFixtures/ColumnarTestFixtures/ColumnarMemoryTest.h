/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES__COLUMNAR_MEMORY_TEST_H
#define COLUMNAR_TEST_FIXTURES__COLUMNAR_MEMORY_TEST_H

#include <AsgTools/AsgTool.h>
#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarInterfaces/IColumnarTool.h>
#include <ColumnarTestFixtures/ToolWrapper.h>
#include <ColumnarTestFixtures/ManualColumnData.h>
#include <PATInterfaces/ISystematicsTool.h>

#include <gtest/gtest.h>

#include <optional>

namespace columnar
{
  namespace TestUtils
  {
    class ColumnarTestToolHandle;
    struct ColumnMapType;
  }

  struct ColumnarMemoryTest : public testing::Test
  {
    ColumnarMemoryTest ();

    /// \brief make a unique tool name to be used in unit tests
    std::string makeUniqueName ();

    /// @brief check whether we have the right mode
    static bool checkMode ();

    using ColumnMapType = TestUtils::ColumnMapType;
    using ColumnarTestToolHandle = TestUtils::ColumnarTestToolHandle;
  };



  namespace TestUtils
  {
    /// @brief a handle to a columnar tool for running tests
    ///
    /// This used to be shared with the python bindings, but there are
    /// sufficient differences between testing and python bindings to
    /// split the two.

    class ColumnarTestToolHandle final
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
      [[nodiscard]] ToolColumnVectorMap& getToolWrapper ();
      [[nodiscard]] const ToolColumnVectorMap& getToolWrapper () const;

      /// get the column header
      [[nodiscard]] const ColumnVectorHeader& getColumnHeader () const {
        return *m_columnHeader;}

      /// get the contained tool
      [[nodiscard]] IColumnarTool* getTool ();



      /// Private Members
      /// ===============

    private:

      IColumnarTool* m_tool = nullptr;
      CP::ISystematicsTool* m_systTool = nullptr;

      std::shared_ptr<ColumnVectorHeader> m_columnHeader;
      std::shared_ptr<ToolColumnVectorMap> m_toolWrapper;
    };



    struct ColumnMapType final
    {
      ColumnMapType (ColumnarTestToolHandle& val_toolHandle);

      void addColumn (const std::string& name, std::vector<std::any> data);

      std::size_t columnSize (const std::string& name);

      void setExpectation (const std::string& name, std::vector<std::any> values);

      /// @brief add the columns we have to the tool
      void connectColumnsToTool ();

      void call ();

      void checkExpectations ();

      template<typename T> void addTypedColumn (const std::string& name, std::vector<T> data)
      {
        addColumn (name, std::vector<std::any> (data.begin(), data.end()));
      }

    private:

      ColumnarTestToolHandle* m_toolHandle = nullptr;

      std::unique_ptr<ColumnVectorData> m_columnData;

      struct MyColumnData final
      {
        std::optional<ColumnInfo> info;
        std::optional<ManualColumnData> input;
        std::optional<ManualColumnData> expectation;
      };
      std::unordered_map<std::string,MyColumnData> m_userColumns;
    };
  }
}

#endif
