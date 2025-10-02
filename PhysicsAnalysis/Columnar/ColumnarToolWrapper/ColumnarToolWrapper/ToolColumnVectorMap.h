/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TOOL_WRAPPER_TOOL_COLUMN_VECTOR_MAP_H
#define COLUMNAR_TOOL_WRAPPER_TOOL_COLUMN_VECTOR_MAP_H

#include <ColumnarToolWrapper/ColumnVectorWrapper.h>

#include <unordered_map>

namespace columnar
{
  /// @brief a class that interfaces an @ref IColumnarTool to a @ref
  /// ColumnVectorHeader
  ///
  /// This takes care of registering all the columns needed by the
  /// tool, storing the name-index map, and setting the column indices
  /// in the tool.
  ///
  /// This class does not try to take care of any tool management, or
  /// working with the @ref ColumnVectorData. It does provide some
  /// helpers implemented using its own public interface, but this is
  /// just for convenience.

  class ToolColumnVectorMap final
  {
    /// Public Members
    /// ==============

  public:

    /// @brief standard constructor
    ///
    /// This will extract the column information from the tool,
    /// configure all the columns for it, and set the proper column
    /// indices in the tool.
    explicit ToolColumnVectorMap (ColumnVectorHeader& val_columnHeader, IColumnarTool& val_tool);


    /// @brief get the wrapped tool
    [[nodiscard]] const IColumnarTool& getTool () const {
      return *m_tool; }

    /// @brief get the header information for the various columns needed
    [[nodiscard]] const ColumnVectorHeader& getColumnHeader () const {
      return *m_columnHeader; }


    /// @brief get the list of all defined columns
    ///
    /// This is mostly to make it easy for the caller to know which
    /// columns are defined. For more complete information ask the
    /// tool directly for the vector of `ColumnInfo`.
    [[nodiscard]] std::vector<std::string> getColumnNames () const;

    /// @brief get the index for the column with the given name
    ///
    /// This is mostly to share the lookup code, as well as the proper
    /// error handling.
    [[nodiscard]] std::size_t getColumnIndex (const std::string& name) const;



    /// Forwarding MemberFunctions
    /// ==========================
    ///
    /// These function are just forwarding to @ref ColumnVectorHeader
    /// and @ref ColumnVectorData. These are just here to make use
    /// easier and insulate the user from changes (Law of Demeter).
  

    /// @brief set the data for the given column picking up the type via
    /// a template
    template<typename CT>
    void setColumn (ColumnVectorData& columnData, const std::string& name, std::size_t size, CT* dataPtr) const {
      columnData.setColumn (getColumnIndex(name), size, dataPtr);
    }

    /// @brief set the data for the given column with the user passing
    /// in the type
    void setColumnVoid (ColumnVectorData& columnData, const std::string& name, std::size_t size, const void *dataPtr, const std::type_info& type, bool isConst) const {
      columnData.setColumnVoid (getColumnIndex(name), size, dataPtr, type, isConst);
    }


    /// @brief get the data for the given column picking up the type via
    /// a template
    template<typename CT>
    [[nodiscard]] std::pair<std::size_t,CT*>
    getColumn (ColumnVectorData& columnData, const std::string& name) const {
      return columnData.getColumn<CT> (getColumnIndex(name));
    }

    /// @brief get the data for the given column in a type-erased manner
    [[nodiscard]] std::pair<std::size_t,const void*>
    getColumnVoid (ColumnVectorData& columnData, const std::string& name, const std::type_info *type, bool isConst) const {
      return columnData.getColumnVoid (getColumnIndex(name), type, isConst);
    }



    /// Private Members
    /// ===============

  private:

    /// @brief the wrapped tool
    const IColumnarTool *m_tool = nullptr;

    /// @brief the header information for the various columns needed
    ColumnVectorHeader *m_columnHeader = nullptr;

    /// @brief my cached information for the various columns needed
    struct MyColumnInfo
    {
      unsigned index = 0u;
    };
    std::unordered_map<std::string,MyColumnInfo> m_columns;
  };
}

#endif
