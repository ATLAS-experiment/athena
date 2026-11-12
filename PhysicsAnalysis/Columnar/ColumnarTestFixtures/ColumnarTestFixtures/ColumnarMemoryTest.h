/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TEST_FIXTURES__COLUMNAR_MEMORY_TEST_H
#define COLUMNAR_TEST_FIXTURES__COLUMNAR_MEMORY_TEST_H

#include <AsgTools/AsgTool.h>
#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarInterfaces/IColumnarTool.h>
#include <ColumnarCore/ColumnarDef.h>
#include <ColumnarCore/ColumnInfoHelpers.h>
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
    /// @warning Do **not** try to replace the template with referencing
    /// ColumnarMode directly in the source file. This construct is
    /// meant to make the `ColumnarTestFixturesLib` library have no
    /// direct dependency on `ColumnarMode`.
    template<ColumnarModeConcept CM = ColumnarMode>
    static bool checkMode () { return std::is_same_v<CM,ColumnarModeArray>; }

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

      /// @brief add the key and index columns for a key+index link
      ///
      /// The key gets calculated for the given target container, with
      /// the link type determining the CLID.
      template<typename ContainerType>
      void addKeyIndexLink (const std::string& name, const std::string& targetName, const std::vector<std::uint32_t>& indices)
      {
        using CM = ColumnarModeArray;
        const auto key = computeSgKey (targetName, ClassID_traits<ContainerType>::ID());
        addColumn (name + "." + std::string (CM::linkKeyColumnName), std::vector<std::any> (indices.size(), std::any (key)));
        addColumn (name + "." + std::string (CM::linkIndexColumnName), std::vector<std::any> (indices.begin(), indices.end()));
      }

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

      /// @brief fill the event-index columns requested by the tool that
      /// were not provided by the user
      void fillEventIndexColumns ();

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
