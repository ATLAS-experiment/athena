/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthHitSummaryAlg.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/TrackingVolume.h"

namespace Muon {    
    using namespace MuonStationIndex;
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
        std::vector<unsigned int> nphiHitsPerChamberLayer;
        std::vector<unsigned int> ntrigEtaHitsPerChamberLayer;

        ntrigEtaHitsPerChamberLayer.resize(toInt(PhiIndex::PhiIndexMax));
        nprecHitsPerChamberLayer.resize(toInt(ChIndex::ChIndexMax));
        nphiHitsPerChamberLayer.resize(toInt(PhiIndex::PhiIndexMax));
        
        ATH_MSG_DEBUG("addHitCounts: unique ID " << HepMC::uniqueID(truthParticle));
        auto truthParticleHistory = HepMC::simulation_history(&truthParticle, -1); // Returns a list of unique IDs
        // loop over detector technologies
        for (SG::ReadHandle<PRD_MultiTruthCollection>& col : m_PRD_TruthNames.makeHandles(ctx)) {
            ATH_CHECK(col.isPresent());
            
            // loop over trajectories
            for (const std::pair<Identifier, HepMcParticleLink> trajectory : *col) {
                 // check if gen particle same as input
                if (std::ranges::find(truthParticleHistory, HepMC::uniqueID(trajectory.second)) == truthParticleHistory.end()) {
                    continue;
                }
                const Identifier& id = trajectory.first;
                bool measPhi = m_idHelperSvc->measuresPhi(id);
                bool isTgc = m_idHelperSvc->isTgc(id);
                ChIndex chIndex = !isTgc ? m_idHelperSvc->chamberIndex(id) : ChIndex::ChUnknown;

                // add identifier to map
                if (m_idHelperSvc->isTgc(id)) {  // TGCS should be added to both EIL and EIS
                    PhiIndex index = m_idHelperSvc->phiIndex(id);
                    if (index == PhiIndex::T4) {
                        ids[ChIndex::EIS].push_back(id);
                        ids[ChIndex::EIL].push_back(id);
                    } else {
                        ids[ChIndex::EMS].push_back(id);
                        ids[ChIndex::EML].push_back(id);
                    }
                } else {
                    ids[m_idHelperSvc->chamberIndex(id)].push_back(id);
                }
                if (m_idHelperSvc->issTgc(id)) {
                    if (measPhi) {
                        PhiIndex index = m_idHelperSvc->phiIndex(id);
                        ++nphiHitsPerChamberLayer.at(toInt(index));
                    }  else {
                        ++nprecHitsPerChamberLayer.at(toInt(chIndex));
                    }
                } else if (m_idHelperSvc->isMM(id)) {
                    ++nprecHitsPerChamberLayer.at(toInt(chIndex));
                } else if (m_idHelperSvc->isTrigger(id)) {
                    PhiIndex index = m_idHelperSvc->phiIndex(id);
                    if (index != PhiIndex::PhiUnknown) {
                        if (measPhi)
                            ++nphiHitsPerChamberLayer.at(toInt(index));
                        else
                            ++ntrigEtaHitsPerChamberLayer.at(toInt(index));
                    }
                } else {
                    if (measPhi) {
                        PhiIndex index = m_idHelperSvc->phiIndex(id);
                        ++nphiHitsPerChamberLayer.at(toInt(index));
                    } else {
                        ++nprecHitsPerChamberLayer.at(toInt(chIndex));
                    }
                }
            }
        }

        uint8_t innerSmallHits = nprecHitsPerChamberLayer[toInt(ChIndex::BIS)] +
                                 nprecHitsPerChamberLayer[toInt(ChIndex::EIS)] +
                                 nprecHitsPerChamberLayer[toInt(ChIndex::CSS)];

        uint8_t innerLargeHits = nprecHitsPerChamberLayer[toInt(ChIndex::BIL)] +
                                 nprecHitsPerChamberLayer[toInt(ChIndex::EIL)] +
                                 nprecHitsPerChamberLayer[toInt(ChIndex::CSL)];

        uint8_t middleSmallHits = nprecHitsPerChamberLayer[toInt(ChIndex::BMS)] +
                                  nprecHitsPerChamberLayer[toInt(ChIndex::EMS)];

        uint8_t middleLargeHits = nprecHitsPerChamberLayer[toInt(ChIndex::BML)] +
                                  nprecHitsPerChamberLayer[toInt(ChIndex::EML)];

        uint8_t outerSmallHits = nprecHitsPerChamberLayer[toInt(ChIndex::BOS)] +
                                 nprecHitsPerChamberLayer[toInt(ChIndex::EOS)];

        uint8_t outerLargeHits = nprecHitsPerChamberLayer[toInt(ChIndex::BML)] +
                                 nprecHitsPerChamberLayer[toInt(ChIndex::EOL)];

        uint8_t extendedSmallHits = nprecHitsPerChamberLayer[toInt(ChIndex::EES)] +
                                    nprecHitsPerChamberLayer[toInt(ChIndex::BEE)];

        uint8_t extendedLargeHits = nprecHitsPerChamberLayer[toInt(ChIndex::EEL)];

        uint8_t phiLayer1Hits = nphiHitsPerChamberLayer[toInt(PhiIndex::BM1)] +
                                nphiHitsPerChamberLayer[toInt(PhiIndex::T4)] +
                                nphiHitsPerChamberLayer[toInt(PhiIndex::CSC)] +
                                nphiHitsPerChamberLayer[toInt(PhiIndex::STGC1)] +
                                nphiHitsPerChamberLayer[toInt(PhiIndex::STGC2)];

        uint8_t phiLayer2Hits = nphiHitsPerChamberLayer[toInt(PhiIndex::BM2)] +
                                nphiHitsPerChamberLayer[toInt(PhiIndex::T1)];

        uint8_t phiLayer3Hits = nphiHitsPerChamberLayer[toInt(PhiIndex::BO1)] +
                                nphiHitsPerChamberLayer[toInt(PhiIndex::T2)];

        uint8_t phiLayer4Hits = nphiHitsPerChamberLayer[toInt(PhiIndex::BO2)] +
                                nphiHitsPerChamberLayer[toInt(PhiIndex::T3)];

        uint8_t etaLayer1Hits = ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::BM1)] +
                                ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::T4)]+
                                ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::CSC)] +
                                ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::STGC1)] +
                                ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::STGC2)];

        uint8_t etaLayer2Hits = ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::BM2)] +
                                ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::T1)];

        uint8_t etaLayer3Hits = ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::BO1)] +
                                ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::T2)];

        uint8_t etaLayer4Hits = ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::BO2)] +
                                ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::T3)];

        uint8_t nprecLayers = 0;
        nprecLayers += (nprecHitsPerChamberLayer[toInt(ChIndex::BIS)] + nprecHitsPerChamberLayer[toInt(ChIndex::BIL)] > 3);
        nprecLayers += (nprecHitsPerChamberLayer[toInt(ChIndex::BMS)] + nprecHitsPerChamberLayer[toInt(ChIndex::BML)] > 2);
        nprecLayers += (nprecHitsPerChamberLayer[toInt(ChIndex::BOS)] + nprecHitsPerChamberLayer[toInt(ChIndex::BOL)] > 2);
        nprecLayers += (nprecHitsPerChamberLayer[toInt(ChIndex::EIS)] + nprecHitsPerChamberLayer[toInt(ChIndex::EIL)] > 3);
        nprecLayers += (nprecHitsPerChamberLayer[toInt(ChIndex::EMS)] + nprecHitsPerChamberLayer[toInt(ChIndex::EML)] > 2);
        nprecLayers += (nprecHitsPerChamberLayer[toInt(ChIndex::EOS)] + nprecHitsPerChamberLayer[toInt(ChIndex::EOL)] > 2);
        nprecLayers += (nprecHitsPerChamberLayer[toInt(ChIndex::EES)] + nprecHitsPerChamberLayer[toInt(ChIndex::EEL)] > 3);
        nprecLayers += (nprecHitsPerChamberLayer[toInt(ChIndex::CSS)] + nprecHitsPerChamberLayer[toInt(ChIndex::CSL)] > 2);
        nprecLayers += (nprecHitsPerChamberLayer[toInt(ChIndex::BEE)] > 3);

        uint8_t nphiLayers = 0;
        nphiLayers += (nphiHitsPerChamberLayer[toInt(PhiIndex::BM1)] > 0);
        nphiLayers += (nphiHitsPerChamberLayer[toInt(PhiIndex::BM2)] > 0);
        nphiLayers += (nphiHitsPerChamberLayer[toInt(PhiIndex::BO1)] > 0);
        nphiLayers += (nphiHitsPerChamberLayer[toInt(PhiIndex::BO2)] > 0);
        nphiLayers += (nphiHitsPerChamberLayer[toInt(PhiIndex::T1)] > 0); 
        nphiLayers += (nphiHitsPerChamberLayer[toInt(PhiIndex::T2)] > 0); 
        nphiLayers += (nphiHitsPerChamberLayer[toInt(PhiIndex::T3)] > 0);
        nphiLayers += (nphiHitsPerChamberLayer[toInt(PhiIndex::T4)] > 0); 
        nphiLayers += (nphiHitsPerChamberLayer[toInt(PhiIndex::CSC)] > 2);
        nphiLayers += (nphiHitsPerChamberLayer[toInt(PhiIndex::STGC1)] + nphiHitsPerChamberLayer[toInt(PhiIndex::STGC2)] > 3);

        uint8_t ntrigEtaLayers = 0;
        ntrigEtaLayers += (ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::BM1)] > 0);
        ntrigEtaLayers += (ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::BM2)] > 0);
        ntrigEtaLayers += (ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::BO1)] > 0);
        ntrigEtaLayers += (ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::BO2)] > 0);
        ntrigEtaLayers += (ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::T1)] > 0); 
        ntrigEtaLayers += (ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::T2)] > 0); 
        ntrigEtaLayers += (ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::T3)] > 0);
        ntrigEtaLayers += (ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::T4)] > 0); 
        ntrigEtaLayers += (ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::CSC)] > 2);
        ntrigEtaLayers += (ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::STGC1)] + ntrigEtaHitsPerChamberLayer[toInt(PhiIndex::STGC2)] > 3);
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
                        msg(MSG::VERBOSE) << " " << chName(static_cast<ChIndex>(index))
                                        << " hits " << nprecHitsPerChamberLayer[index];
                }
            }
            if (nphiLayers > 0) {
                msg(MSG::VERBOSE) << endmsg << " Phi chambers ";
                for (int index = 0; index < static_cast<int>(nphiHitsPerChamberLayer.size()); ++index) {
                    if (nphiHitsPerChamberLayer[index] > 0)
                        msg(MSG::VERBOSE) << " " << phiName(static_cast<PhiIndex>(index))
                                        << " hits " << nphiHitsPerChamberLayer[index];
                }
            }

            if (ntrigEtaLayers > 0) {
                msg(MSG::VERBOSE) << endmsg << " Trigger Eta ";
                for (int index = 0; index < static_cast<int>(ntrigEtaHitsPerChamberLayer.size()); ++index) {
                    if (ntrigEtaHitsPerChamberLayer[index] > 0)
                        msg(MSG::VERBOSE) << " " << phiName(static_cast<PhiIndex>(index))
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
            if (lay.first == ChIndex::EIS || lay.first == ChIndex::EIL) nEI++;
            if (lay.first == ChIndex::EMS || lay.first == ChIndex::EML) nEM++;
            for (const Identifier& id : lay.second) {
                if (m_idHelperSvc->isMdt(id))
                    mdtTruthHits.push_back(id.get_compact());
                else if (m_idHelperSvc->isCsc(id))
                    cscTruthHits.push_back(id.get_compact());
                else if (m_idHelperSvc->isTgc(id)) {
                    if ((lay.first == ChIndex::EIS || lay.first == ChIndex::EIL) && nEI > 1)
                        continue;  // otherwise we double-count
                    if ((lay.first == ChIndex::EMS || lay.first == ChIndex::EML) && nEM > 1)
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
