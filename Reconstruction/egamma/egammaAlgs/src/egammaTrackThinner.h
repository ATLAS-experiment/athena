/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAALGS_EGAMMATRACKTHINNER_H
#define EGAMMAALGS_EGAMMATRACKTHINNER_H
/**
  @class egammaTrackThinner Thinning of GSFTrackParticle collection
*/


// INCLUDE HEADER FILES:
#include "xAODEgamma/ElectronContainerFwd.h"
#include "xAODEgamma/PhotonContainerFwd.h"
#include "xAODTracking/TrackParticleContainerFwd.h"
#include "xAODTracking/VertexContainerFwd.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ThinningHandleKey.h"


class egammaTrackThinner : public AthReentrantAlgorithm
{
 public:

  /** @brief Default constructor*/
  using AthReentrantAlgorithm::AthReentrantAlgorithm;


  /** @brief initialize method*/
  StatusCode initialize() override final;
  /** @brief finalize method*/
  StatusCode finalize() override final;
  /** @brief execute method*/
  StatusCode execute(const EventContext& ctx) const override final;

 private:


  /** @brief electron collection input name*/
  SG::ReadHandleKey<xAOD::ElectronContainer> m_InputElectronContainerKey {this, 
      "InputElectronContainerName", 
      "Electrons", 
      "Name of the input electron container"};

  /** @brief photon collection input name*/
  SG::ReadHandleKey<xAOD::PhotonContainer> m_InputPhotonContainerKey {this,
      "InputPhotonContainerName",
      "Photons",
      "Name of the input photon container"};
		
  /** @brief forward electron collection input name*/
  SG::ReadHandleKey<xAOD::ElectronContainer> m_InputForwardElectronContainerKey
    { this,
      "InputForwardElectronContainerName",
      "",
      "Name of the input electron container" };

  /** @brief GSF Track Particle container to thin */
  SG::ThinningHandleKey<xAOD::TrackParticleContainer> m_TrackParticlesKey {this,
      "TrackParticleContainerName", 
      "",
      "Name of the Track Particle container to thin"};

  /** @brief  GSF vertex container to thin */
  SG::ThinningHandleKey<xAOD::VertexContainer> m_VertexKey {this,
      "VertexContainerName", 
      "",
      "Name of the Vertex container to thin"};

  /** @brief Name of the stream being thinned */
  StringProperty m_streamName
  { this, "StreamName", "", "Name of the stream being thinned" };

  /** @brief Bool to decide if we actually do the Thinning */
  Gaudi::Property<bool> m_doThinning{this, "doThinning", true, "Bool to do Thinning"};
};

#endif

