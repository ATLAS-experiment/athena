/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_OFFLINETAUDECORATORALG_H
#define INDETTRACKPERFMON_OFFLINETAUDECORATORALG_H

/**
 * @file OfflineTauDecoratorAlg.h
 * @brief Algorithm to decorate offline tracks with the corresponding
 *        offline tau object (if required for trigger analysis) 
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date 25 September 2023
 **/

/// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

/// xAOD includes
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTau/TauJetContainer.h"

/// STL includes
#include <string>
#include <vector>

/// Local includes
#include "SafeDecorator.h"


namespace IDTPM {

  class OfflineTauDecoratorAlg :
      public AthReentrantAlgorithm {

  public:

    typedef ElementLink<xAOD::TauJetContainer> ElementTauLink_t;
    typedef ElementLink<xAOD::TrackParticleContainer> ElementTrackLink_t;

    OfflineTauDecoratorAlg( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~OfflineTauDecoratorAlg() = default;

    virtual StatusCode initialize() override;

    virtual StatusCode execute( const EventContext& ctx ) const override;

  private:

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_offlineTrkParticlesName {
        this, "OfflineTrkParticleContainerName", "InDetTrackParticles", "Name of container of offline tracks" };

    StringProperty m_prefix { this, "Prefix", "LinkedTau_", "Decoration prefix to avoid clashes" };

    StringProperty m_tauType { this, "TauType", "RNN", "Type of reconstructed hadronic Tau (BDT or RNN)" };

    UnsignedIntegerProperty m_tauNprongs { this, "TauNprongs", 1, "Number of prongs in hadronic Tau decay (1 or 3)" };

    StatusCode decorateTauTrack(
        const xAOD::TrackParticle& track,
        std::vector< IDTPM::OptionalDecoration< xAOD::TrackParticleContainer,
                                                ElementTauLink_t > >& tau_decor,
        const xAOD::TauJetContainer& taus ) const;

    SG::ReadHandleKey<xAOD::TauJetContainer> m_tausName {
        this, "TauContainerName", "TauJets", "Name of container of offline hadronic taus" };

    enum TauDecorations : size_t {
      All,
      Tight,
      Medium,
      Loose,
      VeryLoose,
      NDecorations
    };

    const std::vector< std::string > m_decor_tau_names {
      "All",
      "Tight",
      "Medium",
      "Loose",
      "VeryLoose"
    };

    std::vector< IDTPM::WriteKeyAccessorPair< xAOD::TrackParticleContainer,
                                              ElementTauLink_t > > m_decor_tau{};

  };

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_OFFLINETAUDECORATORALG_H
