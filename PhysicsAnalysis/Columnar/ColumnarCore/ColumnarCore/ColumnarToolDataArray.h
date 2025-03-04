/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_COLUMNAR_TOOL_DATA_ARRAY_H
#define COLUMNAR_CORE_COLUMNAR_TOOL_DATA_ARRAY_H

#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarInterfaces/ColumnInfo.h>

namespace columnar
{
  struct ColumnDataArray final
  {
    /// @brief standard constructor
    ColumnDataArray () noexcept = default;

    /// @brief standard move constructor
    ColumnDataArray (ColumnDataArray&& other) noexcept;

    ColumnDataArray operator= (ColumnDataArray&& other) noexcept = delete;

    /// @brief standard destructor
    ~ColumnDataArray () noexcept;

    /// @brief whether this column has no associated accessors
    bool empty () const noexcept;

    /// @brief get the column info
    const ColumnInfo& info () const noexcept;

    /// @brief add an info to the column
    void addAccessor (const std::string& name, const ColumnInfo& val_info, ColumnAccessorDataArray* val_accessorData);

    /// @brief remove an accessor data object
    void removeAccessor (ColumnAccessorDataArray& val_accessorData);

    /// @brief merge the data from another column
    void mergeData (const std::string& name, ColumnDataArray&& other);

    /// @brief rename columns
    void updateColumnRef (const std::string& from, const std::string& to);

    /// @brief set the index for this column
    void setIndex (unsigned val_index) noexcept;

  private:

    /// @brief the column info
    ColumnInfo m_info;

    /// @brief the data on all accessors for this column
    std::vector<ColumnAccessorDataArray*> m_accessors;
  };



  struct ColumnarToolDataArray final
  {
    /// @brief the main tool that is associated with this object
    ColumnarTool<ColumnarModeArray>* mainTool = nullptr;

    /// @brief the list of all tools that reference this object
    std::vector<ColumnarTool<ColumnarModeArray>*> sharedTools;

    /// @brief the names associated with all container ids
    std::unordered_map<ContainerId,std::string> containerNames;

    /// @brief the name-column map
    std::unordered_map<std::string,ColumnDataArray> columns;
  };
}

#endif
