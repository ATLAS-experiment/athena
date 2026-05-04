/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EgammaTrackParticleThinning.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_EGAMMATRACKPARTICLETHINNING_H
#define DERIVATIONFRAMEWORK_EGAMMATRACKPARTICLETHINNING_H

#include <atomic>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInDet/TracksInCone.h"
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ThinningHandleKey.h"
#include "xAODEgamma/EgammaContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"


namespace DerivationFramework {

class EgammaTrackParticleThinning
  : public extends<ExpressionParserUser<AthAlgTool>, IThinningTool>
{
public:
  EgammaTrackParticleThinning(const std::string& t,
                              const std::string& n,
                              const IInterface* p);
  virtual ~EgammaTrackParticleThinning();
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode doThinning(const EventContext& ctx) const override;

private:
  mutable std::atomic<unsigned long int> m_ntot = 0;
  mutable std::atomic<unsigned long int> m_ntotGSF = 0;
  mutable std::atomic<unsigned long int> m_ntotGSFVtx = 0;
  mutable std::atomic<unsigned long int> m_npass = 0;
  mutable std::atomic<unsigned long int> m_nGSFPass = 0;
  mutable std::atomic<unsigned long int> m_nGSFVtxPass = 0;
  mutable std::atomic<unsigned long int> m_nEgammas = 0;
  mutable std::atomic<unsigned long int> m_nSelEgammas = 0;

  StringProperty
    m_streamName{ this, "StreamName", "", "Name of the stream being thinned" };

  SG::ReadHandleKey<xAOD::EgammaContainer> m_egammaKey{ this, "SGKey", "", "" };

  SG::ThinningHandleKey<xAOD::TrackParticleContainer>
    m_inDetSGKey{ this, "InDetTrackParticlesKey", "InDetTrackParticles", "" };

  SG::ThinningHandleKey<xAOD::TrackParticleContainer>
    m_gsfSGKey{ this, "GSFTrackParticlesKey", "GSFTrackParticles", "" };

  SG::ThinningHandleKey<xAOD::VertexContainer>
    m_gsfVtxSGKey{ this, "GSFConversionVerticesKey", "", "" };

  StringProperty m_selectionString{ this, "SelectionString", "", "" };

  BooleanProperty m_bestMatchOnly{ this, "BestMatchOnly", true, "" };
  BooleanProperty m_bestVtxMatchOnly{ this, "BestVtxMatchOnly", false, "" };
  FloatProperty m_coneSize{ this, "ConeSize", -1.0, "" };

  void setPhotonMasks(const EventContext& ctx,
                      std::vector<bool>&,
                      std::vector<bool>&,
                      const xAOD::EgammaContainer*,
                      const xAOD::TrackParticleContainer*,
                      const xAOD::TrackParticleContainer*) const;
  void setElectronMasks(std::vector<bool>&,
                        std::vector<bool>&,
                        const xAOD::EgammaContainer*,
                        const xAOD::TrackParticleContainer*,
                        const xAOD::TrackParticleContainer*) const;
  void clearGSFVtx(const EventContext& ctx) const;
};
}

#endif // DERIVATIONFRAMEWORK_EGAMMATRACKPARTICLETHINNING_H
