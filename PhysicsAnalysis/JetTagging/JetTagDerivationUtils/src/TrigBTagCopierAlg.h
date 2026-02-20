/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/* 
    Before Run 4, we historically use BTagging objects associated with online jets to store b-tagging information.
    From Run 4 onward, we use online jets to store b-tagging information directly.
    Therefore, we need to copy the b-tagging information from BTagging objects to online jets, 
    so that the downstream algorithms can access the b-tagging information from jets directly.

    Offline b-tagging information has been all stored in jets since Run 2, so no copying is needed for offline jets.

    Run 3 containers that need to be copied:
    HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTagging -> HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJets
    HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf_BTagging -> HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf_bJets
    HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLA_BTagging -> HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLA

    Run 2 containers that need to be copied:
    HLT_xAOD__BTaggingContainer_HLTBjetFex -> HLT_xAOD__JetContainer_SplitJet,  HLT_xAOD__JetContainer_GSCJet, HLT_xAOD__JetContainer_EFJet
    
    ! This implementation only supports Run 3
    ! Please see https://its.cern.ch/jira/browse/ATR-26904, links from BTagging to jet  are broken
    ! for both Run 2 and Run 3 data. We can only do index matching since link navigation is unavailable.
    ! for Run 2 Data, both index matching and link navigation are unavailable since the BTagging objects
    ! point to multiple jet containers.

    TODO: Figure out a way to support Run 2 data in future if needed.

*/

#ifndef TRIG_BTAG_COPIER_ALG_H
#define TRIG_BTAG_COPIER_ALG_H

#include "xAODJet/JetContainer.h"
#include "xAODBTagging/BTaggingContainer.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

namespace ftag {

    class TrigBTagCopierAlg: public AthReentrantAlgorithm
    {
    public:
        TrigBTagCopierAlg(const std::string& name, ISvcLocator* pSvcLocator);

        // these are the functions inherited from Algorithm
        virtual StatusCode initialize () override;
        virtual StatusCode execute (const EventContext&) const override;
        virtual StatusCode finalize () override;
    private:
        Gaudi::Property<int>  m_trigger_edm_version { this, "TrigEDMVersion", -1, "EDM version of the run"};

        std::vector<
            std::tuple<
                SG::ReadHandleKey<xAOD::BTaggingContainer> /*btag container*/, 
                SG::ReadHandleKey<xAOD::JetContainer> /*jet container*/, 
                std::vector<SG::ReadDecorHandleKey<xAOD::BTaggingContainer>> /*accessor*/, 
                std::vector<SG::WriteDecorHandleKey<xAOD::JetContainer>> /*decorators*/
            >
        > m_decorKeys;

        const static inline std::vector<std::pair<std::string, std::string>> m_conts_pair_run3 = {
            {"HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTagging",      "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJets"},
            {"HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf_BTagging",       "HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf_bJets" },
            {"HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLA_BTagging",  "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLA"}
        };

        // // run 2 discriminants 
        // const static inline std::vector<std::string> m_vars_run2 {
        //     "MV2c00_discriminant", "MV2c10_discriminant", "MV2c20_discriminant",
        // };
        // run 3 discriminants
        const static inline std::vector<std::string> m_vars_run3 {
            "dl1dbb20230314_pb",    "dl1dbb20230314_pbb",   
            "rnnip_pu",             "rnnip_pc",             "rnnip_pb",             "rnnip_ptau",           
            "DL1_pu",               "DL1_pc",               "DL1_pb",                
            "DL1r_pu",              "DL1r_pc",              "DL1r_pb",               
            "dipsLoose20210517_pu", "dipsLoose20210517_pc", "dipsLoose20210517_pb",  
            "dips20210517_pu",      "dips20210517_pc",      "dips20210517_pb",       
            "DL1d20210519r22_pu",   "DL1d20210519r22_pc",   "DL1d20210519r22_pb",    
            "DL1d20210528r22_pu",   "DL1d20210528r22_pc",   "DL1d20210528r22_pb",    
            "dipsLoose20210729_pu", "dipsLoose20210729_pc", "dipsLoose20210729_pb",  
            "DL1dv00_pu",           "DL1dv00_pc",           "DL1dv00_pb",            
            "dips20211116_pu",      "dips20211116_pc",      "dips20211116_pb",       
            "DL1d20211216_pu",      "DL1d20211216_pc",      "DL1d20211216_pb",       
            "GN120220813_pu",       "GN120220813_pc",       "GN120220813_pb",        
            "GN220240122_pu",       "GN220240122_pc",       "GN220240122_pb"         
        };
    };
} // end namespace ftag

#endif