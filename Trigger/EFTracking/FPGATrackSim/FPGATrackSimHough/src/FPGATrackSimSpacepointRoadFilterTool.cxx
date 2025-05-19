// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimSpacepointRoadFilterTool.cxx
 * @author Ben Rosser - brosser@uchicago.edu
 * @date May 24th, 2022
 * @brief Split roads with mixed hits and spacepoints in the same layers.
 */

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"

#include "FPGATrackSimSpacepointRoadFilterTool.h"

#include "TH2.h"

#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <iostream>

// TODO all this code should be rewritten once duplication is removed.


StatusCode FPGATrackSimSpacepointRoadFilterTool::initialize()
{
    // Retrieve info
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    if (m_setSectors) ATH_CHECK(m_FPGATrackSimBankSvc.retrieve());

    // This should be done properly through the monitors, later.
    m_inputRoads = new TH1I((m_isSecondStage) ? "srft_input_roads2" : "srft_input_roads", "srft_input_roads", 1000, -0.5, 1000-0.5);
    m_badRoads = new TH1I((m_isSecondStage) ? "srft_bad_roads2" : "srft_bad_roads", "srft_bad_roads", 1000, -0.5, 1000-0.5);
    ATH_MSG_INFO("Configuring SPRT for " << m_isSecondStage);

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimSpacepointRoadFilterTool::finalize()
{
    ATH_MSG_DEBUG("Spacepoint road filter: processed average of " << m_inputRoads->GetMean() << " input roads, found average of " << m_badRoads->GetMean() << " bad roads per event.");
    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// Main Algorithm

StatusCode FPGATrackSimSpacepointRoadFilterTool::filterRoads(std::vector<std::shared_ptr<const FPGATrackSimRoad>> & prefilter_roads, std::vector<std::shared_ptr<const FPGATrackSimRoad>> & postfilter_roads) {
    // Record the number of input roads and roads with problems.
    int badRoads = 0;
    if (prefilter_roads.size() > 0) {
            m_inputRoads->Fill(prefilter_roads.size());
    }

    // Clear the underlying memory for the filtered roads.
    postfilter_roads.clear();
        m_postfilter_roads.clear();
    if (prefilter_roads.size() == 0) {
        return StatusCode::SUCCESS;
    }


    // This tool takes a road and checks to see if there are any layers containing
    // both spacepoints and unpaired strip hits. If so, it splits the road into
    // multiple roads as many times as needed to remove this ambiguity, such that
    // every layer in each created road contains *only* spacepoints or hits, but
    // not both. Then we can match each new road unambiguously to a single set of
    // spacepoint-dependent fit constants.
    for (auto & road : prefilter_roads) {
        std::shared_ptr<FPGATrackSimRoad> nonConstRoad = std::const_pointer_cast<FPGATrackSimRoad>(road);
        bool success = splitRoad(nonConstRoad.get());
        road = nonConstRoad;
        if (!success) {
            badRoads += 1;
        }
    }

        m_badRoads->Fill(badRoads);

    // copy roads to outputs - borrowed from the eta pattern filter.
    postfilter_roads.reserve(m_postfilter_roads.size());
    for (FPGATrackSimRoad & r : m_postfilter_roads)
        postfilter_roads.emplace_back(std::make_shared<const FPGATrackSimRoad>(r));

    return StatusCode::SUCCESS;
}

bool FPGATrackSimSpacepointRoadFilterTool::splitRoad(FPGATrackSimRoad* initial_road) {

    // Loop through the initial road, keeping track of the spacepoints and single hits as we go.
    std::map<size_t, std::vector<std::shared_ptr<const FPGATrackSimHit>>> strip_hits;
    std::map<size_t, std::vector<std::shared_ptr<const FPGATrackSimHit>>> inner_spacepoints;
    std::map<size_t, std::vector<std::shared_ptr<const FPGATrackSimHit>>> outer_spacepoints;

    bool retval = true;
    // Loop over each pair of strip layers.
    for (size_t layer = 0; layer < initial_road->getNLayers(); layer++) {
        // Do nothing for pixel layers.
        if (m_isSecondStage){
            if(m_FPGATrackSimMapping->PlaneMap_2nd(initial_road->getSubRegion())->isPixel(layer)){
                continue;
            }
        }
        else if (m_FPGATrackSimMapping->PlaneMap_1st(initial_road->getSubRegion())->isPixel(layer)) {
            continue;
        }

        // Get the hits in these two layers, split by whether or not they are SPs.
        std::vector<std::shared_ptr<const FPGATrackSimHit>> strip_hits_in;
        std::vector<std::shared_ptr<const FPGATrackSimHit>> spacepoints_in;
        const std::vector<std::shared_ptr<const FPGATrackSimHit>> hits_in = initial_road->getHits(layer);
        for (auto& hit : hits_in) {
            if (hit->getHitType() == HitType::spacepoint) {
                spacepoints_in.push_back(hit);
            } else {
                strip_hits_in.push_back(hit);
            }
        }

        // Do the same for the next layer.
        std::vector<std::shared_ptr<const FPGATrackSimHit>> strip_hits_out;
        std::vector<std::shared_ptr<const FPGATrackSimHit>> spacepoints_out;
        const std::vector<std::shared_ptr<const FPGATrackSimHit>> hits_out = initial_road->getHits(layer + 1);
        for (auto& hit : hits_out) {
            if (hit->getHitType() == HitType::spacepoint) {
                spacepoints_out.push_back(hit);
            } else {
                strip_hits_out.push_back(hit);
            }
        }

        // Now, we have a potential issue if, for whatever reason, we only have one copy of a duplicated spacepoint.
        // I wrote this kind of clunky logic to find and fix this; where "fix" means either "drop the half-spacepoint"
        // or "convert it back to an unpaired strip hit" depending on whether filering is turned on.
        // In principle the only way this can happen legitimately is in the eta pattern filter; if it happens
        // for any other reason it's a bug.

        std::vector<std::shared_ptr<const FPGATrackSimHit>> new_sp_in;
        std::vector<std::shared_ptr<const FPGATrackSimHit>> new_sp_out;
        unsigned num_unique = findUnique(spacepoints_in, spacepoints_out, strip_hits_in, strip_hits_out, new_sp_in, new_sp_out);

        if (num_unique > 0) {
            // This can now only happen due to eta pattern filtering, so don't drop the road, but leave a way to track.
            retval = false;

            // Update the spacepoint vectors.
            spacepoints_in = std::move(new_sp_in);
            spacepoints_out = std::move(new_sp_out);

            // Now update the two layers accordingly, having converted invalid SPs back to paired hits.
            std::vector<std::shared_ptr<const FPGATrackSimHit>> new_all_in = spacepoints_in;
            new_all_in.insert(std::end(new_all_in), std::begin(strip_hits_in), std::end(strip_hits_in));
            initial_road->setHits(layer, std::move(new_all_in));

            std::vector<std::shared_ptr<const FPGATrackSimHit>> new_all_out = spacepoints_out;
            new_all_out.insert(std::end(new_all_out), std::begin(strip_hits_out), std::end(strip_hits_out));
            initial_road->setHits(layer + 1, std::move(new_all_out));

            // Debug message.
            ATH_MSG_DEBUG("Found inconsistent number of spacepoints in road with x = " << initial_road->getXBin() << ", y = " << initial_road->getYBin());
        }

        strip_hits.emplace(layer, strip_hits_in);
        strip_hits.emplace(layer + 1, strip_hits_out);

        // Update our spacepoint maps now that any uniques have been eliminated.
        inner_spacepoints.emplace(layer, spacepoints_in);
        outer_spacepoints.emplace(layer+1, spacepoints_out);
        if (spacepoints_in.size() != spacepoints_out.size()) {
            ATH_MSG_WARNING("Handling of unique spacepoints failed, " << spacepoints_in.size() << " != " << spacepoints_out.size());
            return false;
        }

        layer += 1;
    }

    // Create a copy of the initial road. this is our first working road.
    std::vector<FPGATrackSimRoad> working_roads;
    FPGATrackSimRoad new_road(*initial_road);
    working_roads.push_back(new_road);

    // Now loop through the processed spacepoints.
    for (auto& entry : inner_spacepoints) {
        size_t layer = entry.first;

        // Look up the associated strip hits.
        std::vector<std::shared_ptr<const FPGATrackSimHit>> strip_hits_in = strip_hits[layer];
        std::vector<std::shared_ptr<const FPGATrackSimHit>> strip_hits_out = strip_hits[layer + 1];

        ATH_MSG_DEBUG("Road (x = " << new_road.getXBin() << ", y = " << new_road.getYBin() << ") has merged spacepoints: " << entry.second.size() << ", unpaired inner hits: " << strip_hits_in.size() << ", unpaired outer hits: " << strip_hits_out.size());

        // If we have a nonzero number of hits AND spacepoints in this pair of layers,
        // then we'll need to split the road.
        if ((strip_hits_in.size() > 0 && entry.second.size() > 0) ||
            (strip_hits_out.size() > 0 && entry.second.size() > 0)) {

            // If there is only a strip hit in one of the two layers, we'll need to add a wildcard.
            int wildcard_layer = -1;
            if (strip_hits_in.size() == 0 || strip_hits_out.size() == 0) {
                std::unique_ptr<FPGATrackSimHit> wcHit = std::make_unique<FPGATrackSimHit>();
                wcHit->setHitType(HitType::wildcard);
                wcHit->setDetType(m_isSecondStage ? m_FPGATrackSimMapping->PlaneMap_2nd(new_road.getSubRegion())->getDetType(layer) : m_FPGATrackSimMapping->PlaneMap_1st(new_road.getSubRegion())->getDetType(layer));
                if (strip_hits_in.size() == 0) {
                    wildcard_layer = layer;
                    wcHit->setLayer(wildcard_layer);
                    strip_hits_in.push_back(std::move(wcHit));
                } else {
                    wildcard_layer = layer + 1;
                    wcHit->setLayer(wildcard_layer);
                    strip_hits_out.push_back(std::move(wcHit));
                }
            }

            // This could probably be expressed with an iterator, but it's a little cleaner
            // this way. we want to replace each working_road with two new working roads.
            unsigned num_working = working_roads.size();
            for (unsigned i = 0; i < num_working; i++) {
                // For each working road, create two new ones!
                FPGATrackSimRoad strips_only(working_roads[i]);
                FPGATrackSimRoad spacepoints_only(working_roads[i]);

                // Update their hits in these layers accordingly.
                strips_only.setHits(layer, std::vector<std::shared_ptr<const FPGATrackSimHit>>(strip_hits_in));
                strips_only.setHits(layer + 1, std::vector<std::shared_ptr<const FPGATrackSimHit>>(strip_hits_out));
                spacepoints_only.setHits(layer, std::vector<std::shared_ptr<const FPGATrackSimHit>>(entry.second));
                spacepoints_only.setHits(layer + 1, std::vector<std::shared_ptr<const FPGATrackSimHit>>(outer_spacepoints[layer + 1]));

                // If we inserted a wildcard hit previously, update the WC layers accordingly
                // in the strips_only road.
                if (wildcard_layer != -1) {
                    layer_bitmask_t wc_layers = strips_only.getWCLayers();
                    wc_layers |= (0x1 << wildcard_layer);
                    strips_only.setWCLayers(wc_layers);

                    layer_bitmask_t hit_layers = strips_only.getHitLayers();
                    hit_layers -= (0x1 << wildcard_layer);
                    if (strips_only.getHitLayers() < (unsigned)(0x1 << wildcard_layer)) {
                        ATH_MSG_WARNING("Found weird problem: current layers is " << strips_only.getHitLayers() << ", adding wildcard in layer " << wildcard_layer);
                    }
                    strips_only.setHitLayers(hit_layers);
                }

                // Because we are only iterating up to the *original* size, we can add new
                // entries to the end like this.
                // Require that we only keep roads that have enough hits to save time.
                working_roads[i] = std::move(spacepoints_only);
                if (strips_only.getNHitLayers() >= m_threshold) {
                    working_roads.push_back(strips_only);
                }
            }
        }
    }

    ATH_MSG_DEBUG("Created " << working_roads.size() << " new roads in spacepoint road filter.");

    // Now, any finalized roads need to have their sectors set/updated, and added to the output vector.
    for (auto road : working_roads) {
        unsigned numSpacePlusPixel = setSector(road);
        ATH_MSG_DEBUG("This road has sector = " << road.getSector() << " and q/pt = " << road.getX() << ", x = " << road.getY());
        ATH_MSG_DEBUG("numSpacePlusPixel = " << numSpacePlusPixel << ", nhitlayers = " << road.getNHitLayers());

        // Filter any roads that don't have enough real hits.
        if (road.getNHitLayers() < m_threshold) {
            continue;
        }

        // Filter any roads that don't have enough spacepoints + pixel hits.
        if (numSpacePlusPixel < m_minSpacePlusPixel) {
            continue;
        }

            m_postfilter_roads.push_back(road);
    }

    return retval;
}

unsigned FPGATrackSimSpacepointRoadFilterTool::setSector(FPGATrackSimRoad& road) {
    // We've now removed any ambiguity, and so can assign the road's sector.
    int sectorbin = road.getSectorBin();
    std::vector<module_t> modules;
    
    ATH_MSG_DEBUG("Road has sector bin ID: " << sectorbin << ", layers: " << road.getNLayers());
    unsigned num_pixel = 0;
    unsigned num_spacepoints = 0;
    for (unsigned int il = 0; il < road.getNLayers(); il++) {
        if (road.getNHits_layer()[il] == 0) {
            modules.push_back(-1);
        } else if (road.getNHits_layer()[il] == 1 && road.getHits(il)[0]->getHitType() == HitType::wildcard) {
            modules.push_back(-1);
            //ATH_MSG_INFO("  modules[" << il << "] = -1");
        } else {
            // Add 100 to the sector bin if this layer contains spacepoints.
            // We know at this stage that a layer will only have spacepoints or hits
            // so we can just check teh first hit.
            bool is_spacepoint = (road.getHits(il)[0]->getHitType() == HitType::spacepoint);
            bool is_pixel = (road.getHits(il)[0]->isPixel());
            modules.push_back(is_spacepoint ? sectorbin + fpgatracksim::SPACEPOINT_SECTOR_OFFSET : sectorbin);
            //ATH_MSG_INFO("  modules[" << il << "] = " << (is_spacepoint ? 100+sectorbin : sectorbin));
            if (is_spacepoint) {
                num_spacepoints += 1;
            }
            if (is_pixel) {
                num_pixel += 1;
            }
        }
    }

    unsigned numSpacePlusPixel = (num_spacepoints/2) + num_pixel;
    if (m_setSectors) {
        const FPGATrackSimSectorBank* sectorbank = m_isSecondStage ? m_FPGATrackSimBankSvc->SectorBank_2nd() : m_FPGATrackSimBankSvc->SectorBank_1st();
        // Written a bit unintuitively, but the "default" should be that it's not second stage
        // NOTE fix this when we add second stage roads back.
        road.setSector(sectorbank->findSector(modules));
    }
    return numSpacePlusPixel;
}

unsigned FPGATrackSimSpacepointRoadFilterTool::findUnique(std::vector<std::shared_ptr<const FPGATrackSimHit>>& sp_in, std::vector<std::shared_ptr<const FPGATrackSimHit>>& sp_out,
    std::vector<std::shared_ptr<const FPGATrackSimHit>>& unique_in, std::vector<std::shared_ptr<const FPGATrackSimHit>>& unique_out,
    std::vector<std::shared_ptr<const FPGATrackSimHit>>& new_sp_in, std::vector<std::shared_ptr<const FPGATrackSimHit>>& new_sp_out) {

    std::map<std::tuple<float, float, float>, std::shared_ptr<const FPGATrackSimHit>> merged_map;
    std::tuple<float, float, float> coords;

    // Loop over the inner spacepoints and fill the (x,y,z)->hit map.
    unsigned num_unique = 0;
    for (const auto& spacepoint : sp_in) {
        coords = {spacepoint->getX(), spacepoint->getY(), spacepoint->getZ()};
        merged_map.try_emplace(coords, spacepoint);
    }

    // Loop over the outer spacepoints. If a hit is *not* in the map already,
    // then it's unique. In which case we either convert it back to a normal hit
    // and add it to the "unique_out" vector, or we
    for (auto& spacepoint : sp_out) {
        coords = {spacepoint->getX(), spacepoint->getY(), spacepoint->getZ()};
        if (merged_map.count(coords) == 0) {
            merged_map.emplace(coords, spacepoint);
            if (!m_filtering) {
                std::unique_ptr<const FPGATrackSimHit> original = std::make_unique<FPGATrackSimHit>(spacepoint->getOriginalHit());
                unique_out.push_back(std::move(original));
            }
            num_unique += 1;
        } else {
            new_sp_out.push_back(spacepoint);
            // Remove this from the map to mark that we found it.
            merged_map.erase(coords);
        }
    }

    // Now loop over the inner hits a second time, and find the unique entries.
    // At this point, if it IS present in the map, it's a unique entry.
    for (auto& spacepoint : sp_in) {
        coords = {spacepoint->getX(), spacepoint->getY(), spacepoint->getZ()};
        if (merged_map.count(coords) != 0) {
            if (!m_filtering) {
                std::unique_ptr<const FPGATrackSimHit> original = std::make_unique<FPGATrackSimHit>(spacepoint->getOriginalHit());
                unique_in.push_back(std::move(original));
            }
            num_unique += 1;
        } else {
            new_sp_in.push_back(spacepoint);
        }
    }

    return num_unique;
}
