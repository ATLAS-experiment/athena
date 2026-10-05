///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef DERIVATIONFRAMEWORK_METREMAPPINGALG_H
#define DERIVATIONFRAMEWORK_METREMAPPINGALG_H

#include <string>
#include <vector>
#include <map>

#include "Gaudi/Property.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthLinks/ElementLink.h"

#include "xAODBase/IParticle.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODMissingET/MissingETAssociationMap.h"
#include "xAODMissingET/MissingETAuxAssociationMap.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETAuxContainer.h"

namespace DerivationFramework {
  class METRemappingAlg : public AthAlgorithm {
  public:
    METRemappingAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~METRemappingAlg() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;
    
  private:
    typedef std::map<const xAOD::IParticle*, ElementLink<xAOD::IParticleContainer>> linkMap_t;
    template<typename handle_t> StatusCode fillLinkMap(linkMap_t &map, handle_t &handle);

    SG::ReadHandleKey<xAOD::JetContainer> m_jetContKey{this, "JetCollectionKey", "AnalysisJets", "SG key for the analysis jets collection"};
    SG::ReadHandleKey<xAOD::PhotonContainer> m_photonContKey{this, "PhotonCollectionKey", "AnalysisPhotons", "SG key for the analysis photons collection"};
    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronContKey{this, "ElectronCollectionKey", "AnalysisElectrons", "SG key for the analysis electrons collection"};
    SG::ReadHandleKey<xAOD::MuonContainer> m_muonContKey{this, "MuonCollectionKey", "AnalysisMuons", "SG key for the analysis muons collection"};
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tauContKey{this, "TauCollectionKey", "AnalysisTauJets", "SG key for the analysis tau jets collection"};
    SG::ReadHandleKey<xAOD::MissingETAssociationMap> m_inputMapKey{this, "AssociationInputKey", "METAssoc_AntiKt4EMPFlow", "SG key for the input MissingETAssociationMap"};
    SG::ReadHandleKey<xAOD::MissingETContainer> m_inputCoreKey{this, "METCoreInputKey", "MET_Core_AntiKt4EMPFlow", "SG key for the input MET core container"};
    SG::WriteHandleKey<xAOD::MissingETAssociationMap> m_outputMapKey{this, "AssociationOutputKey", "METAssoc_AnalysisMET", "SG key for the output MissingETAssociationMap"};
    SG::WriteHandleKey<xAOD::MissingETContainer> m_outputCoreKey{this, "METCoreOutputKey", "MET_Core_AnalysisMET", "SG key for the output MET core container"};

    const SG::ConstAccessor< ElementLink<xAOD::IParticleContainer> > m_accOriginalObject{"originalObjectLink"};

  }; //> end class METRemappingAlg
} //> end namespace DerivationFramework

#endif //> !DERIVATIONFRAMEWORK_METREMAPPINGALG_H
