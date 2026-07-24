/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCCELLVALIDATOR_H
#define ACTSGPUEVENT_TRACCCCELLVALIDATOR_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "StoreGate/ReadHandleKey.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"

namespace ActsTrk {

/*! Algo that compares two traccc silicon_cell_collection
 *
 * Its main usage is to validate that a new RDO to Traccc cells conversion
 * algorithm produce the same cells as a reference version.
 *
 * Since the order of cells matters to the downstream algorithms, cells at the
 * same position must match.
 */

class TracccCellValidationAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  SG::ReadHandleKey<traccc::edm::silicon_cell_collection::const_view> m_referenceCellsKey{
    this, "ReferenceCells", "", "The reference cells which the other will be compared to"};
  SG::ReadHandleKey<traccc::edm::silicon_cell_collection::const_view> m_cellsKey{
    this, "Cells", "", "The cells to validate against the reference"};

  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{
    this, "HostMR", "", "Host memory resource provider"};
  ToolHandle<AthDevice::ICopyTool> m_deviceCopy{
    this, "DeviceCopyTool", "", "Device copy provider"};
};

} // namespace ActsTrk

#endif // ACTSGPUEVENT_RDOTOTRACCCCELLCONVERTERALG_H