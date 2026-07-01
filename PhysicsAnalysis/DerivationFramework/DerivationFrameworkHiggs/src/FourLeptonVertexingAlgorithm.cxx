/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "DerivationFrameworkHiggs/FourLeptonVertexingAlgorithm.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/VertexAuxContainer.h"

namespace {
    using LeptonQuadruplet = DerivationFramework::FourLeptonVertexingAlgorithm::LeptonQuadruplet;
    using TrkLink = ElementLink<xAOD::TrackParticleContainer>;
    constexpr double MeVtoGeV = 1. / Gaudi::Units::GeV;   
} // anonymous namespace

std::ostream& operator<<(std::ostream& sstr, const xAOD::IParticle* part) {
  if (part->type() == xAOD::Type::ObjectType::Muon) {
    const xAOD::Muon* mu = static_cast<const xAOD::Muon*>(part);
    sstr << " Muon pT: " << mu->pt() * MeVtoGeV << " [GeV], eta: " << mu->eta() << ", phi: " << mu->phi() << ", q: " << mu->charge()
         << ", priamry Author: " << mu->author();
  } else if (part->type() == xAOD::Type::ObjectType::Electron) {
    const xAOD::Electron* elec = static_cast<const xAOD::Electron*>(part);
    sstr << " Electron pT: " << elec->pt() * MeVtoGeV << " [GeV], eta: " << elec->eta() << ", phi: " << elec->phi()
         << ", q: " << elec->charge();
  } else if (part->type() == xAOD::Type::ObjectType::TrackParticle) {
    const xAOD::TrackParticle* trk = static_cast<const xAOD::TrackParticle*>(part);
    sstr << " Track pT: " << trk->pt() * MeVtoGeV << " [GeV], eta: " << trk->eta() << ", phi: " << trk->phi()
         << ", q: " << trk->charge() << ", d0: " << trk->d0() << ", z0: " << trk->z0();
  }
  sstr << " index: " << part->index();
  return sstr;
}

std::ostream& operator<<(std::ostream& sstr, const LeptonQuadruplet& quad) {
  sstr << std::endl;
  for (const xAOD::IParticle* lep : quad) { sstr << " **** " << lep << std::endl; }
  return sstr;
}

namespace DerivationFramework {

    FourLeptonVertexingAlgorithm::FourLeptonVertexingAlgorithm(const std::string& n, ISvcLocator* p) : AthReentrantAlgorithm(n, p) {}

    StatusCode FourLeptonVertexingAlgorithm::initialize() {
        ATH_CHECK(m_muonKey.initialize());
        ATH_CHECK(m_elecKey.initialize());
        ATH_CHECK(m_evtKey.initialize());
        ATH_CHECK(m_vtxKey.initialize());
        ATH_CHECK(m_fitter.retrieve());

        ATH_CHECK(m_elecSelTool.retrieve(EnableTool{!m_elecSelTool.empty()}));
        ATH_CHECK(m_muonSelTool.retrieve(EnableTool{!m_muonSelTool.empty()}));

        if (m_muonTrkProp < static_cast<int>(MuonTrk::Primary) || 
            m_muonTrkProp > static_cast<int>(MuonTrk::MSOnlyExtrapolatedMuonSpectrometerTrackParticle)) {
            ATH_MSG_FATAL("A bogous muon track particle has been picked " << m_muonTrkProp);
            return StatusCode::FAILURE;
        }
        m_muonTrk = static_cast<MuonTrk>(m_muonTrkProp.value());
        return StatusCode::SUCCESS;
    }
    StatusCode FourLeptonVertexingAlgorithm::finalize() {
        unsigned int n_processed = std::accumulate(m_num_lep.begin(), m_num_lep.end(), 0, [](const std::atomic<unsigned int>& l, unsigned int n){
            return n +l;
        });        
        ATH_MSG_INFO("Proccessed in total "<<n_processed<<" events. Lepton multiplicities observed:");
        for (unsigned int l = 0 ; l < m_num_lep.size(); ++l) {
            ATH_MSG_INFO("    ---- Events with "<<l<<" leptons: "<<m_num_lep[l]<<" (elec/muon) "<<m_num_ele[l]<<"/"<<m_num_muo[l]<<" ("<<(100.*m_num_lep[l] / std::max(1u,n_processed))<<"%)" );            
        }
        const unsigned int n_trials = m_tried_fits;
        ATH_MSG_INFO("---> Attempted fits "<<m_tried_fits<<" out of which "<<m_good_fits<<" ("<<(100.* m_good_fits / std::max(1u, n_trials) )<<"%) succeeded." );
        return StatusCode::SUCCESS;
    }
    StatusCode FourLeptonVertexingAlgorithm::execute(const EventContext& ctx) const {
        /// Setup the output container
        SG::WriteHandle<xAOD::VertexContainer> vtxContainer{m_vtxKey, ctx};
        ATH_CHECK(vtxContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()));
        xAOD::VertexContainer* out_container = vtxContainer.ptr();

        std::vector<LeptonQuadruplet> all_quads = buildAllQuadruplets(ctx);
        for (const LeptonQuadruplet& quad : all_quads) {
            std::unique_ptr<xAOD::Vertex> candidate = fitQuadruplet(ctx, quad);
            if (candidate) out_container->push_back(std::move(candidate));
        }
        return StatusCode::SUCCESS;
    }

    bool FourLeptonVertexingAlgorithm::passSelection(const xAOD::Electron* elec) const {
        return elec->pt() >= m_minElecPt && (m_elecSelTool.empty() || m_elecSelTool->accept(elec)) &&
               !xAOD::EgammaHelpers::getTrackParticlesVec(elec, !m_elecUseGSF).empty();
    }
    bool FourLeptonVertexingAlgorithm::passSelection(const xAOD::Muon* muon) const {
        return muon->pt() >= m_minMuonPt && (m_muonSelTool.empty() || m_muonSelTool->accept(*muon)) && muon->trackParticle(m_muonTrk);
    }
    std::vector<LeptonQuadruplet> FourLeptonVertexingAlgorithm::buildAllQuadruplets(const EventContext& ctx) const {
        std::vector<LeptonQuadruplet> to_ret{};

        /// Retrieve the container from the store gate
        SG::ReadHandle<xAOD::MuonContainer> muonContainer{m_muonKey, ctx};
        if (!muonContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the muon container for input " << m_muonKey.fullKey());
            throw std::runtime_error("Invalid key access");
        }
        SG::ReadHandle<xAOD::ElectronContainer> elecContainer{m_elecKey, ctx};
        if (!elecContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the electron container for input " << m_elecKey.fullKey());
            throw std::runtime_error("Invalid key access");
        }
        /// Dump the leptons in a common vector to build all possible quadruplets
        using PartVec = std::vector<const xAOD::IParticle*>;
        PartVec selected_lep{};
        const size_t num_lep = elecContainer->size() + muonContainer->size();
        if (num_lep < 4) {
            ATH_MSG_DEBUG("Less than four objects reconstructed");
            ++m_num_lep[num_lep];
            ++m_num_ele[elecContainer->size()];
            ++m_num_muo[muonContainer->size()];            
            return to_ret;
        }
        selected_lep.reserve(num_lep);
        size_t n_ele{0},n_muo{0};
        for (const xAOD::Muon* muon : *muonContainer) {
            if (passSelection(muon)) {
                ATH_MSG_DEBUG("Add " << muon);
                selected_lep.push_back(muon);
                ++n_ele;
            }
        }
        for (const xAOD::Electron* elec : *elecContainer) {
            if (passSelection(elec)) {
                ATH_MSG_DEBUG("Add " << elec);
                selected_lep.push_back(elec);
                ++n_muo;
            }
        }
        /// Increment the lepton counter
        ++m_num_lep[std::min(m_num_lep.size() -1 , n_ele + n_muo)];
        ++m_num_ele[std::min(m_num_ele.size() -1 , n_ele)];
        ++m_num_muo[std::min(m_num_muo.size() -1 , n_muo)];
        
        if (selected_lep.size() < 4) {
            ATH_MSG_DEBUG("Less than four leptons survived.");
            return to_ret;
        }
        std::sort(selected_lep.begin(),selected_lep.end(),
                        [](const xAOD::IParticle* a, const xAOD::IParticle* b) {                            
                            return a->pt() > b->pt();});
        /// None of the leptons has enough momentum to be selected in the offline analysis
        if (selected_lep[0]->pt() < m_LeadPtCut || selected_lep[1]->pt() < m_SubLeadPtCut) return to_ret;

        to_ret.reserve(std::pow(selected_lep.size(), 4));
        for (PartVec::const_iterator itr1 = selected_lep.begin() + 3; itr1 != selected_lep.end(); ++itr1) {
            for (PartVec::const_iterator itr2 = selected_lep.begin() + 2; itr2 != itr1; ++itr2) {
                for (PartVec::const_iterator itr3 = selected_lep.begin() + 1; itr3 != itr2; ++itr3) {
                    if ( (*itr3)->pt() < m_SubLeadPtCut) return to_ret;
                    for (PartVec::const_iterator itr4 = selected_lep.begin(); itr4 != itr3; ++itr4) {
                        LeptonQuadruplet quad{*itr4, *itr3, *itr2, *itr1};
                        /// The leading lepton has too little momentum. All subsequent combinations will 
                        ///  have even less. Bail out of the loop                        
                        if (quad[0]->pt() < m_LeadPtCut) return to_ret;
                        ATH_MSG_DEBUG("Create new quaduplet " << quad);
                        to_ret.push_back(std::move(quad));
                    }
                }
            }
        }
        return to_ret;
    }
    const xAOD::TrackParticle* FourLeptonVertexingAlgorithm::trackParticle(const xAOD::IParticle* part) const {
        if (part->type() == xAOD::Type::Muon)
            return static_cast<const xAOD::Muon*>(part)->trackParticle(m_muonTrk);
        else if (part->type() == xAOD::Type::Electron)
            return xAOD::EgammaHelpers::getTrackParticlesVec(static_cast<const xAOD::Electron*>(part), !m_elecUseGSF)[0];
        return dynamic_cast<const xAOD::TrackParticle*>(part);
    }
    int FourLeptonVertexingAlgorithm::charge(const xAOD::IParticle* part) const{
        static const SG::AuxElement::ConstAccessor<float> acc_charge{"charge"};
        if (acc_charge.isAvailable(*part)) return acc_charge(*part);
        return trackParticle(part)->charge();
    }
    std::unique_ptr<xAOD::Vertex> FourLeptonVertexingAlgorithm::fitQuadruplet(const EventContext& ctx, const LeptonQuadruplet& quad) const {
        if (!passSelection(quad)) return nullptr;
        ++m_tried_fits;
        std::vector<const xAOD::TrackParticle*> trks{};
        trks.reserve(4);
        for (const xAOD::IParticle* lep : quad) { trks.push_back(trackParticle(lep)); }
        /// Use the beam spot as an initial constraint
        SG::ReadHandle<xAOD::EventInfo> evtInfo{m_evtKey, ctx};
        if (!evtInfo.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the event info " << m_evtKey.fullKey());
            throw std::runtime_error("Invalid key access");
        }

        const Amg::Vector3D beam_spot{evtInfo->beamPosX(), evtInfo->beamPosY(), evtInfo->beamPosZ()};
        std::unique_ptr<xAOD::Vertex> common_vtx = m_fitter->fit(ctx, trks, beam_spot);
        if (!common_vtx) {
            ATH_MSG_DEBUG("Fit from  " << quad << " failed.");
            return nullptr;
        }
        const float redChi2 = (common_vtx->chiSquared() / common_vtx->numberDoF());
        ATH_MSG_DEBUG("Fit from " << quad << " gave a vertex with position at " << common_vtx->position() << " with chi2 "
                                  << redChi2 << " nDoF: " << common_vtx->numberDoF());
        if (redChi2 > m_VtxChi2Cut) {
            ATH_MSG_DEBUG("Reject due to bad chi2");
            return nullptr;
        }
        static const std::vector<float> empty_vec{};
        if (m_pruneWeight) common_vtx->setTrackWeights(empty_vec);
        if (m_pruneCov) common_vtx->setCovariance(empty_vec);
        std::vector<TrkLink> track_links{};
        for (const xAOD::TrackParticle* trk : trks) {
            const xAOD::TrackParticleContainer* cont = dynamic_cast<const xAOD::TrackParticleContainer*>(trk->container());
            TrkLink link{*cont, trk->index(), ctx};
            link.toPersistent();
            track_links.push_back(std::move(link));
        }
        common_vtx->setTrackParticleLinks(track_links);
        ++m_good_fits;
        return common_vtx;
    }
    bool FourLeptonVertexingAlgorithm::passSelection(const LeptonQuadruplet& quad) const {
        float max_m{-1.};
        for (unsigned int i = 1; i < quad.size(); ++i) {
            for (unsigned int j = 0; j < i; ++j) {
                const xAOD::TrackParticle* trk_a = trackParticle(quad[i]);
                const xAOD::TrackParticle* trk_b = trackParticle(quad[j]);                
                
                if (std::abs(trk_b->z0() - trk_a->z0()) > m_z0Cut) return false;
                
                /// Check the di lepton masses in the quad
                const float di_m1 = (quad[i]->p4() + quad[j]->p4()).M();
                const bool isSFOS1 = charge(quad[i])*charge(quad[j]) < 0 && quad[i]->type() == quad[j]->type();
                /// If the pair is SFOS check the mass
                if (isSFOS1 && di_m1 < m_lowSFOS_Cut) return false;
                max_m = std::max(di_m1, max_m);                
                for (unsigned int k = 1 ; k < quad.size(); ++k) {
                    if (k == i || k == j) continue;
                    for (unsigned int l = 0; l < k ; ++l) {
                        if (l == i || l == j) continue;
                        const float di_m2 =  (quad[k]->p4() + quad[l]->p4()).M();
                        const bool isSFOS2 = charge(quad[k])*charge(quad[l]) < 0 && quad[k]->type() == quad[l]->type();
                        if (isSFOS2 && di_m2 < m_lowSFOS_Cut) return false;
                        max_m = std::max(max_m, di_m2);
                    }
                }          
            }
        }      
        /// The high mass pair must at least satisfy a certain threshold
        if (m_highPair_Cut > max_m) return false;

        return true;
    }
}  // namespace DerivationFramework
