/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_OFFLINEELECTRONDECORATORALG_H
#define INDETTRACKPERFMON_OFFLINEELECTRONDECORATORALG_H

/**
 * @file OfflineElectronDecoratorAlg.h
 * @brief Algorithm to decorate offline tracks with the corresponding
 *        offline electron object (if required for trigger analysis) 
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date 25 September 2023
 **/

/// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

/// xAOD includes
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODEgamma/ElectronContainer.h"

/// STL includes
#include <string>
#include <vector>

/// Local includes
#include "SafeDecorator.h"


namespace IDTPM {

  class OfflineElectronDecoratorAlg :
      public AthReentrantAlgorithm {

  public:

    typedef ElementLink<xAOD::ElectronContainer> ElementElectronLink_t;

    OfflineElectronDecoratorAlg( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~OfflineElectronDecoratorAlg() = default;

    virtual StatusCode initialize() override;

    virtual StatusCode execute( const EventContext& ctx ) const override;

  private:

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_offlineTrkParticlesName {
        this, "OfflineTrkParticleContainerName", "InDetTrackParticles", "Name of container of offline tracks" };

    StringProperty m_prefix{ this, "Prefix", "LinkedElectron_", "Decoration prefix to avoid clashes" };

    StatusCode decorateElectronTrack(
        const xAOD::TrackParticle& track,
        std::vector< IDTPM::OptionalDecoration< xAOD::TrackParticleContainer,
                                                ElementElectronLink_t > >& ele_decor,
        const xAOD::ElectronContainer& electrons ) const;

    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronsName {
        this, "ElectronContainerName", "Electrons", "Name of container of offline electrons" };

    BooleanProperty m_useGSF { this, "useGSF", false, "Match electron with original ID track after GSF" };
   
    enum ElectronDecorations : size_t {
      All,
      Tight,
      Medium,
      Loose,
      LHTight,
      LHMedium,
      LHLoose,
      NDecorations
    };

    const std::vector< std::string > m_decor_ele_names {
      "All",
      "Tight",
      "Medium",
      "Loose",
      "LHTight",
      "LHMedium",
      "LHLoose"
    };
  
    std::vector< IDTPM::WriteKeyAccessorPair< xAOD::TrackParticleContainer,
                                              ElementElectronLink_t > > m_decor_ele{};

};

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_OFFLINEELECTRONDECORATORALG_H
