/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TruthDECORATORALG_H
#define INDETTRACKPERFMON_TruthDECORATORALG_H

/**
 * @file TruthDecoratorAlg.h
 * @brief Algorithm to decorate offline tracks with the corresponding
 *        offline muon object (if required for trigger analysis) 
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date 25 September 2023
 **/

/// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

/// xAOD includes
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"


/// STL includes
#include <string>
#include <vector>

/// Local includes
#include "SafeDecorator.h"


namespace IDTPM {

  class TruthDecoratorAlg :
      public AthReentrantAlgorithm {

  public:

    //typedef ElementLink<xAOD::MuonContainer> ElementMuonLink_t;

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
        std::vector< IDTPM::OptionalDecoration< xAOD::TruthParticleContainer, int >> &truth_decor) const;


    enum TruthDecorations : size_t {
      truthType,
      truthOrigin
    };

    const std::vector< std::string > m_decor_truth_names {
      "truthType",
      "truthOrigin"
    };

    std::vector< IDTPM::WriteKeyAccessorPair< xAOD::TruthParticleContainer,
                                              int > > m_decor_truth{};


    PublicToolHandle<IMCTruthClassifier> m_truthClassifier {this,"MCTruthClassifier","MCTruthClassifier/MCTruthClassifier",""};


  };

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_TruthDECORATORALG_H
