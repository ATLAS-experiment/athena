/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_INTERFACES_COLUMN_INFO_H
#define COLUMNAR_INTERFACES_COLUMN_INFO_H

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


    /// @brief for link columns: the name(s) of the container(s) we link
    /// to
    ///
    /// Some of our columns contain links to other objects. For those
    /// columns this will contain the names of the containers we link
    /// to (i.e. the names of their offset columns).
    ///
    /// For simple link columns that can only link to a single other
    /// container, this will be a vector of length one and @ref
    /// variantLinkKeyColumn will not be set. In that case the link
    /// column will simply contain the index of the object in the linked
    /// too container.
    ///
    /// For variant link columns (i.e. columns with links that can
    /// reference objects in more than one container), this will contain
    /// the list of all linked to columns that this tool will use. In
    /// that case the most significant bits of the index in this column
    /// will encode a container key that is matched against @ref
    /// variantLinkKeyColumn to identify which container is being
    /// referenced by each link.
    ///
    /// Note that a variant link column may contain links to columns
    /// that are not listed here, but those will not be used by this
    /// tool. There are also no requirements on the exact values of the
    /// keys, as long as they match @ref variantLinkKeyColumn. And
    /// different tools may list the columns in different order. The
    /// thought behind that is that it allows multiple tools to read the
    /// same link column as long as they have each a unique key column,
    /// without having to coordinate the exact list of linked containers
    /// used.
    std::vector<std::string> linkTargetNames {};


    /// @brief if this is a variant link column, this is the name of the
    /// column with the container keys
    ///
    /// If set, this is the name of another column that should have
    /// exactly one entry per entry in @ref linkTargetNames, giving the
    /// container keys for each.
    ///
    /// This doesn't have to be set for simple link columns that only
    /// point to a single container. In that case the link column will
    /// simply contain indices without keys (a.k.a. all keys are zero).
    /// That is actually the more common case, which is also simpler to
    /// handle in columnar code.
    std::string variantLinkKeyColumn {};


    /// @brief whether to add data dependencies in AthenaMT
    ///
    /// In AthenaMT we need to track data dependencies between
    /// algorithms, but we usually only want to use a subset of the data
    /// dependencies used in columnar code. This flag indicates that
    /// this accessor should be added as a data dependency in AthenaMT.
    bool addMTDependency = false;
  };
}

#endif