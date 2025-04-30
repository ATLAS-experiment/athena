/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGEGAMMAHYPO_TRIGPHOTONCALOISOHYPOALG_PRECISION_H
#define TRIGEGAMMAHYPO_TRIGPHOTONCALOISOHYPOALG_PRECISION_H 1

#include <string>

#include "GaudiKernel/ToolHandle.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/PhotonAuxContainer.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "DecisionHandling/HypoBase.h"
#include "ITrigEgammaPrecisionPhotonCaloIsoHypoTool.h"


/**
 * @class TrigEgammaPrecisionPhotonCaloIsoHypoAlg
 * @brief Implements egamma calorimeter isolation selection for precision photon triggers
 **/
class TrigEgammaPrecisionPhotonCaloIsoHypoAlg : public ::HypoBase {
  public: 

    TrigEgammaPrecisionPhotonCaloIsoHypoAlg( const std::string& name, ISvcLocator* pSvcLocator );

    virtual StatusCode  initialize() override;
    virtual StatusCode  execute( const EventContext& context ) const override;

  private: 

    ToolHandleArray< ITrigEgammaPrecisionPhotonCaloIsoHypoTool > m_hypoTools { this, "HypoTools", {}, "Hypo tools" }; 
    SG::ReadHandleKey< xAOD::PhotonContainer > m_photonsKey { this, "Photons", "", "Photons in roi" };  
    SG::WriteHandleKey< xAOD::PhotonContainer > m_IsophotonsKey { this, "IsoPhotons", "", "Output isolated photon container" };

    //Gaudi::Property<std::vector<std::string>> m_isemNames {this, "IsEMNames", {}, "IsEM pid names."};
    // Should I add here the isolation tool and run it as in Egamma reconstruction?

}; 

#endif //> !TRIGEGAMMAHYPO_TESTTRIGPHOTONCALOISOHYPOALG_H
