/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EXAMPLE_TOOLS_VARIANT_LINK_COLUMN_EXAMPLE_TOOL_H
#define COLUMNAR_EXAMPLE_TOOLS_VARIANT_LINK_COLUMN_EXAMPLE_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarMuon/MuonDef.h>
#include <ColumnarTracking/TrackDef.h>
#include <ColumnarVariant/VariantAccessor.h>
#include <ColumnarVariant/VariantDef.h>
#include <ColumnarVariant/VariantLinkColumn.h>
#include <ColumnarVariant/VariantObjectId.h>

namespace ADD_CMODE(columnar)
{
  /// @brief an example of a tool reading a variant element link
  ///
  /// This is a variation on @ref LinkColumnExampleTool, which should be
  /// read first.  A plain link can only point into a single container.
  /// The inner detector track link of a muon can point into one of two
  /// containers though: regular muons link into `InDetTrackParticles`,
  /// while forward muons link into `InDetForwardTrackParticles`.  To
  /// read such a link we need a "variant" link, which combines the
  /// "variant" object ids described in @ref VariantExampleTool with the
  /// link handling of @ref LinkColumnExampleTool.

  class VariantLinkColumnExampleTool final
    : public asg::AsgTool,
      public columnar::ColumnarTool<>
  {
  public:

    // Create a proper constructor for Athena
    ASG_TOOL_CLASS( VariantLinkColumnExampleTool, asg::IAsgTool )

    VariantLinkColumnExampleTool (const std::string& name);

    virtual StatusCode initialize () override;

    /// @brief the function to call a single event
    ///
    /// This is split out so that the tool can also be called on a
    /// single event in xAOD mode (e.g. for tests that compare the
    /// modes).
    void callSingleEvent (columnar::MuonRange muons) const;

    virtual void callEvents (columnar::EventContextRange events) const override;

    /// @brief the pt cut to apply
    Gaudi::Property<float> m_ptCut {this, "ptCut", 10e3, "pt cut (in MeV)"};

    /// @brief the object accessor for the muons
    columnar::MuonAccessor<columnar::ObjectColumn> muonsHandle {*this, "AnalysisMuons"};

    /// @brief the object accessors for the linked track containers
    ///
    /// Each container the link can point to needs to be declared, so
    /// that the tool knows about the container (and its offsets) for
    /// resolving the link.  The two containers need different object id
    /// types (`Track0` and `Track3`), as the accessors for an object id
    /// are tied to a single container.
    columnar::Track0Accessor<columnar::ObjectColumn> trackHandle {*this, "InDetTrackParticles"};
    columnar::Track3Accessor<columnar::ObjectColumn> forwardTrackHandle {*this, "InDetForwardTrackParticles"};

    /// @brief the variant definition we are using
    ///
    /// This lists all the containers the link can point to.  As for
    /// all variant definitions the first container is only used as the
    /// base type, so to also use it as a possible target it has to be
    /// listed twice.
    using TrackVariantDef = columnar::VariantContainerId<columnar::Track0Def,columnar::Track0Def,columnar::Track3Def>;

    /// @brief the link accessor for the particles
    ///
    /// This reads the link as a variant object id, which can refer to
    /// an object in any of the containers listed in the variant
    /// definition.  The link may still be invalid, which is handled in
    /// the same way as for a plain link.
    columnar::MuonAccessor<columnar::ObjectLink<TrackVariantDef>> trackLinkAcc {*this, "inDetTrackParticleLink"};

    /// @brief the q/p accessor for the track containers
    ///
    /// This is a "variant" accessor, which creates a column accessor
    /// for each of the containers and picks the right one for each
    /// track at runtime.
    columnar::ColumnAccessor<TrackVariantDef,float> trackQOverPAcc {*this, "qOverP"};

    /// @brief the selection decorator for the particles
    columnar::MuonDecorator<char> selectionDec {*this, "selection"};
  };
}

#endif
