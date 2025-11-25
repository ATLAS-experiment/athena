/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MuonExtrapolationTool_H
#define MuonExtrapolationTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/Muon.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkExInterfaces/IExtrapolator.h"


namespace DerivationFramework {
  class MuonExtrapolationTool : public extends<AthAlgTool, IAugmentationTool> {

  public:
    MuonExtrapolationTool(const std::string& t, const std::string& n, const IInterface *p);

    virtual StatusCode initialize();
    virtual StatusCode addBranches(const EventContext& ctx) const;

  private:

    /// run the extrapolation - only available in full athena
    const Trk::TrackParameters* extrapolateToTriggerPivotPlane(const xAOD::TrackParticle& track, const EventContext& ctx) const;

  // Utility method to handle extrapolation and decoration for one TrackParticle.
  // It looks for the decoration, and, if it is missing, runs track extrapolation, decorating the result
  // to the particle to avoid repeating the process unnecessarily.
  // Returns success (true) or failure (false) of the procedure, fills eta and phi coordinates via reference
  // If the extrapolation fails or the decoration is missing in AthAnalysis, it will *not* change eta and phi
  // So you can set them to defaults before calling this guy, and they will be preserved in case of failure.
  // Will not run outside athena, because it requires the extrapolator
    bool extrapolateAndDecorateTrackParticle(const xAOD::TrackParticle* particle, float & eta, float & phi, const EventContext& ctx) const;

    // utility method: Obtains the track particle which we want to extrapolate into the MS.
    // Works for all kinds of probes.
    const xAOD::TrackParticle* getPreferredTrackParticle (const xAOD::IParticle* probe) const;

    SG::ReadHandleKey<xAOD::MuonContainer> m_muonContainerName{this, "MuonCollection", "Muons"};
    PublicToolHandle<Trk::IExtrapolator> m_extrapolator{this, "Extrapolator", "Trk::Extrapolator/AtlasExtrapolator"};

    // these define the surfaces that we extrapolate to.
    // We approximate the pivot plane in the form of a cylinder surface and two disks
    Gaudi::Property<double> m_endcapPivotPlaneZ{this, "EndcapPivotPlaneZ", 15525., "z position of pivot plane in endcap region"};
    Gaudi::Property<double> m_endcapPivotPlaneMinimumRadius{this, "EndcapPivotPlaneMinimumRadius", 0., "minimum radius of pivot plane in endcap region"};
    Gaudi::Property<double> m_endcapPivotPlaneMaximumRadius{this, "EndcapPivotPlaneMaximumRadius", 11977., "maximum radius of pivot plane in endcap region"};
    Gaudi::Property<double> m_barrelPivotPlaneRadius{this, "BarrelPivotPlaneRadius", 8000., "radius of pivot plane in barrel region"};
    Gaudi::Property<double> m_barrelPivotPlaneHalfLength{this, "BarrelPivotPlaneHalfLength", 9700., "half length of pivot plane in barrel region"};
  };
}
#endif // MuonExtrapolationTool_H
