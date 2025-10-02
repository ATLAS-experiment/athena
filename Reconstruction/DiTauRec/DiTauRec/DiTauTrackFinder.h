/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_DITAUTRACKFINDER_H
#define DITAUREC_DITAUTRACKFINDER_H

#include "DiTauToolBase.h"
#include "AsgTools/PropertyWrapper.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"

class DiTauTrackFinder : public DiTauToolBase {
 public:

  //-------------------------------------------------------------
  //! Constructor
  //-------------------------------------------------------------
  DiTauTrackFinder(const std::string& type,
		   const std::string& name,
		   const IInterface * parent);

  //-------------------------------------------------------------
  //! Destructor
  //-------------------------------------------------------------
  virtual ~DiTauTrackFinder();

  virtual StatusCode initialize() override;

  virtual StatusCode execute(DiTauCandidateData * data,
			     const EventContext& ctx) const override;

 
  // ------------------------------------------------------------
  // Definition of track types
  // ------------------------------------------------------------
  enum DiTauTrackType { 
    DiTauSubjetTrack = 0,
    DiTauIsoTrack = 1,
    DiTauOtherTrack = 2,
    OutsideTrack = 3
  };

  void getTracksFromPV( const DiTauCandidateData*,
			const xAOD::TrackParticleContainer*,
			const xAOD::Vertex*,
			std::vector<const xAOD::TrackParticle*>&,
			std::vector<const xAOD::TrackParticle*>&,
			std::vector<const xAOD::TrackParticle*>& ) const;

  DiTauTrackType diTauTrackType( const DiTauCandidateData*,
				 const xAOD::TrackParticle*,
				 const xAOD::Vertex* ) const;




 private:

  Gaudi::Property<float> m_MaxDrJet{this, "MaxDrJet", 1.0};
  Gaudi::Property<float> m_MaxDrSubjet{this, "MaxDrSubjet", 0.2};
  Gaudi::Property<int> m_MaxNTracksSubjet{this, "MaxNTracksSubjet", -1}; 

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrackParticleContainerName
    { this, "TrackParticleContainer", "InDetTrackParticles", "" };

  ToolHandle<Trk::ITrackSelectorTool> m_TrackSelectorTool{this, "TrackSelectorTool", ""};

};

#endif // DITAUREC_DITAUTRACKFINDER_H
