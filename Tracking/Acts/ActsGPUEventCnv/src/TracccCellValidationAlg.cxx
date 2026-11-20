/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TracccCellValidationAlg.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"
#include <GaudiKernel/StatusCode.h>
#include <traccc/edm/silicon_cell_collection.hpp>

namespace {
  using size_type = traccc::edm::silicon_cell_collection::buffer::size_type;

  // A printer for traccc cells used to display values of unmatched cells
  template <typename SILICON_CELL_BASE>
  MsgStream & operator<<(MsgStream & ms, traccc::edm::silicon_cell<SILICON_CELL_BASE> const & cell) {
    ms << "mi: " << cell.module_index()
      << ", c0: " << cell.channel0()
      << ", c1: " << cell.channel1()
      << ", ac: " << cell.activation()
      << ", ti: " << cell.time()
      ;
    return ms;
  }

  struct CellOrder {
    traccc::edm::silicon_cell_collection::const_device const & m_cells;
    CellOrder(traccc::edm::silicon_cell_collection::const_device const & cells)
      : m_cells(cells)
    {}
    bool operator() (size_type const & lhs, size_type const & rhs) const {
      auto const & lc = m_cells.at(lhs);
      auto const & rc = m_cells.at(rhs);
      if (lc.module_index() < rc.module_index()) {
        return true;
      } else if (lc.module_index() > rc.module_index()) {
        return false;
      }
      if (lc.channel0() < rc.channel0()) {
        return true;
      } else if (lc.channel0() > rc.channel0()) {
        return false;
      }
      if (lc.channel1() < rc.channel1()) {
        return true;
      } else if (lc.channel1() > rc.channel1()) {
        return false;
      }
      // module_index, channel0, and channel1 are equal
      return true;
    }
  };
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
  copy->setup(vecmem::ignore_event, reference_cells_host_buffer);
  (*copy)(*reference_cells_handle, reference_cells_host_buffer)->wait();

  traccc::edm::silicon_cell_collection::buffer cells_host_buffer{
    copy->get_size(*cells_handle), m_hostMR->mr()};
  copy->setup(vecmem::ignore_event, cells_host_buffer);
  (*copy)(*cells_handle, cells_host_buffer)->wait();

  traccc::edm::silicon_cell_collection::const_device reference_cells{reference_cells_host_buffer};
  traccc::edm::silicon_cell_collection::const_device cells{cells_host_buffer};

  if (reference_cells.size() != cells.size()) {
    ATH_MSG_ERROR("different sizes:"
      << " reference: " << reference_cells.size()
      << " monitored: " << cells.size());
    return StatusCode::FAILURE;
  }

  std::vector<size_type> order_reference(cells.size());
  std::iota(order_reference.begin(), order_reference.end(), 0);
  std::vector<size_type> order_cells(cells.size());
  std::iota(order_cells.begin(), order_cells.end(), 0);

  if (m_SortBeforeCompare) {
    std::sort(order_reference.begin(), order_reference.end(), CellOrder(reference_cells));
    std::sort(order_cells.begin(), order_cells.end(), CellOrder(cells));
  }

  size_type nb_errors = 0;
  for (size_type i = 0; i < cells.size(); ++i) {
    const size_type ref_i = order_reference[i];
    const size_type mon_i = order_cells[i];
    if (cells.at(mon_i) != reference_cells.at(ref_i)) {
      nb_errors += 1;
      ATH_MSG_DEBUG("cells differ at " << i
          << ": ref=[" << reference_cells.at(ref_i) << ']'
          << ", mon=[" << cells.at(mon_i) << ']');
    }
  }

  if (nb_errors != 0) {
    ATH_MSG_ERROR(nb_errors << " cells mismatch");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG("all cells match");
    return StatusCode::SUCCESS;
  }
}

} // namespace ActsTrk