/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarCore/ColumnarTool.h>

#include <ColumnarCore/ColumnAccessorDataArray.h>
#include <ColumnarCore/ColumnInfoHelpers.h>
#include <ColumnarCore/ColumnarToolDataArray.h>
#include <ColumnarInterfaces/ColumnInfo.h>

//
// method implementations
//

namespace columnar
{
  ColumnarTool<ColumnarModeArray> ::
  ColumnarTool ()
    : m_data (std::make_shared<ColumnarToolDataArray> ())
  {
    m_data->mainTool = this;
    m_data->sharedTools.push_back (this);

    setContainerUserName (ContainerId::eventContext::idName, numberOfEventsName);

    // this name matches the ContainerId::eventInfo::idName, make sure
    // to keep them in sync. the reason for hard-coding this in two
    // places is because ContainerId::eventInfo is defined in a separate
    // package
    setContainerUserName ("eventInfo", numberOfEventsName);
    m_eventsData = std::make_unique<ColumnAccessorDataArray> (&m_eventsIndex, &m_eventsData, &typeid (ColumnarOffsetType), ColumnAccessMode::input);
    addColumn (std::string (ContainerId::eventContext::idName), m_eventsData.get(), {.isOffset = true});
  }

  ColumnarTool<ColumnarModeArray> ::
  ColumnarTool (ColumnarTool<ColumnarModeArray>* val_parent)
    : m_data (val_parent->m_data)
  {
    m_eventsData = std::make_unique<ColumnAccessorDataArray> (&m_eventsIndex, &m_eventsData, &typeid (ColumnarOffsetType), ColumnAccessMode::input);
    addColumn (numberOfEventsName, m_eventsData.get(), {.isOffset = true});
  }

  ColumnarTool<ColumnarModeArray> ::
  ~ColumnarTool ()
  {
    if (m_data->mainTool == this)
      m_data->mainTool = nullptr;

    auto iter = std::find (m_data->sharedTools.begin(), m_data->sharedTools.end(), this);
    if (iter != m_data->sharedTools.end())
      m_data->sharedTools.erase (iter);
  }

  void ColumnarTool<ColumnarModeArray> ::
  addSubtool (ColumnarTool<ColumnarModeArray>& subtool)
  {
    // make a copy of the shared data for the subtool, so that I don't
    // accidentally release it when resetting the shared data pointer on
    // the subtool
    auto subtoolData = subtool.m_data;

    if (m_data == subtoolData)
      return;

    if (subtoolData->mainTool != &subtool)
      throw std::runtime_error ("subtool already has a different parent tool");

    for (auto& containerName : subtoolData->containerInternalToUserNames)
    {
      auto [iter,success] = m_data->containerInternalToUserNames.emplace (containerName.first, containerName.second);
      if (!success && iter->second != containerName.second)
        throw std::runtime_error ("container assigned different name in subtool: " + iter->second + " vs " + containerName.second);
      if (success)
        m_data->containerUserToInternalNames[containerName.second].push_back (containerName.first);
    }
    for (auto& columnName : subtoolData->columnInternalToUserNames)
    {
      auto [iter,success] = m_data->columnInternalToUserNames.emplace (columnName.first, columnName.second);
      if (!success && iter->second != columnName.second)
        throw std::runtime_error ("column assigned different name in subtool: " + iter->second + " vs " + columnName.second);
      if (success)
        m_data->columnUserToInternalNames[columnName.second].push_back (columnName.first);
    }

    for (auto& column : subtoolData->columns)
    {
      auto [iter,success] = m_data->columns.try_emplace (column.first, std::move (column.second));
      if (!success)
        iter->second.mergeData (column.first, std::move (column.second));
    }

    m_data->sharedTools.insert (m_data->sharedTools.end(), subtoolData->sharedTools.begin(), subtoolData->sharedTools.end());
    for (auto& tool : subtoolData->sharedTools)
      tool->m_data = m_data;
  }



  StatusCode ColumnarTool<ColumnarModeArray> ::
  initializeColumns ()
  {
    return StatusCode::SUCCESS;
  }



  void ColumnarTool<ColumnarModeArray> ::
  callVoid (void **data) const
  {
    auto eventsOffset = static_cast<const ColumnarOffsetType*> (data[m_eventsIndex]);
    callEvents (ObjectRange<ContainerId::eventContext,ColumnarModeArray> (data, eventsOffset[0], eventsOffset[1]));
  }



  std::vector<ColumnInfo> ColumnarTool<ColumnarModeArray> ::
  getColumnInfo () const
  {
    std::vector<std::pair<std::string,std::string>> names;
    names.reserve (m_data->columns.size());
    for (auto& [name, column] : m_data->columns)
    {
      if (!column.empty())
        names.emplace_back (name, m_data->convertInternalToUserName (name));
    }
    std::sort (names.begin(), names.end(), [] (const auto& a, const auto& b) { return a.second < b.second; });

    std::vector<ColumnInfo> result;
    result.reserve (names.size());
    for (auto& name : names)
    {
      auto& column = m_data->columns.at (name.first);
      auto info = column.info();
      info.name = name.second;
      info.offsetName = m_data->convertInternalToUserName (info.offsetName);
      info.replacesColumn = m_data->convertInternalToUserName (info.replacesColumn);
      for (auto& targetName : info.linkTargetNames)
        targetName = m_data->convertInternalToUserName (targetName);
      info.variantLinkKeyColumn = m_data->convertInternalToUserName (info.variantLinkKeyColumn);
      if (result.empty() || result.back().name != info.name)
        result.push_back (std::move(info));
      else
        mergeColumnInfo (result.back(), info);
    }
    return result;
  }



  void ColumnarTool<ColumnarModeArray> ::
  renameColumn (const std::string& from, const std::string& to)
  {
    if (auto iter = m_data->columnUserToInternalNames.find (from);
        iter != m_data->columnUserToInternalNames.end())
    {
      for (auto& internalName : iter->second)
        m_data->columnInternalToUserNames[internalName] = to;
      auto internalNames = iter->second;
      m_data->columnUserToInternalNames.erase (iter);
      m_data->columnUserToInternalNames[to].insert (m_data->columnUserToInternalNames[to].end(), internalNames.begin(), internalNames.end());
      return;
    }

    auto internalNames = m_data->convertUserToInternalNames (from);
    m_data->columnUserToInternalNames[to].insert (m_data->columnUserToInternalNames[to].end(), internalNames.begin(), internalNames.end());
    for (auto& internalName : internalNames)
      m_data->columnInternalToUserNames[internalName] = to;
  }



  void ColumnarTool<ColumnarModeArray> ::
  setColumnIndex (const std::string& name, std::size_t index)
  {
    auto internalNames = m_data->convertUserToInternalNames (name);
    bool wasSet = false;
    for (auto& internalName : internalNames)
    {
      if (auto column = m_data->columns.find (internalName);
          column != m_data->columns.end())
      {
        column->second.setIndex (index);
        wasSet = true;
      }
    }
    if (!wasSet)
      throw std::runtime_error ("column not found: " + name);
  }



  void ColumnarTool<ColumnarModeArray> ::
  callEvents (ObjectRange<ContainerId::eventContext,ColumnarModeArray> /*events*/) const
  {
    throw std::runtime_error ("tool didn't implement callEvents");
  }



  void ColumnarTool<ColumnarModeArray> ::
  setContainerUserName (std::string_view container, const std::string& name)
  {
    auto [iter, success] = m_data->containerInternalToUserNames.emplace (container, name);
    if (!success && iter->second != name)
      throw std::runtime_error ("container already registered with different name: " + name + " vs " + iter->second);
    m_data->containerUserToInternalNames[name].emplace_back (container);
  }



  void ColumnarTool<ColumnarModeArray> ::
  addColumn (const std::string& name, ColumnAccessorDataArray *accessorData, ColumnInfo&& info)
  {
    info.type = accessorData->type;
    info.accessMode = accessorData->accessMode;

    m_data->columns[name].addAccessor (name, info, accessorData);
  }



  ColumnDataArray ::
  ColumnDataArray (ColumnDataArray&& other) noexcept
    : m_info (std::move (other.m_info))
    , m_accessors (std::move (other.m_accessors))
  {
    for (auto *ptr : m_accessors)
    {
      if (ptr->dataRef == &other)
        ptr->dataRef = this;
    }
  }



  ColumnDataArray ::
  ~ColumnDataArray () noexcept
  {
    for (auto *ptr : m_accessors)
    {
      if (ptr->dataRef == this)
        ptr->dataRef = nullptr;
    }
  }



  bool ColumnDataArray ::
  empty () const noexcept
  {
    return m_accessors.empty();
  }



  const ColumnInfo& ColumnDataArray ::
  info () const noexcept
  {
    return m_info;
  }



  void ColumnDataArray ::
  addAccessor (const std::string& name, const ColumnInfo& val_info, ColumnAccessorDataArray* val_accessorData)
  {
    if (m_accessors.empty())
    {
      m_info = val_info;

      // ensure that any index is not zero.  zero is reserved for a
      // nullptr, so that if an index has never been initialized, we
      // get a segfault on access.
      m_info.index = 0;
    } else
    {
      if (val_info.type != m_info.type)
        throw std::runtime_error ("multiple types for column: " + name);
      if (m_info.accessMode == ColumnAccessMode::update || val_info.accessMode == ColumnAccessMode::update)
        m_info.accessMode = ColumnAccessMode::update;
      else if (m_info.accessMode == ColumnAccessMode::output || val_info.accessMode == ColumnAccessMode::output)
        m_info.accessMode = ColumnAccessMode::output;
      if (!val_info.isOptional)
        m_info.isOptional = false;
      if (val_info.offsetName != m_info.offsetName)
        throw std::runtime_error ("inconsistent offset map for column: " + name);
    }

    if (val_accessorData)
    {
      m_accessors.push_back (val_accessorData);
      val_accessorData->dataRef = this;
    }
  }



  void ColumnDataArray ::
  removeAccessor (ColumnAccessorDataArray& val_accessorData)
  {
    auto iter = std::find (m_accessors.begin(), m_accessors.end(), &val_accessorData);
    if (iter == m_accessors.end())
      return;
    m_accessors.erase (iter);
    val_accessorData.dataRef = nullptr;
  }



  void ColumnDataArray ::
  mergeData (const std::string& name, ColumnDataArray&& other)
  {
    mergeColumnInfo (m_info, other.m_info);
    addAccessor (name, other.m_info, nullptr);
    m_accessors.insert (m_accessors.end(), other.m_accessors.begin(), other.m_accessors.end());
    for (auto *ptr : other.m_accessors)
      ptr->dataRef = this;
    other.m_accessors.clear();
  }



  void ColumnDataArray ::
  setIndex (unsigned index) noexcept
  {
    m_info.index = index;
    for (auto *ptr : m_accessors)
      *ptr->dataIndexPtr = index;
  }



  std::string ColumnarToolDataArray ::
  convertInternalToUserName (std::string_view name) const
  {
    if (name.empty())
      return std::string{};
    if (auto iter = columnInternalToUserNames.find (name);
        iter != columnInternalToUserNames.end())
      return iter->second;
    auto split = name.find ('.');
    if (split == std::string::npos)
      split = name.size();
    auto containerName = name.substr (0, split);
    auto iter = containerInternalToUserNames.find (containerName);
    if (iter == containerInternalToUserNames.end())
      return std::string (name);
    return iter->second + std::string (name.substr (split));
  }



  std::vector<std::string> ColumnarToolDataArray ::
  convertUserToInternalNames (std::string_view name) const
  {
    if (auto iter = columnUserToInternalNames.find (name);
        iter != columnUserToInternalNames.end())
      return iter->second;
    auto split = name.find ('.');
    if (split == std::string::npos)
      split = name.size();
    auto containerName = name.substr (0, split);
    auto iter = containerUserToInternalNames.find (containerName);
    if (iter == containerUserToInternalNames.end())
      return {std::string (name)};
    std::vector<std::string> result;
    for (auto& internalContainerName : iter->second)
      result.push_back (internalContainerName + std::string (name.substr (split)));
    if (result.empty())
      result.push_back (std::string (name));
    return result;
  }
}
