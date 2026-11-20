/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TracccCellsToPhaseIIRDOConverterAlg.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"
#include <GaudiKernel/StatusCode.h>
#include <traccc/edm/silicon_cell_collection.hpp>

namespace ActsTrk {

StatusCode TracccCellsToPhaseIIRDOConverterAlg::initialize()
{
  ATH_CHECK(m_tracccCellsKey.initialize());
  ATH_CHECK(m_pixelRDOsKey.initialize());
  ATH_CHECK(m_stripRDOsKey.initialize());
  ATH_CHECK(m_hostMR.retrieve());
  ATH_CHECK(m_copiesTool.retrieve());
  ATH_CHECK(m_detDescSvc.retrieve());
  ATH_CHECK(detStore()->retrieve(m_pixelIdHelper, "PixelID"));
  return StatusCode::SUCCESS;
}

StatusCode TracccCellsToPhaseIIRDOConverterAlg::execute(const EventContext& ctx) const
{
  auto device_copy = m_copiesTool->deviceCopy(ctx);
  auto host_copy = m_copiesTool->hostCopy(ctx);

  // Transfer cells from GPU
  auto gpu_cell_buffer = SG::makeHandle(m_tracccCellsKey, ctx);
  ATH_CHECK(gpu_cell_buffer.isValid());

  const size_type nb_cells = gpu_cell_buffer->capacity();
  traccc::edm::silicon_cell_collection::buffer host_cell_buffer(nb_cells, m_hostMR->mr());
  host_copy->setup(host_cell_buffer)->ignore();
  (*device_copy)(*gpu_cell_buffer, host_cell_buffer)->wait();

  // Detector description
  const std::vector<StaticCondEntry>& detector_entries = m_detDescSvc->staticCondEntries();

  // Create output containers
  size_type nb_pixels{0}, nb_strips{0};
  constexpr int container_list_size = 2;
  auto pixel_cont_coll = std::make_unique<PhaseIIPixelRawDataContainerMT>(
    m_pixelIdHelper->wafer_hash_max(), container_list_size);
  PhaseIIPixelRawDataContainerMT::ContainerPtr pixel_rdo_cont = pixel_cont_coll->getNewContainerPtr();
  TPixelRangeGuard pixel_range_guard(pixel_rdo_cont);

  traccc::edm::silicon_cell_collection::const_device cells{host_cell_buffer};
  unsigned int current_module_index = static_cast<unsigned int>(-1);
  IdentifierHash current_pixel_id_hash{0};
  bool current_is_pixel = false;
  for (size_type i = 0; i < nb_cells; ++i) {
    auto const & cell = cells.at(i);
    if (cell.module_index() != current_module_index) {

      // Processing cells for a new module: register the current one
      if (current_is_pixel && !pixel_range_guard.empty()) {
        ATH_CHECK(registerCurrentModulePixel(*pixel_cont_coll, pixel_rdo_cont,
          pixel_range_guard, current_pixel_id_hash, nb_pixels));
      }
      //TODO else if (!current_is_pixel && !strip_range_guard.empty()) {}

      current_module_index = cell.module_index();
      current_is_pixel = detector_entries[current_module_index].isPixel;
      if (current_is_pixel) {
        current_pixel_id_hash = m_pixelIdHelper->wafer_hash(
          detector_entries[current_module_index].athenaId);
      }
      //TODO else strip
    }

    if (current_is_pixel) {
      constexpr int bcid = 0, lvl0a = 0, lvl0d = 0;
      PhaseII::addDataForModule(*pixel_cont_coll, pixel_range_guard,
          std::array<std::int16_t, 2>{
              static_cast<std::int16_t>(cell.channel0()),
              static_cast<std::int16_t>(cell.channel1())},
          PhaseII::PixelRawDataContainer::makeWord(
              // Cells activation is + 1 compared to RDOs: see comments in
              // cells producers (PhaseII/RDOtoTracccCellConverterAlg for examples)
              cell.activation() - 1, bcid, lvl0a, lvl0d));
    } else {
      //TODO strip
    }
  }

  ATH_CHECK(registerCurrentModulePixel(*pixel_cont_coll, pixel_rdo_cont,
    pixel_range_guard, current_pixel_id_hash, nb_pixels));
  // TODO registerCurrentModuleStrip(...);

  // Record to StoreGate
  auto pixel_container_handle = SG::makeHandle(m_pixelRDOsKey, ctx);
  ATH_CHECK(pixel_container_handle.record(std::move(pixel_cont_coll)));

  ATH_MSG_DEBUG("nb input cells: " << nb_cells
    << ", nb output pixels: " << nb_pixels
    << ", nb output strips: " << nb_strips);
  return StatusCode::SUCCESS;
}

StatusCode TracccCellsToPhaseIIRDOConverterAlg::registerCurrentModulePixel(
  PhaseIIPixelRawDataContainerMT & collection,
  PhaseIIPixelRawDataContainerMT::ContainerPtr & container,
  TPixelRangeGuard& range_guard, IdentifierHash id_hash, size_type & nb_rdos) const
{
  if (!collection.registerOrEraseNewData(id_hash, range_guard.range())) {
      ATH_MSG_ERROR("pixel decoding error: data for this module was "
        << "already registered, older data were discarded"
        << ", module hash: " << id_hash);
      return StatusCode::FAILURE;
  }
  container = range_guard.ptr();
  nb_rdos += range_guard.range().size();
  range_guard = TPixelRangeGuard(container);
  return StatusCode::SUCCESS;
}

} // ActsTrk namespace
