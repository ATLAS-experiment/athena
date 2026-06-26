/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Tool to decorate the EventInfo object with truth categories informations
// Authors: T. Guillemin, J. Lacey, D. Gillberg

#ifndef DerivationFrameworkHiggs_TruthCategoriesDecorator_H
#define DerivationFrameworkHiggs_TruthCategoriesDecorator_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

// EDM: typedefs, so must be included and not forward-referenced
#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthEventContainer.h"

// Note: must include TLorentzVector before the next one

#include "GenInterfaces/IHiggsTruthCategoryTool.h"
#include "GenInterfaces/IxAODtoHepMCTool.h"
#include "TruthRivetTools/HiggsTemplateCrossSectionsDefs.h"

namespace DerivationFramework {

    class TruthCategoriesDecorator : public AthReentrantAlgorithm {
    public:
        TruthCategoriesDecorator(const std::string& n, ISvcLocator* p);
        virtual ~TruthCategoriesDecorator() = default;
        StatusCode initialize();
        StatusCode execute(const EventContext& ctx) const;

    private:
        ToolHandle<IxAODtoHepMCTool> m_xAODtoHepMCTool{this, "HepMCTool", "xAODtoHepMCTool"};
        ToolHandle<IHiggsTruthCategoryTool> m_higgsTruthCatTool{this, "CategoryTool", "HiggsTruthCategoryTool"};

        // Path to TEnv file containing MC-channel-number <-> HiggsProdMode map
        Gaudi::Property<std::string> m_configPath{this, "ConfigPath", "DerivationFrameworkHiggs/HiggsMCsamples.cfg"};
        struct HTXSSample {
            /// Higgs production modes, corresponding to input sample
            HTXS::HiggsProdMode prod{HTXS::HiggsProdMode::UNKNOWN};
            /// Additional identifier flag for TH production modes
            HTXS::tH_type th_type{HTXS::tH_type::noTH};
            // DSIDs satisfying this production mode
            std::set<int> dsids{};
        };

        std::vector<HTXSSample> m_htxs_samples{};

        // Detail level. Steers amount of decoration.
        //  0: basic information. Categoization ints, Higgs prod mode, Njets, Higgs pT
        //  1: the above + Higgs boson 4-vec + associated V 4-vec
        //  2: the above + truth jets built excluding the Higgs boson decay
        //  3: the above + 4-vector sum of all decay products from Higgs boson and V-boson
        Gaudi::Property<int> m_detailLevel{this, "DetailLevel", 3};

        SG::ReadHandleKey<xAOD::TruthEventContainer> m_truthEvtKey{this, "TruthEvtKey", "TruthEvents"};
        SG::ReadHandleKey<xAOD::EventInfo> m_evtInfoKey{this, "EvtKey", "EventInfo"};

        using EvtInfoDecorKey = SG::WriteDecorHandleKey<xAOD::EventInfo>;

        EvtInfoDecorKey m_dec_prodModeKey
          { this, "ProdModeDecorKey", m_evtInfoKey, "HTXS_prodMode", "" };
        EvtInfoDecorKey m_dec_errorCodeKey
          { this, "ErrorCodeDecorKey", m_evtInfoKey, "HTXS_errorCore", "" };
        EvtInfoDecorKey m_dec_stage0CatKey
          { this, "Stage0CatDecorKey", m_evtInfoKey, "HTXS_Stage0_Category", "" };
        // Stage 1 binning
        EvtInfoDecorKey m_dec_stage1CatPt25Key
          { this, "Stage1CatPt25Key", m_evtInfoKey, "HTXS_Stage1_Category_pTjet25", "" };
        EvtInfoDecorKey m_dec_stage1CatPt30Key
          { this, "Stage1CatPt30Key", m_evtInfoKey, "HTXS_Stage1_Category_pTjet30", "" };
        EvtInfoDecorKey m_dec_stage1IdxPt25Key
          { this, "Stage1IdxPt25Key", m_evtInfoKey, "HTXS_Stage1_FineIndex_pTjet25", "" };
        EvtInfoDecorKey m_dec_stage1IdxPt30Key
          { this, "Stage1IdxPt30Key", m_evtInfoKey, "HTXS_Stage1_FineIndex_pTjet30", "" };

        // Stage-1.2 binning
        EvtInfoDecorKey m_dec_stage1p2_CatPt25Key
          { this, "Stage1p2_CatPt25Key", m_evtInfoKey, "HTXS_Stage1_2_Category_pTjet25", "" };
        EvtInfoDecorKey m_dec_stage1p2_CatPt30Key
          { this, "Stage1p2_CatPt30Key", m_evtInfoKey, "HTXS_Stage1_2_Category_pTjet30", "" };
        EvtInfoDecorKey m_dec_stage1p2_IdxPt25Key
          { this, "Stage1p2_IdxPt25Key", m_evtInfoKey, "HTXS_Stage1_2_FineIndex_pTjet25", "" };
        EvtInfoDecorKey m_dec_stage1p2_IdxPt30Key
          { this, "Stage1p2_IdxPt30Key", m_evtInfoKey, "HTXS_Stage1_2_FineIndex_pTjet30", "" };

        // Stage-1.2 finer binning
        EvtInfoDecorKey m_dec_stage1p2_Fine_CatPt25Key
          { this, "Stage1p2_Fine_CatPt25Key", m_evtInfoKey, "HTXS_Stage1_2_Fine_Category_pTjet25", "" };
        EvtInfoDecorKey m_dec_stage1p2_Fine_CatPt30Key
          { this, "Stage1p2_Fine_CatPt30Key", m_evtInfoKey, "HTXS_Stage1_2_Fine_Category_pTjet30", "" };
        EvtInfoDecorKey m_dec_stage1p2_Fine_IdxPt25Key
          { this, "Stage1p2_Fine_IdxPt25Key", m_evtInfoKey, "HTXS_Stage1_2_Fine_FineIndex_pTjet25", "" };
        EvtInfoDecorKey m_dec_stage1p2_Fine_IdxPt30Key
          { this, "Stage1p2_Fine_IdxPt30Key", m_evtInfoKey, "HTXS_Stage1_2_Fine_FineIndex_pTjet30", "" };

        // Stage-1.3 binning
        EvtInfoDecorKey m_dec_stage1p3_CatPt25Key
          { this, "Stage1p3_CatPt25Key", m_evtInfoKey, "HTXS_Stage1_3_Category_pTjet25", "" };
        EvtInfoDecorKey m_dec_stage1p3_CatPt30Key
          { this, "Stage1p3_CatPt30Key", m_evtInfoKey, "HTXS_Stage1_3_Category_pTjet30", "" };
        EvtInfoDecorKey m_dec_stage1p3_IdxPt25Key
          { this, "Stage1p3_IdxPt25Key", m_evtInfoKey, "HTXS_Stage1_3_FineIndex_pTjet25", "" };
        EvtInfoDecorKey m_dec_stage1p3_IdxPt30Key
          { this, "Stage1p3_IdxPt30Key", m_evtInfoKey, "HTXS_Stage1_3_FineIndex_pTjet30", "" };

        // Stage-1.3 finer binning
        EvtInfoDecorKey m_dec_stage1p3_Fine_CatPt25Key
          { this, "Stage1p3_Fine_CatPt25Key", m_evtInfoKey, "HTXS_Stage1_3_Fine_Category_pTjet25", "" };
        EvtInfoDecorKey m_dec_stage1p3_Fine_CatPt30Key
          { this, "Stage1p3_Fine_CatPt30Key", m_evtInfoKey, "HTXS_Stage1_3_Fine_Category_pTjet30", "" };
        EvtInfoDecorKey m_dec_stage1p3_Fine_IdxPt25Key
          { this, "Stage1p3_Fine_IdxPt25Key", m_evtInfoKey, "HTXS_Stage1_3_Fine_FineIndex_pTjet25", "" };
        EvtInfoDecorKey m_dec_stage1p3_Fine_IdxPt30Key
          { this, "Stage1p3_Fine_IdxPt30Key", m_evtInfoKey, "HTXS_Stage1_3_Fine_FineIndex_pTjet30", "" };

        EvtInfoDecorKey m_dec_NJets25Key
          { this, "NJets25Key", m_evtInfoKey, "HTXS_Njets_pTjet25", "" };
        EvtInfoDecorKey m_dec_NJets30Key
          { this, "NJets30Key", m_evtInfoKey, "HTXS_Njets_pTjet30", "" };
        EvtInfoDecorKey m_dec_isZnunuKey
          { this, "IsZnunuKey", m_evtInfoKey, "HTXS_isZ2vvDecay", "" };

        /// Set of DecorHandleKeys to write the four momenta needed for the HTXS categorization.
        struct FourMomDecorationKeys {
            FourMomDecorationKeys(TruthCategoriesDecorator* parent,
                                  const SG::ReadHandleKey<xAOD::EventInfo>& ei_key,
                                  const std::string& the_prefix) :
                prefix{the_prefix},
                pt{parent, prefix + "_ptKey", ei_key, prefix + "_pt"},
                eta{parent, prefix + "_etaKey", ei_key, prefix + "_eta"},
                phi{parent, prefix + "_phiKey", ei_key, prefix + "_phi"},
                m{parent, prefix + "_mKey", ei_key, prefix + "_m"} {}
            std::string prefix;
            EvtInfoDecorKey pt;
            EvtInfoDecorKey eta;
            EvtInfoDecorKey phi;
            EvtInfoDecorKey m;
            StatusCode initialize(bool used=true){
                if (!pt.initialize(used).isSuccess() || !eta.initialize(used).isSuccess() || !phi.initialize(used).isSuccess() || !m.initialize(used).isSuccess()){
                    return StatusCode::FAILURE;
                }
                return StatusCode::SUCCESS;
            }
        };

        EvtInfoDecorKey m_dec_Higgs_ptKey
          { this, "Higgs_ptKey", m_evtInfoKey, "HTXS_Higgs_pt", "" };
        FourMomDecorationKeys m_decp4_HiggsKeys
          { this, m_evtInfoKey, "HTXS_Higgs" };
        FourMomDecorationKeys m_decp4_VKeys
          { this, m_evtInfoKey, "HTXS_V" };
        FourMomDecorationKeys m_decp4_V_jets25Keys
          { this, m_evtInfoKey, "HTXS_V_jets25" };
        FourMomDecorationKeys m_decp4_V_jets30Keys
          { this, m_evtInfoKey, "HTXS_V_jets30" };
        FourMomDecorationKeys m_decp4_Higgs_decayKeys
          { this, m_evtInfoKey, "HTXS_Higgs_decay" };
        FourMomDecorationKeys m_decp4_V_decayKeys
          { this, m_evtInfoKey, "HTXS_V_decay" };

        // Methods for decoration of four vectors
        StatusCode decorateFourVec(const EventContext& ctx,
                                   const FourMomDecorationKeys& keys,
                                   const xAOD::EventInfo& eventInfo,
                                   const TLorentzVector& p4) const;
        StatusCode decorateFourVecs(const EventContext& ctx,
                                    const FourMomDecorationKeys& keys,
                                    const xAOD::EventInfo& eventInfo,
                                    const std::vector<TLorentzVector>& p4s) const;
    };  /// class

}  // namespace DerivationFramework

#endif
