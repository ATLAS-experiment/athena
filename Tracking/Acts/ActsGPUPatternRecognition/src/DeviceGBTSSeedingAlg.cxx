/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceGBTSSeedingAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

// traccc EDM
#include "traccc/edm/silicon_cell_collection.hpp"
#include "traccc/edm/measurement_collection.hpp"
#include "PixelReadoutGeometry/PixelDetectorManager.h"

// vecmem
#include "vecmem/memory/memory_resource.hpp"

// ACTS logging
#include "ActsInterop/Logger.h"

#include <fstream>
#include <memory>
#include <cstdint>
#include <unordered_map>
#include <algorithm>
#include <vector>

namespace ActsTrk {

// -----------------------------------------------------------------------
StatusCode DeviceGBTSSeedingAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name());

  ATH_CHECK(m_seedingAlgProviderTool.retrieve());
  ATH_CHECK(m_inputPixelSPKey.initialize());
  ATH_CHECK(m_inputMeasKey.initialize());
  ATH_CHECK(m_outputPixelSeedsKey.initialize());
  ATH_CHECK(detStore()->retrieve(m_idMapping, m_geoIdMappingObjectName.value()));

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
  auto seeding_alg = m_seedingAlgProviderTool->getGBTSAlgorithm(ctx, m_gbts_config);

  // ---- 3. Run traccc pixel seed formation ---------------------------------------------
  traccc::edm::seed_collection::buffer pixel_seeds_gpu_buffer = (*seeding_alg)(*inputTracccPixelSpacepoints, *inputTracccMeasurements);

  ATH_MSG_DEBUG("Reconstructed " << seeding_alg.copy().get_size(pixel_seeds_gpu_buffer) << " pixel seeds.");

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

    // traccc-gbts defaults are ITk tuned

    // The layer tool builds the GBTS layers, in dense layer index order, and
    // knows which layer each module hash belongs to and what it is made of.
    const std::vector<Acts::Experimental::GbtsLayerDescription>& layers =
      m_layerNumberTool->layerDescriptions();

    m_pixelHashToLayer = &m_layerNumberTool->pixelLayers();
    m_stripHashToLayer = &m_layerNumberTool->stripLayers();

    std::vector<Acts::Experimental::GbtsLayerConnection> connections;
    float etaBinWidth = 0.0f;
    ATH_CHECK(m_layerNumberTool->readConnections(layers, connections, etaBinWidth, m_connectorInputFile, true, false));

    // create geoemtry object that holds allowed pairing of allowed eta regions in each layer
    // holds all geometry information (m_layergeomtry and connection table)
    auto gbtsGeo = std::make_shared<Acts::Experimental::GbtsGeometry>(
      layers, connections, etaBinWidth, Acts::Experimental::GbtsZ0Range{}, logger());

    traccc::device::gbts_layerInfo layerInfo;
    // convert save and convert layer info to SoA
    layerInfo.reserve(static_cast<unsigned int>(gbtsGeo->numLayers()));

    for (unsigned int index = 0; index < gbtsGeo->numLayers(); ++index) {
      Acts::Experimental::GbtsLayerBinning binning = gbtsGeo->layerBinning(index);
      Acts::Experimental::GbtsLayerDescription desc =
          gbtsGeo->layerDescription(index);
      int vol_id = (desc.id - (desc.id % 1000)) / 1000;
      bool is_inc_barrel = (vol_id == 97) | (vol_id == 95) | (vol_id == 93) |
                           (vol_id == 77) | (vol_id == 75) | (vol_id == 73);
      char type = 0;
      if (desc.technology == Acts::Experimental::GbtsLayerTechnology::Strip) {
        type = 3;
      } else if (is_inc_barrel) {
        type = 2;
      } else if (desc.type == Acts::Experimental::GbtsLayerType::Endcap) {
        type = 1;
      }
      layerInfo.addLayer(type, binning.firstBin, binning.numBins, binning.minEta,
                         binning.etaBinWidth);
    }

    std::vector<std::pair<std::uint64_t, short>> identifierBinning;
    identifierBinning.reserve(m_idMapping->size());

    // construct identifier -> layer table
    IdContext pixel_context = m_pixelID->wafer_context();
    for (const auto& [detrayId, compactId] : m_idMapping->detrayToAthenaMap()) {
        Identifier athenaId(compactId);
        if (m_pixelManager->identifierBelongs(athenaId)) {
            IdentifierHash idHash{};//default c'tor produces detectable invalid hash
            int rc = m_pixelID->get_hash(athenaId, idHash, &pixel_context); //rc=0 is ok
            if (rc!=0)[[unlikely]] continue;
            const short layer = m_pixelHashToLayer->at(static_cast<int>(idHash));
            if (layer == IGbtsLayerTool::kNoLayer) [[unlikely]] continue;
            identifierBinning.push_back(std::make_pair(
                detrayId, layer));
        }
    }
    ATH_MSG_INFO(identifierBinning.size() << " identifiers with a layer");

    std::vector<std::pair<unsigned int, std::vector<unsigned int>>> binGroups;
    for (Acts::Experimental::GbtsBinGroup groups : gbtsGeo->binGroups()) {
      binGroups.emplace_back(groups.bin, groups.links);
    }

    if (!m_gbts_config.setLinkingScheme(binGroups, std::move(layerInfo), identifierBinning,
                                    900.0f, makeActsAthenaLogger(this, "GBTSConfig")))
        return StatusCode::FAILURE;

    return StatusCode::SUCCESS;
}


} // namespace ActsTrk
