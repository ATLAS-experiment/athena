/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_OFFLINEMUONDECORATORALG_H
#define INDETTRACKPERFMON_OFFLINEMUONDECORATORALG_H

/**
 * @file OfflineMuonDecoratorAlg.h
 * @brief Algorithm to decorate offline tracks with the corresponding
 *        offline muon object (if required for trigger analysis) 
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date 25 September 2023
 **/

/// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

/// xAOD includes
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODMuon/MuonContainer.h"

/// STL includes
#include <string>
#include <vector>

/// Local includes
#include "InDetTrackPerfMon/SafeDecorator.h"


namespace IDTPM {

  class OfflineMuonDecoratorAlg :
      public AthReentrantAlgorithm {

  public:

    typedef ElementLink<xAOD::MuonContainer> ElementMuonLink_t;

    OfflineMuonDecoratorAlg( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~OfflineMuonDecoratorAlg() = default;

    virtual StatusCode initialize() override;

    virtual StatusCode execute( const EventContext& ctx ) const override;

  private:

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_offlineTrkParticlesName {
        this, "OfflineTrkParticleContainerName", "InDetTrackParticles", "Name of container of offline tracks" };

    StringProperty m_prefix { this, "Prefix", "LinkedMuon_", "Decoration prefix to avoid clashes" };

    StatusCode decorateMuonTrack(
        const xAOD::TrackParticle& track,
        std::vector< IDTPM::OptionalDecoration< xAOD::TrackParticleContainer,
                                                ElementMuonLink_t > >& mu_decor,
        const xAOD::MuonContainer& muons ) const;

    SG::ReadHandleKey<xAOD::MuonContainer> m_muonsName {
        this, "MuonContainerName", "Muons", "Name of container of offline muons" };

    BooleanProperty m_useCombinedMuonTracks {
        this, "useCombinedMuonTracks", false, "Match combined muon track to muons instead of ID tracks" };

    enum MuonDecorations : size_t {
      All,
      Tight,
      Medium,
      Loose,
      VeryLoose,
      NDecorations
    };

    const std::vector< std::string > m_decor_mu_names {
      "All",
      "Tight",
      "Medium",
      "Loose",
      "VeryLoose"
    };

    std::vector< IDTPM::WriteKeyAccessorPair< xAOD::TrackParticleContainer,
                                              ElementMuonLink_t > > m_decor_mu{};

  };

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_OFFLINEMUONDECORATORALG_H
