/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughPatternTools/MuonLayerHoughTool.h"

#include "AtlasHepMC/GenEvent.h"
#include "CxxUtils/sincos.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "MuonPattern/MuonPatternChamberIntersect.h"
#include "MuonPattern/MuonPatternCombination.h"
#include "MuonReadoutGeometry/MuonChannelDesign.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadCondHandle.h"
#include "MuonReadoutGeometry/MuonPadDesign.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
namespace Muon {
    using namespace MuonStationIndex;

    StatusCode MuonLayerHoughTool::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        m_ntechnologies = m_idHelperSvc->mdtIdHelper().technologyNameIndexMax() + 1;
        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_muonManagerKey.initialize());
        ATH_CHECK(m_truthNames.initialize());
        constexpr int nSelect = toInt(ChIndex::ChIndexMax);
        // initialize cuts, if only one cut, use make_pair to avoid compiler issues, format is (position, cut)
        m_selectors.resize(nSelect);
        m_selectors[toInt(ChIndex::BIS)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 5.9)});  // old values: 6.9; optimized: 7.9
        m_selectors[toInt(ChIndex::BIL)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 5.9)});  // old values: 6.9; optimized: 7.9
        m_selectors[toInt(ChIndex::BMS)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 4.9)});  // old values: 7.9; optimized: 7.9
        m_selectors[toInt(ChIndex::BML)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 4.9)});  // old values: 7.9; optimized: 7.9
        m_selectors[toInt(ChIndex::BOS)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 4.9)});  // old values: 4.9; optimized: 5.9
        m_selectors[toInt(ChIndex::BOL)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 4.9)});  // old values: 4.9; optimized: 5.9
        m_selectors[toInt(ChIndex::BEE)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 3.9)});  // old values: 5.9; optimized: 5.9
        m_selectors[toInt(ChIndex::EIS)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 5.9)});  // old values: 6.9; optimized: 7.9
        m_selectors[toInt(ChIndex::EIL)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 5.9)});  // old values: 6.9; optimized: 7.9
        m_selectors[toInt(ChIndex::EMS)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 4.9)});  // old values: 7.9; optimized: 5.9
        m_selectors[toInt(ChIndex::EML)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 4.9)});  // old values: 7.9; optimized: 5.9
        m_selectors[toInt(ChIndex::EOS)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 4.9)});  // old values: 4.9; optimized: 5.9
        m_selectors[toInt(ChIndex::EOL)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 4.9)});  // old values: 4.9; optimized: 5.9
        m_selectors[toInt(ChIndex::EES)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 4.9)});  // old values: 4.9; optimized: 5.9
        m_selectors[toInt(ChIndex::EEL)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 4.9)});  // old values: 4.9; optimized: 5.9

        m_selectorsLoose.resize(nSelect);
        m_selectorsLoose[toInt(ChIndex::BIS)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 1.9)});  // old values: 2.9; optimized: 3.9
        m_selectorsLoose[toInt(ChIndex::BIL)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 2.9)});  // old values: 2.9; optimized: 3.9
        m_selectorsLoose[toInt(ChIndex::BMS)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 1.9)});  // old values: 4.9; optimized: 2.9
        m_selectorsLoose[toInt(ChIndex::BML)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 2.9)});  // old values: 4.9; optimized: 2.9
        m_selectorsLoose[toInt(ChIndex::BOS)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 1.9)});  // old values: 2.9; optimized: 2.9
        m_selectorsLoose[toInt(ChIndex::BOL)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 2.9)});  // old values: 2.9; optimized: 2.9
        m_selectorsLoose[toInt(ChIndex::BEE)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 1.9)});  // old values: 3.9; optimized: 2.9
        m_selectorsLoose[toInt(ChIndex::EIS)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 1.9)});  // old values: 4.9; optimized: 3.9
        m_selectorsLoose[toInt(ChIndex::EIL)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 1.9)});  // old values: 4.9; optimized: 3.9
        m_selectorsLoose[toInt(ChIndex::EMS)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 2.9)});  // old values: 5.9; optimized: 2.9
        m_selectorsLoose[toInt(ChIndex::EML)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 2.9)});  // old values: 5.9; optimized: 2.9
        m_selectorsLoose[toInt(ChIndex::EOS)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 2.9)});  // old values: 2.9; optimized: 2.9
        m_selectorsLoose[toInt(ChIndex::EOL)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 2.9)});  // old values: 2.9; optimized: 2.9
        m_selectorsLoose[toInt(ChIndex::EES)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 2.9)});  // old values: 2.9; optimized: 2.9
        m_selectorsLoose[toInt(ChIndex::EEL)] = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 2.9)});  // old values: 2.9; optimized: 2.9

        return StatusCode::SUCCESS;
    }

    std::pair<std::unique_ptr<MuonPatternCombinationCollection>, std::unique_ptr<HoughDataPerSectorVec>> MuonLayerHoughTool::find(
        const std::vector<const MdtPrepDataCollection*>& mdtCols, const std::vector<const CscPrepDataCollection*>& cscCols,
        const std::vector<const TgcPrepDataCollection*>& tgcCols, const std::vector<const RpcPrepDataCollection*>& rpcCols,
        const MuonSegmentCombinationCollection*, const EventContext& ctx) const {
        
        using namespace MuonStationIndex;
        initializeSectorMapping(ctx);
        State state;
        ATH_MSG_DEBUG("MuonLayerHoughTool::find");

        // create structure to hold data per sector and set the sector indices
        state.houghDataPerSectorVec->vec.resize(16);
        for (unsigned int i = 0; i < state.houghDataPerSectorVec->vec.size(); ++i) state.houghDataPerSectorVec->vec[i].sector = i + 1;

        // return DetectorRegionIndex and sectorLayerHash
        auto getHashes = [this](const Identifier& id) {
            DetRegIdx regionIndex = m_idHelperSvc->regionIndex(id);
            MuonStationIndex::LayerIndex layerIndex = m_idHelperSvc->layerIndex(id);
            unsigned int sectorLayerHash = MuonStationIndex::sectorLayerHash(regionIndex, layerIndex);
            return std::make_pair(regionIndex, sectorLayerHash);
        };

        for (const MdtPrepDataCollection* col : mdtCols) {
            if (!col) continue;
            Identifier id = col->identify();
            int sector = m_idHelperSvc->sector(id);
            auto hashes = getHashes(id);
            fill(ctx, state.truthHits, *col, state.houghDataPerSectorVec->vec[sector - 1].hitVec[hashes.second]);
        }

        for (const RpcPrepDataCollection* col : rpcCols) {
            if (!col) continue;
            Identifier id = col->identify();
            int sector = m_idHelperSvc->sector(id);
            auto hashes = getHashes(id);
            fill(ctx, state.truthHits, *col, state.houghDataPerSectorVec->vec[sector - 1].hitVec[hashes.second],
                 state.houghDataPerSectorVec->vec[sector - 1].phiHitVec[toInt(hashes.first)]);
        }
        for (const CscPrepDataCollection* col : cscCols) {
            if (!col) continue;
            const Identifier id = col->identify();
            int sector = m_idHelperSvc->sector(id);
            auto hashes = getHashes(id);
            fill(ctx, state.truthHits, *col, state.houghDataPerSectorVec->vec[sector - 1].hitVec[hashes.second],
                 state.houghDataPerSectorVec->vec[sector - 1].phiHitVec[toInt(hashes.first)]);
        }
        auto hashInSector = [this](IdentifierHash hash, int sector, unsigned int sectorLayerHash) {
            constexpr int tgcTech = toInt(TechnologyIndex::TGC);
            const std::vector<IdentifierHash>& hashes = m_collectionsPerSector[sector - 1].technologyRegionHashVecs[tgcTech][sectorLayerHash];
            return std::binary_search(hashes.begin(), hashes.end(), hash);
        };

        for (const TgcPrepDataCollection* col : tgcCols) {
            if (!col) continue;
            Identifier id = col->identify();
            int sector = m_idHelperSvc->sector(id);
            auto hashes = getHashes(id);
            // fill current sector
            fill(ctx, state.truthHits, state.houghDataPerSectorVec->tgcClusteringObjs, *col,
                 state.houghDataPerSectorVec->vec[sector - 1].hitVec[hashes.second],
                 state.houghDataPerSectorVec->vec[sector - 1].phiHitVec[toInt(hashes.first)], sector);

            // fill neighbours if in overlap
            int neighbourSectorDown = sector == 1 ? 16 : sector - 1;
            if (hashInSector(col->identifyHash(), neighbourSectorDown, hashes.second))
                fill(ctx, state.truthHits, state.houghDataPerSectorVec->tgcClusteringObjs, *col,
                     state.houghDataPerSectorVec->vec[neighbourSectorDown - 1].hitVec[hashes.second],
                     state.houghDataPerSectorVec->vec[neighbourSectorDown - 1].phiHitVec[toInt(hashes.first)], neighbourSectorDown);

            int neighbourSectorUp = sector == 16 ? 1 : sector + 1;
            if (hashInSector(col->identifyHash(), neighbourSectorUp, hashes.second))
                fill(ctx, state.truthHits, state.houghDataPerSectorVec->tgcClusteringObjs, *col,
                     state.houghDataPerSectorVec->vec[neighbourSectorUp - 1].hitVec[hashes.second],
                     state.houghDataPerSectorVec->vec[neighbourSectorUp - 1].phiHitVec[toInt(hashes.first)], neighbourSectorUp);
        }

        return analyse(state);
    }

    std::pair<std::unique_ptr<MuonPatternCombinationCollection>, std::unique_ptr<HoughDataPerSectorVec>> MuonLayerHoughTool::find(
        const MdtPrepDataContainer* mdtCont, const CscPrepDataContainer* cscCont, const TgcPrepDataContainer* tgcCont,
        const RpcPrepDataContainer* rpcCont, const sTgcPrepDataContainer* stgcCont, const MMPrepDataContainer* mmCont,
        const EventContext& ctx) const {
        initializeSectorMapping(ctx);
        State state;
        ATH_MSG_DEBUG("MuonLayerHoughTool::analyse");

        state.houghDataPerSectorVec->vec.resize(16);

        // loops over all sectors, contains hashes for technology and region and chamber (?)
        for (const CollectionsPerSector& sit : m_collectionsPerSector) {
            ATH_MSG_DEBUG("analyse: Filling hits sector " << sit.sector);

            HoughDataPerSector& houghData = state.houghDataPerSectorVec->vec[sit.sector - 1];
            houghData.sector = sit.sector;

            // fill hits for this sector -> hitsVec and PhiHitsVec are known now
            fillHitsPerSector(ctx, state, sit.sector, sit, mdtCont, cscCont, tgcCont, rpcCont, stgcCont, mmCont);
        }
        return analyse(state);
    }

    std::pair<std::unique_ptr<MuonPatternCombinationCollection>, std::unique_ptr<HoughDataPerSectorVec>> MuonLayerHoughTool::analyse(
        State& state) const {
        auto patternCombis = std::make_unique<MuonPatternCombinationCollection>();

        // loop over data and fill the hough transform
        for (auto& houghData : state.houghDataPerSectorVec->vec) {
            ATH_MSG_DEBUG("analyse: Filling Hough sector " << houghData.sector);


            // loop over all possible station layers in the sector and run the eta transform
            for (unsigned int layerHash = 0; layerHash < MuonStationIndex::sectorLayerHashMax(); ++layerHash) {
                using namespace MuonStationIndex;
                // get hits for layer, skip empty layers
                HitVec& hits = houghData.hitVec.at(layerHash);
                if (hits.empty()) continue;

                // decompose hash, calculate indices etc
                const auto [region , layer] = decomposeSectorLayerHash(layerHash);
                StIndex index = toStationIndex(region, layer);

                // get Hough transform
                MuonHough::MuonLayerHough& hough =
                    state.houghDataPerSectorVec->detectorHoughTransforms.hough(houghData.sector, region, layer);

                ATH_MSG_VERBOSE("analyse: Filling Summary: loc s" << houghData.sector << " " << regionName(region) << " "
                                                                << layerName(layer)<<" -> hash: "<<layerHash << " -> stIndex: "
                                                                << stName(index) << " etaHits: " << hits.size());

                // look for maxima using hough in eta per layer
                if (!findMaxima(state.seedMaxima, hough, hits, houghData.maxVec.at(layerHash)) ||
                    houghData.maxVec.at(layerHash).empty())
                    continue;

                ++houghData.nlayersWithMaxima[toInt(region)];
                houghData.nmaxHitsInRegion[toInt(region)] += houghData.maxVec[layerHash].front()->max;

                ATH_MSG_VERBOSE("analyse: Eta maxima Summary: loc s"
                              << houghData.sector << " " << regionName(region) << " "
                              << layerName(layer) << " -> stIndex: " << stName(index)
                              << " hash: " << layerHash << " nMaxima: " << houghData.maxVec[layerHash].size());
            }  // loop over layerHash -> maxima per layer in eta are known now
        }      // loop over sectors

        if (m_useSeeds) {
            std::vector<Road> roads;
            buildRoads(state.seedMaxima, state.houghDataPerSectorVec->detectorHoughTransforms, 
                       state.houghDataPerSectorVec, roads);

            // create association map
            ATH_MSG_DEBUG("analyse: Building pattern combinations using roads " << roads.size());
            for (auto& road : roads) {
                std::map<MuonHough::MuonPhiLayerHough::Maximum*, MuonLayerHoughTool::MaximumVec> phiEtaAssMap;
                MuonLayerHoughTool::RegionMaximumVec unassociatedEtaMaxima;

                int sector = road.seed->hough->m_descriptor.sector;
                using namespace MuonStationIndex;
                ChIndex chIndex = road.seed->hough->m_descriptor.chIndex;
                LayerIndex layer = toLayerIndex(chIndex);
                DetectorRegionIndex region = road.seed->hough->m_descriptor.region;
                ATH_MSG_DEBUG("analyse: Seeding new road: eta maxima "
                              << road.maxima.size() << " phi " << road.phiMaxima.size() << " seed : sector " << sector << " "
                              << regionName(region) << " " << layerName(layer)
                              << " maximum " << road.seed->max << " position " << road.seed->pos << " angle " << road.seed->theta);

                if (road.phiMaxima.empty())
                    unassociatedEtaMaxima.push_back(road.maxima);
                else {
                    for (auto& max : road.mergedPhiMaxima) { phiEtaAssMap[&max] = road.maxima; }
                }
                createPatternCombinations(phiEtaAssMap, *patternCombis);
                createPatternCombinations(unassociatedEtaMaxima, *patternCombis);
            }

        } else {
            // now that the full hough transform is filled, order sectors by maxima
            std::vector<HoughDataPerSector*> sectorData(state.houghDataPerSectorVec->vec.size());
            for (unsigned int i = 0; i < state.houghDataPerSectorVec->vec.size(); ++i) sectorData[i] = &state.houghDataPerSectorVec->vec[i];
            std::stable_sort(sectorData.begin(), sectorData.end(), SortHoughDataPerSector());

            std::vector<HoughDataPerSector*>::iterator spit = sectorData.begin();
            std::vector<HoughDataPerSector*>::iterator spit_end = sectorData.end();
            for (; spit != spit_end; ++spit) {
                // get data for this sector
                HoughDataPerSector& houghData = **spit;
                using namespace MuonStationIndex;

                // loop over regions
                for (int reg = 0; reg < toInt(DetectorRegionIndex::DetectorRegionIndexMax); ++reg) {
                    DetectorRegionIndex region = static_cast<DetectorRegionIndex>(reg);

                    // only run analysis on sectors with maxima
                    if (houghData.nlayersWithMaxima[reg] == 0) continue;
                    ATH_MSG_DEBUG("Analyzing sector "
                                  << (*spit)->sector << " " << regionName(region) << " nmax " << (*spit)->maxEtaHits()
                                  << " layers with eta maxima " << houghData.nlayersWithMaxima[toInt(region)] << " hits "
                                  << houghData.nmaxHitsInRegion[toInt(region)] << " layers with phi maxima "
                                  << houghData.nphilayersWithMaxima[toInt(region)] << " hits " << houghData.nphimaxHitsInRegion[toInt(region)]);

                    // look for maxima in the overlap regions of sectors
                    associateMaximaInNeighbouringSectors(houghData, state.houghDataPerSectorVec->vec);

                    // layers in this region
                    constexpr int nlayers = toInt(LayerIndex::LayerIndexMax);

                    // first link phi maxima with eta maxima
                    RegionMaximumVec unassociatedEtaMaxima(nlayers);
                    std::map<MuonHough::MuonPhiLayerHough::Maximum*, MaximumVec> phiEtaAssociations;
                    associateMaximaToPhiMaxima(region, houghData, phiEtaAssociations, unassociatedEtaMaxima);

                    // create pattern combinations for combined patterns
                    createPatternCombinations(phiEtaAssociations, *patternCombis);

                    // create pattern combinations for unassociated patterns
                    createPatternCombinations(unassociatedEtaMaxima, *patternCombis);
                }
            }
        }

        ATH_MSG_DEBUG("Found " << patternCombis->size() << " pattern combinations " << std::endl << m_printer->print(*patternCombis));

        if (msgLvl(MSG::DEBUG)) {
            ATH_MSG_DEBUG("Hough performance ");
            printTruthSummary(state.truthHits, state.foundTruthHits);
            ATH_MSG_DEBUG("Association performance ");
            printTruthSummary(state.foundTruthHits, state.outputTruthHits);
        }

        return {std::move(patternCombis), std::move(state.houghDataPerSectorVec)};
    }

    void MuonLayerHoughTool::buildRoads(MaximumVec& seedMaxima, MuonHough::MuonDetectorHough& detectorHoughTransforms,
                                        std::unique_ptr<HoughDataPerSectorVec>& houghDataPerSectorVec,
                                        std::vector<MuonLayerHoughTool::Road>& roads) const {
        // sort maxima according to hits
        std::stable_sort(seedMaxima.begin(), seedMaxima.end(),
                         [](const std::shared_ptr<MuonHough::MuonLayerHough::Maximum>& m1,
                            const std::shared_ptr<MuonHough::MuonLayerHough::Maximum>& m2) { return m1->max > m2->max; });
        // loop over seed maxima (which are maxima) that pass certain thresholds detailed in cut_values
        std::set<std::shared_ptr<MuonHough::MuonLayerHough::Maximum>> associatedMaxima;
        for (const auto& seed : seedMaxima) {
            // if this maximum is already in the set of associated maxima, do not do anything
            if (associatedMaxima.count(seed)) continue;

            // maximum becomes our new seed

            // decomposing the locality information for the seed
            int sector = seed->hough->m_descriptor.sector;
            MuonStationIndex::ChIndex chIndex = seed->hough->m_descriptor.chIndex;
            MuonStationIndex::LayerIndex layer = MuonStationIndex::toLayerIndex(chIndex);
            using DetRegIdx = DetRegIdx;
            DetRegIdx region = seed->hough->m_descriptor.region;

            // creating new road with said seed
            Road road(seed);
            ATH_MSG_DEBUG(" New seed: sector " << seed->hough->m_descriptor.sector << " " << MuonStationIndex::regionName(region)
                                               << " " << MuonStationIndex::layerName(layer) << " maximum " << seed->max
                                               << " position " << seed->pos << " angle " << seed->theta << " ptr " << seed.get());
            /// In the NSW setup it can happen that the hits in the collection are only
            /// made up of TGCHits which are clustered beforehand and hence have no
            /// associated prd. In order to prevent later a crash in bool isNSW = ...
            /// let's first find a hit with associated PRD
            MuonHough::HitVec::const_iterator ref_itr = std::find_if(
                seed->hits.begin(), seed->hits.end(), [](const std::shared_ptr<MuonHough::Hit>& hit) -> bool { return hit->prd; });

            const bool isNSW = ref_itr != seed->hits.end() &&
                               (m_idHelperSvc->issTgc((*ref_itr)->prd->identify()) || m_idHelperSvc->isMM((*ref_itr)->prd->identify()));
            // extend seed within the current sector
            // sector indices have an offset of -1 because the numbering of the sectors are from 1 to 16 but the indices in the vertices are
            // of course 0 to 15
            extendSeed(detectorHoughTransforms, road, houghDataPerSectorVec->vec[sector - 1]);

            // look for maxima in the overlap regions of sectors
            int sectorN = sector - 1;
            if (sectorN < 1) sectorN = 16;
            int sectorP = sector + 1;
            if (sectorP > 16) sectorP = 1;

            // associate the road with phi maxima
            associatePhiMaxima(road, houghDataPerSectorVec->vec[sector - 1].phiMaxVec[toInt(region)]);
            //
            if (m_addSectors && isNSW) {
                extendSeed(detectorHoughTransforms, road, houghDataPerSectorVec->vec[sectorN - 1]);
                associatePhiMaxima(road, houghDataPerSectorVec->vec[sectorN - 1].phiMaxVec[toInt(region)]);
                extendSeed(detectorHoughTransforms, road, houghDataPerSectorVec->vec[sectorP - 1]);
                associatePhiMaxima(road, houghDataPerSectorVec->vec[sectorP - 1].phiMaxVec[toInt(region)]);
            }

            if (road.neighbouringRegion != DetRegIdx::DetectorRegionUnknown) {
                associatePhiMaxima(road, houghDataPerSectorVec->vec[sector - 1].phiMaxVec[toInt(road.neighbouringRegion)]);
            }
            // if close to a sector boundary, try adding maxima in that sector as well
            if (road.neighbouringSector != -1) {
                ATH_MSG_DEBUG("  Adding neighbouring sector " << road.neighbouringSector);
                extendSeed(detectorHoughTransforms, road,
                           houghDataPerSectorVec->vec[road.neighbouringSector - 1]);
                associatePhiMaxima(road, houghDataPerSectorVec->vec[road.neighbouringSector - 1].phiMaxVec[toInt(region)]);
            }

            // finally deal with the case that we have both neighbouring region and sector
            if (road.neighbouringRegion != DetRegIdx::DetectorRegionUnknown && road.neighbouringSector != -1) {
                associatePhiMaxima(road, houghDataPerSectorVec->vec[road.neighbouringSector - 1].phiMaxVec[toInt(road.neighbouringRegion)]);
            }

            // merge phi maxima
            mergePhiMaxima(road);

            // add maxima to seed exclusion list
            associatedMaxima.insert(road.maxima.begin(), road.maxima.end());

            if (msgLevel(MSG::DEBUG)) {
                ATH_MSG_DEBUG(" New road " << road.maxima.size());
                for (const auto& max : road.maxima) {
                    MuonStationIndex::ChIndex chIndex = max->hough->m_descriptor.chIndex;
                    MuonStationIndex::LayerIndex layer = MuonStationIndex::toLayerIndex(chIndex);
                    DetRegIdx region = max->hough->m_descriptor.region;
                    ATH_MSG_DEBUG(" Sector " << max->hough->m_descriptor.sector << " " << MuonStationIndex::regionName(region) << " "
                                             << MuonStationIndex::layerName(layer) << " maximum " << max->max << " position "
                                             << max->pos << " angle " << max->theta << " ptr " << max);
                }
            }
            bool insert = true;
            for (auto& oldRoad : roads) {
                std::vector<std::shared_ptr<MuonHough::MuonLayerHough::Maximum>> intersection;
                std::set_intersection(oldRoad.maximumSet.begin(), oldRoad.maximumSet.end(), road.maximumSet.begin(), road.maximumSet.end(),
                                      std::back_inserter(intersection));
                unsigned int intersectionSize = intersection.size();
                unsigned int oldRoadSize = oldRoad.maximumSet.size();
                unsigned int roadSize = road.maximumSet.size();
                ATH_MSG_VERBOSE(" Overlap check " << intersectionSize << " old " << oldRoadSize << " new " << roadSize << " old ptr "
                                                  << oldRoad.seed);
                if (intersectionSize == 0) continue;
                if (intersectionSize == roadSize) {
                    insert = false;  // discard
                    break;
                } else if (intersectionSize == oldRoadSize) {
                    oldRoad = road;  // replace
                    insert = false;
                    break;
                }
            }

            // add road to list
            if (insert) roads.push_back(road);
        }
    }

    void MuonLayerHoughTool::mergePhiMaxima(MuonLayerHoughTool::Road& road) const {
        // input -> list of phiMaxima on road
        // returns some mergedPhiMaxima -> is this "summed" over layers?

        auto maximaSortingLambda = [road](const std::shared_ptr<MuonHough::MuonPhiLayerHough::Maximum>& m1,
                                          const std::shared_ptr<MuonHough::MuonPhiLayerHough::Maximum>& m2) {
            if (m1->max != m2->max) return m1->max > m2->max;
            // prefer the same sector as the seed sector
            if (m1->sector != m2->sector) return m1->sector == road.seed->hough->m_descriptor.sector;

            if (m1->hits.size() != m2->hits.size()) return m1->hits.size() < m2->hits.size();  // least hits -> most collimated maximum

            if (m1->pos != m2->pos) return m1->pos < m2->pos;

            if (std::abs(m1->binposmax - m1->binposmin) == std::abs(m2->binposmax - m2->binposmin)) {
                return (m1->binposmin) < (m2->binposmin);
            }
            return std::abs(m1->binposmax - m1->binposmin) < std::abs(m2->binposmax - m2->binposmin);
        };

        std::stable_sort(road.phiMaxima.begin(), road.phiMaxima.end(), maximaSortingLambda);

        ATH_MSG_VERBOSE("Merging phi maxima " << road.phiMaxima.size());
        std::set<MuonHough::MuonPhiLayerHough::Maximum*> associatedPhiMaxima;
        for (auto pit = road.phiMaxima.begin(); pit != road.phiMaxima.end(); ++pit) {  // loop over phi maxima
            if (associatedPhiMaxima.count((*pit).get())) continue;                     // check if maximum is already in associatedPhiMaxima
            associatedPhiMaxima.insert((*pit).get());
            MuonHough::MuonPhiLayerHough::Maximum phiMaximum = **pit;
            ATH_MSG_VERBOSE("  phi maxima " << phiMaximum.pos << " val " << phiMaximum.max);

            bool wasExtended = false;
            for (auto pit1 = pit + 1; pit1 != road.phiMaxima.end(); ++pit1) {
                if ((*pit1)->binposmax >= phiMaximum.binposmin && (*pit1)->binposmin <= phiMaximum.binposmax) {
                    ATH_MSG_VERBOSE("    merging maxima " << phiMaximum.pos << " val " << phiMaximum.max << " " << (*pit1)->pos << " val "
                                                          << (*pit1)->max);
                    phiMaximum.hits.insert(phiMaximum.hits.end(), (*pit1)->hits.begin(), (*pit1)->hits.end());
                    associatedPhiMaxima.insert((*pit1).get());
                    wasExtended = true;
                }
            }

            if (wasExtended) {
                // refind maximum
                MuonHough::MuonPhiLayerHough localHough(
                    60, -M_PI, M_PI, ((*pit)->hough ? (*pit)->hough->m_region : DetRegIdx::DetectorRegionUnknown));
                MuonHough::PhiHitVec hits = phiMaximum.hits;
                /* too ambiguous producing irreproducibilities because of sorting by pointer value
                std::stable_sort(hits.begin(),hits.end(),[]( const MuonHough::PhiHit* h1,
                                                             const MuonHough::PhiHit* h2 ){ return h1->layer < h2->layer; } );
                */

                std::stable_sort(hits.begin(), hits.end(),
                                 [](const std::shared_ptr<MuonHough::PhiHit>& h1, const std::shared_ptr<MuonHough::PhiHit>& h2) {
                                     if (h1->layer != h2->layer) return h1->layer < h2->layer;
                                     if (h1->w != h2->w) return h1->w > h2->w;
                                     if (h1->r != h2->r) return h1->r < h2->r;

                                     const double dPhi1 = std::abs(h1->phimax - h1->phimin);
                                     const double dPhi2 = std::abs(h2->phimax - h2->phimin);
                                     if (dPhi1 != dPhi2) return dPhi1 < dPhi2;
                                     if (h1->phimin == h2->phimin) return h1->phimax < h2->phimax;
                                     return h1->phimin < h2->phimin;
                                 });

                ATH_MSG_VERBOSE("  updating phi maximum " << phiMaximum.pos << " bin " << phiMaximum.binpos << " val " << phiMaximum.max
                                                          << " number of hits " << hits.size());
                if (msgLvl(MSG::VERBOSE)) localHough.setDebug(true);
                localHough.fillLayer2(hits);
                localHough.findMaximum(phiMaximum, 0.9);
                localHough.associateHitsToMaximum(phiMaximum, hits);
                ATH_MSG_VERBOSE("  updated phi maxima " << phiMaximum.pos << " bin " << phiMaximum.binpos << " val " << phiMaximum.max
                                                        << " number of hits " << phiMaximum.hits.size());
                phiMaximum.hough = (*pit)->hough;  // set back pointer to transform
            }
            road.mergedPhiMaxima.push_back(phiMaximum);
        }
    }

    // maximum in middle layer
    // says look in other layers
    // if yes, combine them
    // gets on road
    // roads are combinations of maxima

    void MuonLayerHoughTool::extendSeed(MuonHough::MuonDetectorHough& detectorHoughTransforms, 
                                        MuonLayerHoughTool::Road& road,
                                        MuonLayerHoughTool::HoughDataPerSector& sectorData) const {  // const {
        if (!road.seed) return;

        RegionMaximumVec& maxVec = sectorData.maxVec;

        // gather locality information on seed
        MuonHough::MuonLayerHough::Maximum& seed = *road.seed;
        MuonStationIndex::LayerIndex seedLayer = MuonStationIndex::toLayerIndex(seed.hough->m_descriptor.chIndex);
        DetRegIdx region = seed.hough->m_descriptor.region;

        // loop over layers in the same region as the seed ( inner, middle, outer)
        for (int lay = 0; lay < toInt(LayerIndex::LayerIndexMax); ++lay) {
            MuonStationIndex::LayerIndex layer = static_cast<MuonStationIndex::LayerIndex>(lay);
            if (layer == seedLayer && seed.hough->m_descriptor.sector == sectorData.sector) continue;

            // untrue -> look in neighboring layer
            // true -> look only in this layer
            double distanceCut = layer == seedLayer ? 500. : (double)m_extrapolationDistance;

            unsigned int layerHash = MuonStationIndex::sectorLayerHash(region, layer);

            // fetching vector of maxima for given region and layer
            const MaximumVec& maxima = maxVec[layerHash];
            if (maxima.empty()) continue;

            ATH_MSG_DEBUG("Associating maxima in " << MuonStationIndex::regionName(region) << " " << MuonStationIndex::layerName(layer)
                                                   << " size " << maxima.size());
            // loop over maxima in layer
            for (const auto& candMaximum : maxima) {
                // extrapolate seed to layer assuming a pointing straight line or parabolic
                // add maximum to road if close enough
                float yloc_diff = MuonHough::extrapolate(seed, *candMaximum, m_doParabolicExtrapolation);
                if (std::abs(MuonHough::extrapolate(seed, *candMaximum, m_doParabolicExtrapolation)) < distanceCut) {
                    ATH_MSG_VERBOSE(" Adding maximum position " << candMaximum->pos << " intersect diff" << yloc_diff);
                    road.add(candMaximum);
                } else {
                    ATH_MSG_VERBOSE(" Maximum position: y "
                                    << candMaximum->pos << " x " << candMaximum->hough->m_descriptor.referencePosition << " seed y "
                                    << seed.hough->m_descriptor.referencePosition << " x " << seed.pos << " intersect diff " << yloc_diff);
                }
            }
        }

        // check if the maximum is close to the detector boundary, if yes look for maxima in the neighbouring region, skip BarrelExtended
        if (seedLayer == LayerIndex::BarrelExtended) return;

        ATH_MSG_DEBUG("Checking Barrel/Endcap overlaps: min dist edge "
                      << seed.pos - seed.hough->m_descriptor.yMinRange << " max dist edge " << seed.pos - seed.hough->m_descriptor.yMaxRange
                      << " pos " << seed.pos << " range " << seed.hough->m_descriptor.yMinRange << " "
                      << seed.hough->m_descriptor.yMaxRange);

        if (std::abs(seed.pos - seed.hough->m_descriptor.yMinRange) < 4000. ||
            std::abs(seed.pos - seed.hough->m_descriptor.yMaxRange) < 4000.) {
            // asumes region is barrel and looks in adjacent regions (clever logic TM here)
            DetRegIdx neighbourRegion = DetRegIdx::Barrel;
            if (region == DetRegIdx::Barrel) {
                if (seed.pos < 0)
                    neighbourRegion = DetRegIdx::EndcapC;
                else
                    neighbourRegion = DetRegIdx::EndcapA;
            }  // in all other cases the neigbourRegion is definitely barrel

            // looping over all layers in neigbouring region
            for (int lay = 0; lay < toInt(LayerIndex::LayerIndexMax); ++lay) {
                MuonStationIndex::LayerIndex layer = static_cast<MuonStationIndex::LayerIndex>(lay);

                // skip barrel combinations with BEE
                if (region == DetRegIdx::Barrel && layer == LayerIndex::BarrelExtended) continue;

                double distanceCut = 1000.;

                // get maxima from neigboring region
                unsigned int layerHash = MuonStationIndex::sectorLayerHash(neighbourRegion, layer);
                const MaximumVec& maxima = maxVec[layerHash];
                if (maxima.empty()) continue;
                ATH_MSG_DEBUG("Associating maxima in neighbouring region " << MuonStationIndex::regionName(neighbourRegion) << " "
                                                                           << MuonStationIndex::layerName(layer) << " hash " << layerHash
                                                                           << " size " << maxima.size());

                // loop over maxima per layer
                for (const auto& candMaximum : maxima) {
                    // extrapolate seed to layer assuming a pointing straight line, swap coordinates
                    float yloc_diff = MuonHough::extrapolate(seed, *candMaximum, m_doParabolicExtrapolation);
                    ATH_MSG_VERBOSE(" Maximum position: y "
                                    << candMaximum->pos << " x " << candMaximum->hough->m_descriptor.referencePosition << " seed y "
                                    << seed.hough->m_descriptor.referencePosition << " x " << seed.pos << " intersect diff " << yloc_diff);

                    if (std::abs(yloc_diff) < distanceCut) {
                        road.add(candMaximum);
                        road.neighbouringRegion = neighbourRegion;
                    }
                }
            }
        }

        // search for phiMaxima using the etaMaximum of the road in the current sector
        std::set<const TgcClusterObj3D*> tgcClusters;
        std::set<Identifier> triggerLayers;
        const MaximumVec& maxima = road.maxima;
        for (const auto& maximum : maxima) {
            if (maximum->hough->m_descriptor.sector != sectorData.sector)
                continue;  // skip cases where a maximum on the road does not belong to the currently examined sector

            // gather tgcClusters associated to the hits of the maxima
            for (auto ehit = maximum->hits.begin(); ehit != maximum->hits.end(); ++ehit) {
                const MuonHough::Hit& etaHit = **ehit;
                if (etaHit.tgc) {
                    if (!etaHit.tgc->phiCluster.empty()) tgcClusters.insert(etaHit.tgc);
                } else if (etaHit.prd) {
                    triggerLayers.insert(m_idHelperSvc->gasGapId(etaHit.prd->identify()));
                }
            }
        }

        MuonHough::MuonPhiLayerHough& phiHough =
            detectorHoughTransforms.phiHough(region);  // get phi transform in the same region as the seed

        // gather phiHits in sector that match the etaHits of the maximum
        PhiHitVec phiHitsInMaximum;
        PhiHitVec& phiHits = sectorData.phiHitVec[toInt(region)];
        for (const auto& phiHit : phiHits) {
            if (phiHit->tgc) {
                if (tgcClusters.find(phiHit->tgc) != tgcClusters.end()) phiHitsInMaximum.push_back(phiHit);
            } else if (phiHit->prd) {
                if (triggerLayers.find(m_idHelperSvc->gasGapId(phiHit->prd->identify())) != triggerLayers.end())
                    phiHitsInMaximum.push_back(phiHit);
            }
        }

        // fill phi hits
        ATH_MSG_DEBUG("extendSeed: Filling s" << sectorData.sector << " " << MuonStationIndex::regionName(region) << " phiHitsInMaxima "
                                              << phiHitsInMaximum.size() << " phi hits:  " << phiHits.size());

        if (!findMaxima(phiHough, phiHitsInMaximum, sectorData.phiMaxVec[toInt(region)], sectorData.sector) ||
            sectorData.phiMaxVec[toInt(region)].empty()) {
            ATH_MSG_DEBUG("extendSeed: No phi maxima found in  s" << sectorData.sector << " " << MuonStationIndex::regionName(region));
            return;
        }

        ++sectorData.nphilayersWithMaxima[toInt(region)];
        sectorData.nphimaxHitsInRegion[toInt(region)] += sectorData.phiMaxVec[toInt(region)].front()->max;

        ATH_MSG_DEBUG("extendSeed: Sector phiMaxima Summary:  s" << sectorData.sector << " " << MuonStationIndex::regionName(region) << " "
                                                                 << sectorData.nphilayersWithMaxima[toInt(region)]
                                                                 << " -> nPhiMaxima: " << sectorData.phiMaxVec[toInt(region)].size()
                                                                 << " max sum: " << sectorData.nphimaxHitsInRegion[toInt(region)]);
    }

    // phi hits are not separated into inner middle outer
    // maxima found in road
    void MuonLayerHoughTool::associatePhiMaxima(MuonLayerHoughTool::Road& road, MuonLayerHoughTool::PhiMaximumVec& phiMaxima) const {
        ATH_MSG_DEBUG("associateMaximaToPhiMaxima: phi maxima " << phiMaxima.size());
        if (!road.seed) return;

        // loop over phi maxima
        for (const auto& pmaximum : phiMaxima) {
            // reference to phi maximum

            ATH_MSG_DEBUG(" new phi maximum " << pmaximum->max << " hits " << pmaximum->hits.size());

            // precalculate the layers + TGC clusters and add them to a set for easy access
            std::map<Identifier, std::pair<float, float>> triggerLayersPhiMinMax;
            std::map<MuonStationIndex::StIndex, std::set<const TgcClusterObj3D*>> tgcClusters;

            // loop over hits
            for (const auto& phiHit : pmaximum->hits) {
                // two cases
                // case 1: phiHit measured in TGC -> get phiHits from phiCluster
                // case 2: phiHit is prepared raw data -> use phiHit to extend the triggerLayersPhinMinMax map
                if (phiHit->tgc) {
                    if (phiHit->tgc->phiCluster.empty())
                        ATH_MSG_WARNING(" TGC 3D cluster without phi hits ");
                    else
                        tgcClusters[m_idHelperSvc->stationIndex(phiHit->tgc->phiCluster.front()->identify())].insert(phiHit->tgc);
                } else if (phiHit->prd) {
                    Identifier gpId = m_idHelperSvc->gasGapId(phiHit->prd->identify());
                    auto mit = triggerLayersPhiMinMax.find(gpId);
                    if (mit == triggerLayersPhiMinMax.end())
                        triggerLayersPhiMinMax[gpId] = std::make_pair(phiHit->phimin, phiHit->phimax);
                    else {
                        mit->second.first = std::min(phiHit->phimin, mit->second.first);
                        mit->second.second = std::max(phiHit->phimax, mit->second.second);
                    }
                }
            }
            // print out information on the triggerLayersPhiMinMax
            if (msgLevel(MSG::VERBOSE)) {
                ATH_MSG_DEBUG("Trigger layers " << triggerLayersPhiMinMax.size() << " tgc layers " << tgcClusters.size());
                for (auto tgcit = triggerLayersPhiMinMax.begin(); tgcit != triggerLayersPhiMinMax.end(); ++tgcit) {
                    ATH_MSG_VERBOSE("  " << m_idHelperSvc->toString(tgcit->first));
                }

                // loop over the stations and the contained tgcClusters found in the previous step, print out information
                std::map<MuonStationIndex::StIndex, std::set<const TgcClusterObj3D*>>::const_iterator stit = tgcClusters.begin();
                std::map<MuonStationIndex::StIndex, std::set<const TgcClusterObj3D*>>::const_iterator stit_end = tgcClusters.end();
                for (; stit != stit_end; ++stit) {
                    std::set<const TgcClusterObj3D*>::const_iterator ttit = stit->second.begin();
                    std::set<const TgcClusterObj3D*>::const_iterator ttit_end = stit->second.end();
                    for (; ttit != ttit_end; ++ttit) {
                        ATH_MSG_VERBOSE("  " << m_idHelperSvc->toString((*ttit)->phiCluster.front()->identify()) << "  nhits "
                                             << (*ttit)->phiCluster.size());
                    }
                }
            }

            // check if there are matching maxima in neighbouring sectors, add maximum values if confirmation is found
            // overlap counters
            int noverlaps = 0;
            int nNoOverlaps = 0;
            float phimin{10}, phimax{-10};

            // loop over all maxima found on road
            for (const auto& road_max : road.maxima) {
                // get station information for maximum on road
                MuonStationIndex::StIndex stIndex = MuonStationIndex::toStationIndex(road_max->hough->m_descriptor.chIndex);

                // loop over eta hits
                for (const auto& etaHit : road_max->hits) {
                    if (etaHit->tgc) {
                        if (etaHit->tgc->etaCluster.empty())
                            ATH_MSG_WARNING(" TGC 3D cluster without eta hits ");
                        else {
                            if (tgcClusters[stIndex].count(etaHit->tgc)) {
                                // now loop over phi maximum and find phi hit
                                for (const auto& phiHit : pmaximum->hits) {
                                    if (phiHit->tgc == etaHit->tgc) {
                                        phimin = std::min(phiHit->phimin, phimin);
                                        phimax = std::max(phiHit->phimax, phimax);
                                        break;
                                    }
                                }
                                ++noverlaps;
                            } else {
                                ++nNoOverlaps;
                            }
                        }
                    } else if (etaHit->prd) {
                        if (!m_idHelperSvc->isRpc(etaHit->prd->identify()) && !m_idHelperSvc->issTgc(etaHit->prd->identify())) continue;
                        Identifier gpId = m_idHelperSvc->gasGapId(etaHit->prd->identify());
                        auto mit = triggerLayersPhiMinMax.find(gpId);
                        if (mit == triggerLayersPhiMinMax.end())
                            ++nNoOverlaps;
                        else {
                            phimin = std::min(mit->second.first, phimin);
                            phimax = std::max(mit->second.second, phimax);
                            ++noverlaps;
                        }
                    }
                }  // loop over hits in maximum
            }      // loop over maxima in road

            // if overlaps are found, add the phi maximum in question to the road
            if (noverlaps > 0) {
                road.add(pmaximum);
                // check if we are close to a sector boundary
                std::vector<int> sectors;
                m_sectorMapping.getSectors(phimin, sectors);
                if (sectors.size() > 1) {
                    for (const int& sec : sectors) {
                        if (sec != road.seed->hough->m_descriptor.sector) road.neighbouringSector = sec;
                    }
                } else {
                    std::vector<int> sectors;
                    m_sectorMapping.getSectors(phimax, sectors);
                    if (sectors.size() > 1) {
                        for (const int& sec : sectors) {
                            if (sec != road.seed->hough->m_descriptor.sector) road.neighbouringSector = sec;
                        }
                    }
                }
            }
            ATH_MSG_DEBUG(" Overlap with Phi maximum: overlap " << noverlaps << " no overlap " << nNoOverlaps << " phimin " << phimin
                                                                << " phimax " << phimax << " neighbouring sector "
                                                                << road.neighbouringSector);
        }
    }

    // takes the maxima from a given sector and tries to associate it with the maxima of the adjacent sectors
    void MuonLayerHoughTool::associateMaximaInNeighbouringSectors(
        MuonLayerHoughTool::HoughDataPerSector& houghData,
        std::vector<MuonLayerHoughTool::HoughDataPerSector>& houghDataPerSectorVec) const {
        ATH_MSG_DEBUG(" looping over eta maxima");

        // now loop over eta maxima per layer
        for (unsigned int regLay = 0; regLay < houghData.maxVec.size(); ++regLay) {
            MaximumVec& maxima = houghData.maxVec[regLay];
            int sector = houghData.sector;

            // loop over two neighbouring sectors
            for (int i = 0; i < 2; ++i) {
                // calculate neighbouring sector index
                int sectorN = (i == 0) ? sector - 1 : sector + 1;
                if (i == 0 && sector == 1) sectorN = 16;
                if (i == 1 && sector == 16) sectorN = 1;

                MuonLayerHoughTool::HoughDataPerSector& houghDataN = houghDataPerSectorVec[sectorN - 1];

                MaximumVec& maximaN = houghDataN.maxVec[regLay];

                // loop over maxima in layer
                for (const auto& maximum : maxima) {
                    // reference to maximum

                    if (!maximum->hough) {
                        ATH_MSG_WARNING("Maximum without associated hough transform! ");
                        continue;
                    }

                    // loop over maxima per layer in neighbouring sector
                    for (const auto& maximumN : maximaN) {
                        // reference to maximum
                        if (!maximumN->hough) {
                            ATH_MSG_WARNING("Maximum without associated hough transform! ");
                            continue;
                        }

                        // calculate the position of the first maximum in the reference frame of the other sector
                        double rcor = maximumN->hough->m_descriptor.referencePosition *
                                      m_sectorMapping.transformRToNeighboringSector(maximum->pos, sector, sectorN) /
                                      maximum->hough->m_descriptor.referencePosition;
                        double dist = rcor - maximumN->pos;
                        ATH_MSG_DEBUG(" maximumN->hough " << maximumN->hough->m_descriptor.referencePosition << " maximum->hough "
                                                          << maximum->hough->m_descriptor.referencePosition << " maximumN->pos "
                                                          << maximumN->pos << " maximum->pos " << maximum->pos << rcor << " distance "
                                                          << dist);
                        if (std::abs(dist) > 100) continue;
                        houghData.maxAssociationMap[maximum.get()].push_back(maximumN);
                        houghDataN.associatedToOtherSector.insert(maximumN.get());

                        ATH_MSG_DEBUG(" Found maximum in neighbouring sector: max " << maximum->max << " pos " << rcor << " maxN "
                                                                                    << maximumN->max << " pos " << maximumN->pos
                                                                                    << " distance " << dist);

                        // loop over first and second maximum
                        for (int nn = 0; nn < 2; ++nn) {
                            // setting info for the debug-info objects of the hits
                            const auto& maxi = nn == 0 ? maximum : maximumN;
                            const auto& maxi2 = nn == 0 ? maximumN : maximum;
                            ATH_MSG_VERBOSE(" Maximum " << nn << " hits " << maxi->hits.size());
                            for (auto& hit : maxi->hits) {
                                if (hit->debugInfo()) {
                                    hit->debugInfo()->phn = maxi2->max;
                                    Identifier id = hit->tgc ? hit->tgc->etaCluster.front()->identify() : hit->prd->identify();
                                    ATH_MSG_VERBOSE(" " << m_idHelperSvc->toString(id) << " setphn " << hit->debugInfo()->phn);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void MuonLayerHoughTool::associateMaximaToPhiMaxima(
        DetRegIdx region, MuonLayerHoughTool::HoughDataPerSector& houghData,
        std::map<MuonHough::MuonPhiLayerHough::Maximum*, MuonLayerHoughTool::MaximumVec>& phiEtaAssociations,
        MuonLayerHoughTool::RegionMaximumVec& unassEtaMaxima) const {
        std::set<std::shared_ptr<MuonHough::MuonLayerHough::Maximum>> associatedMaxima;

        PhiMaximumVec& phiMaxima = houghData.phiMaxVec[MuonStationIndex::toInt(region)];

        ATH_MSG_DEBUG("associateMaximaToPhiMaxima in sector " << houghData.sector << ": phi maxima " << phiMaxima.size());  // !!!!
        // loop over phi maxima
        for (const auto& phiMaximum : phiMaxima) {
            // reference to phi maximum

            ATH_MSG_DEBUG(" Considering phi maximum " << phiMaximum->max << " hits " << phiMaximum->hits.size());  // !!!!

            // store associated maxima
            MaximumVec associatedMaximaVec;  // !!!!

            // precalculate the layers + TGC clusters and add them to a set for easy access
            // std::map< Identifier,std::pair<double,double> > triggerLayersP;
            std::set<Identifier> triggerLayers;
            std::map<MuonStationIndex::StIndex, std::set<const TgcClusterObj3D*>> tgcClusters;

            // loop over hits
            for (const auto& phiHit : phiMaximum->hits) {
                if (phiHit->tgc) {
                    if (phiHit->tgc->phiCluster.empty())
                        ATH_MSG_WARNING(" TGC 3D cluster without phi hits ");
                    else
                        tgcClusters[m_idHelperSvc->stationIndex(phiHit->tgc->phiCluster.front()->identify())].insert(phiHit->tgc);
                } else if (phiHit->prd) {
                    Identifier colId = phiHit->prd->identify();
                    Identifier layId = m_idHelperSvc->gasGapId(colId);
                    triggerLayers.insert(layId);
                }
            }
            if (msgLvl(MSG::DEBUG)) {
                ATH_MSG_DEBUG("Trigger layers " << triggerLayers.size() << " tgc layers " << tgcClusters.size());
                for (const Identifier& id : triggerLayers) { ATH_MSG_VERBOSE("  " << m_idHelperSvc->toString(id)); }

                std::map<MuonStationIndex::StIndex, std::set<const TgcClusterObj3D*>>::const_iterator stit = tgcClusters.begin();
                std::map<MuonStationIndex::StIndex, std::set<const TgcClusterObj3D*>>::const_iterator stit_end = tgcClusters.end();
                for (; stit != stit_end; ++stit) {
                    std::set<const TgcClusterObj3D*>::const_iterator ttit = stit->second.begin();
                    std::set<const TgcClusterObj3D*>::const_iterator ttit_end = stit->second.end();
                    for (; ttit != ttit_end; ++ttit) {
                        ATH_MSG_VERBOSE("  " << m_idHelperSvc->toString((*ttit)->phiCluster.front()->identify()) << "  nhits "
                                             << (*ttit)->phiCluster.size());
                    }
                }
            }

            ATH_MSG_DEBUG(" looping over eta maxima");

            // now loop over eta maxima per layer
            for (unsigned int lay = 0; lay < toInt(LayerIndex::LayerIndexMax); ++lay) {
                MuonStationIndex::LayerIndex layer = static_cast<MuonStationIndex::LayerIndex>(lay);
                unsigned int layerHash = MuonStationIndex::sectorLayerHash(region, layer);
                MaximumVec& maxima = houghData.maxVec[layerHash];
                if (maxima.empty()) continue;
                MuonStationIndex::StIndex stIndex = MuonStationIndex::toStationIndex(region, layer);

                // loop over maxima per layer
                for (const auto& maximum : maxima) {
                    // skip maxima that were already associated to a neighbouring sector
                    if (houghData.associatedToOtherSector.count(maximum.get())) continue;

                    // check if there are matching maxima in neighbouring sectors, add maximum values if confirmation is found
                    float totmax = 0;
                    int ntrigconfirm = 0;
                    MaximumAssociationMap::iterator pos = houghData.maxAssociationMap.find(maximum.get());
                    if (pos != houghData.maxAssociationMap.end()) {
                        for (const auto& max_itr : pos->second) {
                            totmax = std::max(max_itr->max, totmax);
                            ntrigconfirm += max_itr->triggerConfirmed;
                        }
                    }
                    totmax += maximum->max;
                    ntrigconfirm += maximum->triggerConfirmed;

                    ATH_MSG_DEBUG("   new eta maximum " << MuonStationIndex::stName(stIndex) << " val " << maximum->max
                                                        << " neightbour confirmed value " << totmax << " trigger confirmations "
                                                        << ntrigconfirm);

                    // overlap counters
                    int nmmHits{0}, ntgcOverlaps{0}, nrpcOverlaps{0}, nstgcOverlaps{0}, nstgcNoOverlaps{0};

                    // loop over hits
                    for (const auto& etaHit : maximum->hits) {
                        if (etaHit->tgc) {
                            if (tgcClusters[stIndex].count(etaHit->tgc))
                                ++ntgcOverlaps;
                               
                        } else if (etaHit->prd) {
                            Identifier layId = m_idHelperSvc->gasGapId(etaHit->prd->identify());
                            ATH_MSG_VERBOSE(" eta layer hit " << m_idHelperSvc->toString(layId));
                            if (m_idHelperSvc->isMM(layId)) ++nmmHits;
                            if (triggerLayers.count(layId)) {
                                if (m_idHelperSvc->isRpc(layId))
                                    ++nrpcOverlaps;
                                else if (m_idHelperSvc->issTgc(layId))
                                    ++nstgcOverlaps;
                            } else {
                                if (m_idHelperSvc->issTgc(layId))
                                    ++nstgcNoOverlaps;
                            }
                        }
                    }

                    // cuts on NSW endcap only for now
                    if (nmmHits + nstgcNoOverlaps + nstgcOverlaps > 0) {
                        // select
                        if (maximum->pos < 1200.) {
                            if (totmax < 8) {
                                ATH_MSG_DEBUG("  maximum failed cut " << totmax << " cut 8, position " << maximum->pos);
                                continue;
                            }
                        } else if (maximum->pos > 4300.) {
                            if (totmax < 8) {
                                ATH_MSG_DEBUG("  maximum failed cut " << totmax << " cut 8, position " << maximum->pos);
                                continue;
                            }
                        } else {
                            if (totmax < 12) {
                                ATH_MSG_DEBUG("  maximum failed cut " << totmax << " cut 12, position " << maximum->pos);
                                continue;
                            }
                        }
                    }

                    ATH_MSG_DEBUG(" Overlap with Phi maximum: tgc " << ntgcOverlaps << " stgc " << nstgcOverlaps << " rpc " << nrpcOverlaps
                                                                    << " nphiTgc " << tgcClusters[stIndex].size() << " trigLay "
                                                                    << triggerLayers.size());
                    if (stIndex == StIndex::EM && !tgcClusters[stIndex].empty() && ntgcOverlaps == 0) {
                        ATH_MSG_VERBOSE(" No association in StationLayer " << MuonStationIndex::stName(stIndex) << " tgcs overlaps "
                                                                           << ntgcOverlaps << " on phi maximum "
                                                                           << tgcClusters[stIndex].size());
                        continue;
                    }
                    if (stIndex == StIndex::EI && !tgcClusters[stIndex].empty() && ntgcOverlaps == 0) {
                        ATH_MSG_VERBOSE(" No association in StationLayer " << MuonStationIndex::stName(stIndex) << " tgcs overlaps "
                                                                           << ntgcOverlaps << " on phi maximum "
                                                                           << tgcClusters[stIndex].size());
                        continue;
                    }
                    if (stIndex ==  StIndex::EI && nstgcOverlaps == 0 && nstgcNoOverlaps != 0) {
                        ATH_MSG_VERBOSE(" No association in StationLayer " << MuonStationIndex::stName(stIndex)
                                                                           << " stgcs without overlaps " << nstgcNoOverlaps);
                        continue;
                    }
                    // require STGC confirmation
                    if (m_requireTriggerConfirmationNSW && nmmHits > 0 && ntrigconfirm == 0) continue;

                    associatedMaxima.insert(maximum);
                    associatedMaximaVec.push_back(maximum);

                    // check if there are matching maxima in neighbouring sectors
                    if (pos != houghData.maxAssociationMap.end()) {
                        associatedMaxima.insert(pos->second.begin(), pos->second.end());
                        associatedMaximaVec.insert(associatedMaximaVec.end(), pos->second.begin(), pos->second.end());
                    }
                }
            }

            if (associatedMaximaVec.empty()) continue;
            ATH_MSG_DEBUG(" processed phi maximum, associated eta maxima " << associatedMaximaVec.size());
            phiEtaAssociations[phiMaximum.get()] = std::move(associatedMaximaVec);
        }

        // finally idenitify all unassociated maxima and add them to the unassociated maxima list
        // now loop over eta maxima per layer
        for (unsigned int lay = 0; lay < toInt(LayerIndex::LayerIndexMax); ++lay) {
            MuonStationIndex::LayerIndex layer = static_cast<MuonStationIndex::LayerIndex>(lay);
            unsigned int layerHash = MuonStationIndex::sectorLayerHash(region, layer);

            if (lay >= unassEtaMaxima.size()) {
                ATH_MSG_WARNING(" size of unassEtaMaxima too small for region " << unassEtaMaxima.size() << " region "
                                                                                << MuonStationIndex::regionName(region));
                break;
            }
            MaximumVec& maxima = houghData.maxVec[layerHash];

            // loop over maxima per layer
            for (const auto& mit : maxima) {
                if (associatedMaxima.count(mit)) continue;
                unassEtaMaxima[lay].push_back(mit);
                ATH_MSG_DEBUG(" unassociated maximum in layer " << MuonStationIndex::layerName(layer) << " max-val " << mit->max);
            }
        }
    }

    void MuonLayerHoughTool::createPatternCombinations(MuonLayerHoughTool::RegionMaximumVec& maxima,
                                                       MuonPatternCombinationCollection& patternCombis) const {
        ATH_MSG_DEBUG("Creating pattern combinations for eta patterns ");

        std::vector<MuonPatternChamberIntersect> chamberData;

        // bool isEndcap = maxima.size() == 5;

        // loop over layers
        for (const auto& max_sec : maxima) {
            // create vector for prds per chamber
            std::map<Identifier, std::set<const Trk::PrepRawData*>> prdsPerChamber;

            // loop over maxima per layer
            for (const auto& max : max_sec) {
                ATH_MSG_DEBUG("  new maximum  " << max->max << " hits " << max->hits.size());

                // sanity check
                if (max->hits.empty()) {
                    ATH_MSG_WARNING(" Maximum without hits  ");
                    continue;
                }
                ATH_MSG_DEBUG("  adding hits " << max->hits.size());

                // loop over hits in maximum and add them to the hit list
                for (const auto& hit : max->hits) {
                    if (hit->tgc) {
                        const Identifier chId = m_idHelperSvc->chamberId(hit->tgc->etaCluster.front()->identify());
                        prdsPerChamber[chId].insert(hit->tgc->etaCluster.begin(), hit->tgc->etaCluster.end());
                    } else if (hit->prd) {
                        const Identifier chId = m_idHelperSvc->chamberId(hit->prd->identify());
                        prdsPerChamber[chId].insert(hit->prd);
                    }
                }
            }

            auto sortPrdIds = [](const Trk::PrepRawData* prd1, const Trk::PrepRawData* prd2) {
                return prd1->identify() < prd2->identify();
            };
            std::map<Identifier, std::set<const Trk::PrepRawData*>>::iterator chit = prdsPerChamber.begin();
            std::map<Identifier, std::set<const Trk::PrepRawData*>>::iterator chit_end = prdsPerChamber.end();
            for (; chit != chit_end; ++chit) {
                ATH_MSG_DEBUG("Adding chamber " << m_idHelperSvc->toStringChamber(chit->first) << " hits " << chit->second.size());
                std::vector<const Trk::PrepRawData*> prds;
                prds.insert(prds.end(), chit->second.begin(), chit->second.end());
                std::stable_sort(prds.begin(), prds.end(), sortPrdIds);
                const Trk::PrepRawData& prd = **prds.begin();
                Amg::Vector3D gpos = prd.detectorElement()->surface(prd.identify()).center();
                // create intersection and add it to combination
                ATH_MSG_DEBUG("Adding chamber with intersect phi direction " << gpos.phi() << " theta " << gpos.theta());
                MuonPatternChamberIntersect intersect(gpos, gpos.unit(), prds);
                chamberData.push_back(intersect);
            }
        }
        if (chamberData.empty()) return;

        MuonPatternCombination* combi = new MuonPatternCombination(nullptr, chamberData);

        ATH_MSG_DEBUG(" creating new unassociated " << m_printer->print(*combi));
        patternCombis.push_back(combi);
    }

    void MuonLayerHoughTool::createPatternCombinations(
        std::map<MuonHough::MuonPhiLayerHough::Maximum*, MuonLayerHoughTool::MaximumVec>& phiEtaAssociations,
        MuonPatternCombinationCollection& patternCombis) const {
        ATH_MSG_DEBUG("Creating pattern combinations from eta/phi combinations " << phiEtaAssociations.size());

        // loop over the phi maxima
        std::map<MuonHough::MuonPhiLayerHough::Maximum*, MaximumVec>::const_iterator pit = phiEtaAssociations.begin();
        std::map<MuonHough::MuonPhiLayerHough::Maximum*, MaximumVec>::const_iterator pit_end = phiEtaAssociations.end();
        for (; pit != pit_end; ++pit) {
            if (pit->second.empty()) continue;

            // collect phi hits per chamber
            std::map<Identifier, std::set<const Trk::PrepRawData*>> phiHitsPerChamber;

            // loop over hits
            for (const auto& hit : pit->first->hits) {
                if (hit->tgc) {
                    const Identifier chId = m_idHelperSvc->chamberId(hit->tgc->phiCluster.front()->identify());
                    phiHitsPerChamber[chId].insert(hit->tgc->phiCluster.begin(), hit->tgc->phiCluster.end());
                } else if (hit->prd) {
                    const Identifier chId = m_idHelperSvc->chamberId(hit->prd->identify());
                    phiHitsPerChamber[chId].insert(hit->prd);
                }
            }

            // create chamber intersections
            std::vector<MuonPatternChamberIntersect> chamberData;
            std::set<Identifier> addedPhiHits;

            // create vector for prds per chamber
            std::map<Identifier, std::set<const Trk::PrepRawData*>> prdsPerChamber;

            // store position and direction of the first maximum in the chamber layer
            std::map<MuonStationIndex::ChIndex, std::pair<Amg::Vector3D, Amg::Vector3D>> directionsPerChamberLayer;

            // loop over eta maxima
            for (const auto& max : pit->second) {
                ATH_MSG_DEBUG("  new maximum  " << max->max << " hits " << max->hits.size());

                if (!max->hough) { ATH_MSG_WARNING("Maximum without associated Hough Transform"); }

                // sanity check
                if (max->hits.empty()) {
                    ATH_MSG_WARNING(" Maximum without hits  ");
                    continue;
                }
                ATH_MSG_DEBUG("  adding hits " << max->hits.size());

                // loop over hits in maximum and add them to the hit list
                for (const auto& hit : max->hits) {
                    Identifier chId;
                    if (hit->tgc) {
                        chId = m_idHelperSvc->chamberId(hit->tgc->etaCluster.front()->identify());
                        prdsPerChamber[chId].insert(hit->tgc->etaCluster.begin(), hit->tgc->etaCluster.end());
                    } else if (hit->prd) {
                        chId = m_idHelperSvc->chamberId(hit->prd->identify());
                        prdsPerChamber[chId].insert(hit->prd);
                    } else {
                        ATH_MSG_WARNING("Hit without associated PRDs");
                        continue;
                    }
                    // the first time we have a maximun in this layer store the position and direction
                    MuonStationIndex::ChIndex chIndex = m_idHelperSvc->chamberIndex(chId);
                    if (!directionsPerChamberLayer.count(chIndex)) {
                        // eta maximum has z(r) and theta parameters but these are local
                        double maxpos = max->pos;
                        double refPlane = 0.;
                        bool isBarrel = !m_idHelperSvc->isEndcap(chId) || chIndex == ChIndex::BEE;
                        if (max->hough)
                            refPlane = max->hough->m_descriptor.referencePosition;
                        else if (hit->tgc)
                            refPlane = hit->tgc->getEdge(TgcEdge::LowEtaLowPhi).z();
                        else if (isBarrel)
                            refPlane = hit->prd->detectorElement()->surface(hit->prd->identify()).center().perp();
                        else
                            refPlane = hit->prd->detectorElement()->surface(hit->prd->identify()).center().z();

                        double r = isBarrel ? refPlane : maxpos;
                        double z = isBarrel ? maxpos : refPlane;
                        double theta = max->theta;

                        // go to global
                        double sign = 1.;
                        if (isBarrel) {
                            theta += M_PI_2;
                            sign = -1.;
                        }

                        // phi maximum has one phi from position assume global Phi definition
                        double phi = pit->first->pos;  // phiCor(pit->first->pos,pit->first->sector,false);

                        CxxUtils::sincos scphi(phi);
                        double sinphi = scphi.sn;
                        double cosphi = scphi.cs;

                        CxxUtils::sincos sctheta(theta);
                        double sintheta = sctheta.sn;
                        double costheta = sctheta.cs;

                        std::pair<Amg::Vector3D, Amg::Vector3D>& posDir = directionsPerChamberLayer[chIndex];
                        posDir.first = Amg::Vector3D(r * cosphi, r * sinphi, z);
                        posDir.second = Amg::Vector3D(sign * cosphi * costheta, sign * sinphi * costheta, sintheta);
                        ATH_MSG_DEBUG(MuonStationIndex::chName(chIndex)
                                      << " setting position: perp " << posDir.first.perp() << " z " << posDir.first.z() << " phi pos "
                                      << posDir.first.phi() << " direction phi  " << posDir.second.phi() << " theta pos "
                                      << posDir.first.theta() << " direction theta " << posDir.second.theta() << " ref perp " << r << " z "
                                      << z << " phi " << phi << " theta " << theta);
                        if (posDir.first.dot(posDir.second) < 0.) {
                            ATH_MSG_WARNING(" direction not pointing to IP " << posDir.first.unit().dot(posDir.second));
                        }
                    }

                    std::map<Identifier, std::set<const Trk::PrepRawData*>>::iterator pos = phiHitsPerChamber.find(chId);
                    if (pos != phiHitsPerChamber.end()) {
                        std::pair<std::set<Identifier>::iterator, bool> ipos = addedPhiHits.insert(chId);
                        if (ipos.second) { prdsPerChamber[chId].insert(pos->second.begin(), pos->second.end()); }
                    }
                }
            }

            auto sortPrdIds = [](const Trk::PrepRawData* prd1, const Trk::PrepRawData* prd2) {
                return prd1->identify() < prd2->identify();
            };
            std::map<Identifier, std::set<const Trk::PrepRawData*>>::iterator chit = prdsPerChamber.begin();
            std::map<Identifier, std::set<const Trk::PrepRawData*>>::iterator chit_end = prdsPerChamber.end();
            for (; chit != chit_end; ++chit) {
                ATH_MSG_DEBUG("Adding chamber " << m_idHelperSvc->toStringChamber(chit->first) << " hits " << chit->second.size());
                std::vector<const Trk::PrepRawData*> prds;
                prds.insert(prds.end(), chit->second.begin(), chit->second.end());
                std::stable_sort(prds.begin(), prds.end(), sortPrdIds);
                const Trk::PrepRawData& prd = **prds.begin();

                MuonStationIndex::ChIndex chIndex = m_idHelperSvc->chamberIndex(prd.identify());
                std::map<MuonStationIndex::ChIndex, std::pair<Amg::Vector3D, Amg::Vector3D>>::const_iterator pos =
                    directionsPerChamberLayer.find(chIndex);
                Amg::Vector3D gpos{Amg::Vector3D::Zero()};
                Amg::Vector3D gdir{Amg::Vector3D::Zero()};
                if (pos != directionsPerChamberLayer.end()) {
                    gpos = pos->second.first;
                    gdir = pos->second.second;
                } else {
                    ATH_MSG_WARNING("No global position and direction found, calculating from surface");
                    gpos = prd.detectorElement()->surface(prd.identify()).center();
                    gdir = -1 * gpos.unit();
                }

                ATH_MSG_DEBUG("Creating intersection " << MuonStationIndex::chName(chIndex) << " setting position: perp " << gpos.perp()
                                                       << " z " << gpos.z() << " phi pos " << gpos.phi() << " direction phi " << gdir.phi()
                                                       << " theta pos " << gpos.theta() << " theta " << gdir.theta() << " hits "
                                                       << prds.size());

                // create intersection and add it to combination
                MuonPatternChamberIntersect intersect(gpos, gdir, prds);
                chamberData.push_back(intersect);

            }
            if (chamberData.empty()) continue;
            if (addedPhiHits.empty()) {
                ATH_MSG_DEBUG("No phi hits selected, skipping combi ");
                continue;
            }
            MuonPatternCombination* combi = new MuonPatternCombination(nullptr, chamberData);
            ATH_MSG_DEBUG("adding pattern combination with chambers " << chamberData.size() << " phi layers " << addedPhiHits.size()
                                                                      << std::endl
                                                                      << m_printer->print(*combi));
            patternCombis.push_back(combi);
        }
    }

    bool MuonLayerHoughTool::findMaxima(MaximumVec& seedMaxima,
                                        MuonHough::MuonLayerHough& hough, MuonLayerHoughTool::HitVec& hits,
                                        MuonLayerHoughTool::MaximumVec& maxima) const {
        if (hits.empty()) return false;

        using namespace MuonStationIndex;
        if (hough.m_descriptor.chIndex == ChIndex::ChUnknown || hough.m_descriptor.chIndex == ChIndex::ChIndexMax) {
            Identifier id = hits.front()->tgc ? hits.front()->tgc->etaCluster.front()->identify() : hits.front()->prd->identify();
            ATH_MSG_WARNING("Bad ChIndex " << m_idHelperSvc->toString(id) << "  " << chName(hough.m_descriptor.chIndex));
            return false;
        }

        // populate hough transform with hits
        std::stable_sort(hits.begin(), hits.end(), MuonHough::SortHitsPerLayer());
        if (m_debugHough) hough.setDebug(true);
        hough.fillLayer2(hits);

        Identifier id_hit = hits.front()->tgc ? hits.front()->tgc->etaCluster.front()->identify() : hits.front()->prd->identify();
        MuonHough::MuonLayerHoughSelector selectorLoose;
        MuonHough::MuonLayerHoughSelector selector;

        if (m_idHelperSvc->issTgc(id_hit) || m_idHelperSvc->isMM(id_hit)) {
            selectorLoose = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 3.9)});
            selector = MuonHough::MuonLayerHoughSelector({std::make_pair(0, 7.9)});
        } else {
            selectorLoose = m_selectorsLoose[toInt(hough.m_descriptor.chIndex)];
            selector = m_selectors[toInt(hough.m_descriptor.chIndex)];
        }

        //    MuonStationIndex::StIndex stIndex = MuonStationIndex::toStationIndex(hough.m_descriptor.chIndex);
        unsigned int nmaxima = 0;
        while (nmaxima < 5) {
            MuonHough::MuonLayerHough::Maximum maximum;
            if (hough.findMaximum(maximum, selectorLoose)) {
                hough.associateHitsToMaximum(maximum, hits);
                ATH_MSG_VERBOSE("findMaxima: Found Eta Maximum "
                              << nmaxima << "  " << maximum.max << " trigConfirmed " << maximum.triggerConfirmed << " pos " << maximum.pos
                              << " theta " << maximum.theta << " binPos " << maximum.binpos << " binRange " << maximum.binposmin << " -- "
                              << maximum.binposmax << " binTheta " << maximum.bintheta << " nHits " << maximum.hits.size());

                int nmdt = 0;
                int nmm = 0;
                int nstgc = 0;

                const unsigned int nHitsInMaximum = maximum.hits.size();
                for (unsigned int i = 0; i < nHitsInMaximum; ++i) {
                    MuonHough::Hit& hit = *(maximum.hits[i]);
                    Identifier id = hit.tgc ? hit.tgc->etaCluster.front()->identify() : hit.prd->identify();
                    int nhits = hit.tgc ? hit.tgc->etaCluster.size() : 1;

                    nmdt += m_idHelperSvc->isMdt(id);
                    nstgc  += m_idHelperSvc->issTgc(id);
                    nmm += m_idHelperSvc->isMM(id);

                    ATH_MSG_VERBOSE("findMaxima: hit " << hit.layer << "  " << m_idHelperSvc->toString(id) << " hits " << nhits);
                }

                // only store maxima that have MDT hits
                if (nmdt > 0 || (nmm + nstgc) > 0) {
                    maxima.emplace_back(std::make_unique<MuonHough::MuonLayerHough::Maximum>(maximum));
                    // add to seed list if
                    if (maximum.max > selector.getCutValue(maximum.pos)) seedMaxima.push_back(maxima.back());
                    ++nmaxima;
                }
                hough.fillLayer2(maximum.hits, true);
            } else {
                if (nmaxima > 0) { ATH_MSG_VERBOSE("findMaxima: No more maxima found " << nmaxima); }
                // ?!? if nmaximo == 0 here the function should return false, I think
                break;
            }
        }
        return true;
    }

    bool MuonLayerHoughTool::findMaxima(MuonHough::MuonPhiLayerHough& hough, MuonLayerHoughTool::PhiHitVec& hits,
                                        MuonLayerHoughTool::PhiMaximumVec& maxima, int sector) const {
        if (hits.empty()) return false;

        std::stable_sort(hits.begin(), hits.end(), MuonHough::SortHitsPerLayer());
        if (m_debugHough) hough.setDebug(true);
        hough.fillLayer2(hits);

        unsigned int nmaxima = 0;
        while (nmaxima < 5) {
            MuonHough::MuonPhiLayerHough::Maximum maximum;
            if (hough.findMaximum(maximum, 1.9)) {
                hough.associateHitsToMaximum(maximum, hits);

                ATH_MSG_DEBUG("findMaxima(Phi): Found Phi maximum " << nmaxima << " height " << maximum.max << " pos " << maximum.pos
                                                                    << " bin pos " << maximum.binpos << " binRange " << maximum.binposmin
                                                                    << " -- " << maximum.binposmax << " nHits " << maximum.hits.size());

                const unsigned int nHitsInMaximum = maximum.hits.size();
                for (unsigned int i = 0; i < nHitsInMaximum; ++i) {
                    MuonHough::PhiHit& hit = *(maximum.hits[i]);
                    Identifier id = hit.tgc ? hit.tgc->phiCluster.front()->identify() : hit.prd->identify();

                    int nhits = hit.tgc ? hit.tgc->phiCluster.size() : 1;
                    ATH_MSG_VERBOSE("findMaxima(Phi) phiHit " << m_idHelperSvc->toString(id) << " hits " << nhits);
                }

                maximum.sector = sector;  // very fragile passing on of sector

                // check if the maximum is already filled, if so, don't add it again
                bool maximum_matched = false;
                for (auto pit = maxima.begin(); pit != maxima.end(); ++pit) {
                    // reference to phi maximum
                    MuonHough::MuonPhiLayerHough::Maximum& pmaximum = **pit;
                    if (pmaximum.sector == maximum.sector && pmaximum.max == maximum.max && pmaximum.pos == maximum.pos &&
                        pmaximum.hits.size() == maximum.hits.size() && pmaximum.binpos == maximum.binpos &&
                        pmaximum.binposmin == maximum.binposmin && pmaximum.binposmax == maximum.binposmax) {
                        ATH_MSG_DEBUG("extendSeed: sector has already been added! Skip. ");
                        bool maximum_hitmatched = true;  //  check if there is a hit that is not the same
                        for (unsigned int k = 0; k < maximum.hits.size(); ++k) {
                            if (maximum.hits[k] != pmaximum.hits[k]) {  // directly compare pointer address
                                maximum_hitmatched = false;
                                break;
                            }
                        }
                        if (maximum_hitmatched) {
                            maximum_matched = true;
                            break;
                        }
                    }
                }
                // remove the hits from hough
                hough.fillLayer2(maximum.hits, true);
                if (maximum_matched) {
                    //++nmaxima;
                    continue;
                } else {
                    maxima.push_back(std::make_shared<MuonHough::MuonPhiLayerHough::Maximum>(maximum));
                    ++nmaxima;
                }
            } else {
                if (nmaxima > 0) { ATH_MSG_VERBOSE("findMaxima(Phi): No more maxima found " << nmaxima); }
                // ?!? same here, the function should return false if nothing was found, right?
                break;
            }
        }
        hough.reset();
        return true;
    }

    void MuonLayerHoughTool::fillHitsPerSector(const EventContext& ctx, State& state, const int sector,
                                               const CollectionsPerSector& collectionsPerSector, const MdtPrepDataContainer* mdtCont,
                                               const CscPrepDataContainer* /*cscCont*/, const TgcPrepDataContainer* tgcCont,
                                               const RpcPrepDataContainer* rpcCont, const sTgcPrepDataContainer* stgcCont,
                                               const MMPrepDataContainer* mmCont) const {
        MuonLayerHoughTool::HoughDataPerSector& houghData = state.houghDataPerSectorVec->vec[sector - 1];
        houghData.sector = sector;
        // loop over all possible station layers in the sector
        for (unsigned int tech = 0; tech < m_ntechnologies; ++tech) {
            for (unsigned int layerHash = 0; layerHash < MuonStationIndex::sectorLayerHashMax(); ++layerHash) {
                const HashVec& hashes = collectionsPerSector.technologyRegionHashVecs[tech][layerHash];
                if (hashes.empty()) continue;
                auto regionLayer = MuonStationIndex::decomposeSectorLayerHash(layerHash);

                for (const IdentifierHash& id_hash : hashes) {
                    // !?! else if made by Felix
                    if (mdtCont && !mdtCont->empty() && tech == toInt(TechnologyIndex::MDT)) {
                        const MdtPrepDataCollection* pos = mdtCont->indexFindPtr(id_hash);
                        if (pos) fill(ctx, state.truthHits, *pos, houghData.hitVec[layerHash]);
                    } else if (rpcCont && !rpcCont->empty() && tech == toInt(TechnologyIndex::RPC)) {
                        const RpcPrepDataCollection* pos = rpcCont->indexFindPtr(id_hash);
                        if (pos) fill(ctx, state.truthHits, *pos, houghData.hitVec[layerHash], houghData.phiHitVec[toInt(regionLayer.first)]);
                    } else if (tgcCont && !tgcCont->empty() && tech == toInt(TechnologyIndex::TGC)) {
                        const TgcPrepDataCollection* pos = tgcCont->indexFindPtr(id_hash);
                        if (pos)
                            fill(ctx, state.truthHits, state.houghDataPerSectorVec->tgcClusteringObjs, *pos, houghData.hitVec[layerHash],
                                 houghData.phiHitVec[toInt(regionLayer.first)], collectionsPerSector.sector);
                    } else if (stgcCont && !stgcCont->empty() && tech == toInt(TechnologyIndex::STGC)) {
                        const sTgcPrepDataCollection* pos = stgcCont->indexFindPtr(id_hash);
                        if (pos)
                            fill(ctx, state.truthHits, *pos, houghData.hitVec[layerHash], houghData.phiHitVec[toInt(regionLayer.first)],
                                 collectionsPerSector.sector);
                    } else if (mmCont && !mmCont->empty() && tech == toInt(TechnologyIndex::MM)) {
                        const MMPrepDataCollection* pos = mmCont->indexFindPtr(id_hash);
                        if (pos) fill(ctx, state.truthHits, *pos, houghData.hitVec[layerHash]);
                    }
                }
            }
        }
    }

    void MuonLayerHoughTool::matchTruth(std::set<Identifier>& truthHits, const PRD_MultiTruthCollection& truthCol, const Identifier& id,
                                        MuonHough::HitDebugInfo& debug) const {
        typedef PRD_MultiTruthCollection::const_iterator iprdt;
        std::pair<iprdt, iprdt> range = truthCol.equal_range(id);
        // Loop over particles contributing to this cluster
        for (iprdt i = range.first; i != range.second; ++i) {
            if (!i->second.isValid()) {
                ATH_MSG_WARNING("Unexpected invalid HepMcParticleLink in PRD_MultiTruthCollection");
            } else {
                const HepMcParticleLink& link = i->second;
                if (link.cptr() && abs(link.cptr()->pdg_id()) == 13) {
                  debug.barcode = HepMC::barcode(link); // FIXME barcode-based - requires MuonHough::HitDebugInfo to be migrated to uniqueID
                  debug.pdgId = link.cptr()->pdg_id();
                  truthHits.insert(id);
                }
            }
        }
    }

    void MuonLayerHoughTool::fill(const EventContext& ctx, std::set<Identifier>& truthHits, const MdtPrepDataCollection& mdts,
                                  MuonLayerHoughTool::HitVec& hits) const {
        if (mdts.empty()) return;
        auto truthCollections = m_truthNames.makeHandles(ctx);
        Identifier chid = mdts.identify();
        DetRegIdx region = m_idHelperSvc->regionIndex(chid);
        MuonStationIndex::LayerIndex layer = m_idHelperSvc->layerIndex(chid);
        int sector = m_idHelperSvc->sector(chid);
        unsigned int technology = toInt(m_idHelperSvc->technologyIndex(chid));
        bool barrelLike = (region == DetRegIdx::Barrel || layer == LayerIndex::BarrelExtended);
        unsigned int nmdts(mdts.size()), nmdtsBad{0};
        for (const MdtPrepData* prd : mdts) {
            if (prd->adc() < 50 || prd->status() != MdtStatusDriftTime) {
                ++nmdtsBad;
                continue;
            }
            const Identifier id = prd->identify();
            float r = rCor(*prd);
            float x = barrelLike ? r : prd->globalPosition().z();
            float y = barrelLike ? prd->globalPosition().z() : r;
            int sublayer = sublay(id);

            float ymin = y - prd->localPosition()[Trk::locR];
            float ymax = y + prd->localPosition()[Trk::locR];
            MuonHough::HitDebugInfo* debug = new MuonHough::HitDebugInfo(technology, sector, region, layer, sublayer);
            debug->time = prd->tdc();
            debug->r = prd->localPosition()[Trk::locR];

            std::map<unsigned int, unsigned int>::const_iterator pos = m_techToTruthNameIdx.find(technology);
            if (pos != m_techToTruthNameIdx.end()) { matchTruth(truthHits, *truthCollections[pos->second], id, *debug); }
            MuonHough::Hit* hit = new MuonHough::Hit(sublayer, x, ymin, ymax, 1., debug, prd);
            hits.emplace_back(hit);
        }

        ATH_MSG_DEBUG("fillMDT: Filling " << m_idHelperSvc->toStringChamber(chid) << ": loc s" << sector << " "
                                          << MuonStationIndex::regionName(region) << " " << MuonStationIndex::layerName(layer)
                                          << " -> hits: " << nmdts << " bad " << nmdtsBad << " isSmallChamber "
                                          << m_idHelperSvc->isSmallChamber(chid));
    }

    void MuonLayerHoughTool::fill(const EventContext& ctx, std::set<Identifier>& truthHits, const CscPrepDataCollection& cscs, HitVec& hits,
                                  PhiHitVec& phiHits) const {
        /// CSCs were not part of the pattern finding yet..
        return;
        auto truthCollections = m_truthNames.makeHandles(ctx);
        Identifier chid = cscs.identify();
        unsigned int technology = toInt(m_idHelperSvc->technologyIndex(chid));
        MuonStationIndex::LayerIndex layer = m_idHelperSvc->layerIndex(chid);
        DetRegIdx region = m_idHelperSvc->regionIndex(chid);
        int sector = m_idHelperSvc->sector(chid);
        unsigned int neta{0}, nphi{0};
        for (const CscPrepData* prd : cscs) {
            const bool meas_phi = m_idHelperSvc->rpcIdHelper().measuresPhi(prd->identify());
            nphi += meas_phi;
            neta += !meas_phi;
        }
        ATH_MSG_DEBUG("fillCscs: Filling " << m_idHelperSvc->toStringChamber(chid) << ": loc s" << sector << " "
                                           << MuonStationIndex::regionName(region) << " " << MuonStationIndex::layerName(layer)
                                           << " -> eta hits " << neta << " phi hits " << nphi);
        for (const CscPrepData* prd : cscs) {
            const Identifier id = prd->identify();
            int sublayer = sublay(id);
            MuonHough::HitDebugInfo* debug = new MuonHough::HitDebugInfo(technology, sector, region, layer, sublayer);
            debug->isEtaPhi = (neta && nphi);
            debug->trigConfirm = 1;
            debug->time = prd->time();
            std::map<unsigned int, unsigned int>::const_iterator pos = m_techToTruthNameIdx.find(technology);
            if (pos != m_techToTruthNameIdx.end()) { matchTruth(truthHits, *truthCollections[pos->second], id, *debug); }
            float weight = (neta && nphi) ? 2 : 1;
            if (m_idHelperSvc->rpcIdHelper().measuresPhi(id)) {
                const float r = rCor(*prd);
                const float phi = prd->globalPosition().phi();
                const double phi1 = phi;  // phiCor(phi,sector);
                debug->r = -99999;
                MuonHough::PhiHit* hit = new MuonHough::PhiHit(sublayer, r, phi1, phi1, weight, debug, prd);
                phiHits.emplace_back(hit);
            } else {
                const float x = rCor(*prd);
                const float y = prd->globalPosition().z();
                const float stripCor = 0.5 * prd->detectorElement()->StripWidth(false);
                const float ymin = y - stripCor;
                const float ymax = y + stripCor;
                debug->r = stripCor;
                MuonHough::Hit* hit = new MuonHough::Hit(sublayer, x, ymin, ymax, weight, debug, prd);
                hits.emplace_back(hit);
            }
        }
    }
    void MuonLayerHoughTool::fill(const EventContext& ctx, std::set<Identifier>& truthHits, const RpcPrepDataCollection& rpcs,
                                  MuonLayerHoughTool::HitVec& hits, MuonLayerHoughTool::PhiHitVec& phiHits) const {
        if (rpcs.empty()) return;
        auto truthCollections = m_truthNames.makeHandles(ctx);
        Identifier chid = rpcs.identify();
        unsigned int technology = toInt(m_idHelperSvc->technologyIndex(chid));
        MuonStationIndex::LayerIndex layer = m_idHelperSvc->layerIndex(chid);
        DetRegIdx region = m_idHelperSvc->regionIndex(chid);
        int sector = m_idHelperSvc->sector(chid);
        // check whether there are eta and phi hits
        unsigned int neta{0}, nphi{0};
        for (const RpcPrepData* prd : rpcs) {
            const bool meas_phi = m_idHelperSvc->rpcIdHelper().measuresPhi(prd->identify());
            nphi += meas_phi;
            neta += !meas_phi;
        }
        ATH_MSG_DEBUG("fillRPC: Filling " << m_idHelperSvc->toStringChamber(chid) << ": loc s" << sector << " "
                                          << MuonStationIndex::regionName(region) << " " << MuonStationIndex::layerName(layer)
                                          << " -> eta hits " << neta << " phi hits " << nphi);

        for (const RpcPrepData* prd : rpcs) {
            const Identifier id = prd->identify();
            int sublayer = sublay(id);
            MuonHough::HitDebugInfo* debug = new MuonHough::HitDebugInfo(technology, sector, region, layer, sublayer);
            debug->isEtaPhi = (neta && nphi);
            debug->trigConfirm = 1;
            debug->time = prd->time();
            std::map<unsigned int, unsigned int>::const_iterator pos = m_techToTruthNameIdx.find(technology);
            if (pos != m_techToTruthNameIdx.end()) { matchTruth(truthHits, *truthCollections[pos->second], id, *debug); }
            float weight = (neta && nphi) ? 2 : 1;
            if (m_idHelperSvc->rpcIdHelper().measuresPhi(id)) {
                const float r = rCor(*prd);
                const float phi = prd->globalPosition().phi();
                const double phi1 = phi;  // phiCor(phi,sector);
                debug->r = -99999;
                MuonHough::PhiHit* hit = new MuonHough::PhiHit(sublayer, r, phi1, phi1, weight, debug, prd);
                phiHits.emplace_back(hit);
            } else {
                const float x = rCor(*prd);
                const float y = prd->globalPosition().z();
                const float stripCor = 0.5 * prd->detectorElement()->StripWidth(false);
                const float ymin = y - stripCor;
                const float ymax = y + stripCor;
                debug->r = stripCor;
                MuonHough::Hit* hit = new MuonHough::Hit(sublayer, x, ymin, ymax, weight, debug, prd);
                hits.emplace_back(hit);
            }
        }
    }

    void MuonLayerHoughTool::fill(const EventContext& ctx, std::set<Identifier>& truthHits, const MMPrepDataCollection& mms,
                                  MuonLayerHoughTool::HitVec& hits) const {
        if (mms.empty()) return;
        auto truthCollections = m_truthNames.makeHandles(ctx);
        Identifier chid = mms.identify();
        DetRegIdx region = m_idHelperSvc->regionIndex(chid);
        MuonStationIndex::LayerIndex layer = m_idHelperSvc->layerIndex(chid);
        int sector = m_idHelperSvc->sector(chid);
        unsigned int technology = toInt(m_idHelperSvc->technologyIndex(chid));
        ATH_MSG_DEBUG("fillMM: Filling " << m_idHelperSvc->toStringChamber(chid) << ": loc s" << sector << " "
                                         << MuonStationIndex::regionName(region) << " " << MuonStationIndex::layerName(layer) << " -> hits "
                                         << mms.size());

        std::array<double,8> multiplicity{};
        for (const MMPrepData* prd : mms) {
            const Identifier id = prd->identify();
            int sublayer = sublay(id) % 10;
            multiplicity[sublayer]++;
        }

        if( msgLvl(MSG::DEBUG) ){
            for (int i = 0; i<8 ; i++) if(multiplicity[i]>0) ATH_MSG_DEBUG(" sublayer " << i << " hits " <<  multiplicity[i]);
        }

        for (const MMPrepData* prd : mms) {
            const Identifier id = prd->identify();
            float x = prd->globalPosition().z();
            float y = rCor(*prd);
            int sublayer = sublay(id) % 10;
            float stripCor = prd->detectorElement()->getDesign(id)->inputPitch;
            float ymin = y - stripCor;
            float ymax = y + stripCor;

            //Downweight noise channels
            const double weight = 1. / std::max(1., multiplicity[sublayer]);

            MuonHough::HitDebugInfo* debug = new MuonHough::HitDebugInfo(technology, sector, region, layer, sublayer);
            debug->r = stripCor;
            std::map<unsigned int, unsigned int>::const_iterator pos = m_techToTruthNameIdx.find(technology);
            if (pos != m_techToTruthNameIdx.end()) { matchTruth(truthHits, *truthCollections[pos->second], id, *debug); }
            std::unique_ptr<MuonHough::Hit> hit = std::make_unique<MuonHough::Hit>(sublayer, x, ymin, ymax, weight, debug, prd);
            hits.emplace_back(std::move(hit));
        }
    }

    void MuonLayerHoughTool::fill(const EventContext& ctx, std::set<Identifier>& truthHits, const sTgcPrepDataCollection& stgcs,
                                  MuonLayerHoughTool::HitVec& hits, MuonLayerHoughTool::PhiHitVec& phiHits, int selectedSector) const {
        if (stgcs.empty()) return;
        auto truthCollections = m_truthNames.makeHandles(ctx);
        Identifier chid = stgcs.identify();
        DetRegIdx region = m_idHelperSvc->regionIndex(chid);
        MuonStationIndex::LayerIndex layer = m_idHelperSvc->layerIndex(chid);
        int sector = m_idHelperSvc->sector(chid);
        bool isNeighbouringSector = sector != selectedSector;
        unsigned int technology = toInt(m_idHelperSvc->technologyIndex(chid));
        ATH_MSG_DEBUG("fillsTGC: Filling " << m_idHelperSvc->toStringChamber(chid) << ": loc s" << sector << " "
                                           << MuonStationIndex::regionName(region) << " " << MuonStationIndex::layerName(layer)
                                           << " -> hits: " << stgcs.size());

        for (const sTgcPrepData* prd : stgcs) {
            const Identifier id = prd->identify();
            int channelType = m_idHelperSvc->stgcIdHelper().channelType(id);

            // only pick up phi hits in neighbouring sectors
            if (isNeighbouringSector && channelType == 1) continue;
            int sublayer = sublay(id);

            std::unique_ptr<MuonHough::HitDebugInfo> debug =
                std::make_unique<MuonHough::HitDebugInfo>(technology, sector, region, layer, sublayer);
            debug->isEtaPhi = 1;
            debug->trigConfirm = true;

            std::map<unsigned int, unsigned int>::const_iterator pos = m_techToTruthNameIdx.find(technology);
            if (pos != m_techToTruthNameIdx.end()) { matchTruth(truthHits, *truthCollections[pos->second], id, *debug); }
            if (m_idHelperSvc->stgcIdHelper().channelType(id) == 1) {
                // eta strips
                float x = prd->globalPosition().z();
                float y = rCor(*prd);
                float stripCor = 1.5;  // get from det el
                const MuonGM::MuonChannelDesign* design = prd->detectorElement()->getDesign(id);
                if (design) {
                    double stripWidth = design->inputWidth;
                    double stripLength = design->channelLength(m_idHelperSvc->stgcIdHelper().channel(id));
                    if (m_debugHough) ATH_MSG_DEBUG(" eta strip width " << stripWidth << " stripLength " << stripLength);
                    stripCor = 0.5 * stripWidth;
                }
                debug->r = stripCor;
                float ymin = y - stripCor;
                float ymax = y + stripCor;
                MuonHough::Hit* hit = new MuonHough::Hit(sublayer, x, ymin, ymax, 1., debug.release(), prd);
                hits.emplace_back(hit);
            } else {
                double chWidth = 0;
                /// Pad channels  -- We should think about enums!
                if (m_idHelperSvc->stgcIdHelper().channelType(id) == 0) {
                    const MuonGM::MuonPadDesign* design = prd->detectorElement()->getPadDesign(id);
                    if (!design) {
                        ATH_MSG_WARNING("No design found for " << m_idHelperSvc->toString(id));
                        continue;
                    }
                    // get the pad width from the detector design
                    chWidth = 0.5 * design->channelWidth(prd->localPosition(), true);
                    ATH_MSG_DEBUG(" Pad chWidth " << chWidth << " phi global " << prd->globalPosition().phi());
                } else if (m_idHelperSvc->stgcIdHelper().channelType(id) == 2) {
                    const MuonGM::MuonChannelDesign* design = prd->detectorElement()->getDesign(id);
                    if (!design) {
                        ATH_MSG_WARNING("No design found for " << m_idHelperSvc->toString(id));
                        continue;
                    }
                    chWidth = 0.5 * design->channelWidth();
                    ATH_MSG_DEBUG(" Wire Gang chWidth " << chWidth << " phi global " << prd->globalPosition().phi());
                }

                Amg::Vector2D lp1(prd->localPosition().x() + chWidth, prd->localPosition().y());
                Amg::Vector3D gp1;
                prd->detectorElement()->surface(id).localToGlobal(lp1, gp1, gp1);

                lp1[0] = prd->localPosition().x() - chWidth;
                Amg::Vector3D gp2;
                prd->detectorElement()->surface(id).localToGlobal(lp1, gp2, gp2);

                double phi1 = gp1.phi();
                double phi2 = gp2.phi();
                double phi1c = phi1;  // phiCor(phi1,selectedSector);
                double phi2c = phi2;  // phiCor(phi2,selectedSector);
                double phi_check = std::abs(xAOD::P4Helpers::deltaPhi(phi1c, phi2c));
                if (phi_check > 0.3) {
                    ATH_MSG_WARNING("bad local phi: in " << phi1 << ", " << phi2 << " sector phi "
                                                         << m_sectorMapping.sectorPhi(selectedSector) << " phicor " << phi1c << ", "
                                                         << phi2c);
                }
                if (isNeighbouringSector &&
                    !(m_sectorMapping.insideSector(selectedSector, phi1) || m_sectorMapping.insideSector(selectedSector, phi2))) {
                    ATH_MSG_DEBUG("Dropping phi hit in neighbouring sector " << m_idHelperSvc->toString(id) << " phi min "
                                                                             << std::min(phi1c, phi2c) << " max " << std::max(phi1c, phi2c)
                                                                             << " global phi: in " << phi1 << ", " << phi2 << " sector phi "
                                                                             << m_sectorMapping.sectorPhi(selectedSector));
                    continue;
                }
                float r = rCor(*prd);
                float phiMin = std::min(phi1c, phi2c);
                float phiMax = std::max(phi1c, phi2c);
                ATH_MSG_VERBOSE("Phi hit " << m_idHelperSvc->toString(id) << " r " << r << " phi min " << phiMin << " phi max "
                                           << phiMax << " bc " << debug->barcode << " chw " << chWidth << " trigC " // FIXME barcode-based - requires MuonHough::HitDebugInfo to be migrated to uniqueID
                                           << debug->trigConfirm << " g phi " << phi1 << " " << phi2);
                MuonHough::PhiHit* phiHit =
                    new MuonHough::PhiHit(sublayer, r, phiMin, phiMax, 1, debug.release(), prd);
                phiHits.emplace_back(phiHit);
            }
        }
    }

    void MuonLayerHoughTool::fill(const EventContext& ctx, std::set<Identifier>& truthHits,
                                  std::vector<std::unique_ptr<TgcHitClusteringObj>>& tgcClusteringObjs, const TgcPrepDataCollection& tgcs,
                                  MuonLayerHoughTool::HitVec& hits, MuonLayerHoughTool::PhiHitVec& phiHits, int sector) const {
        if (tgcs.empty()) return;
        tgcClusteringObjs.push_back(std::make_unique<TgcHitClusteringObj>(&m_idHelperSvc->tgcIdHelper()));
        TgcHitClusteringObj& clustering = *tgcClusteringObjs.back();
        std::vector<const TgcPrepData*> prds;
        prds.insert(prds.begin(), tgcs.begin(), tgcs.end());
        clustering.cluster(prds);
        clustering.buildClusters3D();

        Identifier chid = tgcs.identify();
        DetRegIdx region = m_idHelperSvc->regionIndex(chid);
        MuonStationIndex::LayerIndex layer = m_idHelperSvc->layerIndex(chid);

        if (clustering.clusters3D.empty()) {
            ATH_MSG_DEBUG("TgcHitClusteringObj, no 3D clusters! ");
            if (msgLvl(MSG::DEBUG)) {
                for (const TgcPrepData* prd : tgcs) { ATH_MSG_DEBUG("   " << m_idHelperSvc->toString(prd->identify())); }
            }
            return;
        }
        if (clustering.bestEtaCluster().empty()) {
            ATH_MSG_DEBUG("TgcHitClusteringObj, no eta cluster selected! ");
            if (msgLvl(MSG::DEBUG)) {
                for (const TgcPrepData* prd : prds) { ATH_MSG_DEBUG("   " << m_idHelperSvc->toString(prd->identify())); }
            }
            return;
        }
        auto truthCollections = m_truthNames.makeHandles(ctx);
        std::vector<int> sectors;
        getSectors(clustering.clusters3D.front(), sectors);
        unsigned int technology = toInt(m_idHelperSvc->technologyIndex(chid));
        for (unsigned int si = 0; si < sectors.size(); ++si) {
            if (sectors[si] != sector) continue;

            for (const TgcClusterObj3D& cl : clustering.clusters3D) {
                const Identifier id = cl.etaCluster.front()->identify();

                double x = cl.getEdge(TgcEdge::LowEtaLowPhi).z();
                double y11 = rCor(cl, TgcEdge::LowEtaLowPhi, sector);
                double y12 = rCor(cl, TgcEdge::LowEtaHighPhi, sector);
                double y21 = rCor(cl, TgcEdge::LowEtaLowPhi, sector);
                double y22 = rCor(cl, TgcEdge::HighEtaHighPhi, sector);
                double phi11 = cl.getEdge(TgcEdge::LowEtaLowPhi).phi();
                double phi12 = cl.getEdge(TgcEdge::LowEtaHighPhi).phi();
                double phi21 = cl.getEdge(TgcEdge::LowEtaLowPhi).phi();
                double phi22 = cl.getEdge(TgcEdge::HighEtaHighPhi).phi();
                double ymin = std::min(std::min(y11, y12), std::min(y21, y22));
                double ymax = std::max(std::max(y11, y12), std::max(y21, y22));
                double phimin = std::min(std::min(phi11, phi12), std::min(phi21, phi22));
                double phimax = std::max(std::max(phi11, phi12), std::max(phi21, phi22));
                double phi1 = phimin;  // phiCor(phimin,sector);
                double phi2 = phimax;  // phiCor(phimax,sector);
                int sublayer = sublay(id, x);
                ATH_MSG_VERBOSE("Cluster "<<m_idHelperSvc->toString(id)<<" x: "<<x<<", y11: "<<y11
                             <<", y12: "<<y12<<", y21: "<<y21<<", y22: "<<y22<<", phi11: "<<phi11<<", "
                             <<"phi12: "<<phi12<<", phi21: "<<phi21<<", phi22: "<<phi22<<" ymin: "<<ymin<<", ymax: "<<ymax
                             <<", phimin: "<<phimin<<", phimax: "<<phimax);

                MuonHough::HitDebugInfo* debug = new MuonHough::HitDebugInfo(technology, sector, region, layer, sublayer);
                debug->clusterSize = cl.etaCluster.size();
                debug->clusterLayers = 2;
                debug->isEtaPhi = true;
                debug->time = cl.etaCluster.front()->getBcBitMap();
                std::map<unsigned int, unsigned int>::const_iterator pos = m_techToTruthNameIdx.find(technology);
                if (pos != m_techToTruthNameIdx.end()) { matchTruth(truthHits, *truthCollections[pos->second], id, *debug); }

                MuonHough::HitDebugInfo* phiDebug = new MuonHough::HitDebugInfo(*debug);
                phiDebug->clusterSize = cl.phiCluster.size();
                phiDebug->clusterLayers = 1;
                phiDebug->isEtaPhi = true;

                std::unique_ptr<MuonHough::Hit> hit = std::make_unique<MuonHough::Hit>(sublayer, x, ymin, ymax, 2, debug, nullptr, &cl);
                std::unique_ptr<MuonHough::PhiHit> phiHit =
                    std::make_unique<MuonHough::PhiHit>(sublayer, y11, phi1, phi2, 2, phiDebug, nullptr, &cl);
                hits.emplace_back(std::move(hit));
                phiHits.emplace_back(std::move(phiHit));
            }
        }
        ATH_MSG_DEBUG("fillTGC: Filling " << m_idHelperSvc->toStringChamber(chid) << ": loc s" << sector << " "
                                          << MuonStationIndex::regionName(region) << " " << MuonStationIndex::layerName(layer)
                                          << " -> etaHits: " << hits.size() << " phiHits: " << phiHits.size()
                                          << " sectors: " << sectors.size());
    }

    void MuonLayerHoughTool::insertHash(const IdentifierHash& hash, const Identifier& id) const{
        insertHash(m_idHelperSvc->sector(id), hash, id);
    }

    void MuonLayerHoughTool::insertHash(int sector, const IdentifierHash& hash, const Identifier& id) const{
        MuonStationIndex::TechnologyIndex techIndex = m_idHelperSvc->technologyIndex(id);
        int sectorLayerHash = MuonStationIndex::sectorLayerHash(m_idHelperSvc->regionIndex(id), m_idHelperSvc->layerIndex(id));
        m_collectionsPerSector[sector - 1].technologyRegionHashVecs[toInt(techIndex)][sectorLayerHash].push_back(hash);
    }

    // all chambers are mapped onto a layer and sector map
    void MuonLayerHoughTool::initializeSectorMapping(const EventContext& ctx) const{
        if (m_sectorSetup) return;
        std::lock_guard kuchen(m_mutex);
        // cppcheck-suppress identicalConditionAfterEarlyExit; false positive
        if (m_sectorSetup) return;
        SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{m_muonManagerKey, ctx};
        m_collectionsPerSector.resize(MuonStationIndex::numberOfSectors());
        // set sector numbers
        unsigned int nsectorHashMax = MuonStationIndex::sectorLayerHashMax();
        for (unsigned int i = 0; i < m_collectionsPerSector.size(); ++i) {
            m_collectionsPerSector[i].sector = i + 1;
            m_collectionsPerSector[i].technologyRegionHashVecs.resize(m_ntechnologies);
            for (auto it = m_collectionsPerSector[i].technologyRegionHashVecs.begin();
                 it != m_collectionsPerSector[i].technologyRegionHashVecs.end(); ++it) {
                it->resize(nsectorHashMax);
            }
        }
        ATH_MSG_DEBUG("Initializing hashes: number of sectors " << MuonStationIndex::numberOfSectors() << " technologies "
                                                                << m_ntechnologies << " sectorLayers "
                                                                << MuonStationIndex::sectorLayerHashMax());
        // loop over all available MDT collection identifiers and order them per sector
        
        auto loadHashes = [this] (const MuonIdHelper& idHelper){
            auto it = idHelper.module_begin();
            const auto it_end = idHelper.module_end();
            for (; it != it_end; ++it) {
                IdentifierHash hash;
                idHelper.get_module_hash(*it, hash);
                insertHash(hash, *it);
            }
        };

        if (m_idHelperSvc->hasMDT()) {
            loadHashes(m_idHelperSvc->mdtIdHelper());
        }
        if (m_idHelperSvc->hasRPC()) {
            loadHashes(m_idHelperSvc->rpcIdHelper());
        }
        if (m_idHelperSvc->hasCSC()) {
            loadHashes(m_idHelperSvc->cscIdHelper());
        }
        // loop over all available MM collection identifiers and order them per sector
        if (m_idHelperSvc->hasMM()) {
            auto it = m_idHelperSvc->mmIdHelper().detectorElement_begin();
            const auto it_end = m_idHelperSvc->mmIdHelper().detectorElement_end();
            for (; it != it_end; ++it) {
                IdentifierHash hash;
                m_idHelperSvc->mmIdHelper().get_module_hash(*it, hash);
                insertHash(hash, *it);
            }
        }
        // loop over all available STGC collection identifiers and order them per sector
        if (m_idHelperSvc->hasSTGC()) {
            auto it = m_idHelperSvc->stgcIdHelper().detectorElement_begin();
            const auto it_end = m_idHelperSvc->stgcIdHelper().detectorElement_end();
            for (; it != it_end; ++it) {
                IdentifierHash hash;
                m_idHelperSvc->stgcIdHelper().get_module_hash(*it, hash);
                int sector = m_idHelperSvc->sector(*it);
                insertHash(sector, hash, *it);
                int sectorU = sector != 1 ? sector - 1 : 16;
                int sectorD = sector != 16 ? sector + 1 : 1;
                insertHash(sectorU, hash, *it);
                insertHash(sectorD, hash, *it);
            }
        }

        if (m_idHelperSvc->hasTGC()) {
            // loop over all available TGC collection identifiers and order them per sector
            auto it = m_idHelperSvc->tgcIdHelper().module_begin();
            const auto it_end = m_idHelperSvc->tgcIdHelper().module_end();
            for (; it != it_end; ++it) {
                 const MuonGM::TgcReadoutElement* detEl = detMgr->getTgcReadoutElement(*it);
                 IdentifierHash hash;
                m_idHelperSvc->tgcIdHelper().get_module_hash(*it, hash);
                int nstrips = detEl->nStrips(1);
                const Amg::Vector3D p1 = detEl->channelPos(1, 1, 1);
                const Amg::Vector3D p2 = detEl->channelPos(1, 1, nstrips);
                std::vector<int> sectors1{}, sectors2{};
                getSectors(p1, sectors1);
                getSectors(p2, sectors2);
                std::unordered_set<int> added{};
                for (const int sector : sectors1) {
                    insertHash(sector, hash, *it);
                    added.insert(sector);
                }
                for (const int sector: sectors2) {
                    if (added.insert(sector).second){
                        insertHash(sector, hash, *it);
                    }
                }
            }
        }


        ATH_MSG_DEBUG(" Printing collections per sector, number of technologies " << m_ntechnologies);
        for (int sector = 1; sector <= 16; ++sector) {
            DetRegIdx currentRegion = DetRegIdx::DetectorRegionUnknown;
            ATH_MSG_DEBUG(" sector " << sector);
            TechnologyRegionHashVec& vec = m_collectionsPerSector[sector - 1].technologyRegionHashVecs;
            for (unsigned int hash = 0; hash < nsectorHashMax; ++hash) {
                std::pair<DetRegIdx, MuonStationIndex::LayerIndex> regionLayer =
                    MuonStationIndex::decomposeSectorLayerHash(hash);
               
                if (regionLayer.first != currentRegion) ATH_MSG_DEBUG("  " << MuonStationIndex::regionName(regionLayer.first));
                bool first = true;
                currentRegion = regionLayer.first;
                for (unsigned int tech = 0; tech < m_ntechnologies; ++tech) {
                    std::stable_sort(vec[tech][hash].begin(), vec[tech][hash].end());
                    if (!vec[tech][hash].empty()) {
                        if (msgLvl(MSG::DEBUG)) {
                            if (first) {
                                ATH_MSG_DEBUG("  " << std::setw(7) << MuonStationIndex::layerName(regionLayer.second));
                                first = false;
                            }
                            ATH_MSG_DEBUG(" " << std::setw(4)
                                              << MuonStationIndex::technologyName(static_cast<MuonStationIndex::TechnologyIndex>(tech))
                                              << " " << std::setw(4) << vec[tech][hash].size());
                        }
                    }
                }
            }
        }
        m_sectorSetup = true;
    }

    void MuonLayerHoughTool::printTruthSummary(std::set<Identifier>& truth, std::set<Identifier>& found) const {
        if (truth.size() == found.size()) {
            ATH_MSG_DEBUG(" All hits found: truth " << truth.size() << " found " << found.size());
        } else {
            ATH_MSG_DEBUG(" Some truth hits not found: truth " << truth.size() << " found " << found.size());
            std::vector<Identifier> result(truth.size() - found.size());
            std::vector<Identifier>::iterator pos =
                std::set_difference(truth.begin(), truth.end(), found.begin(), found.end(), result.begin());
            result.resize(pos - result.begin());
            for (std::vector<Identifier>::iterator it = result.begin(); it != result.end(); ++it) {
                ATH_MSG_DEBUG("  " << m_idHelperSvc->toString(*it));
            }
        }
    }
}  // namespace Muon
