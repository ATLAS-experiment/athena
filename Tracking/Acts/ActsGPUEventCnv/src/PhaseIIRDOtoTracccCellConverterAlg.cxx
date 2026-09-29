/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PhaseIIRDOtoTracccCellConverterAlg.h"
#include "ActsGPUEvent/GeometryIdMapping.h"
#include "StoreGate/ReadHandle.h"
#include <cstdint>
#include <optional>

namespace ActsTrk {

using detray_id_type = GeometryIdMapping::detray_id_type;

StatusCode PhaseIIRDOtoTracccCellConverterAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing");

  ATH_CHECK(m_common.initialize());

  ATH_CHECK(m_ph2PixelRDOKey.initialize());
  ATH_CHECK(m_ph2StripRDOKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PhaseIIRDOtoTracccCellConverterAlg::execute(const EventContext& ctx) const
{
  using size_type = traccc::edm::silicon_cell_collection::buffer::size_type;
  using PixelRawDataContainerProxy = PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy;
  using PixelRawDataProxy = PhaseII::PixelRawDataTypeTraits<>::RawDataProxy;
  using StripRawDataContainerProxy = PhaseII::StripRawDataTypeTraits<>::RawDataContainerProxy;
  using StripRawDataProxy = PhaseII::StripRawDataTypeTraits<>::RawDataProxy;

  // ---- 0. Init
  auto ph2PixelRDOHandle = SG::makeHandle(m_ph2PixelRDOKey, ctx);
  ATH_CHECK(ph2PixelRDOHandle.isValid());
  auto ph2StripRDOHandle = SG::makeHandle(m_ph2StripRDOKey, ctx);
  ATH_CHECK(ph2StripRDOHandle.isValid());

  // ---- 1. Count Pixel and Strip hits: creating the traccc SoA requires
  // knowing their size upon creation.

  size_type nPix = 0, nStrip = 0;

  // Pixels
  auto pixel_rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(*ph2PixelRDOHandle);
  for (PixelRawDataContainerProxy module_rdo_container_proxy : pixel_rdo_container_collection_proxy) {
    if (!module_rdo_container_proxy.empty()) {
      nPix += module_rdo_container_proxy.size();
    }
  }

  // Strips
  auto strip_rdo_container_collection_proxy = PhaseII::makeRawDataCollectionProxy(*ph2StripRDOHandle);

  for (StripRawDataContainerProxy module_rdo_container_proxy : strip_rdo_container_collection_proxy) {
    if (!module_rdo_container_proxy.empty()) {
      for (StripRawDataProxy strip_rdo: module_rdo_container_proxy) {
        if (!m_common.passTiming(strip_rdo.getTimeBin())) {
            ATH_MSG_DEBUG("Strip failed timing check");
            continue;
        }
        nStrip += strip_rdo.getGroupSize();
      }
    }
  }

  size_type const nCells = nPix + nStrip;

  // ---- 2. Create the output cell buffer
  auto host_copy = m_common.m_copiesTool->hostCopy(ctx);
  traccc::edm::silicon_cell_collection::buffer traccc_cells_host_buffer{
    nCells, m_common.m_hostMR->mr()};
  host_copy->setup(traccc_cells_host_buffer)->wait();

  if (nCells == 0) {
    ATH_MSG_DEBUG("no input hits — writing empty cell collection");
    ATH_CHECK(m_common.copyToGpuAndRecordToSG(ctx, traccc_cells_host_buffer));
    return StatusCode::SUCCESS;
  }

  // Create a device collection around the buffer.
  traccc::edm::silicon_cell_collection::device cells{traccc_cells_host_buffer};

  // ---- 3. Convert RDOs to traccc cells
  // The traccc buffers are not default initialized: all members must be set.

  size_type cell_index = 0;
  detray_id_type current_geometry_id = detray::geometry::identifier{}.value();
  unsigned int current_det_cond_idx = -1;

  // Convert Pixel RDOs
  for (PixelRawDataContainerProxy module_rdo_container_proxy : pixel_rdo_container_collection_proxy) {
    if (module_rdo_container_proxy.empty()) {
      continue;
    }
    IdentifierHash const module_id_hash(module_rdo_container_proxy.identifyHash());
    Identifier const module_id = m_common.m_pixelID->wafer_id(module_id_hash);
    std::optional<detray_id_type> const detray_geometry_id_opt = m_common.m_geoIdMapping->athenaToDetray(module_id);
    if (!detray_geometry_id_opt.has_value()) {
      ATH_MSG_FATAL("No detray id found for Athena identifier " << module_id);
      return StatusCode::FAILURE;
    }
    const detray_id_type detray_geometry_id{detray_geometry_id_opt.value()};
    if (detray_geometry_id != current_geometry_id) {
      current_geometry_id = detray_geometry_id;
      std::optional<unsigned int> det_cond_idx_opt =
        m_common.m_geoIdMapping->detrayToDetDescIndex(current_geometry_id);
      if (!det_cond_idx_opt.has_value()) {
        ATH_MSG_FATAL("No detector conditions index found for detray identifier " << current_geometry_id);
        return StatusCode::FAILURE;
      }
      current_det_cond_idx = det_cond_idx_opt.value();
    }

    for (PixelRawDataProxy pixel_rdo: module_rdo_container_proxy) {
      float activation = 1.;
      if (m_common.m_UsePixelToTForCellActivation) {
        activation = static_cast<float>(pixel_rdo.getToT());
        if (activation == 0.) {
          ATH_MSG_ERROR("input data error: RDO must not have ToT=0; RDO index: "
            << cell_index);
        }
      }

      traccc::edm::silicon_cell cell = cells.at(cell_index++);
      cell.channel0() = static_cast<uint32_t>(pixel_rdo.coordinates()[0]);
      cell.channel1() = static_cast<uint32_t>(pixel_rdo.coordinates()[1]);
      cell.module_index() = current_det_cond_idx;
      cell.activation() = activation;
      cell.time() = 0;
    }
  }

  // Convert Strip RDOs
  for (StripRawDataContainerProxy module_rdo_container_proxy : strip_rdo_container_collection_proxy) {
    if (module_rdo_container_proxy.empty()) {
      continue;
    }
    IdentifierHash const module_id_hash(module_rdo_container_proxy.identifyHash());
    Identifier const module_id = m_common.m_stripID->wafer_id(module_id_hash);
    std::optional<detray_id_type> const detray_geometry_id_opt = m_common.m_geoIdMapping->athenaToDetray(module_id);
    if (!detray_geometry_id_opt.has_value()) {
      ATH_MSG_FATAL("No detray id found for Athena identifier " << module_id);
      return StatusCode::FAILURE;
    }
    const detray_id_type detray_geometry_id{detray_geometry_id_opt.value()};
    if (detray_geometry_id != current_geometry_id) {
      current_geometry_id = detray_geometry_id;
      std::optional<unsigned int> det_cond_idx_opt =
        m_common.m_geoIdMapping->detrayToDetDescIndex(current_geometry_id);
      if (!det_cond_idx_opt.has_value()) {
        ATH_MSG_FATAL("No detector conditions index found for detray identifier " << current_geometry_id);
        return StatusCode::FAILURE;
      }
      current_det_cond_idx = det_cond_idx_opt.value();
    }

    for (StripRawDataProxy strip_rdo: module_rdo_container_proxy) {
      if (!m_common.passTiming(strip_rdo.getTimeBin())) {
          ATH_MSG_DEBUG("Strip failed timing check");
          continue;
      }

      if (m_common.m_stripID->barrel_ec(module_id) == 0) {
        for (int i = 0; i < strip_rdo.getGroupSize(); ++i) {
          traccc::edm::silicon_cell cell = cells.at(cell_index++);
          cell.channel0() = static_cast<uint32_t>(strip_rdo.coordinates()[0] + i);
          cell.channel1() = 0;
          cell.module_index() = current_det_cond_idx;
          cell.activation() = 1.;
          cell.time() = 0;
        }
      } else {
        for (int i = 0; i < strip_rdo.getGroupSize(); ++i) {
          traccc::edm::silicon_cell cell = cells.at(cell_index++);
          cell.channel0() = 0;
          cell.channel1() = static_cast<uint32_t>(strip_rdo.coordinates()[0] + i);
          cell.module_index() = current_det_cond_idx;
          cell.activation() = 1.;
          cell.time() = 0;
        }
      }
    }
  }

  if (m_common.m_CPUCellSorting) {
    // ---- 4. Sort cells
    traccc::edm::silicon_cell_collection::buffer sorted_cells_host_buffer =
      m_common.sortCells(*host_copy, cells);

    // ---- 5. Copy host -> device buffer and write to StoreGate ---------------
    ATH_CHECK(m_common.copyToGpuAndRecordToSG(ctx, sorted_cells_host_buffer));
  } else {
    // In this case the GPU clusterization algorithm must be configured to sort
    // the cells.

    // ---- 4/5. Copy host -> device buffer and write to StoreGate ---------------
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

StatusCode PhaseIIRDOtoTracccCellConverterAlg::finalize()
{
  ATH_MSG_DEBUG("Finalizing.");

  ATH_CHECK(m_common.finalize());

  return StatusCode::SUCCESS;
}

} // namespace ActsTrk