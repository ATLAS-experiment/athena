/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_INTERFACES_COLUMN_INFO_H
#define COLUMNAR_INTERFACES_COLUMN_INFO_H

#include <cstdint>
#include <string>
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


    /// @brief for simple link columns: the name of the target container
    ///
    /// For link columns that only reference a single container type, this
    /// contains the name of that container (i.e. the name of its offset
    /// column). For variant links (links that can reference multiple
    /// containers), this will be empty and @ref variantLinkTargetNames
    /// should be used instead.
    std::string soleLinkTargetName {};


    /// @brief for simple link columns: the CLID of the target container
    ///
    /// This is the class ID (from the CLASS_DEF macro) of the xAOD
    /// container type the link points into, or 0 if it is not known.
    /// Together with the StoreGate name of the target container it
    /// determines the hashed keys (sgkeys) stored as `m_persKey` for
    /// persistified element links, allowing those keys to be computed
    /// and checked at runtime.
    ///
    /// This is deliberately a plain integer rather than `CLID` to avoid
    /// a dependency on the EDM headers in this interface package.
    std::uint32_t soleLinkTargetClid = 0;


    /// @brief whether this is a variant link column
    ///
    /// If true, this column contains variant links that can reference
    /// objects in more than one container. In that case @ref
    /// variantLinkTargetNames and @ref variantLinkKeyColumn will be set.
    bool isVariantLink = false;


    /// @brief for variant link key columns: the names of the containers
    /// we can link to
    ///
    /// This field is set on the key column (the column that has @ref
    /// keyColumnForVariantLink set) rather than on the variant link
    /// column itself.  It lists the container names that the associated
    /// variant link column can reference, in the order corresponding to
    /// the key values stored in this column.
    ///
    /// For simple link columns (non-variant), this will be empty and
    /// @ref soleLinkTargetName should be used instead.
    ///
    /// Note that a variant link column may contain links to columns
    /// that are not listed here, but those will not be used by this
    /// tool.  There are also no requirements on the exact values of the
    /// keys.  And different tools may list the columns in different
    /// order.  The thought behind that is that it allows multiple tools
    /// to read the same link column as long as they have each a unique
    /// key column, without having to coordinate the exact list of
    /// linked containers used.
    std::vector<std::string> variantLinkTargetNames {};


    /// @brief if this is a key column for a variant link, the name of
    /// the associated link column
    ///
    /// If set on a column, it indicates that this column serves as the
    /// key column for a variant link, and points to the link column
    /// that uses it.  The key column should have exactly one entry per
    /// entry in @ref variantLinkTargetNames, giving the container keys
    /// for each target container.
    ///
    /// This field works together with @ref variantLinkTargetNames on
    /// the same (key) column to define which containers the associated
    /// variant link can reference.
    std::string keyColumnForVariantLink {};
  };
}

#endif