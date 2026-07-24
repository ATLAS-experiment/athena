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

namespace ActsTrk {

RDOtoTracccCellConverterCommons::RDOtoTracccCellConverterCommons(
      AthReentrantAlgorithm& parent)
  : AthMessaging{"RDOtoTracccCellConverterCommons"}
  , m_parent{parent}
  , m_tracccCellsKey{&parent, "TracccCells", "", "Output traccc cell collection buffer"}
  , m_hostMR{&parent, "HostMR", "", "The host memory resource tool to use"}
  , m_deviceMR{&parent, "DeviceMR", "", "The device memory resource tool to use"}
  , m_copiesTool{&parent, "CopiesTool", "", "Tool that provides host and device copy objects"}
  , m_detDescSvc{&parent, "DetectorDescriptionSvc", "ActsTrk::JSONDeviceDetectorDescriptionProviderSvc"}
  , m_hostCondObjectName{&parent, "HostConditionsObjectName", "",
      "Traccc host conditions object"}
  , m_CPUCellSorting{&parent, "CPUCellSorting", false,
      "Whether to sort traccc cells on CPU or GPU"}
  , m_UsePixelToTForCellActivation{&parent, "UsePixelToTForCellActivation", true,
      "Use Pixel hit time over threshold value to set traccc cell activation value, otherwise defaults to 1"}
{
}

StatusCode RDOtoTracccCellConverterCommons::initialize()
{
  ATH_CHECK(m_parent.detStore()->retrieve(m_pixelID, "PixelID"));
  ATH_CHECK(m_parent.detStore()->retrieve(m_stripID, "SCT_ID"));

  ATH_CHECK(m_hostMR.retrieve());
  ATH_CHECK(m_tracccCellsKey.initialize());
  ATH_CHECK(m_copiesTool.retrieve());

  m_athenaToDetray = &m_detDescSvc->athenaToDetrayMap();
  ATH_CHECK(m_parent.detStore()->retrieve(m_hostCond, m_hostCondObjectName.value()));

  const auto& gids = m_hostCond->geometry_id();
  m_DetrayIdToDetDescrIndexMap.reserve(gids.size());
  for (unsigned int i = 0; i < gids.size(); ++i) {
    m_DetrayIdToDetDescrIndexMap[gids[i].value()] = i;
  }
  ATH_MSG_INFO("Built detray→detcond map with "
      << m_DetrayIdToDetDescrIndexMap.size() << " entries");

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

} // namespace ActsTrk
