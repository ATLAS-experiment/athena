/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <array>
#include <vector>

#include "FPGATrackSimMaps/FPGATrackSimSlicingEngineTool.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimObjects/FPGATrackSimTowerInputHeader.h"
#include <nlohmann/json.hpp>

FPGATrackSimSlicingEngineTool::FPGATrackSimSlicingEngineTool(std::string const & algname, std::string const & name, IInterface const * ifc) :
  AthAlgTool(algname,name,ifc) {}


StatusCode FPGATrackSimSlicingEngineTool::initialize()
{
    ATH_MSG_INFO("Reading first stage module/layer map in slicing engine: " << m_layerMap);
    if (m_doSecondStage) readLayerMap();
    return StatusCode::SUCCESS;
}

void FPGATrackSimSlicingEngineTool::readLayerMap() {
    // Adapted from FPGATrackSimBinnedHits
    std::ifstream stream(m_layerMap);
    nlohmann::json data = nlohmann::json::parse(stream);

    std::set<unsigned> modules;

    // Loop over entries in the layer map.
    for (const auto &binelem : data) {
        std::vector<unsigned> bin;
        binelem.at("bin").get_to(bin);
        auto& lyrmap = binelem["lyrmap"];
        ATH_MSG_DEBUG("bin = " << bin);
        ATH_MSG_DEBUG("lyrmap = " << lyrmap);
        for (auto &lyrelem : lyrmap) {
            unsigned layer;
            lyrelem.at("lyr").get_to(layer);
            lyrelem.at("mods").get_to(modules);
            ATH_MSG_DEBUG("layer = " << layer);
            ATH_MSG_DEBUG("mods = " << modules);

            // Build a single set containing all module ID hashes used in the layer map.
            // NOTE: this assumes the pixel module hashes are all unique?
            m_layerMapModules.merge(modules);
        }
    }
}

// While this method returns *two* streams, it outputs *three* branches since in the firmware
// second stage pixels and strips will probably be split.
void FPGATrackSimSlicingEngineTool::sliceHits(const std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits,
                   std::vector<std::shared_ptr<const FPGATrackSimHit>>& firstHits,
                   std::vector<std::shared_ptr<const FPGATrackSimHit>>& secondHits) {

    const FPGATrackSimRegionMap* rmap_1st = m_FPGATrackSimMapping->SubRegionMap();
    const FPGATrackSimRegionMap* rmap_2nd = m_FPGATrackSimMapping->SubRegionMap_2nd();

    // NOTE: this now assumes that there is ONE "slice" for each of the three configurations (1st pixels,
    // 2nd pixels, strips). the subregion map is NO LONGER USED.
    if (m_rootOutput) {
        FPGATrackSimTowerInputHeader towerFirstPixel = FPGATrackSimTowerInputHeader(0);
        FPGATrackSimTowerInputHeader towerSecondPixel = FPGATrackSimTowerInputHeader(0);
        FPGATrackSimTowerInputHeader towerStrips = FPGATrackSimTowerInputHeader(0);
        m_slicedFirstPixelHeader->addTower(towerFirstPixel);
        m_slicedSecondPixelHeader->addTower(towerSecondPixel);
        m_slicedStripHeader->addTower(towerStrips);
    }

    // Loop over all of the hits. Test if they pass region boundaries or not.
    for (const std::shared_ptr<const FPGATrackSimHit>& hit : hits) {
        // If the hit falls within the boundaries of ANY subregion in the first or second stage,
        // it's to be considered "IN REGION". Technically the second stage *should* be a superset
        // of the first stage but let's be redundant to be safe here.
        if ((rmap_1st->getRegions(*hit).size() == 0) && (rmap_2nd->getRegions(*hit).size() == 0)) {
            continue;
        }

        // Test if the remaining hit is strip or pixel, sort accordingly.
        // If doSecondStage is false, then all hits are first stage-- so don't worry about the layer map.
        if (hit->isPixel()) {
            // In this case, test using the layer map (if we are doing second stage).
            if (!m_doSecondStage || m_layerMapModules.contains(hit->getIdentifierHash())) {
                firstHits.push_back(hit);
                if (m_rootOutput) m_slicedFirstPixelHeader->getTower(0)->addHit(*hit);
            } else {
                secondHits.push_back(hit);
                if (m_rootOutput) m_slicedSecondPixelHeader->getTower(0)->addHit(*hit);
            }
        } else {
            // Strip hits need to be post-processed in LogicalHitsProcessAlg, so we only put them in a header here.
            if (m_rootOutput) m_slicedStripHeader->getTower(0)->addHit(*hit);
        }
    }

    ATH_MSG_DEBUG("From " << hits.size() << " total input hits, sent " << firstHits.size() << " (" << secondHits.size() << ") to first (second) stage in region");
}
