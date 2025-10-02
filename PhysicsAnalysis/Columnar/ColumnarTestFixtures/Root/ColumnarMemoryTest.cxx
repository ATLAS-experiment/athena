/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarTestFixtures/ColumnarMemoryTest.h>

#include <ColumnarCore/ColumnarDef.h>
#include <ColumnarTestFixtures/ExpectationCompare.h>
#include <ColumnarToolWrapper/ColumnarToolHelpers.h>
#include <PATInterfaces/SystematicsUtil.h>

//
// method implementations
//

namespace columnar
{
  ColumnarMemoryTest ::
  ColumnarMemoryTest ()
  {
  }



  std::string ColumnarMemoryTest ::
  makeUniqueName ()
  {
    static std::atomic<unsigned> index = 0;
    return "UniqueMemoryTestTool" + std::to_string(++index);
  }



  bool ColumnarMemoryTest ::
  checkMode ()
  {
    return std::is_same_v<ColumnarModeDefault,ColumnarModeArray>;
  }



  namespace TestUtils
  {
    ColumnarTestToolHandle ::
    ColumnarTestToolHandle (asg::AsgTool& val_tool)
    {
      m_tool = dynamic_cast<IColumnarTool*> (&val_tool);
      if (m_tool == nullptr)
        throw std::runtime_error ("tool does not implement IColumnarTool");
      m_systTool = dynamic_cast<CP::ISystematicsTool*> (m_tool);
    }



    void ColumnarTestToolHandle ::
    renameContainers (const std::vector<std::pair<std::string,std::string>>& renames)
    {
      columnar::renameContainers (*m_tool, renames);
      if (m_toolWrapper)
      {
        m_columnHeader = std::make_shared<ColumnVectorHeader> ();
        m_toolWrapper = std::make_shared<ToolColumnVectorMap> (*m_columnHeader, *m_tool);
      }
    }



    void ColumnarTestToolHandle ::
    initialize ()
    {
      m_columnHeader = std::make_shared<ColumnVectorHeader> ();
      m_toolWrapper = std::make_shared<ToolColumnVectorMap> (*m_columnHeader, *m_tool);
    }



    void ColumnarTestToolHandle ::
    applySystematicVariation (const std::string& sysName)
    {
      // by convention setting a systematic on a non-systematics tool
      // will do nothing
      if (m_systTool == nullptr)
        return;
      if (!m_systTool->applySystematicVariation (CP::SystematicSet (sysName)).isSuccess())
        throw std::runtime_error ("failed to apply systematic variation");
    }



    [[nodiscard]] std::vector<ColumnInfo> ColumnarTestToolHandle ::
    getColumnInfo () const
    {
      if (!m_toolWrapper)
        throw std::runtime_error ("tool not initialized");
      return m_tool->getColumnInfo ();
    }



    std::vector<std::string> ColumnarTestToolHandle ::
    getColumnNames () const
    {
      if (!m_toolWrapper)
        throw std::runtime_error ("tool not initialized");
      return m_toolWrapper->getColumnNames ();
    }



    std::vector<std::string> ColumnarTestToolHandle ::
    getRecommendedSystematics () const
    {
      if (!m_systTool)
        return {""};
      std::vector<std::string> result;
      for (auto& sys : CP::make_systematics_vector (m_systTool->recommendedSystematics()))
        result.push_back (sys.name());
      return result;
    }



    [[nodiscard]] ToolColumnVectorMap& ColumnarTestToolHandle ::
    getToolWrapper ()
    {
      if (!m_toolWrapper)
        throw std::runtime_error ("tool not initialized");
      return *m_toolWrapper;
    }



    [[nodiscard]] const ToolColumnVectorMap& ColumnarTestToolHandle ::
    getToolWrapper () const
    {
      if (!m_toolWrapper)
        throw std::runtime_error ("tool not initialized");
      return *m_toolWrapper;
    }



    [[nodiscard]] IColumnarTool* ColumnarTestToolHandle ::
    getTool ()
    {
      return m_tool;
    }



    ColumnMapType ::
    ColumnMapType (ColumnarTestToolHandle& val_toolHandle)
      : m_toolHandle (&val_toolHandle), m_columnData (std::make_unique<ColumnVectorData> (&val_toolHandle.getColumnHeader()))
    {
      for (auto& column : m_toolHandle->getColumnInfo())
      {
        auto& userColumn = m_userColumns[column.name];
        if (userColumn.info.has_value())
          throw std::runtime_error ("column already exists: " + column.name);
        userColumn.info = column;
      }
    }



    void ColumnMapType ::
    addColumn (const std::string& name, std::vector<std::any> data)
    {
      auto& userColumn = m_userColumns[name];
      if (userColumn.input.has_value())
        throw std::runtime_error ("column added twice: " + name);
      ManualColumnData manualData {std::move (data)};
      userColumn.input = std::move (manualData);
    }



    void ColumnMapType ::
    setExpectation (const std::string& name, std::vector<std::any> values)
    {
      auto& userColumn = m_userColumns[name];
      if (userColumn.expectation.has_value())
        throw std::runtime_error ("column added twice: " + name);
      ManualColumnData manualData {std::move (values)};
      userColumn.expectation = std::move (manualData);
    }



    std::size_t ColumnMapType ::
    columnSize (const std::string& name)
    {
      auto iter = m_userColumns.find (name);
      if (iter == m_userColumns.end() || !iter->second.input.has_value())
        throw std::runtime_error ("column not found: " + name);
      return iter->second.input.value().columnSize ();
    }



    void ColumnMapType ::
    connectColumnsToTool ()
    {
      std::vector<std::string> missingColumns;
      for (auto& [name, userColumn] : m_userColumns)
      {
        if (!userColumn.info.has_value())
        {
          // report missing column to gtest, but continue
          ADD_FAILURE() << "provided a column not requested by tool: " << name;
          continue;
        }
        auto& columnInfo = userColumn.info.value();
        if (!userColumn.input.has_value())
        {
          if (!columnInfo.isOptional)
          {
            // for missing non-optional inputs we need to throw, because
            // the tool will fail otherwise. potentially we could try to
            // fill dummy input values, but that is more complicated.
            missingColumns.push_back (name);
          }
          continue;
        }
        auto& input = userColumn.input.value();
        switch (columnInfo.accessMode)
        {
        case ColumnAccessMode::input:
        case ColumnAccessMode::update:
        case ColumnAccessMode::output:
          {
            const bool isConst = columnInfo.accessMode == ColumnAccessMode::input;
            input.configureType (name, *columnInfo.type);
            m_toolHandle->getToolWrapper().setColumnVoid (*m_columnData, name, input.columnSize(), input.columnVoidData(), *columnInfo.type, isConst);
          }
          break;
        default:
          throw std::runtime_error ("column mode unknown: " + columnInfo.name);
        }
      }
      if (!missingColumns.empty())
      {
        std::string message = "missing input columns:";
        for (auto& column : missingColumns)
          message += " " + column;
        throw std::runtime_error (message);
      }
    }



    void ColumnMapType ::
    call ()
    {
      m_columnData->checkData ();
      m_columnData->callNoCheck (*m_toolHandle->getTool());
    }



    void ColumnMapType ::
    checkExpectations ()
    {
      for (auto& [columnName, userColumn] : m_userColumns)
      {
        if (!userColumn.info.has_value())
          continue;
        auto& info = userColumn.info.value();
        if (info.accessMode == ColumnAccessMode::input)
        {
          if (userColumn.expectation.has_value())
            ADD_FAILURE() << "expectation provided for input-only column: " << columnName;
          continue;
        }
        if (!userColumn.input.has_value())
        {
          if (info.isOptional)
          {
            if (userColumn.expectation.has_value())
              ADD_FAILURE() << "expectation provided for omitted column: " << columnName;
            continue;
          }
          throw std::logic_error ("output column does not exist: " + columnName);
        }
        auto& output = userColumn.input.value();

        if (!userColumn.expectation.has_value())
        {
          ADD_FAILURE() << "no expectation provided for output column: " << columnName;
        } else
        {
          auto& expectation = userColumn.expectation.value();
          expectation.configureType (columnName, *info.type);
          TestUtils::checkExpectation (columnName, *output.type(), output.columnSize(), output.columnVoidData(), *expectation.type(), expectation.columnSize(), expectation.columnVoidData());
        }
        TestUtils::printExpectedOutput (columnName, *output.type(), output.columnSize(), output.columnVoidData());
      }
    }
  }
}