/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_VERTEXFINDER_H
#define DITAUREC_VERTEXFINDER_H

#include "DiTauToolBase.h"
#include "AsgTools/PropertyWrapper.h"
#include "JetEDM/TrackVertexAssociation.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "StoreGate/ReadHandleKey.h"

class VertexFinder : public DiTauToolBase {
 public:

  //-------------------------------------------------------------
  //! Constructor
  //-------------------------------------------------------------
  VertexFinder(const std::string& type,
	       const std::string& name,
	       const IInterface * parent);

  //-------------------------------------------------------------
  //! Destructor
  //-------------------------------------------------------------
  virtual ~VertexFinder();

  virtual StatusCode initialize() override;

  virtual StatusCode execute(DiTauCandidateData * data,
			     const EventContext& ctx) const override;

  ElementLink<xAOD::VertexContainer> getPV_TJVA(const xAOD::DiTauJet*, const xAOD::VertexContainer*,
						float& maxJVF,
						const EventContext& ctx) const;

  static float getJetVertexFraction(const xAOD::Vertex*, const std::vector<const xAOD::TrackParticle*>&, const jet::TrackVertexAssociation*) ;


 private:
  SG::ReadHandleKey<xAOD::VertexContainer> m_primVtxContainerName
    { this, "PrimVtxContainerName", "PrimaryVertices", "" };
  SG::ReadHandleKey<jet::TrackVertexAssociation> m_trackVertexAssocName
    { this, "TrackVertexAssociation", "JetTrackVtxAssoc_forDiTaus", "" };

  Gaudi::Property<std::string> m_assocTracksName{this, "AssociatedTracks", "GhostTrack"};
  Gaudi::Property<bool> m_useTJVA {this, "UseTJVA", true};

};

#endif  // DITAUREC_VERTEXFINDER_H
