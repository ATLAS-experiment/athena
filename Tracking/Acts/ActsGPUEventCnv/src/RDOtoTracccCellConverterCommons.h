/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_RDOTOTRACCCCELLCONVERTERCOMMONS_H
#define ACTSGPUEVENT_RDOTOTRACCCCELLCONVERTERCOMMONS_H

#include "AthenaBaseComps/AthMessaging.h"
#include "StoreGate/WriteHandleKey.h"
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopiesTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "Identifier/Identifier.h"
#include "GaudiKernel/ToolHandle.h"

#include "ActsGPUInterfaces/IDeviceDetectorDescriptionProviderSvc.h"

#include <traccc/io/csv/cell.hpp>

#include <unordered_map>
#include <cstdint>

class AthReentrantAlgorithm;

namespace ActsTrk {

/*! This struct contains the components common to RDOtoTracccCellConverterAlg
 *  and PhaseIIRDOtoTracccCellConverterAlg.
 *
 * Output key, copy objects, memory resources, detector descriptions and
 * conditions, athena to detray conversion maps, ...
 */

struct RDOtoTracccCellConverterCommons : public AthMessaging
{
  RDOtoTracccCellConverterCommons(AthReentrantAlgorithm& parent);

  StatusCode initialize();
  StatusCode finalize();

  AthReentrantAlgorithm& m_parent;

  SG::WriteHandleKey<traccc::edm::silicon_cell_collection::buffer> m_tracccCellsKey;

  mutable std::atomic<int> m_nPix = 0;
  mutable std::atomic<int> m_nStrip = 0;
  mutable std::atomic<int> m_nCells = 0;

  const PixelID* m_pixelID{nullptr};
  const SCT_ID*  m_stripID{nullptr};

  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR;
  ToolHandle<AthDevice::IMemoryResourceTool> m_deviceMR;
  ToolHandle<AthDevice::ICopiesTool> m_copiesTool;

  ServiceHandle<ActsTrk::IDeviceDetectorDescriptionProviderSvc> m_detDescSvc;
  Gaudi::Property<std::string> m_hostCondObjectName;
  const traccc::detector_conditions_description::host* m_hostCond{nullptr};

  // Geometry conversion maps
  const std::unordered_map<Identifier, uint64_t>* m_athenaToDetray{nullptr};
  std::unordered_map<uint64_t, unsigned int> m_DetrayIdToDetDescrIndexMap{};

  Gaudi::Property<bool> m_CPUCellSorting;
  Gaudi::Property<bool> m_UsePixelToTForCellActivation;

  // Returns the input cells sorted in a object setup with host_copy
  traccc::edm::silicon_cell_collection::buffer sortCells(
      vecmem::copy const & host_copy
    , traccc::edm::silicon_cell_collection::device const & cells
  ) const;
  StatusCode copyToGpuAndRecordToSG(
      EventContext const & ctx
    , traccc::edm::silicon_cell_collection::buffer const & cells
  ) const;
};

void sort_traccc_soa(
    traccc::edm::silicon_cell_collection::device & sorted_cells
  , traccc::edm::silicon_cell_collection::device const & cells
  );

} // namespace ActsTrk

#endif
