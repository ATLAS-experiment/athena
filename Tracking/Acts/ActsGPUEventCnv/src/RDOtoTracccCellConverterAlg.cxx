/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "RDOtoTracccCellConverterAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "InDetRawData/PixelRDORawData.h"
#include "InDetRawData/SCT_RDORawData.h"
#include "traccc/io/csv/cell.hpp"

#include <algorithm>
#include <stdexcept>

// Comparator for sorting cells by module index then channel
struct cell_order_module_indices {
  bool operator()(const traccc::io::csv::cell& lhs,
                  const traccc::io::csv::cell& rhs) const {
    if (lhs.geometry_id != rhs.geometry_id)
      return lhs.geometry_id < rhs.geometry_id;
    if (lhs.channel1 != rhs.channel1)
      return lhs.channel1 < rhs.channel1;
    return lhs.channel0 < rhs.channel0;
  }
};

namespace ActsTrk {

StatusCode RDOtoTracccCellConverterAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing.");

  ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));
  ATH_CHECK(m_pixelRDOKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_stripID, "SCT_ID"));
  ATH_CHECK(m_stripRDOKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_pixelManager));
  ATH_CHECK(detStore()->retrieve(m_stripManager));

  ATH_CHECK(m_hostMR.retrieve());
  ATH_CHECK(m_tracccCellsKey.initialize());
  ATH_CHECK(m_copy.retrieve());

  m_athenaToDetray = &m_detDescSvc->athenaToDetrayMap();
  ATH_CHECK(detStore()->retrieve(m_hostCond, m_hostCondObjectName.value()));

  const auto& gids = m_hostCond->geometry_id();
  m_DetrayIdToDetDescrIndexMap.reserve(gids.size());
  for (unsigned int i = 0; i < gids.size(); ++i) {
    m_DetrayIdToDetDescrIndexMap[gids[i].value()] = i;
  }
  ATH_MSG_INFO("Built detray→detcond map with "
               << m_DetrayIdToDetDescrIndexMap.size() << " entries");

  return StatusCode::SUCCESS;
}

StatusCode RDOtoTracccCellConverterAlg::execute(const EventContext& ctx) const
{

  // ---- 1. Read RDOs from StoreGate ----------------------------------------
  std::vector<const InDetRawDataCollection<PixelRDORawData>*> pixel_rdos;
  std::vector<const InDetRawDataCollection<SCT_RDORawData>*>  strip_rdos;
  {
    auto pixelRDOHandle = SG::makeHandle(m_pixelRDOKey, ctx);
    ATH_CHECK(pixelRDOHandle.isValid());
    pixel_rdos.reserve(pixelRDOHandle->fullSize());
    for (const auto* col : *pixelRDOHandle) {
      if (col) pixel_rdos.push_back(col);
    }

    auto stripRDOHandle = SG::makeHandle(m_stripRDOKey, ctx);
    ATH_CHECK(stripRDOHandle.isValid());
    strip_rdos.reserve(stripRDOHandle->fullSize());
    for (const auto* col : *stripRDOHandle) {
      if (col) strip_rdos.push_back(col);
    }
  }

  // ---- 2. Convert RDOs to AoS cells ---------------------------------------
  int nPix = 0, nStrip = 0;
  std::vector<traccc::io::csv::cell> cells_aos;

  // 8 here is for the time being a placeholder
  // the user will need to provde the correct cell activation values and time

  float staticToTValue = 8.f;
  float staticTimeStampValue = 1.f;

  ATH_MSG_DEBUG("Reading pixel hits");
  for (const auto* coll : pixel_rdos) {
    for (const PixelRDORawData* rdo : *coll) {
      const Identifier rdoId = rdo->identify();
      const InDetDD::SiDetectorElement* el =
          m_pixelManager->getDetectorElement(rdoId);
      if (!el) continue;
      const Identifier modId = el->identify();
      const InDetDD::SiCellId cellId = el->cellIdFromIdentifier(rdoId);
      const uint64_t geoId = m_athenaToDetray->at(modId);

      cells_aos.push_back({geoId, 0,
          static_cast<uint32_t>(cellId.phiIndex()),
          static_cast<uint32_t>(cellId.etaIndex()),
          staticTimeStampValue, static_cast<float>(rdo->getToT())});
      ++nPix;
    }
  }
  ATH_MSG_DEBUG("Read " << nPix << " pixel hits");

  ATH_MSG_DEBUG("Reading strip hits");
  for (const auto* coll : strip_rdos) {
    for (const SCT_RDORawData* rdo : *coll) {
      const Identifier rdoId = rdo->identify();
      const InDetDD::SiDetectorElement* el =
          m_stripManager->getDetectorElement(rdoId);
      if (!el) continue;
      const Identifier modId = el->identify();
      const InDetDD::SiCellId cellId = el->cellIdFromIdentifier(rdoId);
      const uint64_t geoId = m_athenaToDetray->at(modId);

      if (m_stripID->barrel_ec(modId) == 0) {
        for (int i = 0; i < rdo->getGroupSize(); ++i) {
          cells_aos.push_back({geoId, 0,
              static_cast<uint32_t>(cellId.phiIndex() + i), 0, staticTimeStampValue, staticToTValue});
          ++nStrip;
        }
      } else {
        for (int i = 0; i < rdo->getGroupSize(); ++i) {
          cells_aos.push_back({geoId, 0, 0,
              static_cast<uint32_t>(cellId.phiIndex() + i), staticTimeStampValue, staticToTValue});
          ++nStrip;
        }
      }
    }
  }
  ATH_MSG_DEBUG("Read " << nStrip << " strip hits");

  if (cells_aos.empty()) {
    ATH_MSG_DEBUG("Created traccc cells: 0");
    return StatusCode::SUCCESS;
  }

  // ---- 3. Sort cells -------------------------------------------------------
  std::sort(cells_aos.begin(), cells_aos.end(), cell_order_module_indices{});

  // ---- 4. Convert to traccc cell collection (host collection) ---------------------
  // Build the host collection using the memory resource from the tool
  traccc::edm::silicon_cell_collection::host cells_soa{m_hostMR->mr()};

  uint64_t current_geometry_id = cells_aos[0].geometry_id;
  unsigned int current_det_cond_idx = m_DetrayIdToDetDescrIndexMap.at(current_geometry_id);

  for (const auto& cell : cells_aos) {
    if (cell.geometry_id != current_geometry_id) {
      current_geometry_id = cell.geometry_id;
      current_det_cond_idx = m_DetrayIdToDetDescrIndexMap.at(current_geometry_id);
    }
    cells_soa.push_back({cell.channel0, cell.channel1, cell.value,
                         cell.timestamp, current_det_cond_idx});
  }
  ATH_MSG_DEBUG("Created traccc cells: " << cells_soa.size());

  // ---- 5. Copy host -> device buffer and write to StoreGate ---------------
  auto copy = m_copy->copy(ctx);
  auto traccc_cells_buffer = std::make_unique<traccc::edm::silicon_cell_collection::buffer>(
    static_cast<unsigned int>(cells_soa.size()), m_deviceMR->mr());
  copy->setup(*traccc_cells_buffer)->wait();
  (*copy)(vecmem::get_data(cells_soa), *traccc_cells_buffer)->wait();

  ATH_MSG_DEBUG("Creating " << copy->get_size(*traccc_cells_buffer) << " cells.");

  auto outputTracccCells = SG::makeHandle(m_tracccCellsKey, ctx);
  ATH_CHECK(outputTracccCells.record(std::move(traccc_cells_buffer)));

  m_nPix += nPix;
  m_nStrip += nStrip;
  m_nCells += cells_soa.size();

  ATH_MSG_DEBUG("Wrote " << cells_soa.size() << " cells to '"
            << m_tracccCellsKey.key() << "'");

  return StatusCode::SUCCESS;
}

StatusCode RDOtoTracccCellConverterAlg::finalize()
{
  ATH_MSG_DEBUG("Finalizing.");

  ATH_MSG_DEBUG("Read total number of pixel hits = " << m_nPix
                  << ", total number of strip hits = " << m_nStrip
                  << " and created total number of traccc cells = " << m_nCells);

  return StatusCode::SUCCESS;
}

} // namespace ActsTrk