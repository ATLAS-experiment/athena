/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRUTHHITDECORATORALG_H
#define INDETTRACKPERFMON_TRUTHHITDECORATORALG_H

/**
 * @file TruthHitDecoratorAlg.h
 * header file for class of same name
 * adapted from original IDPVM InDetPhysValTruthDecoratorAlg
 * @author shaun roe
 * @date 27 March 2014
 * @brief Algorithm to decorate xAOD::TruthParticles with additional
 *        information regarding the corresponding track hits
 *        required for validation
 **/

/// Gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

/// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "TrkExInterfaces/IExtrapolator.h"

/// EDM includes
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

/// STD includes
#include <string>
#include <atomic>
#include <unordered_map>
#include <vector>

/// Local includes
#include "SafeDecorator.h"


namespace IDTPM {

  class TruthHitDecoratorAlg :
      public AthReentrantAlgorithm {

  public:

    TruthHitDecoratorAlg( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~TruthHitDecoratorAlg() = default;

    virtual StatusCode initialize() override;

    virtual StatusCode execute( const EventContext& ctx ) const override;

  private:

    StatusCode decorateTruth(
      const xAOD::TruthParticle& particle,
      std::vector< IDTPM::OptionalDecoration< xAOD::TruthParticleContainer,
                                              float > >& float_decor,
      const Amg::Vector3D& beamPos, 
      std::unordered_map< int, float >& pixelMap,
      std::unordered_map< int, float >& sctMap,
      const EventContext& ctx ) const;

    PublicToolHandle< Trk::IExtrapolator > m_extrapolator {
        this, "Extrapolator", "Trk::Extrapolator/AtlasExtrapolator", "" };

    SG::ReadDecorHandleKeyArray< xAOD::EventInfo > m_beamSpotDecoKey {
        this, "BeamSpotDecoKeys", 
        { "EventInfo.beamPosX", "EventInfo.beamPosY", "EventInfo.beamPosZ" },
        "Beamspot position decoration keys" };

    mutable std::atomic<bool> m_errorEmitted{ false };

    ///TruthParticle container's name needed to create decorators
    SG::ReadHandleKey< xAOD::TruthParticleContainer > m_truthParticleName {
        this, "TruthParticleContainerName",  "TruthParticles", "" };

    StringProperty m_prefix { this, "Prefix", "", "Decoration prefix to avoid clashes." };
  
    SG::ReadHandleKey< xAOD::TrackMeasurementValidationContainer > m_truthPixelClusterName {
        this, "PixelClusterContainerName", "PixelClusters", "" };
  
    SG::ReadHandleKey< xAOD::TrackMeasurementValidationContainer > m_truthSCTClusterName {
        this, "SCTClusterContainerName",  "SCT_Clusters", "" };
   
    enum TruthDecorations {
      D0,
      Z0,
      Phi,
      Theta,
      Z0st,
      QOverP,
      ProdR,
      ProdZ,
      NSilHits,
      NDecorations
    };

    const std::vector< std::string > m_decor_names {
      "d0",
      "z0",
      "phi",
      "theta",
      "z0st",
      "qOverP",
      "prodR",
      "prodZ",
      "nSilHits"
    };

    std::vector< std::pair< 
        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer >,
        SG::AuxElement::ConstAccessor< float > > > m_decor{};
  };

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_TRUTHHITDECORATORALG_H 
