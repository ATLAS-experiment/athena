/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceGBTSSeedingAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

// traccc EDM
#include "traccc/edm/silicon_cell_collection.hpp"
#include "traccc/edm/measurement_collection.hpp"

// vecmem
#include "vecmem/memory/memory_resource.hpp"

// ACTS logging
#include "ActsInterop/Logger.h"

#include <fstream>

namespace ActsTrk {

// -----------------------------------------------------------------------
StatusCode DeviceGBTSSeedingAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name());

  ATH_CHECK(m_seedingAlgProviderTool.retrieve());
  ATH_CHECK(m_deviceMR.retrieve());
  ATH_CHECK(m_inputPixelSPKey.initialize());
  ATH_CHECK(m_inputMeasKey.initialize());
  ATH_CHECK(m_outputPixelSeedsKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_pixelManager, "ITkPixel"));
  ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID") );
  ATH_CHECK(m_layerNumberTool.retrieve());
  ATH_CHECK(configureGBTS());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode DeviceGBTSSeedingAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Executing device GBTS seeding.");

  // ---- 1. Read input traccc measurements from StoreGate --------------------------------
  auto inputTracccPixelSpacepoints = SG::makeHandle(m_inputPixelSPKey, ctx);
  ATH_CHECK(inputTracccPixelSpacepoints.isValid());
  ATH_MSG_DEBUG("Read traccc spacepoints from '"
                         << inputTracccPixelSpacepoints.key() << "'");

  auto inputTracccMeasurements = SG::makeHandle(m_inputMeasKey, ctx);
  ATH_CHECK(inputTracccMeasurements.isValid());
  ATH_MSG_DEBUG("Read traccc measurements from '"
                         << inputTracccMeasurements.key() << "'");

  // ---- 2. Get traccc seeding alg ---------------------------------------------
  auto seeding_pair = m_seedingAlgProviderTool->getGBTSAlgorithm(ctx, m_gbts_config);
  std::shared_ptr<const traccc::device::gbts_seeding_algorithm> seeding_alg = seeding_pair.second;
  
  // ---- 3. Run traccc pixel seed formation ---------------------------------------------
  traccc::edm::seed_collection::buffer pixel_seeds_gpu_buffer = (*seeding_alg)(*inputTracccPixelSpacepoints, *inputTracccMeasurements);

  ATH_MSG_DEBUG("Reconstructed " << (seeding_pair.first)->get_size(pixel_seeds_gpu_buffer) << " pixel seeds.");

  // ---- 4. Write output traccc seeds to StoreGate -------------------------
  auto outputTracccPixelSeeds = SG::makeHandle(m_outputPixelSeedsKey, ctx);
  ATH_CHECK(outputTracccPixelSeeds.record(
    std::make_unique<traccc::edm::seed_collection::buffer>(
        std::move(pixel_seeds_gpu_buffer))));
  ATH_MSG_DEBUG("Wrote spacepoint buffer to '" << m_outputPixelSeedsKey.key() << "'");

  return StatusCode::SUCCESS;
}

StatusCode DeviceGBTSSeedingAlg::configureGBTS()
{

    const std::unordered_map<uint64_t, Identifier> detrayToAthena = m_detDescSvc->detrayToAthenaMap();
    // traccc-gbts defaults are ITk tuned
    // get layer linking scheme from the athena tools
    std::string conn_fileName =
        PathResolver::find_file(m_connectionFileName, "DATAPATH");
    if (conn_fileName.empty()) {
        ATH_MSG_FATAL("Cannot find layer connections file for GBTS "
                      << conn_fileName);
        return StatusCode::FAILURE;
    }
    std::ifstream ifs(conn_fileName.c_str());
    std::unique_ptr<GNN_FASTRACK_CONNECTOR> gbts_connector =
        std::make_unique<GNN_FASTRACK_CONNECTOR>(ifs, false);
    ATH_MSG_INFO("Layer connections are initialized from file for GBTS "
                 << conn_fileName);

    const std::vector<TrigInDetSiLayer>* pVL =
        m_layerNumberTool->layerGeometry();
    std::vector<TrigInDetSiLayer> layerGeometry;
    std::copy(pVL->begin(), pVL->end(), std::back_inserter(layerGeometry));
    
    std::unique_ptr<TrigFTF_GNN_Geometry> GBTS_geo =
        std::make_unique<TrigFTF_GNN_Geometry>(layerGeometry, gbts_connector);
    
    traccc::device::gbts_layerInfo layerInfo;
    // convert save and convert layer info to SoA
    layerInfo.reserve(GBTS_geo->num_layers());
   
    for (unsigned int index = 0; index < GBTS_geo->num_layers(); ++index) {
        const TrigFTF_GNN_Layer* layer =
            GBTS_geo->getTrigFTF_GNN_LayerByIndex(index);
        // pixel barrel=0 pixel endcap=1 pixel inc. barrel=2 strip=3
        int vol_id =
            (layer->m_layer.m_subdet - (layer->m_layer.m_subdet % 1000)) / 1000;
        int is_inc_barrel = (vol_id == 97) | (vol_id == 95) | (vol_id == 93) |
                            (vol_id == 77) | (vol_id == 75) | (vol_id == 73);
        char type = (layer->m_layer.m_type != 0) + is_inc_barrel;
        if (layer->m_layer.m_subdet <= 20000) {
            type = 3;
        }
        // eta prediction cut occurs for type=0 and cluster width cut for type=1
        layerInfo.addLayer(type, layer->m_bins[0], layer->num_bins(),
                           layer->m_minEta, layer->m_etaBin);
    }
    
    const std::vector<short>* pixel_h2l = m_layerNumberTool->pixelLayers();
    

    std::vector<std::pair<std::uint64_t, short>> identifierBinning;
    identifierBinning.reserve(detrayToAthena.size());
    
    // construct identifier -> layer table
    IdContext pixel_context = m_pixelID->wafer_context();
    for (std::pair<std::uint64_t, Identifier> dToI : detrayToAthena) {
        if (m_pixelManager->identifierBelongs(dToI.second)) {
            IdentifierHash idHash = 0;
            m_pixelID->get_hash(dToI.second, idHash, &pixel_context);
            identifierBinning.push_back(std::make_pair(
                dToI.first, pixel_h2l->at(static_cast<int>(idHash))));
        }
    }
    ATH_MSG_INFO(identifierBinning.size() << " identifiers with a layer");

    std::vector<std::pair<unsigned int, std::vector<unsigned int>>> binGroups;
    {
        const auto rawBinGroups = GBTS_geo->bin_groups();
        binGroups.reserve(rawBinGroups.size());
        for (const auto& p : rawBinGroups) {
            binGroups.emplace_back(static_cast<unsigned int>(p.first),
                                    std::vector<unsigned int>(p.second.begin(), p.second.end()));
        }
    }

    for (auto& pair : binGroups) {
        bool barrel = (pair.first <= 83);  // unsigned, so `0 <= pair.first` is always true — dropped
        if (barrel) {
            pair.second.push_back(pair.first);
        }
    }

    // traccc::gbts_seedfinder_config gbts_config; 
    if (!m_gbts_config.setLinkingScheme(binGroups, layerInfo, identifierBinning,
                                    900.0f, makeActsAthenaLogger(this, "GBTSConfig")))
        return StatusCode::FAILURE;

    return StatusCode::SUCCESS;
}


} // namespace ActsTrk