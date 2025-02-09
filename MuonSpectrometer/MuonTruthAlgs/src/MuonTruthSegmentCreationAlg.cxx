/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTruthSegmentCreationAlg.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "Identifier/Identifier.h"
#include "TruthUtils/TruthClasses.h"
#include "MuonReadoutGeometry/CscReadoutElement.h"
#include "MuonSimData/CscSimDataCollection.h"
#include "MuonSimData/MuonSimDataCollection.h"
#include "xAODMuon/MuonSegment.h"
#include "xAODMuon/MuonSegmentAuxContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include <cstddef>
#include <memory>
#include "StoreGate/ReadDecorHandle.h"
namespace {

    // Only reject muons from light quark deays
    const std::set<int> bad_origins{
        ParticleOrigin ::LightMeson,     // 23
        ParticleOrigin ::StrangeMeson,   // 24
        ParticleOrigin ::LightBaryon,    // 30
        ParticleOrigin ::StrangeBaryon,  // 31
        ParticleOrigin ::PionDecay,      // 34
        ParticleOrigin ::NucReact,       // 41
        ParticleOrigin ::PiZero,         // 42

    };

}  // namespace

namespace Muon {

    // Initialize method:
    StatusCode MuonTruthSegmentCreationAlg::initialize() {
        ATH_CHECK(m_muonTruth.initialize());

        ATH_CHECK(m_muonTruthSegmentContainerName.initialize());
        ATH_CHECK(m_SDO_TruthNames.initialize());
        ATH_CHECK(m_CSC_SDO_TruthNames.initialize(!m_CSC_SDO_TruthNames.empty()));

        ATH_CHECK(m_detMgrKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());

        ATH_CHECK(m_truthOriginKey.initialize());
        if(m_idHelperSvc->hasRPC()) {m_truthHitsKeyArray.emplace_back(m_muonTruth, "truthRpcHits");}
        if(m_idHelperSvc->hasTGC()) {m_truthHitsKeyArray.emplace_back(m_muonTruth, "truthTgcHits");}
        if(m_idHelperSvc->hasMDT()) {m_truthHitsKeyArray.emplace_back(m_muonTruth, "truthMdtHits");}
        if(m_idHelperSvc->hasMM()) {m_truthHitsKeyArray.emplace_back(m_muonTruth, "truthMMHits");}
        if(m_idHelperSvc->hasSTGC()) {m_truthHitsKeyArray.emplace_back(m_muonTruth, "truthStgcHits");}
        if(m_idHelperSvc->hasCSC()) {m_truthHitsKeyArray.emplace_back(m_muonTruth, "truthCscHits");}
        ATH_CHECK(m_truthHitsKeyArray.initialize());
        return StatusCode::SUCCESS;
    }

    // Execute method:
    StatusCode MuonTruthSegmentCreationAlg::execute(const EventContext& ctx) const {

        SG::ReadHandle<xAOD::TruthParticleContainer> muonTruthContainer(m_muonTruth, ctx);
        ATH_CHECK(muonTruthContainer.isPresent());

        SG::ReadDecorHandle<xAOD::TruthParticleContainer, int> truthOrigin(m_truthOriginKey, ctx);
        ATH_CHECK(truthOrigin.isPresent());

        // create output container
        SG::WriteHandle segmentContainer(m_muonTruthSegmentContainerName, ctx);
        ATH_CHECK(
            segmentContainer.record(std::make_unique<xAOD::MuonSegmentContainer>(), std::make_unique<xAOD::MuonSegmentAuxContainer>()));
        ATH_MSG_DEBUG("Recorded MuonSegmentContainer with key: " << segmentContainer.name());
        
        size_t itr = 0;
        for (const xAOD::TruthParticle* truthParticle : *muonTruthContainer) {
            
            const int iOrigin = truthOrigin(*truthParticle);
            bool goodMuon = bad_origins.find(iOrigin) == bad_origins.end();

            // create segments
            if (goodMuon) {
                ElementLink<xAOD::TruthParticleContainer> truthLink(*muonTruthContainer, itr);
                truthLink.toPersistent();

                ChamberIdMap ids{};
                ATH_CHECK(fillChamberIdMap(ctx, *truthParticle, ids));
                ATH_CHECK(createSegments(ctx, truthLink, ids, *segmentContainer));
            }
            itr++;
        }

        ATH_MSG_DEBUG("Registered " << segmentContainer->size() << " truth muon segments ");

        return StatusCode::SUCCESS;
    }

    StatusCode MuonTruthSegmentCreationAlg::fillChamberIdMap(const EventContext& ctx,
                                                       const xAOD::TruthParticle& truthParticle, 
                                                       ChamberIdMap& ids) const{

        for (SG::ReadDecorHandle<xAOD::TruthParticleContainer, std::vector<unsigned long long>>& hitCollection : m_truthHitsKeyArray.makeHandles(ctx)){

            ATH_CHECK(hitCollection.isPresent());
            for (const unsigned long long& hit_compID : hitCollection(truthParticle)){

                const Identifier id{hit_compID};
                const bool isTgc = m_idHelperSvc->isTgc(id);
                const Muon::MuonStationIndex::ChIndex chIndex = !isTgc ? m_idHelperSvc->chamberIndex(id) : Muon::MuonStationIndex::ChUnknown;
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
            }
        }
        return StatusCode::SUCCESS;
    }

    StatusCode MuonTruthSegmentCreationAlg::createSegments(const EventContext& ctx,
                                                           const ElementLink<xAOD::TruthParticleContainer>& truthLink,
                                                           const ChamberIdMap& ids,
                                                           xAOD::MuonSegmentContainer& segmentContainer) const {

        SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{m_detMgrKey, ctx};

        std::vector<SG::ReadHandle<MuonSimDataCollection> > sdoCollections(6);
        for (const SG::ReadHandleKey<MuonSimDataCollection>& k : m_SDO_TruthNames) {
            SG::ReadHandle col(k, ctx);
            ATH_CHECK(col.isPresent());
            
            if (col->empty()) {
                continue;
            }
            Identifier id = col->begin()->first;
            int index = m_idHelperSvc->technologyIndex(id);
            if (index >= (int)sdoCollections.size()) {
                ATH_MSG_WARNING("SDO collection index out of range " << index << "  " << m_idHelperSvc->toStringChamber(id));
            } else {
                sdoCollections[index] = std::move(col);
            }

        }

        bool useSDO = (!sdoCollections.empty() || !m_CSC_SDO_TruthNames.empty());
        ATH_MSG_DEBUG(" Creating Truth segments ");

        // loop over chamber layers
        for (const auto& lay : ids) {
            // skip empty layers
            Amg::Vector3D firstPos{Amg::Vector3D::Zero()}, secondPos{Amg::Vector3D::Zero()};
            bool firstPosSet{false}, secondPosSet{false};
            Identifier chId;
            int index = -1;
            uint8_t nprecLayers{0}, nphiLayers{0}, ntrigEtaLayers{0};
            std::set<int> phiLayers, etaLayers, precLayers;
            ATH_MSG_DEBUG(" new chamber layer " << Muon::MuonStationIndex::chName(lay.first) << " hits " << ids.size());
            // loop over hits
            for (const auto& id : lay.second) {
                ATH_MSG_VERBOSE(" hit " << m_idHelperSvc->toString(id));
                bool measPhi = m_idHelperSvc->measuresPhi(id);
                bool isCsc = m_idHelperSvc->isCsc(id);
                bool isMM = m_idHelperSvc->isMM(id);
                bool isTrig = m_idHelperSvc->isTrigger(id);
                bool isEndcap = m_idHelperSvc->isEndcap(id);
                if (measPhi) {
                    phiLayers.insert(m_idHelperSvc->gasGap(id));
                } else {
                    if (!isTrig) {
                        if (!chId.is_valid()) chId = id;  // use first precision hit in list
                        if (isCsc || isMM) {
                            precLayers.insert(m_idHelperSvc->gasGap(id));
                        } else {
                            int iid = 10 * m_idHelperSvc->mdtIdHelper().multilayer(id) + m_idHelperSvc->mdtIdHelper().tubeLayer(id);
                            precLayers.insert(iid);
                            // ATH_MSG_VERBOSE("iid " << iid << " precLayers size " << precLayers.size() );
                        }
                    } else {
                        etaLayers.insert(m_idHelperSvc->gasGap(id));
                    }
                }
                // use SDO to look-up truth position of the hit
                if (!useSDO) {
                    continue;
                }
                Amg::Vector3D gpos{Amg::Vector3D::Zero()};
                if (!isCsc) {
                    bool ok = false;
                    int index = m_idHelperSvc->technologyIndex(id);
                    if (index < (int)sdoCollections.size() && !sdoCollections[index]->empty()) {
                        auto pos = sdoCollections[index]->find(id);
                        if (pos != sdoCollections[index]->end()) {
                            gpos = pos->second.globalPosition();
                            if (gpos.perp() > 0.1) ok = true;  // sanity check
                        }
                    }
                    // look up successful, calculate
                    if (!ok) continue;

                    // small comparison function
                    auto isSmaller = [isEndcap](const Amg::Vector3D& p1, const Amg::Vector3D& p2) {
                        if (isEndcap)
                            return std::abs(p1.z()) < std::abs(p2.z());
                        else
                            return p1.perp() < p2.perp();
                    };
                    if (!firstPosSet) {
                        firstPos = gpos;
                        firstPosSet = true;
                    } else if (!secondPosSet) {
                        secondPos = gpos;
                        secondPosSet = true;
                        if (isSmaller(secondPos, firstPos)) std::swap(firstPos, secondPos);
                    } else {
                        // update position if we can increase the distance between the two positions
                        if (isSmaller(gpos, firstPos))
                            firstPos = gpos;
                        else if (isSmaller(secondPos, gpos))
                            secondPos = gpos;
                    }
                } else {
                    SG::ReadHandle cscCollection(m_CSC_SDO_TruthNames, ctx);
                    ATH_CHECK(cscCollection.isPresent());
                    auto pos = cscCollection->find(id);
                    if (pos == cscCollection->end()) {
                        continue;
                    }
                    const MuonGM::CscReadoutElement* descriptor = detMgr->getCscReadoutElement(id);
                    ATH_MSG_DEBUG("found csc sdo with " << pos->second.getdeposits().size() << " deposits");
                    Amg::Vector3D locpos(0, pos->second.getdeposits()[0].second.ypos(), pos->second.getdeposits()[0].second.zpos());
                    gpos = descriptor->localToGlobalCoords(locpos, m_idHelperSvc->cscIdHelper().elementID(id));
                    ATH_MSG_DEBUG("got CSC global position " << gpos);
                    if (!firstPosSet) {
                        firstPos = gpos;
                        firstPosSet = true;
                    } else if (!secondPosSet) {
                        secondPos = gpos;
                        secondPosSet = true;
                        if (secondPos.perp() < firstPos.perp()) std::swap(firstPos, secondPos);
                    } else {
                        if (gpos.perp() < firstPos.perp())
                            firstPos = gpos;
                        else if (secondPos.perp() < gpos.perp())
                            secondPos = gpos;
                    }
                }
            }
            if (precLayers.size() > 2) {
                if (!phiLayers.empty()) nphiLayers = phiLayers.size();
                ntrigEtaLayers = etaLayers.size();
                nprecLayers = precLayers.size();
                ATH_MSG_DEBUG(" total counts: precision " << static_cast<int>(nprecLayers) << " phi layers " << static_cast<int>(nphiLayers)
                              << " eta trig layers " << static_cast<int>(ntrigEtaLayers)
                              << " associated reco muon " << index << " barcode " << HepMC::barcode(*truthLink) // FIXME barcode-based
                              << " truthLink " << truthLink);
                xAOD::MuonSegment* segment =  segmentContainer.push_back(std::make_unique<xAOD::MuonSegment>());
               
                segment->setNHits(nprecLayers, nphiLayers, ntrigEtaLayers);
                static const SG::Accessor<ElementLink<xAOD::TruthParticleContainer> >
                  truthParticleLinkAcc("truthParticleLink");
                truthParticleLinkAcc(*segment) = truthLink;
                if (chId.is_valid()) {    //we should always enter here if we have precision measurements
                    int eta = m_idHelperSvc->stationEta(chId);
                    int sector = m_idHelperSvc->sector(chId);
                    MuonStationIndex::TechnologyIndex technology = m_idHelperSvc->technologyIndex(chId);
                    MuonStationIndex::ChIndex chIndex = m_idHelperSvc->chamberIndex(chId);
                    segment->setIdentifier(sector, chIndex, eta, technology);
                }
                if (firstPosSet && secondPosSet) {
                    Amg::Vector3D gpos = (firstPos + secondPos) / 2.;
                    Amg::Vector3D gdir = (firstPos - secondPos).unit();
                    ATH_MSG_DEBUG(" got position : r " << gpos.perp() << " z " << gpos.z() << "  and direction: theta " << gdir.theta()
                                                       << " phi " << gdir.phi());
                    segment->setPosition(gpos.x(), gpos.y(), gpos.z());
                    segment->setDirection(gdir.x(), gdir.y(), gdir.z());
                }
            }
        }
        return StatusCode::SUCCESS;
    }

    

}  // namespace Muon
