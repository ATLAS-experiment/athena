/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarTestFixtures/ColumnarMemoryTest.h>

#include <ColumnarCore/ColumnarDef.h>

#ifdef XAOD_STANDALONE
#include <ComponentFactoryPreloader/ComponentFactoryPreloader.h>
#endif

//
// method implementations
//

namespace columnar
{
  ColumnarMemoryTest ::
  ColumnarMemoryTest ()
  {
#ifdef XAOD_STANDALONE
    static std::once_flag flag;
    std::call_once (flag, [] ()
    {
      // Preload the component factories:  Alternately this could be
      // loaded and executed via a dictionary, but I had some technical
      // issue with that, and this seems to be working.
      CP::preloadComponentFactories();
    });
#endif
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



  ColumnarMemoryTest::ColumnarTestToolHandle ::
  ColumnarTestToolHandle (asg::AsgTool& val_tool)
  {
    m_tool = dynamic_cast<IColumnarTool*> (&val_tool);
    if (m_tool == nullptr)
      throw std::runtime_error ("tool does not implement IColumnarTool");
    m_systTool = dynamic_cast<CP::ISystematicsTool*> (m_tool);
  }



  void ColumnarMemoryTest::ColumnarTestToolHandle ::
  renameContainers (const std::vector<std::pair<std::string,std::string>>& renames)
  {
    columnar::renameContainers (*m_tool, renames);
    if (m_toolWrapper)
    {
      m_toolWrapper = std::make_shared<ColumnarToolWrapper> (m_tool);
    }
  }



  void ColumnarMemoryTest::ColumnarTestToolHandle ::
  initialize ()
  {
    m_toolWrapper = std::make_shared<ColumnarToolWrapper> (m_tool);
  }



  void ColumnarMemoryTest::ColumnarTestToolHandle ::
  applySystematicVariation (const std::string& sysName)
  {
    // by convention setting a systematic on a non-systematics tool
    // will do nothing
    if (m_systTool == nullptr)
      return;
    if (!m_systTool->applySystematicVariation (CP::SystematicSet (sysName)).isSuccess())
      throw std::runtime_error ("failed to apply systematic variation");
  }



  [[nodiscard]] std::vector<ColumnInfo> ColumnarMemoryTest::ColumnarTestToolHandle ::
  getColumnInfo () const
  {
    if (!m_toolWrapper)
      throw std::runtime_error ("tool not initialized");
    return m_toolWrapper->getColumnInfo ();
  }



  std::vector<std::string> ColumnarMemoryTest::ColumnarTestToolHandle ::
  getColumnNames () const
  {
    if (!m_toolWrapper)
      throw std::runtime_error ("tool not initialized");
    return m_toolWrapper->getColumnNames ();
  }



  std::vector<std::string> ColumnarMemoryTest::ColumnarTestToolHandle ::
  getRecommendedSystematics () const
  {
    if (!m_systTool)
      return {""};
    std::vector<std::string> result;
    for (auto& sys : CP::make_systematics_vector (m_systTool->recommendedSystematics()))
      result.push_back (sys.name());
    return result;
  }



  [[nodiscard]] const ColumnarToolWrapper& ColumnarMemoryTest::ColumnarTestToolHandle ::
  getToolWrapper () const
  {
    if (!m_toolWrapper)
      throw std::runtime_error ("tool not initialized");
    return *m_toolWrapper;
  }



  [[nodiscard]] IColumnarTool* ColumnarMemoryTest::ColumnarTestToolHandle ::
  getTool ()
  {
    return m_tool;
  }



  ColumnarMemoryTest::ColumnMapType ::
  ColumnMapType (ColumnarTestToolHandle& val_toolHandle)
    : m_toolHandle (&val_toolHandle), m_columnData (std::make_unique<ColumnarToolWrapperData> (&val_toolHandle.getToolWrapper()))
  {
    for (auto& column : m_toolHandle->getColumnInfo())
    {
      auto [iter, success] = m_columnMap.emplace (column.name, ColumnInfo {column});
      if (!success)
        throw std::runtime_error ("column already exists: " + column.name);
    }
  }



  void ColumnarMemoryTest::ColumnMapType ::
  addColumn (const std::string& name, std::vector<std::any> data)
  {
    auto column = m_columnMap.find (name);
    if (column == m_columnMap.end())
      throw std::runtime_error ("adding unknown column: " + name);
    if (m_inputs.contains (name))
      throw std::runtime_error ("column added twice: " + name);
    if (column->second.type == &typeid(float))
      addInputTyped<float> (name, data);
    else if (column->second.type == &typeid(char))
      addInputTyped<char> (name, data);
    else if (column->second.type == &typeid(int))
      addInputTyped<int> (name, data);
    else if (column->second.type == &typeid(std::uint8_t))
      addInputTyped<std::uint8_t> (name, data);
    else if (column->second.type == &typeid(std::uint16_t))
      addInputTyped<std::uint16_t> (name, data);
    else if (column->second.type == &typeid(std::uint32_t))
      addInputTyped<std::uint32_t> (name, data);
    else if (column->second.type == &typeid(std::uint64_t))
      addInputTyped<std::uint64_t> (name, data);
    else
      throw std::logic_error ("column name " + name + " has unsupported type " + column->second.type->name() + ", extend test handler to support it");
  }



  void ColumnarMemoryTest::ColumnMapType ::
  setExpectation (const std::string& name, std::vector<std::any> values)
  {
    auto column = m_columnMap.find (name);
    if (column == m_columnMap.end())
      throw std::runtime_error ("adding unknown column: " + name);
    if (m_expectations.contains (name))
      throw std::runtime_error ("column added twice: " + name);
    if (column->second.type == &typeid(float))
      addExpectationTyped<float> (name, values);
    else if (column->second.type == &typeid(char))
      addExpectationTyped<char> (name, values);
    else if (column->second.type == &typeid(int))
      addExpectationTyped<int> (name, values);
    else if (column->second.type == &typeid(std::uint8_t))
      addExpectationTyped<std::uint8_t> (name, values);
    else if (column->second.type == &typeid(std::uint16_t))
      addExpectationTyped<std::uint16_t> (name, values);
    else if (column->second.type == &typeid(std::uint32_t))
      addExpectationTyped<std::uint32_t> (name, values);
    else if (column->second.type == &typeid(std::uint64_t))
      addExpectationTyped<std::uint64_t> (name, values);
    else
      throw std::logic_error ("column name " + name + " has unsupported type " + column->second.type->name() + ", extend test handler to support it");
  }



  ColumnarOffsetType ColumnarMemoryTest::ColumnMapType ::
  columnSize (const std::string& name)
  {
    auto iter = m_inputs.find (name);
    if (iter == m_inputs.end())
      throw std::runtime_error ("column not found: " + name);
    return std::visit ([] (const auto& data) { return data.size(); }, iter->second);
  }



  void ColumnarMemoryTest::ColumnMapType ::
  connectColumnsToTool ()
  {
    for (auto& [name, columnInfo] : m_columnMap)
    {
      if (auto iter = m_inputs.find (columnInfo.name); iter != m_inputs.end())
      {
        switch (columnInfo.accessMode)
        {
        case ColumnAccessMode::input:
          std::visit ([&] (const auto& data)
          {
            m_columnData->setColumn (iter->first, data.size(), data.data());
          }, iter->second);
          break;
        case ColumnAccessMode::update:
        case ColumnAccessMode::output:
          std::visit ([&] (auto& data)
          {
            m_columnData->setColumn (iter->first, data.size(), data.data());
          }, m_activeColumns[iter->first] = iter->second);
          break;
        default:
          throw std::runtime_error ("column mode unknown: " + columnInfo.name);
        }
      }
    }
  }



  void ColumnarMemoryTest::ColumnMapType ::
  call ()
  {
    m_columnData->call ();
  }



  void ColumnarMemoryTest::ColumnMapType ::
  checkExpectations ()
  {
    for (auto& [name, info] : m_columnMap)
    {
      if (info.accessMode == ColumnAccessMode::input)
        continue;

      auto iter = m_activeColumns.find (name);
      if (iter == m_activeColumns.end())
        continue;

      if (info.type == &typeid(float))
        checkExpectationTyped<float> (name);
      else if (info.type == &typeid(char))
        checkExpectationTyped<char> (name);
      else if (info.type == &typeid(int))
        checkExpectationTyped<int> (name);
      else if (info.type == &typeid(std::uint8_t))
        checkExpectationTyped<std::uint8_t> (name);
      else if (info.type == &typeid(std::uint16_t))
        checkExpectationTyped<std::uint16_t> (name);
      else if (info.type == &typeid(std::uint32_t))
        checkExpectationTyped<std::uint32_t> (name);
      else if (info.type == &typeid(std::uint64_t))
        checkExpectationTyped<std::uint64_t> (name);
      else
        throw std::logic_error ("column name " + name + " has unsupported type " + info.type->name() + ", extend test handler to support it");
    }
  }
}