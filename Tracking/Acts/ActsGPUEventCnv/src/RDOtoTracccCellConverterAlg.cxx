/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "RDOtoTracccCellConverterAlg.h"
#include "StoreGate/ReadHandle.h"
#include <InDetRawData/SCT3_RawData.h>

namespace ActsTrk {

StatusCode RDOtoTracccCellConverterAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing");

  ATH_CHECK(m_common.initialize());

  ATH_CHECK(m_pixelRDOKey.initialize());
  ATH_CHECK(m_stripRDOKey.initialize());

  ATH_MSG_DEBUG("Reading from Pixel RDO key: " << m_pixelRDOKey.key());
  ATH_MSG_DEBUG("Reading from Strip RDO key: " << m_stripRDOKey.key());

  ATH_CHECK(detStore()->retrieve(m_pixelManager, m_pixelManagerKey));
  ATH_CHECK(detStore()->retrieve(m_stripManager, m_stripManagerKey));
  ATH_CHECK(decodeTimeBins());

  return StatusCode::SUCCESS;
}


// Two boolean checks taken from ACTS StripClusteringTool

bool RDOtoTracccCellConverterAlg::passTiming(const std::bitset<3>& timePattern) const {
  // Convert the given timebin to a bit set and test each bit
  // if bit is -1 (i.e. X) it always passes, other wise require exact match of 0/1
  // N.B bitset has opposite order to the bit pattern we define
  if (m_timeBinBits[0] != -1 and timePattern.test(2) != static_cast<bool>(m_timeBinBits[0])) return false;
  if (m_timeBinBits[1] != -1 and timePattern.test(1) != static_cast<bool>(m_timeBinBits[1])) return false;
  if (m_timeBinBits[2] != -1 and timePattern.test(0) != static_cast<bool>(m_timeBinBits[2])) return false;
  return true;
}

StatusCode RDOtoTracccCellConverterAlg::decodeTimeBins()
{
    for (size_t i = 0; i < m_timeBinStr.size(); i++) {
	if (i >= 3) {
	    ATH_MSG_WARNING("Time bin string has excess characters");
	    break;
	}
	switch (std::toupper(m_timeBinStr[i])) {
	case 'X': m_timeBinBits[i] = -1; break;
	case '0': m_timeBinBits[i] =  0; break;
	case '1': m_timeBinBits[i] =  1; break;
	default:
	    ATH_MSG_FATAL("Invalid time bin string: " << m_timeBinStr);
	    return StatusCode::FAILURE;
	}
    }
    return StatusCode::SUCCESS;
}

StatusCode RDOtoTracccCellConverterAlg::execute(const EventContext& ctx) const
{
  using size_type = traccc::edm::silicon_cell_collection::buffer::size_type;

  // ---- 0. Init
  auto pixelRDOHandle = SG::makeHandle(m_pixelRDOKey, ctx);
  ATH_CHECK(pixelRDOHandle.isValid());
  auto stripRDOHandle = SG::makeHandle(m_stripRDOKey, ctx);
  ATH_CHECK(stripRDOHandle.isValid());

  // ---- 1. Count Pixel and Strip hits: creating the traccc SoA requires
  // knowing their size upon creation.

  size_type nPix = 0, nStrip = 0;

  for (const auto* coll : *pixelRDOHandle) {
    if (coll) {
      nPix += coll->size();
    }
  }
  for (const auto* coll : *stripRDOHandle) {
    if (coll) {
      for (const SCT_RDORawData* rdo : *coll) {
        //Check type in debug build otherwise assume it is correct
        assert(dynamic_cast<const SCT3_RawData*>(rdo)!=nullptr);
        const SCT3_RawData* raw3 = static_cast<const SCT3_RawData*>(rdo);

        std::bitset<3> timePattern(raw3->getTimeBin());
        if (!passTiming(timePattern)) {
            ATH_MSG_DEBUG("Strip failed timing check");
            continue;
        }
        nStrip += rdo->getGroupSize();
      }
    }
  }

  ATH_MSG_DEBUG("Found " << nPix << " Pixel RDOs and " << nStrip
                << " Strip RDOs, total " << (nPix + nStrip) << " RDOs");
  size_type const nCells = nPix + nStrip;

  if (nCells == 0) {
    ATH_MSG_DEBUG("no input hits");
    return StatusCode::SUCCESS;
  }

  // ---- 2. Create the output cell buffer.
  auto host_copy = m_common.m_copiesTool->hostCopy(ctx);
  traccc::edm::silicon_cell_collection::buffer traccc_cells_host_buffer{
    nCells, m_common.m_hostMR->mr()};
  host_copy->setup(traccc_cells_host_buffer)->wait();

  // Create a "device" collection around the buffer to work on it
  traccc::edm::silicon_cell_collection::device cells{traccc_cells_host_buffer};

  // ---- 3. Convert RDOs to traccc cells
  // The traccc buffers are not default initialized: all members must be set.

  size_type cell_index = 0;
  uint64_t current_geometry_id = detray::geometry::identifier{}.value();
  unsigned int current_det_cond_idx = -1;

  // Convert Pixel RDOs
  for (const auto* coll : *pixelRDOHandle) {

    for (const PixelRDORawData* rdo : *coll) {
      const Identifier rdoId = rdo->identify();
      const InDetDD::SiDetectorElement* el =
          m_pixelManager->getDetectorElement(rdoId);
      if (!el) continue;
      const Identifier modId = el->identify();
      const InDetDD::SiCellId cellId = el->cellIdFromIdentifier(rdoId);
      const uint64_t geoId = m_common.m_athenaToDetray->at(modId);

      if (geoId != current_geometry_id) {
        current_geometry_id = geoId;
        current_det_cond_idx = m_common.m_DetrayIdToDetDescrIndexMap.at(current_geometry_id);
      }

      float activation = 1.;
      if (m_common.m_UsePixelToTForCellActivation) {
        activation = static_cast<float>(rdo->getToT());
        if (activation == 0.) {
          ATH_MSG_ERROR("input data error: RDO must not have ToT=0; RDO index: "
            << cell_index);
        }
      }

      traccc::edm::silicon_cell cell = cells.at(cell_index++);
      cell.channel0() = static_cast<uint32_t>(cellId.phiIndex());
      cell.channel1() = static_cast<uint32_t>(cellId.etaIndex());
      cell.module_index() = current_det_cond_idx;
      cell.activation() = activation;
      cell.time() = 0;
    }
  }

  // Convert Strip RDOs
  for (const auto* coll : *stripRDOHandle) {
    for (const SCT_RDORawData* rdo : *coll) {
      //Check type in debug build otherwise assume it is correct
      assert(dynamic_cast<const SCT3_RawData*>(rdo)!=nullptr);
      const SCT3_RawData* raw3 = static_cast<const SCT3_RawData*>(rdo);

      std::bitset<3> timePattern(raw3->getTimeBin());
      if (!passTiming(timePattern)) {
          ATH_MSG_DEBUG("Strip failed timing check");
          continue;
      }

      const Identifier rdoId = rdo->identify();
      const InDetDD::SiDetectorElement* el =
          m_stripManager->getDetectorElement(rdoId);
      if (!el) continue;
      const Identifier modId = el->identify();
      const InDetDD::SiCellId cellId = el->cellIdFromIdentifier(rdoId);
      const uint64_t geoId = m_common.m_athenaToDetray->at(modId);

      if (geoId != current_geometry_id) {
        current_geometry_id = geoId;
        current_det_cond_idx = m_common.m_DetrayIdToDetDescrIndexMap.at(current_geometry_id);
      }

      if (m_common.m_stripID->barrel_ec(modId) == 0) {
        for (int i = 0; i < rdo->getGroupSize(); ++i) {
          
          traccc::edm::silicon_cell cell = cells.at(cell_index++);
          cell.channel0() = static_cast<uint32_t>(cellId.phiIndex() + i);
          cell.channel1() = 0;
          cell.module_index() = current_det_cond_idx;
          cell.activation() = 1.;
          cell.time() = 0;
           
        }
      } else {
        for (int i = 0; i < rdo->getGroupSize(); ++i) {
          
          traccc::edm::silicon_cell cell = cells.at(cell_index++);
          cell.channel0() = 0;
          cell.channel1() = static_cast<uint32_t>(cellId.phiIndex() + i);
          cell.module_index() = current_det_cond_idx;
          cell.activation() = 1.;
          cell.time() = 0;
          
        }
      }
    }
  }

  // ---- 4. Sort cells
  if (m_common.m_CPUCellSorting) {
    traccc::edm::silicon_cell_collection::buffer sorted_cells_host_buffer =
      m_common.sortCells(*host_copy, cells);

    // ---- 5. Copy host -> device buffer and write to StoreGate ---------------
    ATH_CHECK(m_common.copyToGpuAndRecordToSG(ctx, sorted_cells_host_buffer));
  } else {
    // In this case the GPU clusterization algorithm must be configured to sort
    // the cells.

    // ---- 4/5. Copy host -> device buffer and write to StoreGate
    ATH_CHECK(m_common.copyToGpuAndRecordToSG(ctx, traccc_cells_host_buffer));
  }

  // ---- 6. Accounting
  m_common.m_nPix += nPix;
  m_common.m_nStrip += nStrip;
  m_common.m_nCells += nCells;

  ATH_MSG_DEBUG("Wrote " << nCells << " cells to '"
            << m_common.m_tracccCellsKey.key() << "'");
  return StatusCode::SUCCESS;
}

StatusCode RDOtoTracccCellConverterAlg::finalize()
{
  ATH_MSG_DEBUG("Finalizing.");

  ATH_CHECK(m_common.finalize());

  return StatusCode::SUCCESS;
}

} // namespace ActsTrk