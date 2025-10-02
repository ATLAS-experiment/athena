// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file TrackD3PDMaker/src/TrackParticlePerigeeAtBSAssociationTool.h
 * @author remi zaidan <remi.zaidan@cern.ch>
 * @date Feb, 2010
 * @brief Associate from a TrackParticle to its perigee at the beam spot.
 */
#ifndef TRACKD3PDMAKER_TRACKPARTICLEPERIGEEATBSASSOCIATIONTOOL_H
#define TRACKD3PDMAKER_TRACKPARTICLEPERIGEEATBSASSOCIATIONTOOL_H

#include "D3PDMakerUtils/SingleAssociationTool.h"
#include "ITrackToVertex/ITrackToVertex.h"
#include "TrkParameters/TrackParameters.h"
#include "xAODTracking/TrackParticle.h"
#include "GaudiKernel/ToolHandle.h"
#include <vector>
#include "BeamSpotConditionsData/BeamSpotData.h"

namespace Rec {
  class TrackParticle;
}

namespace D3PD {

/**
 * @brief Associate from a VxCandidate to its perigee at the beam spot.
 */
class TrackParticlePerigeeAtBSAssociationTool
  : public SingleAssociationTool<Types<xAOD::TrackParticle>, Trk::TrackParameters>
{
public:
  typedef SingleAssociationTool<Types<xAOD::TrackParticle>, Trk::TrackParameters> Base;

  using Base::Base;


  virtual StatusCode initialize() override;


  /**
   * @brief Return the target object.
   * @param p The source object for the association.
   *
   * Return the target of the association, or 0.
   */
  virtual const Trk::TrackParameters* get (const xAOD::TrackParticle& p) override;


  virtual void releaseObject (const Trk::TrackParameters* p) override;


 private:

  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

  /* Track to vertex extrapolator **/
  ToolHandle<Reco::ITrackToVertex> m_trackToVertexTool
    { this, "TrackToVertexTool", "Reco::TrackToVertex", "" };

};


} // namespace D3PD



#endif // not TRACKD3PDMAKER_TRACKPARTICLEPERIGEEATBSASSOCIATIONTOOL_H
