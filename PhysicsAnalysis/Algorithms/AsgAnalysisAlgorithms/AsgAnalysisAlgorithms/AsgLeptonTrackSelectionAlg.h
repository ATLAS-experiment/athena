/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_LEPTON_TRACK_SELECTION_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_LEPTON_TRACK_SELECTION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <PATCore/IAsgSelectionTool.h>
#include <SelectionHelpers/ISelectionNameSvc.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODBase/IParticleContainer.h>
#include <AsgTools/PropertyWrapper.h>
#include "AsgDataHandles/ReadHandleKey.h"
#include <AsgDataHandles/ReadHandle.h>
#include <xAODTracking/VertexContainer.h>
#include <xAODEventInfo/EventInfo.h>

namespace CP
{
  /// \brief an algorithm for performing track-vertex selection on
  /// leptons
  ///
  /// Originally I meant to implement this as an \ref
  /// IAsgSelectionTool, but since this needs other objects besides
  /// the lepton itself, I made it into an algorithm.  Technically
  /// this could also be addressed by retrieving those extra objects
  /// in the seleciton tool, but this seemed like potential overkill,
  /// given that the selection tools may be called very frequently and
  /// are not really doing any form of heavy lifting at all.  Still,
  /// at some point we may decide to change this into a selection tool
  /// instead (06 Aug 18).

  class AsgLeptonTrackSelectionAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;


    /// algorithm properties
    /// \{

  private:
    Gaudi::Property<float> m_maxD0Significance {this, "maxD0Significance", 0, "maximum d0 significance (or 0 for no cut)"};
    Gaudi::Property<float> m_maxDeltaZ0SinTheta {this, "maxDeltaZ0SinTheta", 0, "maximum Delta z0 sin theta (or 0 for no cut)"};
    Gaudi::Property<int> m_nMinPixelHits {this, "nMinPixelHits", -1, "minimum number of required Pixel hits (or -1 for no cut)"};
    Gaudi::Property<int> m_nMaxPixelHits {this, "nMaxPixelHits", -1, "maximum number of required Pixel hits (or -1 for no cut)"};
    Gaudi::Property<int> m_nMinSCTHits {this, "nMinSCTHits", -1, "minimum number of required SCT hits (or -1 for no cut)"};
    Gaudi::Property<int> m_nMaxSCTHits {this, "nMaxSCTHits", -1, "maximum number of required SCT hits (or -1 for no cut)"};

    /// \}


    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the EventInfo key
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {this, "eventInfo", "EventInfo", "the name of the EventInfo object to retrieve"};

    /// \brief the PrimaryVertex key
    SG::ReadHandleKey<xAOD::VertexContainer> m_primaryVerticesKey {this, "primaryVertices", "PrimaryVertices", "the name of the PrimaryVertex container to retrieve"};

    /// \brief the particle container we run on
  private:
    SysReadHandle<xAOD::IParticleContainer> m_particlesHandle {
      this, "particles", "", "the asg collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the accessor for \ref m_selectionDecoration
  private:
    SysWriteSelectionHandle m_selectionHandle {
      this, "selectionDecoration", "trackSelection", "the decoration for the asg selection"};

    /// \brief the ISelectionNameSvc
  private:
    ServiceHandle<ISelectionNameSvc> m_nameSvc {"SelectionNameSvc", "AsgLeptonTrackSelectionAlg"};


    /// \brief the \ref asg::AcceptInfo we are using
  private:
    asg::AcceptInfo m_accept;
  };
}

#endif
