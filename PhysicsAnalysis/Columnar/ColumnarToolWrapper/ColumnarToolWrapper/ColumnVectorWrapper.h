/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TOOL_WRAPPER_COLUMN_VECTOR_WRAPPER_H
#define COLUMNAR_TOOL_WRAPPER_COLUMN_VECTOR_WRAPPER_H

#include <span>
#include <string>
#include <typeinfo>
#include <vector>

namespace columnar
{
  struct ColumnInfo;
  class IColumnarTool;


  /// @brief the header information for a single element in the
  /// columnar data vector
  ///
  /// This is a stripped-down and slightly reworked version of
  /// `ColumnInfo` that can be used internally for checking
  /// interactions with the data vector from the tool wrapper.

  struct ColumnVectorElementHeader final
  {
    /// @brief the name of the column to use in messages
    ///
    /// This is not used for any actual processing, but is used if I
    /// want to print debug or error messages. It is a bit complicated
    /// because the same name can refer to different columns in
    /// different contexts, and a column could also have a different
    /// name in different contexts. However, I do need a way to refer
    /// to a column, so I have to put something here. Please do not
    /// rely on the exact name being stable. In the future I may also
    /// replace it with a list of names (or similar) to give me more
    /// options.
    std::string debugName;


    /// @brief the type to use for the column
    const std::type_info *type = nullptr;


    /// @brief whether this column is optional
    bool isOptional = true;


    /// @brief whether this column will only be used for read access
    ///
    /// If this is true, it can be set from a `const` pointer, and can
    /// only be retrieved as a `const` pointer.
    bool readOnly = true;


    /// @brief whether this is an offset column
    ///
    /// If this is true it means other columns can use this for their
    /// offsets. It also means that the column should have an extra
    /// entry at the end to hold the total number of entries in
    /// columns using this as an offset. It also implies that the type
    /// ought to be `ColumnarOffsetType`.
    bool isOffset = false;


    /// @brief the total size of all inner array dimensions
    ///
    /// This is 1 for most columns, but some columns contain an array
    /// of values. In that case this is the product of all array
    /// dimensions, and is used for size checks.
    unsigned arraySize = 1u;


    /// @brief the index of the offset column (or `nullIndex` for
    /// none)
    ///
    /// Most columns do have an offset column, which is usually the
    /// offset column for their container. This is used to check that
    /// the size of the column matches the size of the offset column.
    std::size_t offsetIndex = 0u;
  };



  /// @brief the header information for the entire columnar data vector
  ///
  /// At its core this is just a vector of @ref
  /// ColumnVectorElementHeader, with some extra helper functions.
  /// Note that this is completely independent of the tool used, and
  /// can indeed be used with multiple tools.

  class ColumnVectorHeader final
  {
    /// Public Members
    /// ==============
  public:

    /// @brief the index used for an invalid index (always has to be 0)
    static constexpr std::size_t nullIndex = 0u;

    /// @brief the index used for the column size column
    static constexpr std::size_t sizeIndex = 1u;

    /// @brief the number used for an unset but non-null index
    static constexpr std::size_t unsetIndex = static_cast<std::size_t>(-1);

    /// @brief the number of fix elements in the columnar data vector
    static constexpr std::size_t numFixedColumns = 2u;

    /// @brief standard contructor
    ColumnVectorHeader ();


    /// @brief add a column for the given ColumnInfo, returning its index
    [[nodiscard]] std::size_t addColumn (const ColumnInfo& columnInfo);

    /// @brief set the index of the offset column for the given column
    void setOffsetColumn (std::size_t columnIndex, std::size_t offsetIndex);


    /// @brief the number of columns in the columnar data vector
    [[nodiscard]] std::size_t numColumns () const noexcept {
      return m_elements.size(); }

    /// @brief get the column for the given index
    [[nodiscard]] const ColumnVectorElementHeader&
    getColumn (std::size_t index) const {
      return m_elements.at (index); }


    /// @brief check the self-consistency of the header
    void checkSelf () const;

    /// @brief do a basic check of the data vector
    void checkData (std::span<const void*const> data) const;


    /// Private Members
    /// ===============
  private:

    /// @brief the elements in the columnar data vector
    std::vector<ColumnVectorElementHeader> m_elements;
  };



  /// @brief a class that holds the columnar data for a single call
  ///
  /// This manages the data pointer and makes sure the data is set and
  /// retrieved in a consistent manner to the header information.

  class ColumnVectorData final
  {
    /// Public Members
    /// ==============
  public:

    /// @brief standard constructor
    explicit ColumnVectorData (const ColumnVectorHeader *val_header);


    /// @brief set the data for the given column
    template<typename CT>
    void setColumn (std::size_t columnIndex, std::size_t size, CT* dataPtr) {
      auto voidPtr = reinterpret_cast<const void*>(const_cast<const CT*>(dataPtr));
      setColumnVoid (columnIndex, size, voidPtr, typeid (std::decay_t<CT>), std::is_const_v<CT>);
    }
    void setColumnVoid (std::size_t columnIndex, std::size_t size, const void *dataPtr, const std::type_info& type, bool isConst);


    /// @brief get the data for the given column
    template<typename CT>
    [[nodiscard]] std::pair<std::size_t,CT*>
    getColumn (std::size_t columnIndex)
    {
      auto [size, ptr] = getColumnVoid (columnIndex, &typeid (std::decay_t<CT>), std::is_const_v<CT>);
      if constexpr (std::is_const_v<CT>)
        return std::make_pair (size, static_cast<CT*>(ptr));
      else
        return std::make_pair (size, static_cast<CT*>(const_cast<void*>(ptr)));
    }
    [[nodiscard]] std::pair<std::size_t,const void*>
    getColumnVoid (std::size_t columnIndex, const std::type_info *type, bool isConst);


    /// @brief do a basic check of the data vector
    void checkData () const {
      m_header->checkData (m_data);
    }

    /// @brief call the tool with the assembled data, without
    /// performing any checks on the data
    void callNoCheck (const IColumnarTool& tool);


    /// Private Members
    /// ===============
  private:

    const ColumnVectorHeader *m_header = nullptr;
    std::vector<void*> m_data;
    std::vector<std::size_t> m_dataSize;
  };
}

#endif
