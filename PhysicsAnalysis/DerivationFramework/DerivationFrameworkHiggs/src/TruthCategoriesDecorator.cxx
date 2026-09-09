/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkHiggs/TruthCategoriesDecorator.h"

#include <GaudiKernel/SystemOfUnits.h>
#include <CxxUtils/StringUtils.h>
#include <PathResolver/PathResolver.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteDecorHandle.h>
#include <TEnv.h>
#include <TLorentzVector.h>
#include <TString.h>
#include <TruthUtils/HepMCHelpers.h>
#include <xAODJet/JetContainer.h>
#include <xAODTruth/TruthParticleContainer.h>
#include <xAODTruth/TruthVertex.h>

namespace DerivationFramework {

    TruthCategoriesDecorator::TruthCategoriesDecorator(const std::string& n, ISvcLocator* p) : AthReentrantAlgorithm(n, p) {}

    StatusCode TruthCategoriesDecorator::initialize() {
        ATH_MSG_DEBUG("Initialize ");

        // FOR xAOD->HEPMC ::  xAODtoHepMC tool
        ATH_CHECK(m_xAODtoHepMCTool.retrieve());
        ATH_CHECK(m_higgsTruthCatTool.retrieve());

        ATH_CHECK(m_truthEvtKey.initialize());
        ATH_CHECK(m_evtInfoKey.initialize());

        /// Declare everything that's written by the algorithm
        ATH_CHECK(m_dec_prodModeKey.initialize());
        ATH_CHECK(m_dec_errorCodeKey.initialize());
        ATH_CHECK(m_dec_stage0CatKey.initialize());
        ATH_CHECK(m_dec_stage0DecayCatKey.initialize());
        ATH_CHECK(m_dec_stage1CatPt25Key.initialize());
        ATH_CHECK(m_dec_stage1CatPt30Key.initialize());
        ATH_CHECK(m_dec_stage1IdxPt25Key.initialize());
        ATH_CHECK(m_dec_stage1IdxPt30Key.initialize());
        ATH_CHECK(m_dec_stage1p2_CatPt25Key.initialize());
        ATH_CHECK(m_dec_stage1p2_CatPt30Key.initialize());
        ATH_CHECK(m_dec_stage1p2_IdxPt25Key.initialize());
        ATH_CHECK(m_dec_stage1p2_IdxPt30Key.initialize());
        ATH_CHECK(m_dec_stage1p2_Fine_CatPt25Key.initialize());
        ATH_CHECK(m_dec_stage1p2_Fine_CatPt30Key.initialize());
        ATH_CHECK(m_dec_stage1p2_Fine_IdxPt25Key.initialize());
        ATH_CHECK(m_dec_stage1p2_Fine_IdxPt30Key.initialize());
        ATH_CHECK(m_dec_stage1p3_CatPt25Key.initialize());
        ATH_CHECK(m_dec_stage1p3_CatPt30Key.initialize());
        ATH_CHECK(m_dec_stage1p3_IdxPt25Key.initialize());
        ATH_CHECK(m_dec_stage1p3_IdxPt30Key.initialize());
        ATH_CHECK(m_dec_stage1p3_Fine_CatPt25Key.initialize());
        ATH_CHECK(m_dec_stage1p3_Fine_CatPt30Key.initialize());
        ATH_CHECK(m_dec_stage1p3_Fine_IdxPt25Key.initialize());
        ATH_CHECK(m_dec_stage1p3_Fine_IdxPt30Key.initialize());
        ATH_CHECK(m_dec_NJets25Key.initialize());
        ATH_CHECK(m_dec_NJets30Key.initialize());
        ATH_CHECK(m_dec_isZnunuKey.initialize());

        ATH_CHECK(m_dec_Higgs_ptKey.initialize(m_detailLevel == 0));
        // The Higgs and the associated V (last instances prior to decay)
        ATH_CHECK(m_decp4_HiggsKeys.initialize(m_detailLevel > 0));
        ATH_CHECK(m_decp4_VKeys.initialize(m_detailLevel > 0));
        // Jets built excluding Higgs decay products
        ATH_CHECK(m_decp4_V_jets25Keys.initialize(m_detailLevel > 1));
        ATH_CHECK(m_decp4_V_jets30Keys.initialize(m_detailLevel > 1));
        // Everybody might not want this ... but good for validation
        ATH_CHECK(m_decp4_Higgs_decayKeys.initialize(m_detailLevel > 2));
        ATH_CHECK(m_decp4_V_decayKeys.initialize(m_detailLevel > 2));

        // Decay-side information
        ATH_CHECK(m_dec_4ldecay_m34.initialize());
        ATH_CHECK(m_dec_4ldecay_phi.initialize());
        ATH_CHECK(m_dec_4ldecay_cutflow_passed.initialize());
        ATH_CHECK(m_dec_4ldecay_m12.initialize(m_detailLevel > 2));
        ATH_CHECK(m_dec_4ldecay_phi1.initialize(m_detailLevel > 2));
        ATH_CHECK(m_dec_4ldecay_costh1.initialize(m_detailLevel > 2));
        ATH_CHECK(m_dec_4ldecay_costh2.initialize(m_detailLevel > 2));
        ATH_CHECK(m_dec_4ldecay_costhstr.initialize(m_detailLevel > 2));
        ATH_CHECK(m_dec_4ldecay_m14.initialize(m_detailLevel > 2));
        ATH_CHECK(m_dec_4ldecay_m23.initialize(m_detailLevel > 2));
        ATH_CHECK(m_dec_4ldecay_m13.initialize(m_detailLevel > 2));
        ATH_CHECK(m_dec_4ldecay_m24.initialize(m_detailLevel > 2));
        
        // Open the TEnv configuration file
        TEnv config{};
        if (config.ReadFile(PathResolverFindCalibFile(m_configPath).c_str(), EEnvLevel(0))) {
            ATH_MSG_FATAL("Failed to open TEnv file " << m_configPath);
            return StatusCode::FAILURE;
        }

        for (const std::string prod_mode : {"GGF", "VBF", "WH", "QQ2ZH", "GG2ZH", "TTH", "BBH", "TH", "THQB", "WHT"}) {
            HTXSSample smp{};
            if (prod_mode == "GGF")
                smp.prod = HTXS::HiggsProdMode::GGF;
            else if (prod_mode == "VBF")
                smp.prod = HTXS::HiggsProdMode::VBF;
            else if (prod_mode == "WH")
                smp.prod = HTXS::HiggsProdMode::WH;
            else if (prod_mode == "QQ2ZH")
                smp.prod = HTXS::HiggsProdMode::QQ2ZH;
            else if (prod_mode == "GG2ZH")
                smp.prod = HTXS::HiggsProdMode::GG2ZH;
            else if (prod_mode == "TTH")
                smp.prod = HTXS::HiggsProdMode::TTH;
            else if (prod_mode == "BBH")
                smp.prod = HTXS::HiggsProdMode::BBH;
            else if (prod_mode == "TH")
                smp.prod = HTXS::HiggsProdMode::TH;
            else if (prod_mode == "THQB") {
                smp.th_type = HTXS::tH_type::THQB;
                smp.prod = HTXS::HiggsProdMode::TH;
            } else if (prod_mode == "WHT") {
                smp.th_type = HTXS::tH_type::TWH;
                smp.prod = HTXS::HiggsProdMode::TH;
            }
            std::vector<std::string> dsid_str = CxxUtils::tokenize(config.GetValue(Form("HTXS.MCsamples.%s", prod_mode.c_str()), ""), " ");

            for (const std::string decay_mode : {"ZZ4l"}) {
                HTXSSample smp_decay = smp;
                if (decay_mode == "ZZ4l") {
                    smp_decay.decay = HTXS::HiggsDecayMode::ZZ4l;
                }
                
                std::vector<std::string> dsid_str_decay = CxxUtils::tokenize(config.GetValue(Form("HTXS.MCsamples.%s", decay_mode.c_str()), ""), " ");
                std::erase_if(dsid_str,
                    [&dsid_str_decay, &smp_decay](std::string &dsid) { 
                        if(std::find(dsid_str_decay.begin(), dsid_str_decay.end(), dsid) == dsid_str_decay.end()) return false;
                        smp_decay.dsids.insert(std::stoi(dsid));
                        return true;
                    }
                );
                if (!smp_decay.dsids.empty()) m_htxs_samples.push_back(std::move(smp_decay));
            }

            for (const std::string& dsid : dsid_str) { smp.dsids.insert(std::atoi(dsid.c_str())); }
            m_htxs_samples.push_back(std::move(smp));
        }
        return StatusCode::SUCCESS;
    }

    // Save a TLV as 4 floats
   StatusCode TruthCategoriesDecorator::decorateFourVec(const EventContext& ctx,
                                                        const FourMomDecorationKeys& keys,
                                                        const xAOD::EventInfo& eventInfo,
                                                        const TLorentzVector& p4) const {
        auto floatEIDecor = [&] (const EvtInfoDecorKey& key) -> float& {
          return SG::makeHandle<float> (key, ctx)(eventInfo);
        };
        floatEIDecor (keys.pt)  = p4.Pt() * Gaudi::Units::GeV;
        floatEIDecor (keys.eta)  = p4.Eta();
        floatEIDecor (keys.phi)  = p4.Phi();
        floatEIDecor (keys.m)  = p4.M() * Gaudi::Units::GeV;
        ATH_MSG_DEBUG("Decorate "<<keys.prefix<<" with pT: "<<p4.Pt()<<" [GeV], eta: "<<p4.Eta()<<", phi: "<<p4.Phi()<<", M: "<<p4.M());
        return StatusCode::SUCCESS;
    }

    // Save a vector of TLVs as vectors of float
    StatusCode TruthCategoriesDecorator::decorateFourVecs(const EventContext& ctx,
                                                          const FourMomDecorationKeys& keys,
                                                          const xAOD::EventInfo& eventInfo,
                                                          const std::vector<TLorentzVector>& p4s) const {
        auto floatVEIDecor = [&] (const EvtInfoDecorKey& key) -> std::vector<float>& {
          return SG::makeHandle<std::vector<float> > (key, ctx)(eventInfo);
        };
        std::vector<float>&pt  = floatVEIDecor (keys.pt);
        std::vector<float>&eta = floatVEIDecor (keys.eta);
        std::vector<float>&phi = floatVEIDecor (keys.phi);
        std::vector<float>&m   = floatVEIDecor (keys.m);
        pt.reserve (p4s.size());
        eta.reserve (p4s.size());
        phi.reserve (p4s.size());
        m.reserve (p4s.size());
        for (const TLorentzVector& p4 : p4s) {
            pt.push_back(p4.Pt() * Gaudi::Units::GeV);
            eta.push_back(p4.Eta());
            phi.push_back(p4.Phi());
            m.push_back(p4.M() * Gaudi::Units::GeV);
            ATH_MSG_DEBUG("Decorate "<<keys.prefix<<" with pT: "<<p4.Pt()<<" [GeV], eta: "<<p4.Eta()<<", phi: "<<p4.Phi()<<", M: "<<p4.M());
        }
        return StatusCode::SUCCESS;
    }


    StatusCode TruthCategoriesDecorator::execute(const EventContext& ctx) const {
        // Retrieve the xAOD event info
        SG::ReadHandle<xAOD::EventInfo> eventInfo{m_evtInfoKey, ctx};
        if (!eventInfo.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve " << m_evtInfoKey.fullKey());
            return StatusCode::FAILURE;
        }

        auto intEIDecor = [&] (const EvtInfoDecorKey& key) -> int& {
          return SG::makeHandle<int> (key, ctx)(*eventInfo);
        };
        auto floatEIDecor = [&] (const EvtInfoDecorKey& key) -> float& {
          return SG::makeHandle<float> (key, ctx)(*eventInfo);
        };

        int mcChannelNumber = eventInfo->mcChannelNumber();
        if (!mcChannelNumber) mcChannelNumber = eventInfo->runNumber();  // EVNT input

        std::vector<HTXSSample>::const_iterator smp_itr =
            std::find_if(m_htxs_samples.begin(), m_htxs_samples.end(),
                         [mcChannelNumber](const HTXSSample& smp) { return smp.dsids.count(mcChannelNumber); });
        if (smp_itr == m_htxs_samples.end()) {
            intEIDecor (m_dec_prodModeKey) = HTXS::HiggsProdMode::UNKNOWN;
            ATH_MSG_DEBUG("The sample " << mcChannelNumber
                                        << " is not a Higgs sample and not of further interest. Decorate the prod mode information only");
            return StatusCode::SUCCESS;
        }

        const HTXS::HiggsProdMode prodMode = smp_itr->prod;
        const HTXS::tH_type th_type = smp_itr->th_type;
        const HTXS::HiggsDecayMode decayMode = smp_itr->decay;

        // Retrieve the xAOD truth
        SG::ReadHandle<xAOD::TruthEventContainer> xTruthEventContainer{m_truthEvtKey, ctx};
        if (!xTruthEventContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve " << m_truthEvtKey.fullKey());
            return StatusCode::FAILURE;
        }
        // convert xAOD -> HepMC
        std::vector<HepMC::GenEvent> hepmc_evts = m_xAODtoHepMCTool->getHepMCEvents(xTruthEventContainer.cptr(), eventInfo.cptr());

        if (hepmc_evts.empty()) {
            // ANGRY MESSAGE HERE
            ATH_MSG_FATAL("The HEP MC GenEvent conversion failed");
            return StatusCode::FAILURE;
        } else {
            ATH_MSG_DEBUG("Found "<<hepmc_evts.size()<<" HepMC events.");
        }

        // classify event according to simplified template cross section
        std::unique_ptr<HTXS::HiggsClassification> htxs{m_higgsTruthCatTool->getHiggsTruthCategoryObject(hepmc_evts[0], prodMode, decayMode)};
        ATH_MSG_DEBUG("Truth categorization done ");
        // Decorate the enums
        intEIDecor (m_dec_prodModeKey) = htxs->prodMode;
        intEIDecor (m_dec_errorCodeKey) = htxs->errorCode;
        intEIDecor (m_dec_stage0CatKey) = htxs->stage0_cat;
        intEIDecor (m_dec_stage0DecayCatKey) = htxs->decaystage0_cat;

        // Stage-1 binning
        intEIDecor (m_dec_stage1CatPt25Key) = htxs->stage1_cat_pTjet25GeV;
        intEIDecor (m_dec_stage1CatPt30Key) = htxs->stage1_cat_pTjet30GeV;
        /// Last argument switches between 25 GeV (true) and 30 GeV jets (false)
        intEIDecor (m_dec_stage1IdxPt25Key) = HTXSstage1_to_HTXSstage1FineIndex(*htxs, th_type, true);
        intEIDecor (m_dec_stage1IdxPt30Key) = HTXSstage1_to_HTXSstage1FineIndex(*htxs, th_type, false);

        // Stage-1.2 binning
        intEIDecor (m_dec_stage1p2_CatPt25Key) = htxs->stage1_2_cat_pTjet25GeV;
        intEIDecor (m_dec_stage1p2_CatPt30Key) = htxs->stage1_2_cat_pTjet30GeV;
        /// Last argument switches between 25 (true) / 30 (false) GeV jets.
        intEIDecor (m_dec_stage1p2_IdxPt25Key) = HTXSstage1_2_to_HTXSstage1_2_FineIndex(*htxs, th_type, true);
        intEIDecor (m_dec_stage1p2_IdxPt30Key) = HTXSstage1_2_to_HTXSstage1_2_FineIndex(*htxs, th_type, false);

        // Stage-1.2 finer binning
        intEIDecor (m_dec_stage1p2_Fine_CatPt25Key) = htxs->stage1_2_fine_cat_pTjet25GeV;
        intEIDecor (m_dec_stage1p2_Fine_CatPt30Key) = htxs->stage1_2_fine_cat_pTjet30GeV;
        intEIDecor (m_dec_stage1p2_Fine_IdxPt25Key) = HTXSstage1_2_Fine_to_HTXSstage1_2_Fine_FineIndex(*htxs, th_type, true);
        intEIDecor (m_dec_stage1p2_Fine_IdxPt30Key) = HTXSstage1_2_Fine_to_HTXSstage1_2_Fine_FineIndex(*htxs, th_type, false);

        // Stage-1.3 binning
        intEIDecor (m_dec_stage1p3_CatPt25Key) = htxs->stage1_3_cat_pTjet25GeV;
        intEIDecor (m_dec_stage1p3_CatPt30Key) = htxs->stage1_3_cat_pTjet30GeV;
        /// Last argument switches between 25 (true) / 30 (false) GeV jets.
        intEIDecor (m_dec_stage1p3_IdxPt25Key) = HTXSstage1_3_to_HTXSstage1_3_FineIndex(*htxs, th_type, true);
        intEIDecor (m_dec_stage1p3_IdxPt30Key) = HTXSstage1_3_to_HTXSstage1_3_FineIndex(*htxs, th_type, false);

        // Stage-1.3 finer binning
        intEIDecor (m_dec_stage1p3_Fine_CatPt25Key) = htxs->stage1_3_fine_cat_pTjet25GeV;
        intEIDecor (m_dec_stage1p3_Fine_CatPt30Key) = htxs->stage1_3_fine_cat_pTjet30GeV;
        intEIDecor (m_dec_stage1p3_Fine_IdxPt25Key) = HTXSstage1_3_Fine_to_HTXSstage1_3_Fine_FineIndex(*htxs, th_type, true);
        intEIDecor (m_dec_stage1p3_Fine_IdxPt30Key) = HTXSstage1_3_Fine_to_HTXSstage1_3_Fine_FineIndex(*htxs, th_type, false);


        intEIDecor (m_dec_NJets25Key) = htxs->jets25.size();
        intEIDecor (m_dec_NJets30Key) = htxs->jets30.size();

        intEIDecor (m_dec_isZnunuKey) = htxs->isZ2vvDecay;
        if(decayMode != HTXS::HiggsDecayMode::UNKNOWNDecay && htxs->decay_observables.size() > 0){
            intEIDecor (m_dec_4ldecay_cutflow_passed) = htxs->decay_cuts_passed;
            floatEIDecor(m_dec_4ldecay_m34) = htxs->decay_observables[1];
            floatEIDecor(m_dec_4ldecay_phi) = htxs->decay_observables[3];
        }

        // At the very least, save the Higgs boson pT
        if (!m_detailLevel) {
            floatEIDecor (m_dec_Higgs_ptKey) = htxs->higgs.Pt() * Gaudi::Units::GeV;
        }
        if (m_detailLevel > 0) {
            // The Higgs and the associated V (last instances prior to decay)
            ATH_CHECK(decorateFourVec(ctx, m_decp4_HiggsKeys, *eventInfo, htxs->higgs));
            ATH_CHECK(decorateFourVec(ctx, m_decp4_VKeys, *eventInfo, htxs->V));
        }

        if (m_detailLevel > 1) {
            // Jets built excluding Higgs decay products
            ATH_CHECK(decorateFourVecs(ctx, m_decp4_V_jets25Keys, *eventInfo, htxs->jets25));
            ATH_CHECK(decorateFourVecs(ctx, m_decp4_V_jets30Keys, *eventInfo, htxs->jets30));
        }

        if (m_detailLevel > 2) {
            // Everybody might not want this ... but good for validation
            ATH_CHECK(decorateFourVec(ctx, m_decp4_Higgs_decayKeys, *eventInfo, htxs->p4decay_higgs));
            ATH_CHECK(decorateFourVec(ctx, m_decp4_V_decayKeys, *eventInfo, htxs->p4decay_V));
            if(decayMode != HTXS::HiggsDecayMode::UNKNOWNDecay && htxs->decay_observables.size() > 0){
                floatEIDecor(m_dec_4ldecay_m12)      = htxs->decay_observables[0];
                floatEIDecor(m_dec_4ldecay_costhstr) = htxs->decay_observables[2];
                floatEIDecor(m_dec_4ldecay_phi1)     = htxs->decay_observables[4];
                floatEIDecor(m_dec_4ldecay_costh1)   = htxs->decay_observables[5];
                floatEIDecor(m_dec_4ldecay_costh2)   = htxs->decay_observables[6];
                floatEIDecor(m_dec_4ldecay_m14)      = htxs->decay_observables[7];
                floatEIDecor(m_dec_4ldecay_m23)      = htxs->decay_observables[8];
                floatEIDecor(m_dec_4ldecay_m13)      = htxs->decay_observables[9];
                floatEIDecor(m_dec_4ldecay_m24)      = htxs->decay_observables[10];
            }
        }
        /// Summary of the HTXS categorization. Used mainly in the testing algorithm
        ATH_MSG_DEBUG("production mode: " << intEIDecor(m_dec_prodModeKey) << ", errorCode: " << intEIDecor (m_dec_errorCodeKey) << ", Stage0: " << intEIDecor (m_dec_stage0CatKey) 
                                         << ", Stage1  -- Jet25 " << intEIDecor (m_dec_stage1CatPt25Key) << " Idx " << intEIDecor (m_dec_stage1IdxPt25Key) 
                                         << ", Jet30: " << intEIDecor (m_dec_stage1CatPt30Key) << " Idx "<< intEIDecor (m_dec_stage1IdxPt30Key) << ", Stage 1.2 -- Jet25: " 
                                         << intEIDecor (m_dec_stage1p2_CatPt25Key) << " Idx: " << intEIDecor (m_dec_stage1p2_IdxPt25Key) << " Jet30: " << intEIDecor (m_dec_stage1p2_CatPt30Key) 
                                         << " Idx: " << intEIDecor (m_dec_stage1p2_IdxPt30Key) << ", HTXS NJets 25" << intEIDecor (m_dec_NJets25Key) 
                                         << " HTXS NJets 30 " << intEIDecor (m_dec_NJets30Key) << " Z->nunu" << intEIDecor (m_dec_isZnunuKey));

        return StatusCode::SUCCESS;
    }

}  // namespace DerivationFramework
