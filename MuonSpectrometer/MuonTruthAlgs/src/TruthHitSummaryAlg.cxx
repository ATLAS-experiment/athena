/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthHitSummaryAlg.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/TrackingVolume.h"

namespace Muon {    
    // Initialize method:
    StatusCode TruthHitSummaryAlg::initialize() {
        ATH_CHECK(m_muonTruth.initialize());
        ATH_CHECK(m_PRD_TruthNames.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());

        ATH_CHECK(m_nprecLayersKey.initialize());
        ATH_CHECK(m_nphiLayersKey.initialize());
        ATH_CHECK(m_ntrigEtaLayersKey.initialize());
        ATH_CHECK(m_innerSmallHitsKey.initialize());
        ATH_CHECK(m_innerLargeHitsKey.initialize());
        ATH_CHECK(m_middleSmallHitsKey.initialize());
        ATH_CHECK(m_middleLargeHitsKey.initialize());
        ATH_CHECK(m_outerSmallHitsKey.initialize());
        ATH_CHECK(m_outerLargeHitsKey.initialize());
        ATH_CHECK(m_extendedSmallHitsKey.initialize());
        ATH_CHECK(m_extendedLargeHitsKey.initialize());
        ATH_CHECK(m_phiLayer1HitsKey.initialize());
        ATH_CHECK(m_phiLayer2HitsKey.initialize());
        ATH_CHECK(m_phiLayer3HitsKey.initialize());
        ATH_CHECK(m_phiLayer4HitsKey.initialize());
        ATH_CHECK(m_etaLayer1HitsKey.initialize());
        ATH_CHECK(m_etaLayer2HitsKey.initialize());
        ATH_CHECK(m_etaLayer3HitsKey.initialize());
        ATH_CHECK(m_etaLayer4HitsKey.initialize());        
        ATH_CHECK(m_truthMdtHitsKey.initialize(m_idHelperSvc->hasMDT()));        
        ATH_CHECK(m_truthTgcHitsKey.initialize(m_idHelperSvc->hasTGC()));        
        ATH_CHECK(m_truthRpcHitsKey.initialize(m_idHelperSvc->hasRPC()));        
        ATH_CHECK(m_truthCscHitsKey.initialize(m_idHelperSvc->hasCSC()));        
        ATH_CHECK(m_truthStgcHitsKey.initialize(m_idHelperSvc->hasSTGC()));        
        ATH_CHECK(m_truthMMHitsKey.initialize(m_idHelperSvc->hasMM()));        

        return StatusCode::SUCCESS;
    }

    // Execute method:
    StatusCode TruthHitSummaryAlg::execute(const EventContext& ctx) const {
        // skip if no input data found
        SG::ReadHandle muonTruthContainer(m_muonTruth, ctx);
        ATH_CHECK(muonTruthContainer.isPresent());

        summaryDecors myDecors{this, ctx};
        
        // loop over truth coll (muon only)
        for (const xAOD::TruthParticle* truthParticle : *muonTruthContainer) {
            ChamberIdMap ids{};
            ATH_CHECK(addHitCounts(ctx, *truthParticle, ids, myDecors));
            ATH_CHECK(addHitIDVectors(*truthParticle, ids, myDecors));
        }
        return StatusCode::SUCCESS;
    }

    StatusCode TruthHitSummaryAlg::addHitCounts(const EventContext& ctx,
                                                const xAOD::TruthParticle& truthParticle,
                                                ChamberIdMap& ids, summaryDecors& myDecors) const {
        
        std::vector<unsigned int> nprecHitsPerChamberLayer;
        nprecHitsPerChamberLayer.resize(Muon::MuonStationIndex::ChIndexMax);
        std::vector<unsigned int> nphiHitsPerChamberLayer;
        nphiHitsPerChamberLayer.resize(Muon::MuonStationIndex::PhiIndexMax);
        std::vector<unsigned int> ntrigEtaHitsPerChamberLayer;
        ntrigEtaHitsPerChamberLayer.resize(Muon::MuonStationIndex::PhiIndexMax);
        
        ATH_MSG_DEBUG("addHitCounts: barcode " << HepMC::barcode(truthParticle)); // FIXME barcode-based
        auto truthParticleHistory = HepMC::simulation_history(&truthParticle, -1); // Returns a list of uniqueIDs (currently for xAOD::TruthParticle these would be barcodes)
        // loop over detector technologies
        for (SG::ReadHandle<PRD_MultiTruthCollection>& col : m_PRD_TruthNames.makeHandles(ctx)) {
            ATH_CHECK(col.isPresent());
            
            // loop over trajectories
            for (const std::pair<Identifier, HepMcParticleLink> trajectory : *col) {
                 // check if gen particle same as input
                if (std::ranges::find(truthParticleHistory, HepMC::barcode(trajectory.second)) == truthParticleHistory.end()) {
                    continue; // FIXME barcode-based - TrackRecords read in from existing inputs will not have valid id values.
                }
                const Identifier& id = trajectory.first;
                bool measPhi = m_idHelperSvc->measuresPhi(id);
                bool isTgc = m_idHelperSvc->isTgc(id);
                Muon::MuonStationIndex::ChIndex chIndex = !isTgc ? m_idHelperSvc->chamberIndex(id) : Muon::MuonStationIndex::ChUnknown;
                // add identifier to map
                if (isTgc) {  // TGCS should be added to both EIL and EIS
                    Muon::MuonStationIndex::PhiIndex index = m_idHelperSvc->phiIndex(id);
                    if (index == Muon::MuonStationIndex::T4) {
                        ids[Muon::MuonStationIndex::EIS].push_back(id);
                        ids[Muon::MuonStationIndex::EIL].push_back(id);
                    } else {
                        ids[Muon::MuonStationIndex::EMS].push_back(id);
                        ids[Muon::MuonStationIndex::EML].push_back(id);
                    }
                } else {
                    ids[chIndex].push_back(id);
                }

                if (m_idHelperSvc->issTgc(id)) {
                    if (measPhi) {
                        int index = m_idHelperSvc->phiIndex(id);
                        ++nphiHitsPerChamberLayer.at(index);
                    }  else {
                        ++nprecHitsPerChamberLayer.at(chIndex);
                    }
                } else if (m_idHelperSvc->isMM(id)) {
                    ++nprecHitsPerChamberLayer.at(chIndex);
                } else if (m_idHelperSvc->isTrigger(id)) {
                    int index = m_idHelperSvc->phiIndex(id);
                    if (index >= 0) {
                        if (measPhi)
                            ++nphiHitsPerChamberLayer.at(index);
                        else
                            ++ntrigEtaHitsPerChamberLayer.at(index);
                    }
                } else {
                    if (measPhi) {
                        Muon::MuonStationIndex::PhiIndex index = m_idHelperSvc->phiIndex(id);
                        ++nphiHitsPerChamberLayer.at(index);
                    } else {
                        ++nprecHitsPerChamberLayer.at(chIndex);
                    }
                }
            }
        }

        uint8_t innerSmallHits = nprecHitsPerChamberLayer[Muon::MuonStationIndex::BIS] +
                                nprecHitsPerChamberLayer[Muon::MuonStationIndex::EIS] +
                                nprecHitsPerChamberLayer[Muon::MuonStationIndex::CSS];

        uint8_t innerLargeHits = nprecHitsPerChamberLayer[Muon::MuonStationIndex::BIL] +
                                nprecHitsPerChamberLayer[Muon::MuonStationIndex::EIL] +
                                nprecHitsPerChamberLayer[Muon::MuonStationIndex::CSL];

        uint8_t middleSmallHits = nprecHitsPerChamberLayer[Muon::MuonStationIndex::BMS] +
                                nprecHitsPerChamberLayer[Muon::MuonStationIndex::EMS];

        uint8_t middleLargeHits = nprecHitsPerChamberLayer[Muon::MuonStationIndex::BML] +
                                nprecHitsPerChamberLayer[Muon::MuonStationIndex::EML];

        uint8_t outerSmallHits = nprecHitsPerChamberLayer[Muon::MuonStationIndex::BOS] +
                                nprecHitsPerChamberLayer[Muon::MuonStationIndex::EOS];

        uint8_t outerLargeHits = nprecHitsPerChamberLayer[Muon::MuonStationIndex::BML] +
                                nprecHitsPerChamberLayer[Muon::MuonStationIndex::EOL];

        uint8_t extendedSmallHits = nprecHitsPerChamberLayer[Muon::MuonStationIndex::EES] +
                                    nprecHitsPerChamberLayer[Muon::MuonStationIndex::BEE];

        uint8_t extendedLargeHits = nprecHitsPerChamberLayer[Muon::MuonStationIndex::EEL];

        uint8_t phiLayer1Hits = nphiHitsPerChamberLayer[Muon::MuonStationIndex::BM1] +
                                nphiHitsPerChamberLayer[Muon::MuonStationIndex::T4] +
                                nphiHitsPerChamberLayer[Muon::MuonStationIndex::CSC] +
                                nphiHitsPerChamberLayer[Muon::MuonStationIndex::STGC1] +
                                nphiHitsPerChamberLayer[Muon::MuonStationIndex::STGC2];

        uint8_t phiLayer2Hits = nphiHitsPerChamberLayer[Muon::MuonStationIndex::BM2] +
                                nphiHitsPerChamberLayer[Muon::MuonStationIndex::T1];

        uint8_t phiLayer3Hits = nphiHitsPerChamberLayer[Muon::MuonStationIndex::BO1] +
                                nphiHitsPerChamberLayer[Muon::MuonStationIndex::T2];

        uint8_t phiLayer4Hits = nphiHitsPerChamberLayer[Muon::MuonStationIndex::BO2] +
                                nphiHitsPerChamberLayer[Muon::MuonStationIndex::T3];

        uint8_t etaLayer1Hits = ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::BM1] +
                                ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::T4]+
                                ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::CSC] +
                                ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::STGC1] +
                                ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::STGC2];

        uint8_t etaLayer2Hits = ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::BM2] +
                                ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::T1];

        uint8_t etaLayer3Hits = ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::BO1] +
                                ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::T2];

        uint8_t etaLayer4Hits = ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::BO2] +
                                ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::T3];

        uint8_t nprecLayers = 0;
        if (nprecHitsPerChamberLayer[Muon::MuonStationIndex::BIS] + nprecHitsPerChamberLayer[Muon::MuonStationIndex::BIL] > 3)
            ++nprecLayers;
        if (nprecHitsPerChamberLayer[Muon::MuonStationIndex::BMS] + nprecHitsPerChamberLayer[Muon::MuonStationIndex::BML] > 2)
            ++nprecLayers;
        if (nprecHitsPerChamberLayer[Muon::MuonStationIndex::BOS] + nprecHitsPerChamberLayer[Muon::MuonStationIndex::BOL] > 2)
            ++nprecLayers;
        if (nprecHitsPerChamberLayer[Muon::MuonStationIndex::EIS] + nprecHitsPerChamberLayer[Muon::MuonStationIndex::EIL] > 3)
            ++nprecLayers;
        if (nprecHitsPerChamberLayer[Muon::MuonStationIndex::EMS] + nprecHitsPerChamberLayer[Muon::MuonStationIndex::EML] > 2)
            ++nprecLayers;
        if (nprecHitsPerChamberLayer[Muon::MuonStationIndex::EOS] + nprecHitsPerChamberLayer[Muon::MuonStationIndex::EOL] > 2)
            ++nprecLayers;
        if (nprecHitsPerChamberLayer[Muon::MuonStationIndex::EES] + nprecHitsPerChamberLayer[Muon::MuonStationIndex::EEL] > 3)
            ++nprecLayers;
        if (nprecHitsPerChamberLayer[Muon::MuonStationIndex::CSS] + nprecHitsPerChamberLayer[Muon::MuonStationIndex::CSL] > 2)
            ++nprecLayers;
        if (nprecHitsPerChamberLayer[Muon::MuonStationIndex::BEE] > 3) ++nprecLayers;

        uint8_t nphiLayers = 0;
        if (nphiHitsPerChamberLayer[Muon::MuonStationIndex::BM1] > 0) ++nphiLayers;
        if (nphiHitsPerChamberLayer[Muon::MuonStationIndex::BM2] > 0) ++nphiLayers;
        if (nphiHitsPerChamberLayer[Muon::MuonStationIndex::BO1] > 0) ++nphiLayers;
        if (nphiHitsPerChamberLayer[Muon::MuonStationIndex::BO2] > 0) ++nphiLayers;
        if (nphiHitsPerChamberLayer[Muon::MuonStationIndex::T1] > 0) ++nphiLayers;
        if (nphiHitsPerChamberLayer[Muon::MuonStationIndex::T2] > 0) ++nphiLayers;
        if (nphiHitsPerChamberLayer[Muon::MuonStationIndex::T3] > 0) ++nphiLayers;
        if (nphiHitsPerChamberLayer[Muon::MuonStationIndex::T4] > 0) ++nphiLayers;
        if (nphiHitsPerChamberLayer[Muon::MuonStationIndex::CSC] > 2) ++nphiLayers;
        if (nphiHitsPerChamberLayer[Muon::MuonStationIndex::STGC1] + nphiHitsPerChamberLayer[Muon::MuonStationIndex::STGC2] > 3)
            ++nphiLayers;

        uint8_t ntrigEtaLayers = 0;
        if (ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::BM1] > 0) ++ntrigEtaLayers;
        if (ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::BM2] > 0) ++ntrigEtaLayers;
        if (ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::BO1] > 0) ++ntrigEtaLayers;
        if (ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::BO2] > 0) ++ntrigEtaLayers;
        if (ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::T1] > 0) ++ntrigEtaLayers;
        if (ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::T2] > 0) ++ntrigEtaLayers;
        if (ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::T3] > 0) ++ntrigEtaLayers;
        if (ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::T4] > 0) ++ntrigEtaLayers;
        if (ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::CSC] > 2) ++ntrigEtaLayers;
        if (ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::STGC1] + ntrigEtaHitsPerChamberLayer[Muon::MuonStationIndex::STGC2] > 3)
            ++ntrigEtaLayers;

        // copy hit counts onto TruthParticle
        (*myDecors.nprecLayersDecor)(truthParticle) = nprecLayers;
        (*myDecors.nphiLayersDecor)(truthParticle) = nphiLayers;
        (*myDecors.ntrigEtaLayersDecor)(truthParticle) = ntrigEtaLayers;
        (*myDecors.innerSmallHitsDecor)(truthParticle) = innerSmallHits;
        (*myDecors.innerLargeHitsDecor)(truthParticle) = innerLargeHits;
        (*myDecors.middleSmallHitsDecor)(truthParticle) = middleSmallHits;
        (*myDecors.middleLargeHitsDecor)(truthParticle) = middleLargeHits;
        (*myDecors.outerSmallHitsDecor)(truthParticle) = outerSmallHits;
        (*myDecors.outerLargeHitsDecor)(truthParticle) = outerLargeHits;
        (*myDecors.extendedSmallHitsDecor)(truthParticle) = extendedSmallHits;
        (*myDecors.extendedLargeHitsDecor)(truthParticle) = extendedLargeHits;

        (*myDecors.phiLayer1HitsDecor)(truthParticle) = phiLayer1Hits;
        (*myDecors.phiLayer2HitsDecor)(truthParticle) = phiLayer2Hits;
        (*myDecors.phiLayer3HitsDecor)(truthParticle) = phiLayer3Hits;
        (*myDecors.phiLayer4HitsDecor)(truthParticle) = phiLayer4Hits;

        (*myDecors.etaLayer1HitsDecor)(truthParticle) = etaLayer1Hits;
        (*myDecors.etaLayer2HitsDecor)(truthParticle) = etaLayer2Hits;
        (*myDecors.etaLayer3HitsDecor)(truthParticle) = etaLayer3Hits;
        (*myDecors.etaLayer4HitsDecor)(truthParticle) = etaLayer4Hits;


        if (msgLvl(MSG::DEBUG)) {
            ATH_MSG_DEBUG("Precision layers " << static_cast<int>(nprecLayers) << " phi layers " << static_cast<int>(nphiLayers)
                                            << " triggerEta layers " << static_cast<int>(ntrigEtaLayers));

            if (nprecLayers > 0) {
                msg(MSG::VERBOSE) << " Precision chambers ";

                for (int index = 0; index < static_cast<int>(nprecHitsPerChamberLayer.size()); ++index) {
                    if (nprecHitsPerChamberLayer[index] > 0)
                        msg(MSG::VERBOSE) << " " << Muon::MuonStationIndex::chName(static_cast<Muon::MuonStationIndex::ChIndex>(index))
                                        << " hits " << nprecHitsPerChamberLayer[index];
                }
            }
            if (nphiLayers > 0) {
                msg(MSG::VERBOSE) << endmsg << " Phi chambers ";
                for (int index = 0; index < static_cast<int>(nphiHitsPerChamberLayer.size()); ++index) {
                    if (nphiHitsPerChamberLayer[index] > 0)
                        msg(MSG::VERBOSE) << " " << Muon::MuonStationIndex::phiName(static_cast<Muon::MuonStationIndex::PhiIndex>(index))
                                        << " hits " << nphiHitsPerChamberLayer[index];
                }
            }

            if (ntrigEtaLayers > 0) {
                msg(MSG::VERBOSE) << endmsg << " Trigger Eta ";
                for (int index = 0; index < static_cast<int>(ntrigEtaHitsPerChamberLayer.size()); ++index) {
                    if (ntrigEtaHitsPerChamberLayer[index] > 0)
                        msg(MSG::VERBOSE) << " " << Muon::MuonStationIndex::phiName(static_cast<Muon::MuonStationIndex::PhiIndex>(index))
                                        << " hits " << ntrigEtaHitsPerChamberLayer[index];
                }
            }
            msg(MSG::VERBOSE) << endmsg;
        }
        return StatusCode::SUCCESS;
    }

    StatusCode TruthHitSummaryAlg::addHitIDVectors(const xAOD::TruthParticle& truthParticle,
                                                const ChamberIdMap& ids,
                                                summaryDecors& myDecors) const {
        std::vector<unsigned long long> mdtTruthHits{};
        std::vector<unsigned long long> tgcTruthHits{};
        std::vector<unsigned long long> rpcTruthHits{};
        std::vector<unsigned long long> stgcTruthHits{};
        std::vector<unsigned long long> cscTruthHits{};
        std::vector<unsigned long long> mmTruthHits{};

        // loop over chamber layers
        int nEI = 0, nEM = 0;
        for (const auto& lay : ids) {
            // loop over hits
            if (lay.first == Muon::MuonStationIndex::EIS || lay.first == Muon::MuonStationIndex::EIL) nEI++;
            if (lay.first == Muon::MuonStationIndex::EMS || lay.first == Muon::MuonStationIndex::EML) nEM++;
            for (const Identifier& id : lay.second) {
                if (m_idHelperSvc->isMdt(id))
                    mdtTruthHits.push_back(id.get_compact());
                else if (m_idHelperSvc->isCsc(id))
                    cscTruthHits.push_back(id.get_compact());
                else if (m_idHelperSvc->isTgc(id)) {
                    if ((lay.first == Muon::MuonStationIndex::EIS || lay.first == Muon::MuonStationIndex::EIL) && nEI > 1)
                        continue;  // otherwise we double-count
                    if ((lay.first == Muon::MuonStationIndex::EMS || lay.first == Muon::MuonStationIndex::EML) && nEM > 1)
                        continue;  // otherwise we double-count
                    tgcTruthHits.push_back(id.get_compact());
                } else if (m_idHelperSvc->issTgc(id))
                    stgcTruthHits.push_back(id.get_compact());
                else if (m_idHelperSvc->isRpc(id))
                    rpcTruthHits.push_back(id.get_compact());
                else if (m_idHelperSvc->isMM(id))
                    mmTruthHits.push_back(id.get_compact());
            }
        }
        auto attatchHits = [&truthParticle](const WriteDecor_llvec& dec,
                                            std::vector<unsigned long long>& hits) {
            if (dec) {
                (*dec)(truthParticle) = std::move(hits);
            }
        };
        attatchHits(myDecors.truthMdtHitsDecor, mdtTruthHits);
        attatchHits(myDecors.truthTgcHitsDecor, tgcTruthHits);
        attatchHits(myDecors.truthRpcHitsDecor, rpcTruthHits);
        attatchHits(myDecors.truthCscHitsDecor, cscTruthHits);
        attatchHits(myDecors.truthStgcHitsDecor, stgcTruthHits);
        attatchHits(myDecors.truthMMHitsDecor, mmTruthHits);
        ATH_MSG_VERBOSE("Added " << mdtTruthHits.size() << " mdt truth hits, " << cscTruthHits.size() << " csc truth hits, "
                                << rpcTruthHits.size() << " rpc truth hits, and " << tgcTruthHits.size() << " tgc truth hits");
    return StatusCode::SUCCESS;
    }
}  // namespace Muon
