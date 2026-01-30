/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_OFFLINEJETDECORATORALG_H
#define INDETTRACKPERFMON_OFFLINEJETDECORATORALG_H

/**
 * @file OfflineJetDecoratorAlg.h
 * @brief Algorithm to decorate offline tracks with the corresponding
 *        (reco || truth) jet object
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date 22 May 2025
 **/

/// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

/// xAOD includes
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODJet/JetContainer.h"

/// STL includes
#include <string>
#include <vector>

/// Local includes
#include "InDetTrackPerfMon/SafeDecorator.h"


namespace IDTPM {

  class OfflineJetDecoratorAlg :
      public AthReentrantAlgorithm {

  public:

    typedef ElementLink<xAOD::JetContainer> ElementJetLink_t;

    OfflineJetDecoratorAlg( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~OfflineJetDecoratorAlg() = default;

    virtual StatusCode initialize() override;

    virtual StatusCode execute( const EventContext& ctx ) const override;

  private:

    bool passJetCuts( const xAOD::Jet& jet ) const;

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_offlineTrkParticlesName {
        this, "OfflineTrkParticleContainerName", "InDetTrackParticles", "Name of container of offline tracks" };

    StringProperty m_prefix{ this, "Prefix", "LinkedJet_", "Decoration prefix to avoid clashes" };

    StatusCode decorateJetTrack(
        const xAOD::TrackParticle& track,
        std::vector< IDTPM::OptionalDecoration< xAOD::TrackParticleContainer,
                                                ElementJetLink_t > >& jet_decor,
        const xAOD::JetContainer& jets ) const;

    SG::ReadHandleKey< xAOD::JetContainer > m_jetsName {
        this, "JetContainerName", "InTimeAntiKt4TruthJets", "Name of container of jets" };

    FloatProperty m_maxTrkJetDR{ this, "maxTrkJetDR", 0.4, "the maximum DeltaR to jets to allow for track-in-jet plots" };    
    FloatProperty m_jetAbsEtaMin{ this, "JetAbsEtaMin", -999.0, "Minimum Eta value for jet selection" };
    FloatProperty m_jetAbsEtaMax{ this, "JetAbsEtaMax", 4.0, "Maximum Eta value for jet selection" };
    FloatProperty m_jetPtMin{ this, "JetPtMin", 1000.0, "Minimum Jet pT for jet selection" };
    FloatProperty m_jetPtMax{ this, "JetPtMax", 5000000.0, "Maximum Jet pT for jet selection" };

    enum JetDecorations : size_t {
      DRtruthJet,
      DRtruthLightJet,
      DRtruthCjet,
      DRtruthBjet,
      DRtruthHeavyJet,
      DRGhostTruth,
      DRGhostTruthLightJet,
      DRGhostTruthBjet,
      NDecorations
    };

    const std::vector< std::string > m_decor_jet_names {
      "DRtruthJet",
      "DRtruthLightJet",
      "DRtruthCjet",
      "DRtruthBjet",
      "DRtruthHeavyJet",
      "DRGhostTruthJet",
      "DRGhostTruthLightJet",
      "DRGhostTruthBjet"
    };
  
    std::vector< IDTPM::WriteKeyAccessorPair< xAOD::TrackParticleContainer,
                                              ElementJetLink_t > > m_decor_jet{};

};

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_OFFLINEJETDECORATORALG_H
