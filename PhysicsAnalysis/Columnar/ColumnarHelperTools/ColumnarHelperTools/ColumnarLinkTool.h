/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_HELPER_TOOLS_COLUMNAR_LINK_TOOL_H
#define COLUMNAR_HELPER_TOOLS_COLUMNAR_LINK_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnAccessorDataArray.h>
#include <ColumnarCore/ColumnarDef.h>
#include <ColumnarCore/ColumnarTool.h>
#include <SGCore/sgkey_t.h>

#include <memory>
#include <string>
#include <vector>

namespace columnar
{
  /// @brief a columnar tool to merge the two components of an element
  /// link into the proper repesentation for columnar
  ///
  /// This tool reads both the index and sgkey columns of a link
  /// (directly from the xAOD input), adjusts the index with the event
  /// offset, looks up the sgkey, and writes out a combined column.
  ///
  /// With @ref m_nestingDepth set to a non-zero value the tool also
  /// supports vector-of-link and (nested) vector-of-vector-of-link
  /// inputs: the link triple `(linkIndex, linkSGKey, outLink)` is then
  /// expected as flat data columns suffixed `.data`, with one or more
  /// offset columns describing the per-particle nesting.  The tool
  /// only needs to iterate the flat link range per event since the
  /// per-link merging is independent of which particle/sub-vector a
  /// link belongs to.
  ///
  /// This tool only works in `ColumnarModeArray`, in xAOD mode links
  /// are directly represented by `ElementLink` objects.

  class ColumnarLinkTool final
    : public asg::AsgTool,
      public ColumnarTool<CMode>
  {
  public:

    // Create a proper constructor for Athena
    ASG_TOOL_CLASS( ColumnarLinkTool, asg::IAsgTool )

    ColumnarLinkTool (const std::string& name);

    virtual StatusCode initialize () override;

    virtual void callEvents (EventContextRange<CMode> events) const override;


    Gaudi::Property<std::vector<std::string>> m_targetContainerNames {
      this, "targetContainerNames", {},
      "names of the allowed target containers, in order of their columnar-key index"};

    Gaudi::Property<bool> m_errorOnUnknownKey {
      this, "errorOnUnknownKey", true,
      "whether an unknown sgkey should produce an error (true), or an invalid link (false)"};

    Gaudi::Property<unsigned> m_nestingDepth {
      this, "nestingDepth", 0u,
      "nesting depth of vector-of-link inputs (0 = scalar link per particle, 1 = vector of links per particle, 2 = vector of vector of links, ...)"};


    /// @brief generic per-column low-level accessor state
    ///
    /// Used both for the source-container/offset input columns and
    /// for the flat-data link columns.  Since the nesting depth is
    /// dynamic and there is no high-level output accessor for
    /// `std::vector` columns, all three flat data columns
    /// (`linkIndex`, `linkSGKey`, `outLink`) are registered via the
    /// low-level mechanism as well, even at depth 0.  Public so the
    /// implementation file's helpers can refer to it; not part of any
    /// external API.
    struct ColumnSlot final
    {
      unsigned dataIndex = 0u;
      std::unique_ptr<ColumnAccessorDataArray> accessorData;
    };


  private:

    /// @brief per-target container information
    ///
    /// Since there is (currently) no way to have a dynamic number of
    /// input containers, this relies on the low level mechanism to
    /// connect the offset columns.
    struct TargetSlot final
    {
      SG::sgkey_t sgKey = 0;
      unsigned dataIndex = 0u;
      std::unique_ptr<ColumnAccessorDataArray> accessorData;
    };
    std::vector<TargetSlot> m_targetSlots;

    /// @brief per-event particle offsets of the source container
    ColumnSlot m_srcContainerSlot;

    /// @brief offset columns describing the nested-vector structure
    /// of the link triple; outermost first; size matches
    /// @ref m_nestingDepth
    std::vector<ColumnSlot> m_linkOffsetSlots;

    /// @brief flat input column with the per-link index into the
    /// target container
    ColumnSlot m_indexSlot;

    /// @brief flat input column with the raw sgkey identifying the
    /// target container
    ColumnSlot m_keySlot;

    /// @brief flat output column with the merged link
    ColumnSlot m_outLinkSlot;
  };
}

#endif
