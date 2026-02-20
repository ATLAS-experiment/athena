/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigDecisionTool/ChainGroup.h"

#include "MSVtxValidationAlg.h"
#include "Utils.h"


using namespace MSVtxValidationAlgUtils;    


StatusCode MSVtxValidationAlg::initialize() {
    ATH_MSG_DEBUG ("Initializing " << name() << "...");

    ATH_CHECK(m_evtKey.initialize());
    ATH_CHECK(m_TruthParticleKey.initialize(m_isMC));
    ATH_CHECK(m_TruthVertexKey.initialize(m_isMC && m_dumpTruthVertices));
    ATH_CHECK(m_TrackParticleKey.initialize(m_computeVertexIso));
    ATH_CHECK(m_JetKey.initialize(m_fillJets || m_computeVertexIso || m_computeActiveVertices));
    ATH_CHECK(m_MetKey.initialize(m_fillMET));
    ATH_CHECK(m_MuonKey.initialize());
    ATH_CHECK(m_MuonSegKey.initialize());
    ATH_CHECK(m_TrackletKey.initialize());
    ATH_CHECK(m_MSVtxKey.initialize());
    if (m_readTriggers) {
        m_trigDec.setTypeAndName("Trig::TrigDecisionTool/TrigDecisionTool");
        m_matchingTool.setTypeAndName("Trig::R3MatchingTool/R3MatchingTool");
        ATH_CHECK(m_trigDec.retrieve());
        ATH_CHECK(m_matchingTool.retrieve());
    }

    // attach branches to the tree
    m_tree.addBranch(std::make_unique<MuonVal::EventInfoBranch>(m_tree, 0));

    // truth TrackParticles
    m_truthParticle = std::make_shared<MuonVal::IParticleFourMomBranch>(m_tree, "truthParticle");
    m_truthParticle->addVariableGeV<float>(defaults::Float, "m");
    m_truthParticle->addVariable<int>(defaults::Int, "pdgId");
    m_truthParticle->addVariable<int>(defaults::Int, "status");
    m_truthParticle->addVariable<float>(defaults::Float, "pX");
    m_truthParticle->addVariable<float>(defaults::Float, "pY");
    m_truthParticle->addVariable<float>(defaults::Float, "pZ");
    m_tree.addBranch(m_truthParticle);
    
    // portal
    m_portal = std::make_shared<MuonVal::IParticleFourMomBranch>(m_tree, "portal");
    m_portal->addVariableGeV<float>(defaults::Float, "m");
    m_tree.addBranch(m_portal);

    // LLP
    m_llp = std::make_shared<MuonVal::IParticleFourMomBranch>(m_tree, "llp");
    m_llp->addVariableGeV<float>(defaults::Float, "m");
    m_tree.addBranch(m_llp);

    // muon segments: dumps the entire muon segment container without needing an explicit fill call
    m_muonSeg = std::make_shared<MuonPRDTest::SegmentVariables>(m_tree, m_MuonSegKey.key(), "muonSeg", msgLevel());
    m_tree.addBranch(m_muonSeg);

    // muons
    m_muon = std::make_shared<MuonVal::IParticleFourMomBranch>(m_tree, "muon");
    m_tree.addBranch(m_muon);

    // jets
    m_jet = std::make_shared<MuonVal::IParticleFourMomBranch>(m_tree, "jet");
    m_jet->addVariableGeV<float>(defaults::Float, "m");
    m_tree.addBranch(m_jet);
    
    ATH_CHECK(m_tree.init(this));

    // --- //
    // Book output histograms following the THistSvc recommendation on managing ownership. Register via histSvc()->regHist("/<stream>/histName", rawHistPtr); //
    // --- //

    // llp pair
    auto h_LLP1LLP2dR = std::make_unique<TH1F>("h_LLP1LLP2dR","h_LLP1LLP2dR; #Delta R(LLP1, LLP2); Count / bin",50,0,4); 
    m_h_LLP1LLP2dR = h_LLP1LLP2dR.get();
    ATH_CHECK(histSvc()->regHist("/MSVtxValidation/h_LLP1LLP2dR", std::move(h_LLP1LLP2dR))); 

    auto h_diLLPMass = std::make_unique<TH1F>("h_diLLPMass","h_diLLPMass; m_{LL1, LLP2} [GeV]; Count / bin",50,0,1000);
    m_h_diLLPMass = h_diLLPMass.get();
    ATH_CHECK(histSvc()->regHist("/MSVtxValidation/h_diLLPMass", std::move(h_diLLPMass)));

    // leading LLP
    auto h_leadLLPLxy = std::make_unique<TH1F>("h_leadLLPLxy","h_leadLLPLxy; lead LLP L_{xy} [mm]; Count / bin",50,0,10000); 
    m_h_leadLLPLxy = h_leadLLPLxy.get();
    ATH_CHECK(histSvc()->regHist("/MSVtxValidation/h_leadLLPLxy", std::move(h_leadLLPLxy)));

    auto h_leadLLPLz = std::make_unique<TH1F>("h_leadLLPLz","h_leadLLPLz; lead LLP L_{z} [mm]; Count / bin",50,0,14000); 
    m_h_leadLLPLz = h_leadLLPLz.get();
    ATH_CHECK(histSvc()->regHist("/MSVtxValidation/h_leadLLPLz", std::move(h_leadLLPLz)));

    auto h_leadLLPctau = std::make_unique<TH1F>("h_leadLLPctau","h_leadLLPctau; lead LLP c#tau [mm]; Count / bin",50,0,2000); 
    m_h_leadLLPctau = h_leadLLPctau.get();
    ATH_CHECK(histSvc()->regHist("/MSVtxValidation/h_leadLLPctau", std::move(h_leadLLPctau)));

    auto h_leadLLPpt = std::make_unique<TH1F>("h_leadLLPpt","h_leadLLPpt; lead LLP p_{T} [GeV]; Count / bin",50,0,400); 
    m_h_leadLLPpt = h_leadLLPpt.get();
    ATH_CHECK(histSvc()->regHist("/MSVtxValidation/h_leadLLPpt", std::move(h_leadLLPpt)));

    // subleading LLP
    auto h_subleadLLPLxy = std::make_unique<TH1F>("h_subleadLLPLxy","h_subleadLLPLxy; sublead LLP L_{xy} [mm]; Count / bin",50,0,10000); 
    m_h_subleadLLPLxy = h_subleadLLPLxy.get();
    ATH_CHECK(histSvc()->regHist("/MSVtxValidation/h_subleadLLPLxy", std::move(h_subleadLLPLxy)));

    auto h_subleadLLPLz = std::make_unique<TH1F>("h_subleadLLPLz","h_subleadLLPLz; sublead LLP L_{z} [mm]; Count / bin",50,0,10000); 
    m_h_subleadLLPLz = h_subleadLLPLz.get();
    ATH_CHECK(histSvc()->regHist("/MSVtxValidation/h_subleadLLPLz", std::move(h_subleadLLPLz)));

    auto h_subleadLLPctau = std::make_unique<TH1F>("h_subleadLLPctau","h_subleadLLPctau; sublead LLP c#tau [mm]; Count / bin",50,0,2000); 
    m_h_subleadLLPctau = h_subleadLLPctau.get();
    ATH_CHECK(histSvc()->regHist("/MSVtxValidation/h_subleadLLPctau", std::move(h_subleadLLPctau)));

    auto h_subleadLLPpt = std::make_unique<TH1F>("h_subleadLLPpt","h_subleadLLPpt; sublead LLP p_{T} [GeV]; Count / bin",50,0,400); 
    m_h_subleadLLPpt = h_subleadLLPpt.get();
    ATH_CHECK(histSvc()->regHist("/MSVtxValidation/h_subleadLLPpt", std::move(h_subleadLLPpt)));

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillTruth(const EventContext& ctx) {
    ATH_CHECK(fillTruthParticle(ctx)); // need to be run before fillTruthVertex to set up the truth particle - truth vertex linking
    if(m_dumpTruthVertices) ATH_CHECK(fillTruthVertex(ctx));

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillTruthParticle(const EventContext& ctx) {
    // Fill truth particle branches and histograms
    const xAOD::TruthParticleContainer* truth_particles{nullptr};
    ATH_CHECK(SG::get(truth_particles, m_TruthParticleKey, ctx));

    // dump the TruthParticle information and collect portal, LLPs
    std::vector<const xAOD::TruthParticle*> portals{}, llps{};
    for(const xAOD::TruthParticle* tp : *truth_particles){
        if (!tp) continue;
        if (m_dumpTruthParticles) {
            m_truthParticle->push_back(tp);
            // set default link index to -1
            m_truthParticle_llpVtx_link_tmp->push_back(-1); // adjusted in fillLLP
            m_truthParticle_actVtx_link_tmp->push_back(-1); // adjusted in fillJet
            m_truthParticle_truthVtx_link_tmp->push_back(-1); // adjusted in fillTruthVertex
        }

        // fill vector of portal particles, skipping the particle if it is a self-decay
        if(std::abs(tp->pdgId()) == m_pdgId_portal){
          bool selfdecay = false;
          for (size_t p = 0; p < tp->production_vertex()->nIncomingParticles(); ++p) if (tp->parent(p)->pdgId() == tp->pdgId()) { selfdecay = true; break;}
          if (!selfdecay) portals.push_back(tp);
        }

        // fill vector of LLPs
        if(std::abs(tp->pdgId()) == m_pdgId_llp) llps.push_back(tp);
    }

    // portal
    m_portal_N = portals.size();
    for(const xAOD::TruthParticle* portal : portals) m_portal->push_back(portal);

    // LLP and LLP Children
    ATH_CHECK(fillLLP(llps)); 
 
    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillLLP(std::vector<const xAOD::TruthParticle*>& llps) {

    // LLP and LLP children
    m_llp_N = llps.size();
    std::sort(llps.begin(), llps.end(), comparePt); // sort llps by pT
    const xAOD::TruthParticle* leadLLP = nullptr;
    const xAOD::TruthParticle* subleadLLP = nullptr;
    
    int num_vtx = 0;
    for(unsigned int llp_idx=0; const xAOD::TruthParticle* llp : llps){
        m_llp->push_back(llp);

        if(llp->hasDecayVtx()){
            const xAOD::TruthVertex* decVtx = llp->decayVtx();
            m_llpVtx_pos.push_back(decVtx->v4().Vect());
            m_llpVtx_Lxy.push_back(decVtx->perp());
            m_llpVtx_ctau.push_back(getCTau(decVtx));
            ++num_vtx;
        }

        // leading LLP
        if(llp_idx==0){
            if(llp->hasDecayVtx()){
                const xAOD::TruthVertex* decVtx = llp->decayVtx();
                m_h_leadLLPLxy->Fill(decVtx->perp());
                m_h_leadLLPLz->Fill(decVtx->z());
                m_h_leadLLPctau->Fill(getCTau(decVtx));
            }
            m_h_leadLLPpt->Fill(llp->pt()/Gaudi::Units::GeV);
            leadLLP = llp;
        }
        // subleading llp
        if(llp_idx==1){
            if(llp->hasDecayVtx()){
                const xAOD::TruthVertex* decVtx = llp->decayVtx();
                m_h_subleadLLPLxy->Fill(decVtx->perp());
                m_h_subleadLLPLz->Fill(decVtx->z());
                m_h_subleadLLPctau->Fill(getCTau(decVtx));
            }
            m_h_subleadLLPpt->Fill(llp->pt()/Gaudi::Units::GeV);
            subleadLLP = llp;

            // di-llp histograms
            TLorentzVector llpSum = subleadLLP->p4()+leadLLP->p4();
            m_h_LLP1LLP2dR->Fill(leadLLP->p4().DeltaR(subleadLLP->p4()));
            m_h_diLLPMass->Fill(llpSum.E()/Gaudi::Units::GeV);
        }

        // LLP children
        if (llp->hasDecayVtx() && llp->nChildren()>0 && !llp->isGenStable()) {
            std::vector<const xAOD::TruthParticle*> stableChildren = getStableChildren(llp, m_llp_genStableChildren);
            m_llp_Nchildren.push_back((int)stableChildren.size());
            int nChargedChildren = 0;
            for (const xAOD::TruthParticle *child : stableChildren){
                if (child->charge() != 0) ++nChargedChildren;
                if (!m_dumpTruthParticles) continue;
                m_llpVtx_truthParticle_link.push_back(llp_idx, child->index());
                m_truthParticle_llpVtx_link_tmp->at(child->index()) = llp_idx;
            }
            m_llp_NchildrenCharged.push_back(nChargedChildren);
        }
        ++llp_idx;
    }
    m_llpVtx_N = num_vtx;

    // copy linking data from temp vector and clear the temp vector for the next event
    if (m_dumpTruthParticles) {
        for (int link : *m_truthParticle_llpVtx_link_tmp) m_truthParticle_llpVtx_link.push_back(link);
        m_truthParticle_llpVtx_link_tmp->clear();
    }


    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillTruthVertex(const EventContext& ctx) {
    const xAOD::TruthVertexContainer* truth_vertices{nullptr};
    ATH_CHECK(SG::get(truth_vertices, m_TruthVertexKey, ctx));

    int truthVtxIdx = 0;
    for (const xAOD::TruthVertex* vtx : *truth_vertices){
        if (!vtx) continue;
        m_truthVtx_pos.push_back(vtx->v4().Vect());
        m_truthVtx_status.push_back(vtx->status());

        // set the interaction type based on the status code
        int interactionType = -1;
        if (m_hadronicProcessEnum.count(vtx->status() - 1000)) interactionType = 0;
        else if (m_emProcessEnum.count(vtx->status() - 1000)) interactionType = 1;
        m_truthVtx_interactionType.push_back(interactionType);

        // children
        m_truthVtx_NchildrenDirect.push_back(vtx->nOutgoingParticles());
        int nChildren = 0;
        int nChildrenCharged = 0;
        for(const xAOD::TruthParticle* child : vtx->particles_out()){
            std::vector<const xAOD::TruthParticle*> stableChildren = getStableChildren(child, false);
            nChildren += stableChildren.size();
            for (const xAOD::TruthParticle *stableChild : stableChildren){
                if (stableChild->charge() != 0) ++nChildrenCharged;
                if (!m_dumpTruthParticles) continue;
                m_truthVtx_truthParticle_link.push_back(truthVtxIdx, stableChild->index());
                m_truthParticle_truthVtx_link_tmp->at(stableChild->index()) = truthVtxIdx;
            }
        }
        m_truthVtx_Nchildren.push_back(nChildren);
        m_truthVtx_NchildrenCharged.push_back(nChildrenCharged);
        truthVtxIdx++;
    }

    // copy linking data from temp vector and clear the temp vector for the next event
    if (m_dumpTruthParticles) {
        for (int link : *m_truthParticle_truthVtx_link_tmp) m_truthParticle_truthVtx_link.push_back(link);
        m_truthParticle_truthVtx_link_tmp->clear();
    }

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillJet(const EventContext& ctx) {
    const xAOD::JetContainer* jets{nullptr};
    ATH_CHECK(SG::get(jets, m_JetKey, ctx));

    TriggerInfo triggerInfo;
    if (m_readTriggers) triggerInfo = getTriggerDecisions(m_triggerStringJets);

    m_jet_N = jets->size();
    for (const xAOD::Jet* jet : *jets) {
        m_jet->push_back(jet);
        if (m_readTriggers) m_jet_triggers.push_back(getTriggerMatchingDecision(jet, triggerInfo, m_jetTriggerMatchingDR));
        if (m_computeActiveVertices && m_isMC) ATH_CHECK(fillActiveVertices(ctx, jet));
    }

    // copy linking data from temp vector and clear the temp vector for the next event
    if (m_dumpTruthParticles) {
        for (int link : *m_truthParticle_actVtx_link_tmp) m_truthParticle_actVtx_link.push_back(link);
        m_truthParticle_actVtx_link_tmp->clear();
    }

    return StatusCode::SUCCESS;
}


MSVtxValidationAlg::TriggerInfo MSVtxValidationAlg::getTriggerDecisions(std::string triggerString) {
    // for the configured trigger chain, store the trigger decision for this event
    const Trig::ChainGroup* chain = m_trigDec->getChainGroup(triggerString);
    std::vector<std::string> triggerNames = chain->getListOfTriggers();
    
    std::vector<bool> triggerPassed;
    for (const std::string& triggerName : triggerNames) {
        if (m_trigDec->isPassed(triggerName)) triggerPassed.push_back(true);
        else triggerPassed.push_back(false);
    }

    return {triggerString, triggerNames, triggerPassed};
}


template <typename T>
bool MSVtxValidationAlg::getTriggerMatchingDecision(const T* object, TriggerInfo& triggerInfo, float dRmatching) {

    bool firedTrigger = false;
    for (size_t i = 0; i < triggerInfo.passed.size(); ++i) {
        if (!triggerInfo.passed[i]) continue; // skip if trigger not passed
        if (m_matchingTool->match(*object, triggerInfo.names[i], dRmatching, false)) {
            firedTrigger = true;
            break; // this object has fired one for the triggers in the chain so do not need to check the other triggers
        }
    }

    return firedTrigger;
}


StatusCode MSVtxValidationAlg::fillActiveVertices(const EventContext& ctx, const xAOD::Jet* jet) {
    const xAOD::TruthParticleContainer* truth_particles{nullptr};
    ATH_CHECK(SG::get(truth_particles, m_TruthParticleKey, ctx));

    std::vector<ActiveVertex> actVertices = getActiveVertices(jet, *truth_particles);

    if (!actVertices.size()){
        m_jet_actVtx_link.push_back({}); // empty links for this jet as no active vertex was identified 
        return StatusCode::SUCCESS;  
    } 

    for (ActiveVertex actVtx : actVertices) {
        if (!actVtx.vtx) continue;
        // index of active vertex found in the event. Not just a simple counter since each jet can have several active vertices
        size_t actVtxIdx = m_actVtx_NChildren.size();
        // fill the active vertex branches
        m_actVtx_pos.push_back(actVtx.vtx->v4().Vect());
        m_actVtx_NChildren.push_back(actVtx.nChildren);
        m_actVtx_chainDepth.push_back(actVtx.decayDepth);
        m_actVtx_energy.push_back(actVtx.vtxEnergy/Gaudi::Units::GeV);
        m_actVtx_mass.push_back(actVtx.vtxMass/Gaudi::Units::GeV);
        m_actVtx_pT.push_back(actVtx.vtxPt/Gaudi::Units::GeV);
        m_actVtx_scalarPtSum.push_back(actVtx.vtxScalarPtSum/Gaudi::Units::GeV);

        m_actVtx_jet_link.push_back(jet->index()); // link the active vertex to the jet
        m_actVtx_jet_dEta.push_back(jet->eta() - actVtx.vtx->v4().Eta());
        m_actVtx_jet_dPhi.push_back(jet->p4().DeltaPhi(actVtx.vtx->v4()));

        m_jet_actVtx_link.push_back(jet->index(), actVtxIdx); // link the jet to the active vertex

        // note that a truth particle can be linked to multiple active vertices, so the link index is not unique
        if (!m_dumpTruthParticles) continue;
        for (auto tpLink : actVtx.vtx->outgoingParticleLinks()) {
            if (!tpLink) continue;
            m_actVtx_truthParticle_link.push_back(actVtxIdx, tpLink.index());
            m_truthParticle_actVtx_link_tmp->at(tpLink.index()) = actVtxIdx;
        }
    }
    m_jet_NactVtx.push_back(actVertices.size()); // number of active vertices associated to this jet

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillMet(const EventContext& ctx) {
    const xAOD::MissingETContainer* MET{nullptr};
    ATH_CHECK(SG::get(MET, m_MetKey, ctx));

    m_met = (*MET)["Final"]->met()/Gaudi::Units::GeV;
    m_met_x = (*MET)["Final"]->mpx()/Gaudi::Units::GeV;
    m_met_y = (*MET)["Final"]->mpy()/Gaudi::Units::GeV;
    m_met_phi = (*MET)["Final"]->phi();
    m_sumEt = (*MET)["Final"]->sumet()/Gaudi::Units::GeV;

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillMuons(const EventContext& ctx) {
    const xAOD::MuonContainer* muons{nullptr};
    ATH_CHECK(SG::get(muons, m_MuonKey, ctx));

    for (const xAOD::Muon* muon : *muons) m_muon->push_back(muon); 

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillTracklets(const EventContext& ctx) {
    
    const xAOD::TrackParticleContainer* msOnlyTracklets{nullptr};
    ATH_CHECK(SG::get(msOnlyTracklets, m_TrackletKey, ctx));

    m_trklet_N = msOnlyTracklets->size();
    for(const xAOD::TrackParticle* mstrklet : *msOnlyTracklets){
        // perigee parameters
        m_trklet_d0.push_back(mstrklet->d0());
        m_trklet_z0.push_back(mstrklet->z0());
        m_trklet_phi.push_back(mstrklet->phi());
        m_trklet_theta.push_back(mstrklet->theta());
        m_trklet_eta.push_back(mstrklet->eta());
        m_trklet_qOverP.push_back(mstrklet->qOverP());
        m_trklet_q.push_back(mstrklet->charge());
        // cartesian parameters
        const Trk::Perigee &tkl_perigee = mstrklet->perigeeParameters();
        const Amg::Vector3D &trklet_pos = tkl_perigee.position();
        const Amg::Vector3D &trklet_mom = tkl_perigee.momentum()/Gaudi::Units::GeV;
        m_trklet_pos.push_back(trklet_pos);
        m_trklet_mom.push_back(trklet_mom);
        // set default link index to vertex to -1 and adjust when in fillMSVtx
        m_trklet_vtxLink.push_back(-1);
    }

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillMSVtx(const EventContext& ctx) {
    const xAOD::VertexContainer* msVertices{nullptr};
    ATH_CHECK(SG::get(msVertices, m_MSVtxKey, ctx));

    const xAOD::TrackParticleContainer* tracks{nullptr};
    const xAOD::JetContainer* jets{nullptr};
    if (m_computeVertexIso) {
        ATH_CHECK(SG::get(tracks, m_TrackParticleKey, ctx));
        ATH_CHECK(SG::get(jets, m_JetKey, ctx));
    }

    m_msVtx_N = msVertices->size();
    for(const xAOD::Vertex* msVtx : *msVertices){
        m_msVtx_pos.push_back(msVtx->position());
        m_msVtx_chi2.push_back(msVtx->chiSquared());
        m_msVtx_nDoF.push_back(msVtx->numberDoF());

        // hits close to vertex: total, inwards of the vertex, inner layer, extended layer, middle layer, outer layer
        fillHits(msVtx, "nMDT", m_msVtx_nMDT);
        fillHits(msVtx, "nMDT_inwards", m_msVtx_nMDT_inwards);
        fillHits(msVtx, "nMDT_I", m_msVtx_nMDT_I);
        fillHits(msVtx, "nMDT_E", m_msVtx_nMDT_E);
        fillHits(msVtx, "nMDT_M", m_msVtx_nMDT_M);
        fillHits(msVtx, "nMDT_O", m_msVtx_nMDT_O);

        fillHits(msVtx, "nRPC", m_msVtx_nRPC);
        fillHits(msVtx, "nRPC_inwards", m_msVtx_nRPC_inwards);
        fillHits(msVtx, "nRPC_I", m_msVtx_nRPC_I);
        fillHits(msVtx, "nRPC_E", m_msVtx_nRPC_E);
        fillHits(msVtx, "nRPC_M", m_msVtx_nRPC_M);
        fillHits(msVtx, "nRPC_O", m_msVtx_nRPC_O);

        fillHits(msVtx, "nTGC", m_msVtx_nTGC);
        fillHits(msVtx, "nTGC_inwards", m_msVtx_nTGC_inwards);
        fillHits(msVtx, "nTGC_I", m_msVtx_nTGC_I);
        fillHits(msVtx, "nTGC_E", m_msVtx_nTGC_E);
        fillHits(msVtx, "nTGC_M", m_msVtx_nTGC_M);
        fillHits(msVtx, "nTGC_O", m_msVtx_nTGC_O);

        // set a linking index for each tracklet used in the reconstruction of the vertex 
        size_t nTrk = msVtx->nTrackParticles();
        m_msVtx_Ntrklet.push_back(nTrk);

        for(size_t j=0; j<nTrk; ++j){ 
            const xAOD::TrackParticle *consti = msVtx->trackParticle(j);
            if (!consti) continue;
            m_trklet_vtxLink[consti->index()] = msVtx->index();
        }

        // optionally compute the vertex isolation from tracks and jets
        if (m_computeVertexIso) ATH_CHECK(fillMSVtxIsolation(msVtx, tracks, jets));
    }

    return StatusCode::SUCCESS;
}


void MSVtxValidationAlg::fillHits(const xAOD::Vertex* vtx, const std::string& decorator_str, MuonVal::VectorBranch<int>& branch) {
    // fills branch with the number of hits close to the vertex. When the decorator is not available, the default integer value is used
    const SG::AuxElement::Accessor<int> hits_acc(decorator_str);
    if (hits_acc.isAvailable(*vtx)) branch.push_back(hits_acc(*vtx));

    return;
}


StatusCode MSVtxValidationAlg::fillMSVtxIsolation(const xAOD::Vertex* msVtx, const xAOD::TrackParticleContainer* tracks, const xAOD::JetContainer* jets) {

    VtxIso iso = getIso(msVtx, *tracks, *jets, m_trackIso_pT, m_softTrackIso_R, m_jetIso_pT, m_jetIso_LogRatio);
    m_msVtx_isoTracks_mindR.push_back(iso.track_mindR);
    m_msVtx_isoTracks_pTsum.push_back(iso.track_pTsum);
    m_msVtx_isoJets_mindR.push_back(iso.jet_mindR);

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::execute() {  
    ATH_MSG_DEBUG ("Executing " << name() << "...");

    const EventContext& ctx = Gaudi::Hive::currentContext();

    // event variables
    const xAOD::EventInfo* eventInfo{nullptr};
    ATH_CHECK(SG::get(eventInfo, m_evtKey, ctx));

    ATH_MSG_DEBUG("Start to run over event "<<eventInfo->eventNumber()<<" in run" <<eventInfo->runNumber());

    if(m_isMC) ATH_CHECK(fillTruth(ctx));
    if(m_fillJets) ATH_CHECK(fillJet(ctx));
    if(m_fillMET) ATH_CHECK(fillMet(ctx));
    ATH_CHECK(fillMuons(ctx));
    ATH_CHECK(fillTracklets(ctx));
    ATH_CHECK(fillMSVtx(ctx));

    ATH_CHECK(m_tree.fill(ctx));

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::finalize() {
    ATH_MSG_DEBUG ("Finalizing " << name() << "...");
    ATH_CHECK(m_tree.write());

    return StatusCode::SUCCESS;
}
