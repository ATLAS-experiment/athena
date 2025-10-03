/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_FTFROADDEFINER_H
#define  TRIGL2MUONSA_FTFROADDEFINER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonRoad.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "TrkExInterfaces/IExtrapolator.h"

namespace TrigL2MuonSA {

  // --------------------------------------------------------------------------------
  // --------------------------------------------------------------------------------

  class FtfRoadDefiner: public AthAlgTool
  {
  public:

    FtfRoadDefiner(const std::string& type,
		   const std::string& name,
		   const IInterface*  parent);

    virtual StatusCode initialize() override;

  public:
    StatusCode defineRoad( const EventContext& ctx,
                           const xAOD::TrackParticle* idtrack,
                           TrigL2MuonSA::MuonRoad&    muonRoad) const;

    std::unique_ptr<const Trk::TrackParameters> extTrack( const EventContext& ctx, const bool CylinderFirst, const xAOD::TrackParticle* trk, const double R, const double Z, int& extFlag ) const;
    std::unique_ptr<const Trk::TrackParameters> extTrack( const EventContext& ctx, const bool CylinderFirst, const Trk::TrackParameters& param, const double R, const double Z, int& extFlag ) const;

  protected:

  private:
    ToolHandle<Trk::IExtrapolator> m_extrapolator{
      this, "IOExtrapolator", "Trk::Extrapolator/AtlasExtrapolator"};

  };

  // --------------------------------------------------------------------------------
  // --------------------------------------------------------------------------------
}

#endif // TRIGL2MUONSA_FTFROADDEFINER_H
