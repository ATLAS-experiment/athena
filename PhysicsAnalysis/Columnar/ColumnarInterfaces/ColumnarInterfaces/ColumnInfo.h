/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_INTERFACES_COLUMN_INFO_H
#define COLUMNAR_INTERFACES_COLUMN_INFO_H

#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

namespace columnar
{
  /// @brief an enum for the different access modes for a column
  enum class ColumnAccessMode
  {
    /// @brief an input column
    input,

    /// @brief an output column
    output,

    /// @brief an updateable column
    update
  };


  /// @brief the category of type we use for a column
  ///
  /// This is primarily meant to simplify the python interface, as well
  /// as allowing to reinterpret types that have identical memory
  /// representation.
  enum class ColumnTypeCategory
  {
    /// @brief a floating-point type
    floatingPoint,

    /// @brief a signed integer type
    signedInteger,

    /// @brief an unsigned integer type
    unsignedInteger,

    /// @brief a sign-agnostic type
    ///
    /// For some columns we don't care if they are stored as signed or
    /// unsigned integers. In particular for `char`-type columns C++
    /// doesn't define whether it is signed or unsigned, and this is
    /// chosen by default.
    signAgnosticInteger,

    /// @brief a boolean type
    ///
    /// This is separate from the C++ `bool` type, and instead it is
    /// typically one byte (matching numpy `bool`), and it is
    /// distinguished by being only 1 or 0. For input columns any other
    /// value is interpreted as 1.
    boolean,

    /// @brief some other type
    other
  };


  /// @brief a struct that contains meta-information about each column
  /// that's needed to interface the column with the columnar data store

  struct ColumnInfo final
  {
    /// @brief the name of the column
    ///
    /// This is the primary way by which columns should be identified
    /// when interacting with the user, but it should only be used
    /// during the configuration.  During the actual processing this
    /// should rely on the index for identification instead.
    std::string name {};


    /// @brief the index of the column in the data array
    unsigned index = 0u;


    /// @brief the type of the individual entries in the column
    ///
    /// This should generally be something like `float` or `int`,
    /// neither be const-qualified nor in a container like
    /// `std::vector<float>`.
    const std::type_info *type = nullptr;


    /// @brief the size (in bytes) of the individual entries in the column
    unsigned sizeOfType = 0u;


    /// @brief the type category for the individual entries in the column
    ColumnTypeCategory typeCategory = ColumnTypeCategory::other;


    /// @brief the access mode for the column
    ColumnAccessMode accessMode = ColumnAccessMode::input;


    /// @brief the name of the offset column used for this column (or
    /// empty string for none)
    ///
    /// Most of our columns do not represent a regular-shaped tensor,
    /// but has a different number of entries in each row, e.g. the
    /// column holding the muon-pt will have a different number of muons
    /// per event.  To handle this, there needs to be a column that
    /// contains the offsets for each row.
    ///
    /// There can be multiple irregularly shaped dimensions for a
    /// column, in which case this is just the name of the inner-most
    /// one.  The outer-ones can be found by following the offset name
    /// in a chain.
    std::string offsetName {};


    /// @brief the fixed dimensions this column has (if any)
    ///
    /// For the most part we use dynamic dimensions via offset maps, but
    /// sometimes the dimensions are hard-coded, which then uses these
    /// dimensions.
    std::vector<unsigned> fixedDimensions {};


    /// @brief whether this is an offset column
    ///
    /// In part this is for consistency checks, i.e. other columns can
    /// only refer to this as an offset column if this is set.  It also
    /// means that there needs to be an extra element in the column past
    /// the last "regular" one to hold the total number of entries in
    /// columns using this.
    bool isOffset = false;


    /// @brief whether this replaces another column
    ///
    /// For corrections it is quite typical that a column is meant to
    /// replace another column.  In columnar land we will create
    /// genuinely new columns, in xAOD land we will usually overwrite
    /// the content of those columns.  This member is used to indicate
    /// which column gets replaced.
    std::string replacesColumn {};


    /// @brief whether this column is optional
    ///
    /// Essentially this indicates that the column can be skipped, and
    /// the tool will check whether the column is present before trying
    /// to use it.  This allows to adapt the tool somewhat to different
    /// environments.
    ///
    /// The downside here is that overall this is still a bit ambiguous,
    /// i.e. whoever links up the columns needs to decide whether it is
    /// needed.  For columns that are not in the input file that's easy,
    /// there is no choice but omitting them.  However, some columns
    /// only exist as a backup option for other columns, and ideally we
    /// don't want to load the backup columns when the main columns are
    /// missing.  So either that needs to be set during configuration,
    /// or we need to add more meta-information for that case, or the
    /// user needs to do something smart (i.e. manual) in their code.
    bool isOptional = false;


    /// @brief whether this is the index of the event for each entry
    ///
    /// These columns are used when resolving links as the link index
    /// needs to be combined with the offset-map for the target column.
    /// The most robust way to do this is by having the event-indices
    /// passed into the tool, instead of trying to determine them from
    /// the object index (or even the index of the link in a per-object
    /// vector).
    ///
    /// The column has one entry per entry of its offset map (which may
    /// be the offset map of a per-object vector), holding the index of
    /// the event that entry belongs to.
    bool isEventIndex = false;


    /// @brief if this is an offset map that can be the target of links,
    /// the name from which those links `sgkey` is derived
    ///
    /// The way links are stored in PHYSLITE is that they have an object
    /// index and an `sgkey`, with the later indicating the container
    /// they point to. That `sgkey` incorporates the name of the
    /// container. The tool will have a default, but you can override it
    /// as needed.
    ///
    /// This is only defined if this is a top level offset map (one
    /// entry per event).
    std::string nameForLinks {};
  };



  /// @brief the default @ref ColumnTypeCategory for the given C++ type
  ///
  /// `char` is sign-agnostic, since C++ doesn't define whether it is
  /// signed. A C++ `bool` is `other`, since its memory layout is not
  /// guaranteed to match any other type, i.e. it has to be matched
  /// exactly.
  template<typename T>
  [[nodiscard]] constexpr ColumnTypeCategory columnTypeCategoryFor () noexcept
  {
    using Type = std::remove_cv_t<T>;
    if constexpr (std::is_floating_point_v<Type>)
      return ColumnTypeCategory::floatingPoint;
    else if constexpr (std::is_same_v<Type,bool> || !std::is_integral_v<Type>)
      return ColumnTypeCategory::other;
    else if constexpr (std::is_same_v<Type,char>)
      return ColumnTypeCategory::signAgnosticInteger;
    else if constexpr (std::is_signed_v<Type>)
      return ColumnTypeCategory::signedInteger;
    else
      return ColumnTypeCategory::unsignedInteger;
  }


  /// @brief set the type information in the @ref ColumnInfo for the
  /// given C++ type
  template<typename T>
  void setColumnType (ColumnInfo& info) noexcept
  {
    info.type = &typeid (std::remove_cv_t<T>);
    info.sizeOfType = sizeof (T);
    info.typeCategory = columnTypeCategoryFor<T> ();
  }
}

#endif
