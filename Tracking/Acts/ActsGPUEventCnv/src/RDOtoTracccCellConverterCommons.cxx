/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RDOtoTracccCellConverterCommons.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthMessaging.h"
#include <GaudiKernel/IMessageSvc.h>

#include <algorithm>
#include <cstddef>

namespace ActsTrk {

RDOtoTracccCellConverterCommons::RDOtoTracccCellConverterCommons(
      AthReentrantAlgorithm& parent)
  : AthMessaging{"RDOtoTracccCellConverterCommons"}
  , m_parent{parent}
  , m_tracccCellsKey{&parent, "TracccCells", "", "Output traccc cell collection buffer"}
  , m_hostMR{&parent, "HostMR", "", "The host memory resource tool to use"}
  , m_deviceMR{&parent, "DeviceMR", "", "The device memory resource tool to use"}
  , m_copiesTool{&parent, "CopiesTool", "", "Tool that provides host and device copy objects"}
  , m_geoIdMappingObjectName{&parent, "GeoIdMappingObjectName", "",
      "StoreGate name for the detray/acts/athena geo id mapping"} 
  , m_hostDesignObjectName{&parent, "HostDigitizationObjectName", "",
      "Traccc host digitization object"}
  , m_CPUCellSorting{&parent, "CPUCellSorting", false,
      "Whether to sort traccc cells on CPU or GPU"}
  , m_UsePixelToTForCellActivation{&parent, "UsePixelToTForCellActivation", true,
      "Use Pixel hit time over threshold value to set traccc cell activation value, otherwise defaults to 1"}
  , m_stripRDOTimeBinStr{&parent, "timeBins", "Allowed time bins pattern for Strip RDOs"}
{
}

StatusCode RDOtoTracccCellConverterCommons::initialize()
{
  ATH_CHECK(m_parent.detStore()->retrieve(m_pixelID, "PixelID"));
  ATH_CHECK(m_parent.detStore()->retrieve(m_stripID, "SCT_ID"));

  ATH_CHECK(m_hostMR.retrieve());
  ATH_CHECK(m_tracccCellsKey.initialize());
  ATH_CHECK(m_copiesTool.retrieve());

  ATH_CHECK(m_parent.detStore()->retrieve(m_geoIdMapping, m_geoIdMappingObjectName.value()));
  ATH_CHECK(m_parent.detStore()->retrieve(m_hostDesign, m_hostDesignObjectName.value()));

  ATH_CHECK(decodeTimeBins());

  return StatusCode::SUCCESS;
}

StatusCode RDOtoTracccCellConverterCommons::finalize()
{
  //TODO: change to DEBUG once we figure how to set the log level of this class
  // from that of the parent class
  ATH_MSG_INFO("Read total number of pixel hits = " << m_nPix
      << ", total number of strip hits = " << m_nStrip
      << " and created total number of traccc cells = " << m_nCells
      );
  return StatusCode::SUCCESS;
}

traccc::edm::silicon_cell_collection::buffer RDOtoTracccCellConverterCommons::sortCells(
    vecmem::copy const & host_copy
  , traccc::edm::silicon_cell_collection::device const & cells
) const
{
  traccc::edm::silicon_cell_collection::buffer sorted_cells_host_buffer{
    cells.size(), m_hostMR->mr()};
  host_copy.setup(sorted_cells_host_buffer)->wait();
  traccc::edm::silicon_cell_collection::device sorted_cells{sorted_cells_host_buffer};
  sort_traccc_soa(sorted_cells, cells);
  return sorted_cells_host_buffer;
}

StatusCode RDOtoTracccCellConverterCommons::copyToGpuAndRecordToSG(
    EventContext const & ctx
  , traccc::edm::silicon_cell_collection::buffer const & cells
) const
{
  auto device_copy = m_copiesTool->deviceCopy(ctx);
  auto traccc_cells_gpu_buffer = std::make_unique<traccc::edm::silicon_cell_collection::buffer>(
    cells.capacity(), m_deviceMR->mr());

  // We ignore() the setup and wait() on the copy to allow parallelism.
  device_copy->setup(*traccc_cells_gpu_buffer)->ignore();
  (*device_copy)(cells, *traccc_cells_gpu_buffer)->wait();

  auto outputTracccCells = SG::makeHandle(m_tracccCellsKey, ctx);
  ATH_CHECK(outputTracccCells.record(std::move(traccc_cells_gpu_buffer)));
  return StatusCode::SUCCESS;
}

void sort_traccc_soa(
    traccc::edm::silicon_cell_collection::device & sorted_cells
  , traccc::edm::silicon_cell_collection::device const & cells
  )
{
  using size_type = traccc::edm::silicon_cell_collection::buffer::size_type;

  std::vector<size_type> indices(cells.size());
  std::iota(indices.begin(), indices.end(), 0u);

  // Sort the indices according to the cells.
  std::sort(indices.begin(), indices.end(),
      [&](size_type lhs, size_type rhs) {
        return cells.at(lhs) < cells.at(rhs);
      });

  // Fill an output container with the sorted cells.
  size_type s = 0;
  for (size_type i : indices) {
    sorted_cells.at(s++) = cells.at(i);
  }
}

StatusCode RDOtoTracccCellConverterCommons::decodeTimeBins()
{
  static const size_t MAX_BINS = 2;
  if (m_stripRDOTimeBinStr.size() > MAX_BINS) {
    ATH_MSG_WARNING("Time bin string has excess characters");
  }

  for (size_t i = 0; i < MAX_BINS; ++i) {
    switch (std::toupper(m_stripRDOTimeBinStr[i])) {
      case 'X': m_stripRDOTimeBinBits[i] = -1; break;
      case '0': m_stripRDOTimeBinBits[i] =  0; break;
      case '1': m_stripRDOTimeBinBits[i] =  1; break;
      default:
          ATH_MSG_FATAL("Invalid time bin string: " << m_stripRDOTimeBinStr);
          return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

bool RDOtoTracccCellConverterCommons::passTiming(const std::bitset<3>& timePattern) const
{
  // Convert the given timebin to a bit set and test each bit
  // if bit is -1 (i.e. X) it always passes, otherwise require exact match of 0/1
  // N.B. bitset has opposite order to the bit pattern we define
  if (m_stripRDOTimeBinBits[0] != -1 and timePattern.test(2) != static_cast<bool>(m_stripRDOTimeBinBits[0])) return false;
  if (m_stripRDOTimeBinBits[1] != -1 and timePattern.test(1) != static_cast<bool>(m_stripRDOTimeBinBits[1])) return false;
  if (m_stripRDOTimeBinBits[2] != -1 and timePattern.test(0) != static_cast<bool>(m_stripRDOTimeBinBits[2])) return false;
  return true;
}

} // namespace ActsTrk
