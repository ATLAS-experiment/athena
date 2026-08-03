/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccCellValidationAlg.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"

#include <GaudiKernel/StatusCode.h>
#include <traccc/edm/silicon_cell_collection.hpp>

namespace {
  // A printer for traccc cells used to display values of unmatched cells
  template <typename SILICON_CELL_BASE>
  MsgStream & operator<<(MsgStream & ms, traccc::edm::silicon_cell<SILICON_CELL_BASE> const & cell) {
    ms << "c0: " << cell.channel0()
        << ", c1: " << cell.channel1()
        << ", mi: " << cell.module_index()
        << ", ac: " << cell.activation()
        << ", ti: " << cell.time()
        ;
    return ms;
  }
}

namespace ActsTrk {

StatusCode TracccCellValidationAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing");

  ATH_CHECK(m_referenceCellsKey.initialize());
  ATH_CHECK(m_cellsKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode TracccCellValidationAlg::execute(const EventContext& ctx) const
{
  using size_type = traccc::edm::silicon_cell_collection::buffer::size_type;

  auto reference_cells_handle = SG::makeHandle(m_referenceCellsKey, ctx);
  ATH_CHECK(reference_cells_handle.isValid());
  auto cells_handle = SG::makeHandle(m_cellsKey, ctx);
  ATH_CHECK(cells_handle.isValid());

  auto copy = m_deviceCopy->copy(ctx);

  // The data are on device so they need to be copied to host first
  traccc::edm::silicon_cell_collection::buffer reference_cells_host_buffer{
    copy->get_size(*reference_cells_handle), m_hostMR->mr()};
  copy->setup(reference_cells_host_buffer)->ignore();
  (*copy)(*reference_cells_handle, reference_cells_host_buffer)->wait();

  traccc::edm::silicon_cell_collection::buffer cells_host_buffer{
    copy->get_size(*cells_handle), m_hostMR->mr()};
  copy->setup(cells_host_buffer)->ignore();
  (*copy)(*cells_handle, cells_host_buffer)->wait();

  traccc::edm::silicon_cell_collection::const_device reference_cells{reference_cells_host_buffer};
  traccc::edm::silicon_cell_collection::const_device cells{cells_host_buffer};

  if (reference_cells.size() != cells.size()) {
    ATH_MSG_ERROR("different sizes");
    return StatusCode::FAILURE;
  }

  bool error = false;
  for (size_type i = 0; i < cells.size(); ++i) {
    if (cells.at(i) != reference_cells.at(i)) {
      error = true;
      ATH_MSG_ERROR("cells differ at " << i
          << ": ref=[" << reference_cells.at(i) << ']'
          << ", mon=[" << cells.at(i) << ']');
    }
  }

  return error ? StatusCode::FAILURE : StatusCode::SUCCESS;
}

} // namespace ActsTrk