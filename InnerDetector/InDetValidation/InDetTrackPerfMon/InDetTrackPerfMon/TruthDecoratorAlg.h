/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TruthDECORATORALG_H
#define INDETTRACKPERFMON_TruthDECORATORALG_H

/**
 * @file TruthDecoratorAlg.h
 * @brief Algorithm to decorate truth particles with their
 *        origin && type classes from MCTruthClassifier
 * @author Federica Piazza <federica.piazza@cern.ch>, Marco Aparo <marco.aparo@cern.ch>
 * @date 1 November 2024
 **/

/// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

/// xAOD includes
#include "xAODTruth/TruthParticleContainer.h"


/// STL includes
#include <string>
#include <vector>

/// Local includes
#include "InDetTrackPerfMon/SafeDecorator.h"


namespace IDTPM {

  class TruthDecoratorAlg :
      public AthReentrantAlgorithm {

  public:

    TruthDecoratorAlg( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~TruthDecoratorAlg() = default;

    virtual StatusCode initialize() override;

    virtual StatusCode execute( const EventContext& ctx ) const override;

  private:

    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticlesName {
        this, "TruthParticleContainerName", "TruthParticles", "Name of container of truth particles" };

    StringProperty m_prefix { this, "Prefix", "Truth_", "Decoration prefix to avoid clashes" };

    StatusCode decorateTruthParticle(
        const xAOD::TruthParticle& truth,
        std::vector< IDTPM::OptionalDecoration< xAOD::TruthParticleContainer, int >>& truth_decor ) const;


    enum TruthDecorations : size_t {
      Type,
      Origin,
      NDecorations
    };

    const std::vector< std::string > m_decor_truth_names {
      "truthType",
      "truthOrigin"
    };

    std::vector< IDTPM::WriteKeyAccessorPair< xAOD::TruthParticleContainer,
                                              int > > m_decor_truth{}; // FIXME Why not use a WriteDecorHandleKeyArray here?


    PublicToolHandle< IMCTruthClassifier > m_truthClassifier {
        this, "MCTruthClassifier", "MCTruthClassifier/MCTruthClassifier", "Truth classification tool" };

  };

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_TruthDECORATORALG_H
