/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EXAMPLE_TOOLS_LINK_COLUMN_EXAMPLE_TOOL_H
#define COLUMNAR_EXAMPLE_TOOLS_LINK_COLUMN_EXAMPLE_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/LinkColumn.h>
#include <ColumnarMuon/MuonDef.h>
#include <ColumnarTracking/TrackDef.h>

namespace columnar
{
  /// @brief an example of a tool reading an element link
  ///
  /// This is a variation on the pt selector tool that selects on the pt
  /// of the linked track.  That is probably not something you actually
  /// want to do, but it illustrates how it's done.

  class LinkColumnExampleTool final
    : public asg::AsgTool,
      public ColumnarTool<>
  {
  public:

    LinkColumnExampleTool (const std::string& name);

    virtual StatusCode initialize () override;

    virtual void callEvents (EventContextRange events) const override;


    /// @brief the pt cut to apply
    Gaudi::Property<float> m_ptCut {this, "ptCut", 10e3, "pt cut (in MeV)"};


    /// @brief the object accessor for the muons
    ///
    /// This is equivalent to a `ReadHandleKey` in the xAOD world.  It
    /// is used to access the particle range/container for a given
    /// event.
    MuonAccessor<ObjectColumn> muonsHandle {*this, "AnalysisMuons"};


    /// @brief the object accessor for the linked track container
    ///
    /// There is no direct equivalent to this in the xAOD world, but
    /// essentially we need to know what the linked container is, both
    /// for doing the link itself, and for declaring all the associated
    /// accessors.
    TrackAccessor<ObjectColumn> trackHandle {*this, "InDetTrackParticles"};


    /// @brief the link accessor for the particles
    ///
    /// This accessor reads the link from one container to another.  In
    /// xAOD land this is done with `ElementLink`, while in columnar
    /// land this is just a simple integer index.
    MuonAccessor<OptTrackId> trackLinkAcc {*this, "inDetTrackParticleLink"};



    /// @brief the q/p accessor for the track container
    ///
    /// Tracks use their own way of representing momentum.  In an actual
    /// tool we'd use a custom momentum accessor for tracks, but here we
    /// just read the q/p and handle it directly.
    TrackAccessor<float> trackQOverPAcc {*this, "qOverP"};


    /// @brief the selection decorator for the particles
    ///
    /// This is the equivalent to an `AuxElement::Decorator` in the xAOD
    /// world.  One thing to note is that the tools generally run on
    /// many objects, not a a single object.  So the tool doesn't have
    /// the option to return individual output values.  Instead it needs
    /// to provide an output value per object, which in the columnar
    /// world is done by filling a column.
    MuonDecorator<char> selectionDec {*this, "selection"};
  };
}

#endif
