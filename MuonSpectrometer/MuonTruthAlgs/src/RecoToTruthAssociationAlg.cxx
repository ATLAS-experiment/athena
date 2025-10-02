/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RecoToTruthAssociationAlg.h"

#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"
#include "StoreGate/WriteDecorHandle.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "MuonCombinedEvent/TagBase.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "TruthUtils/HepMCHelpers.h"
#include "xAODTruth/TruthVertex.h"
#include "AthContainers/ConstAccessor.h"
using namespace xAOD::P4Helpers;
namespace {
    constexpr unsigned int dummy_unsigned = 999;
    constexpr int com_bit = (1<<xAOD::Muon::Author::Commissioning);
    void increment_unsigned(unsigned& val) {
        if (val == dummy_unsigned)
            val = 1;
        else
            ++val;
    }
    using TruthLink_t = ElementLink<xAOD::TruthParticleContainer>;
    const SG::ConstAccessor<int> acc_origin("truthOrigin");
    const SG::ConstAccessor<int> acc_type("truthType");
    const SG::ConstAccessor<TruthLink_t> acc_link("truthParticleLink");
    //
    const SG::Decorator<int> dec_origin("truthOrigin");
    const SG::Decorator<int> dec_type ("truthType");
    const SG::Decorator<TruthLink_t> dec_link("truthParticleLink");

    static const SG::ConstAccessor<std::vector<unsigned long long>> truthMdtHitsAcc("truthMdtHits");
    static const SG::ConstAccessor<std::vector<unsigned long long>> truthCscHitsAcc("truthCscHits");
    static const SG::ConstAccessor<std::vector<unsigned long long>> truthRpcHitsAcc("truthRpcHits");
    static const SG::ConstAccessor<std::vector<unsigned long long>> truthTgcHitsAcc("truthTgcHits");

}  // namespace

namespace Muon {
using namespace MuonStationIndex;
// Initialize method:
StatusCode RecoToTruthAssociationAlg::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_truthMuKey.initialize());
    ATH_CHECK(m_recoMuKey.initialize());

    if (m_recoLink.empty()){
        m_muonTruthRecoLink = "" ;
    } else {
        m_muonTruthRecoLink = m_recoLink;
    }

    ATH_CHECK(m_muonTruthRecoLink.initialize(!m_recoLink.empty()));
    ATH_CHECK(m_muonTruthParticleLink.initialize());
    ATH_CHECK(m_muonTruthParticleOrigin.initialize());
    ATH_CHECK(m_muonTruthParticleType.initialize());
    ATH_CHECK(m_muonTruthParticleNPrecMatched.initialize());
    ATH_CHECK(m_muonTruthParticleNPhiMatched.initialize());
    ATH_CHECK(m_muonTruthParticleNTrigEtaMatched.initialize());
    for (const std::string& trk_coll : m_assocTrkContainers.value()){
        m_inputDecorKey.emplace_back(trk_coll + ".truthParticleLink");
    }

    m_inputDecorKey.emplace_back(m_truthMuKey, SG::AuxTypeRegistry::instance().getName(acc_origin.auxid()));
    m_inputDecorKey.emplace_back(m_truthMuKey, SG::AuxTypeRegistry::instance().getName(acc_type.auxid()));
    m_inputDecorKey.emplace_back(m_truthMuKey, SG::AuxTypeRegistry::instance().getName(truthMdtHitsAcc.auxid()));

    ATH_CHECK(m_inputDecorKey.initialize());
    return StatusCode::SUCCESS;
}

// Execute method:
StatusCode RecoToTruthAssociationAlg::execute(const EventContext& ctx) const {
    const xAOD::TruthParticleContainer* muonTruthContainer{nullptr};
    ATH_CHECK(SG::get(muonTruthContainer, m_truthMuKey , ctx));
    std::unique_ptr<SG::WriteDecorHandle<xAOD::TruthParticleContainer, ElementLink<xAOD::MuonContainer>>> muonTruthParticleRecoLink{};
    if (!m_muonTruthRecoLink.empty()) {
        muonTruthParticleRecoLink = std::make_unique<SG::WriteDecorHandle<xAOD::TruthParticleContainer, ElementLink<xAOD::MuonContainer>>>(m_muonTruthRecoLink, ctx);
    }

    SG::WriteDecorHandle<xAOD::MuonContainer, ElementLink<xAOD::TruthParticleContainer> > muonTruthParticleLink(m_muonTruthParticleLink,
                                                                                                                ctx);
    if (!muonTruthParticleLink.isValid()) {
        ATH_MSG_WARNING("muon particle container not valid");
        return StatusCode::FAILURE;
    }
    SG::WriteDecorHandle<xAOD::MuonContainer, int> muonTruthParticleType(m_muonTruthParticleType, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, int> muonTruthParticleOrigin(m_muonTruthParticleOrigin, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned int> > muonTruthParticleNPrecMatched(m_muonTruthParticleNPrecMatched,
                                                                                                        ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned int> > muonTruthParticleNPhiMatched(m_muonTruthParticleNPhiMatched, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<unsigned int> > muonTruthParticleNTrigEtaMatched(m_muonTruthParticleNTrigEtaMatched, ctx);

    // add link to reco muons and viceversa
    bool saw_staco = false;
    bool decor_staco = false;

    // loop over muons
    for (const xAOD::Muon* muon : *muonTruthParticleLink) {
        // use primary track particle to get the truth link (except for the case of STACO, where we must use the ID track particle, as the
        // combined is not truth-matched)
        ATH_MSG_DEBUG("muon with pT " << muon->pt() << " MeV, eta: " << muon->eta() << ", phi " << muon->phi() << " and author "
                                      << muon->author());
        const xAOD::TrackParticle* tp = nullptr;
        if (m_associateWithInDetTP || muon->author() == xAOD::Muon::STACO || muon->author() == xAOD::Muon::MuGirl) {
            tp = muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
        } else {
            tp = muon->primaryTrackParticle();
        }

        bool foundTruth{false}, setOrigin{false};
        if (tp) {
            // Associate reco with truth muon. Loop over reconstructed muons, get track particle for each one.
            // Each track particle should carry a link to the corresponding truth particle. Then compare this truth particle link with the
            // given truth muon particle
            if (acc_origin.isAvailable(*tp) && acc_origin(*tp) != 0) {
                muonTruthParticleOrigin(*muon) = acc_origin(*tp);
                muonTruthParticleType(*muon) = acc_type(*tp);
                setOrigin = true;
            }

            ElementLink<xAOD::TruthParticleContainer> truthLink;
            if (acc_link.isAvailable(*tp)) {
                truthLink = acc_link(*tp);
            } else {
                ATH_MSG_DEBUG("Could not find any truth link associated with track having pt:"<<tp->pt()<<" MeV, eta: "<<tp->eta()<<", phi: "<<tp->phi()<<", charge: "<<tp->charge()<<". d0:"<<tp->d0()<<", z0: "<<tp->z0());
            }

            if (truthLink.isValid()) {
                ATH_MSG_VERBOSE(" Got valid truth link for muon author " << muon->author() << " uniqueID " << HepMC::uniqueID(*truthLink));
                // loop over truth particles
                for (const xAOD::TruthParticle* truthParticle : *muonTruthContainer) {
                    if (!MC::isStable(truthParticle)) continue;
                    ATH_MSG_DEBUG("Got truth muon with uniqueID " << HepMC::uniqueID(truthParticle) << " pt " << truthParticle->pt());
                    if ( !HepMC::is_sim_descendant(*truthLink, truthParticle)) {
                        ATH_MSG_VERBOSE("UniqueID truth link: " << HepMC::uniqueID(*truthLink)
                                                               << " is not decendant of " << HepMC::uniqueID(truthParticle));
                        continue;
                    }
                    ATH_MSG_VERBOSE("Truth muon uniqueID matches -> creating link with truth particle " << HepMC::uniqueID(*truthLink));
                    foundTruth = true;
                    /// Link the truth particle to the muon
                    ElementLink<xAOD::TruthParticleContainer> muonTruthLink{*muonTruthContainer,
                                                                            truthParticle->index(),
                                                                            ctx};
                    muonTruthLink.toPersistent();
                    muonTruthParticleLink(*muon) = muonTruthLink;
                    if (!setOrigin) {
                        muonTruthParticleOrigin(*muon) = acc_origin(*tp);
                        muonTruthParticleType(*muon) = acc_type(*tp);
                        setOrigin = true;
                    }
                    /// Check first if the truth link already exists
                    if (muonTruthParticleRecoLink && muonTruthParticleRecoLink->operator()(*truthParticle).isValid()){
                        const xAOD::Muon* decor_muon  = *muonTruthParticleRecoLink->operator()(*truthParticle);
                        ATH_MSG_VERBOSE("Truth particle is already decorated with reco muon "<<decor_muon->pt()*1.e-3
                                        <<" eta: "<<decor_muon->eta()<<" phi: "<<decor_muon->phi()<<" charge: "<<
                                        decor_muon->charge()<<" author: "<<decor_muon->author()<<" all authors: "<<
                                        decor_muon->allAuthors());

                        // Check first if the exiting muon has a better author
                        if (MuonCombined::authorRank(decor_muon->author()) <  MuonCombined::authorRank(muon->author())){
                            ATH_MSG_DEBUG("Author of the decorated muon is better than the one of the new candidate");
                            continue;
                        }
                        /// May be both muons are reconstructed by the same author but one is commissioning
                        const int com_score = (muon->allAuthors() & com_bit) - (decor_muon->allAuthors() &com_bit);
                        if (com_score > 0){
                            ATH_MSG_DEBUG("Found two muons reconstructed by an equivalent author. But this one is from the commissioning chain");
                            continue;
                        }
                        /// The last judge is a simple dR cut but this will hopefully never trigger
                        if (deltaR2(muon,truthParticle) >= deltaR2(muon, decor_muon)) continue;
                    }


                    ElementLink<xAOD::MuonContainer> muonLink{muon, *muonTruthParticleLink, ctx};

                    /// Zero supression do not want to store meaningless zeros
                    std::vector<unsigned int> nprecHitsPerChamberLayer(toInt(ChIndex::ChIndexMax), dummy_unsigned);
                    std::vector<unsigned int> nphiHitsPerChamberLayer(toInt(PhiIndex::PhiIndexMax), dummy_unsigned);
                    std::vector<unsigned int> ntrigEtaHitsPerChamberLayer(toInt(PhiIndex::PhiIndexMax), dummy_unsigned);

                    constexpr int author_sel = (1<<xAOD::Muon::MuidCo) | (1<<xAOD::Muon::MuidSA) | (1<<xAOD::Muon::MuGirl);
                    count_chamber_layers(muon->allAuthors() & author_sel
                                             ? truthParticle
                                             : nullptr,
                                         tp->track(), nprecHitsPerChamberLayer, nphiHitsPerChamberLayer, ntrigEtaHitsPerChamberLayer);
                    /// Decorate the results
                    muonTruthParticleNPrecMatched(*muon) =  nprecHitsPerChamberLayer;
                    muonTruthParticleNPhiMatched(*muon) = nphiHitsPerChamberLayer;
                    muonTruthParticleNTrigEtaMatched(*muon) = ntrigEtaHitsPerChamberLayer;

                    if (muonTruthParticleRecoLink) (*muonTruthParticleRecoLink)(*truthParticle) = muonLink;
                    break;
                }
            } else {
                ATH_MSG_DEBUG("Invalid truth link");
            }
        } else {
            ATH_MSG_WARNING("Could not find the appropiate track particle for muon with pT: " << muon->pt() * 1.e-3 << " GeV,  eta: "
                                                                                              << muon->eta() << ", phi: " << muon->phi()
                                                                                              << " author: " << muon->author());
        }

        if (!setOrigin) {
            muonTruthParticleOrigin(*muon) = 0;
            muonTruthParticleType(*muon) = 0;
        }
        if (!foundTruth) {
            muonTruthParticleLink(*muon) = ElementLink<xAOD::TruthParticleContainer>();
            // add these empty vectors
            muonTruthParticleNPrecMatched(*muon) = std::vector<unsigned int>{};
            muonTruthParticleNPhiMatched(*muon) = std::vector<unsigned int>{};
            muonTruthParticleNTrigEtaMatched(*muon) = std::vector<unsigned int>{};
        }
        /// Patch for STACO muons: Copy the truth information from the muon back to the combined
        /// track to avoid file corruptions reported in ATLASRECTS-6454
        if (muon->author() == xAOD::Muon::STACO) {
            const xAOD::TrackParticle* cmb_trk = muon->trackParticle(xAOD::Muon::CombinedTrackParticle);
            if (!cmb_trk){
                ATH_MSG_WARNING("Even a STACO muon should have a combined track");
                continue;
            } else {
              if (!saw_staco) {
                saw_staco = true;
                decor_staco = !dec_origin.isAvailable (*cmb_trk);
              }
              if (decor_staco) {
                dec_origin(*cmb_trk) = acc_origin(*muon);
                dec_type(*cmb_trk) = acc_type(*muon);
                dec_link(*cmb_trk) = acc_link(*muon);
              }
            }
        }
    }
    /// one more thing: need to have muonlink set for all truth particles to avoid ELReset errors
    if (muonTruthParticleRecoLink && !muonTruthParticleRecoLink->isAvailable()) {
        for (const xAOD::TruthParticle* truthParticle : **muonTruthParticleRecoLink) {
            ATH_MSG_DEBUG("no reco muon link set, add an empty one");
            (*muonTruthParticleRecoLink)(*truthParticle) = ElementLink<xAOD::MuonContainer>();
         }
    }

    return StatusCode::SUCCESS;
}

void RecoToTruthAssociationAlg::count_chamber_layers(const xAOD::IParticle* truthParticle, const Trk::Track* ptrk,
                                                   std::vector<unsigned int>& nprecHitsPerChamberLayer,
                                                   std::vector<unsigned int>& nphiHitsPerChamberLayer,
                                                   std::vector<unsigned int>& ntrigEtaHitsPerChamberLayer) const {

    if (!truthParticle || !truthMdtHitsAcc.isAvailable(*truthParticle)) {
        ATH_MSG_DEBUG("muon has no truth hits vector in the truth association alg");
        nprecHitsPerChamberLayer.clear();
        nphiHitsPerChamberLayer.clear();
        ntrigEtaHitsPerChamberLayer.clear();
        return;
    }
    const std::vector<unsigned long long>& mdtTruth = truthMdtHitsAcc(*truthParticle);
    std::vector<unsigned long long> cscTruth;

    if (m_idHelperSvc->hasCSC()) cscTruth = truthCscHitsAcc(*truthParticle);
    const std::vector<unsigned long long>& rpcTruth = truthRpcHitsAcc(*truthParticle);
    const std::vector<unsigned long long>& tgcTruth = truthTgcHitsAcc(*truthParticle);

    for (const Trk::TrackStateOnSurface* tsit : *ptrk->trackStateOnSurfaces()) {
        if (!tsit || !tsit->trackParameters() || !tsit->measurementOnTrack()) continue;
        const Trk::MeasurementBase* meas = tsit->measurementOnTrack();
        Identifier id;
        const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas);
        if (rot)
            id = rot->identify();
        else {
            const Muon::CompetingMuonClustersOnTrack* crot = dynamic_cast<const Muon::CompetingMuonClustersOnTrack*>(meas);
            if (crot && !crot->containedROTs().empty() && crot->containedROTs().front()) id = crot->containedROTs().front()->identify();
        }
        if (!m_idHelperSvc->isMuon(id)) continue;

        bool measPhi = m_idHelperSvc->measuresPhi(id);
        bool isTgc = m_idHelperSvc->isTgc(id);
        ChIndex chIndex = !isTgc ? m_idHelperSvc->chamberIndex(id) : ChIndex::ChUnknown;
        if (m_idHelperSvc->isMdt(id)) {
            for (unsigned int i = 0; i < mdtTruth.size(); ++i) {
                if (id == mdtTruth[i]) {
                    if (chIndex != ChIndex::ChUnknown) {
                      increment_unsigned(nprecHitsPerChamberLayer.at(toInt(chIndex)));
                    }
                    break;
                }
            }
        } else if (m_idHelperSvc->hasCSC() && m_idHelperSvc->isCsc(id)) {
            for (unsigned int i = 0; i < cscTruth.size(); ++i) {
                if (id != cscTruth[i]) continue;
                if (measPhi) {
                    PhiIndex index = m_idHelperSvc->phiIndex(id);
                    if (index != PhiIndex::PhiUnknown) {
                      increment_unsigned(nphiHitsPerChamberLayer.at(toInt(index)));
                    }
                } else {
                  if (chIndex != ChIndex::ChUnknown) {
                    increment_unsigned(nprecHitsPerChamberLayer.at(toInt(chIndex)));
                  }
                }
                break;
            }
        } else if (m_idHelperSvc->isRpc(id)) {
            for (unsigned int i = 0; i < rpcTruth.size(); ++i) {
                if (id != rpcTruth[i]) { continue; }
                PhiIndex index = m_idHelperSvc->phiIndex(id);
                if (index != PhiIndex::PhiUnknown) {
                  if (measPhi) {
                    increment_unsigned(nphiHitsPerChamberLayer.at(toInt(index)));
                  } else {
                    increment_unsigned(ntrigEtaHitsPerChamberLayer.at(toInt(index)));
                  }
                }
                break;
            }
        } else if (m_idHelperSvc->isTgc(id)) {
            for (unsigned int i = 0; i < tgcTruth.size(); ++i) {
                if (id != tgcTruth[i]) { continue; }
                PhiIndex index = m_idHelperSvc->phiIndex(id);
                if (index != PhiIndex::PhiUnknown) {
                  if (measPhi) {
                    increment_unsigned(nphiHitsPerChamberLayer.at(toInt(index)));
                  } else {
                    increment_unsigned(ntrigEtaHitsPerChamberLayer.at(toInt(index)));
                  }
                }
                break;
            }
        }
    }  // end loop over TSOS
    ATH_MSG_DEBUG("finished loop over TSOS");

    // now, have to check if there are non-zero truth hits in indices without reco hits
    clear_dummys(mdtTruth, nprecHitsPerChamberLayer);
    clear_dummys(cscTruth, nprecHitsPerChamberLayer);

    clear_dummys(cscTruth, nphiHitsPerChamberLayer);
    clear_dummys(rpcTruth, nphiHitsPerChamberLayer);
    clear_dummys(tgcTruth, nphiHitsPerChamberLayer);

    clear_dummys(rpcTruth, ntrigEtaHitsPerChamberLayer);
    clear_dummys(tgcTruth, ntrigEtaHitsPerChamberLayer);
}
void RecoToTruthAssociationAlg::clear_dummys(const std::vector<unsigned long long>& identifiers, std::vector<unsigned int>& vec) const {
    /// If the identifiers are empty then there
    /// is no change that a dummy value could be cleared from this list
    if (identifiers.empty()) { return; }
    for (unsigned int i = 0; i < vec.size(); ++i) {
        if (vec[i] != dummy_unsigned) continue;
        for (unsigned j = 0; j < identifiers.size(); ++j) {
            const Identifier id{identifiers[j]};
            if (m_idHelperSvc->measuresPhi(id)) {
                const auto phiIdx = static_cast<PhiIndex>(i);
                if (m_idHelperSvc->phiIndex(id) == phiIdx) {
                    vec[i] = 0;
                    break;
                }
            } else {
                const auto chIdx = static_cast<ChIndex>(i);
                if (m_idHelperSvc->chamberIndex(id) == chIdx) {
                    vec[i] = 0;
                    break;
                }
            }
        }
    }
}
}
