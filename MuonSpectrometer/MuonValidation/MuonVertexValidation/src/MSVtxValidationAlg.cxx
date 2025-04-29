/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MSVtxValidationAlg.h"
#include "Utils.h"


using namespace MSVtxValidationAlgUtils;    

namespace{
    constexpr float  default_f = -99999.;
}


StatusCode MSVtxValidationAlg::initialize() {
    ATH_MSG_DEBUG ("Initializing " << name() << "...");

    ATH_CHECK(m_evtKey.initialize());
    ATH_CHECK(m_TruthParticleKey.initialize());
    ATH_CHECK(m_TrackParticleKey.initialize(m_computeIso));
    ATH_CHECK(m_JetKey.initialize(m_readJets || m_computeIso));
    ATH_CHECK(m_MetKey.initialize(m_readMET));
    ATH_CHECK(m_TrackletKey.initialize());
    ATH_CHECK(m_MSVtxKey.initialize());

    // attach branches to the tree
    m_tree.addBranch(std::make_unique<MuonVal::EventInfoBranch>(m_tree, 0));

    // portal
    m_portal = std::make_shared<MuonVal::IParticleFourMomBranch>(m_tree, "portal");
    m_portal->addVariableGeV<float>(default_f, "m");
    m_tree.addBranch(m_portal);

    // LLP
    m_llp = std::make_shared<MuonVal::IParticleFourMomBranch>(m_tree, "llp");
    m_llp->addVariableGeV<float>(default_f, "m");
    m_tree.addBranch(m_llp);

    // truth displaced vertex
    m_llpVtx = std::make_shared<MuonVal::ThreeVectorBranch>(m_tree, "llpVtx_");
    m_tree.addBranch(m_llpVtx);

    // reconstructed MS vertex
    m_msVtx = std::make_shared<MuonVal::ThreeVectorBranch>(m_tree, "msVtx_");
    m_tree.addBranch(m_msVtx);
    if (!m_computeIso){
        m_tree.disableBranch("msVtx_isoTracks_mindR");
        m_tree.disableBranch("msVtx_isoTracks_pTsum");
        m_tree.disableBranch("msVtx_isoJets_mindR");
    }

    // tracklets
    m_trklet_pos = std::make_shared<MuonVal::ThreeVectorBranch>(m_tree, "trklet_pos");
    m_trklet_mom = std::make_shared<MuonVal::ThreeVectorBranch>(m_tree, "trklet_mom");
    m_tree.addBranch(m_trklet_pos);
    m_tree.addBranch(m_trklet_mom);

    // muon segments: dumps the entire muon segment container without needing an explicit fill call
    m_muonSeg = std::make_shared<MuonPRDTest::SegmentVariables>(m_tree, m_MuonSegKey, "muonSeg", msgLevel());
    m_tree.addBranch(m_muonSeg);

    // jets
    if (m_readJets){
        m_jet = std::make_shared<MuonVal::IParticleFourMomBranch>(m_tree, "jet");
        m_jet->addVariableGeV<float>(default_f, "m");
        m_tree.addBranch(m_jet);
    }
    else m_tree.disableBranch("jet_N");

    // met
    if (!m_readMET){
        m_tree.disableBranch("met");
        m_tree.disableBranch("met_x");
        m_tree.disableBranch("met_y");
        m_tree.disableBranch("met_phi");
    }

    
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


StatusCode MSVtxValidationAlg::fillTruth(const EventContext& ctx){
    // Fill truth particle branches and histograms
    SG::ReadHandle<xAOD::TruthParticleContainer> truth_particles{m_TruthParticleKey, ctx};
    ATH_CHECK(truth_particles.isPresent());

    // collect portal, LLPs
    std::vector<const xAOD::TruthParticle*> portals{}, llps{};
    for(const xAOD::TruthParticle* tp : *truth_particles){
        if (!tp) continue;
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
            m_llpVtx->push_back(decVtx->v4().Vect());
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
        if (llp->hasDecayVtx() && llp->nChildren()>0 && !llp->isGenStable()){
            std::vector<const xAOD::TruthParticle*> genStableChildren = getGenStableChildren(llp);
            m_llp_Nkids.push_back((int)genStableChildren.size());
            for (const xAOD::TruthParticle *kid : genStableChildren){
                m_llpKid_pdgid.push_back(kid->pdgId());
                m_llpKid_llpLink.push_back(llp_idx);
            }
        }
        ++llp_idx;
    }
    m_llpVtx_N = num_vtx;

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillJet(const EventContext& ctx){
    if (!m_readJets) return StatusCode::SUCCESS;

    SG::ReadHandle Jets{m_JetKey, ctx};
    ATH_CHECK(Jets.isPresent());

    m_jet_N = Jets->size();
    for (const xAOD::Jet* jet : *Jets) m_jet->push_back(jet);

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillMet(const EventContext& ctx){
    if (!m_readMET) return StatusCode::SUCCESS;

    SG::ReadHandle MET{m_MetKey, ctx};
    ATH_CHECK(MET.isPresent());

    m_met = (*MET)["Final"]->met()/Gaudi::Units::GeV;
    m_met_x = (*MET)["Final"]->mpx()/Gaudi::Units::GeV;
    m_met_y = (*MET)["Final"]->mpy()/Gaudi::Units::GeV;
    m_met_phi = (*MET)["Final"]->phi();
    m_sumEt = (*MET)["Final"]->sumet()/Gaudi::Units::GeV;

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillTracklets(const EventContext& ctx){
    
    SG::ReadHandle MSOnlyTracklets{m_TrackletKey , ctx};
    ATH_CHECK(MSOnlyTracklets.isPresent());

    m_trklet_N = MSOnlyTracklets->size();
    for(const xAOD::TrackParticle* mstrklet : *MSOnlyTracklets){
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
        m_trklet_pos->push_back(trklet_pos);
        m_trklet_mom->push_back(trklet_mom);
        // set default link index to vertex to -1 and adjust when in fillMSVtx
        m_trklet_vtxLink.push_back(-1);
    }

    return StatusCode::SUCCESS;
}


void MSVtxValidationAlg::fillHits(const xAOD::Vertex* vtx, const std::string& decorator_str, MuonVal::VectorBranch<int>& branch) {
    // fills branch with the number of hits close to the vertex. When the decorator is not available, the default integer value is used
    const SG::AuxElement::Accessor<int> hits_acc(decorator_str);
    if (hits_acc.isAvailable(*vtx)) branch.push_back(hits_acc(*vtx));

    return;
}


StatusCode MSVtxValidationAlg::fillMSVtx(const EventContext& ctx){
    // Fill MS vertex branches
    SG::ReadHandle MSVertices{m_MSVtxKey, ctx};
    ATH_CHECK(MSVertices.isPresent());

    // fill MSVtx branches and histograms when the read handle is present
    m_msVtx_N = MSVertices->size();
    for(const xAOD::Vertex* msVtx : *MSVertices){
        m_msVtx->push_back(msVtx->position());
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
    }

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::fillMSVtxIso(const EventContext& ctx){
    // Fill the isolation variables for the MS vertices
    if (!m_computeIso) return StatusCode::SUCCESS;

    SG::ReadHandle MSVertices{m_MSVtxKey, ctx};
    ATH_CHECK(MSVertices.isPresent());
    SG::ReadHandle Tracks{m_TrackParticleKey, ctx};
    ATH_CHECK(Tracks.isPresent());
    SG::ReadHandle Jets{m_JetKey, ctx};
    ATH_CHECK(Jets.isPresent());

    for(const xAOD::Vertex* msVtx : *MSVertices){
        VtxIso iso = getIso(msVtx, *Tracks, *Jets, m_trackIso_pT, m_softTrackIso_R, m_jetIso_pT, m_jetIso_LogRatio);
        m_msVtx_isoTracks_mindR.push_back(iso.track_mindR);
        m_msVtx_isoTracks_pTsum.push_back(iso.track_pTsum);
        m_msVtx_isoJets_mindR.push_back(iso.jet_mindR);
    }

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::execute() {  
    ATH_MSG_DEBUG ("Executing " << name() << "...");

    const EventContext& ctx = Gaudi::Hive::currentContext();

    // event variables
    SG::ReadHandle eventInfo{m_evtKey, ctx};
    ATH_CHECK(eventInfo.isPresent());
    ATH_MSG_DEBUG("Start to run over event "<<eventInfo->eventNumber()<<" in run" <<eventInfo->runNumber());

    ATH_CHECK(fillTruth(ctx));
    ATH_CHECK(fillJet(ctx));
    ATH_CHECK(fillMet(ctx));
    ATH_CHECK(fillTracklets(ctx));
    ATH_CHECK(fillMSVtx(ctx));
    ATH_CHECK(fillMSVtxIso(ctx));

    ATH_CHECK(m_tree.fill(ctx));

    return StatusCode::SUCCESS;
}


StatusCode MSVtxValidationAlg::finalize() {
    ATH_MSG_DEBUG ("Finalizing " << name() << "...");
    ATH_CHECK(m_tree.write());

    return StatusCode::SUCCESS;
}
