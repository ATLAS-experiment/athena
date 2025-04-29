/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarCore/ColumnarTool.h>

#include <ColumnarCore/ColumnAccessorDataArray.h>
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

    setContainerName (ContainerId::eventContext, numberOfEventsName);
    setContainerName (ContainerId::eventInfo, numberOfEventsName);
    m_eventsData = std::make_unique<ColumnAccessorDataArray> (&m_eventsIndex, &m_eventsData, &typeid (ColumnarOffsetType), ColumnAccessMode::input);
    addColumn (numberOfEventsName, m_eventsData.get(), {.isOffset = true});
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

    for (auto& containerName : subtoolData->containerNames)
    {
      auto [iter,success] = m_data->containerNames.emplace (containerName.first, containerName.second);
      if (!success && iter->second != containerName.second)
        throw std::runtime_error ("container assigned different name in subtool: " + iter->second + " vs " + containerName.second);
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
    std::vector<std::string> names;
    names.reserve (m_data->columns.size());
    for (auto& [name, column] : m_data->columns)
    {
      if (!column.empty())
        names.push_back (name);
    }
    std::sort (names.begin(), names.end());

    std::vector<ColumnInfo> result;
    result.reserve (names.size());
    for (auto& name : names)
    {
      auto& column = m_data->columns.at (name);
      result.push_back (column.info());
      result.back().name = name;
    }
    return result;
  }



  void ColumnarTool<ColumnarModeArray> ::
  renameColumn (const std::string& from, const std::string& to)
  {
    // Note: This is not the most efficient way to do this, if you want
    // to rename all columns it is at O(n^2).  However, since n is
    // supposed to be small this shouldn't be a problem.  Should this
    // become a problem I'd have to introduce back-references to
    // offset-columns, etc.  However, that also introduces a fair bit of
    // complexity, so I'm not doing it unless I have to.

    auto from_iter = m_data->columns.find (from);
    if (from_iter == m_data->columns.end())
      throw std::runtime_error ("column to rename from not found: " + from);
    auto to_iter = m_data->columns.find (to);
    if (to_iter != m_data->columns.end())
    {
      to_iter->second.mergeData (to, std::move (from_iter->second));
      m_data->columns.erase (from_iter);
    } else
    {
      auto node = m_data->columns.extract (from_iter);
      node.key() = to;
      m_data->columns.insert (std::move (node));
    }
    for (auto& column : m_data->columns)
      column.second.updateColumnRef (from, to);
  }



  void ColumnarTool<ColumnarModeArray> ::
  setColumnIndex (const std::string& name, std::size_t index)
  {
    if (auto column = m_data->columns.find (name);
        column != m_data->columns.end())
      column->second.setIndex (index);
  }



  void ColumnarTool<ColumnarModeArray> ::
  callEvents (ObjectRange<ContainerId::eventContext,ColumnarModeArray> /*events*/) const
  {
    throw std::runtime_error ("tool didn't implement callEvents");
  }



  const std::string& ColumnarTool<ColumnarModeArray> ::
  objectName (ContainerId objectType) const
  {
    auto iter = m_data->containerNames.find (objectType);
    if (iter == m_data->containerNames.end())
      throw std::runtime_error ("object type not registered, make sure to register object handle first: " + std::to_string (unsigned(objectType)));
    return iter->second;
  }



  void ColumnarTool<ColumnarModeArray> ::
  setContainerName (ContainerId container, const std::string& name)
  {
    auto [iter, success] = m_data->containerNames.emplace (container, name);
    if (!success && iter->second != name)
      throw std::runtime_error ("container already registered with different name: " + name + " vs " + iter->second);
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
    addAccessor (name, other.m_info, nullptr);
    m_accessors.insert (m_accessors.end(), other.m_accessors.begin(), other.m_accessors.end());
    for (auto *ptr : other.m_accessors)
      ptr->dataRef = this;
    other.m_accessors.clear();
  }



  void ColumnDataArray ::
  updateColumnRef (const std::string& from, const std::string& to)
  {
    auto remapName = [&] (std::string& name)
    {
      if (name == from)
        name = to;
    };

    remapName (m_info.name);
    remapName (m_info.offsetName);
    remapName (m_info.replacesColumn);
    remapName (m_info.linkToName);
  }



  void ColumnDataArray ::
  setIndex (unsigned index) noexcept
  {
    m_info.index = index;
    for (auto *ptr : m_accessors)
      *ptr->dataIndexPtr = index;
  }
}
