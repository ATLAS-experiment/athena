/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_RDOTOTRACCCCELLCONVERTERCOMMONS_H
#define ACTSGPUEVENT_RDOTOTRACCCCELLCONVERTERCOMMONS_H

#include "AthenaBaseComps/AthMessaging.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "ActsGPUEvent/TracccDetectorDesignDescription.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopiesTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "Identifier/Identifier.h"
#include "GaudiKernel/ToolHandle.h"
#include "ActsGPUEvent/GeometryIdMapping.h"

#include <atomic>
#include <unordered_map>
#include <cstdint>
#include <bitset>

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

  Gaudi::Property<std::string> m_geoIdMappingObjectName;
  const ActsTrk::GeometryIdMapping* m_geoIdMapping{nullptr};

  // The detector design description is geometry-static (unlike the
  // conditions description), so it is retrieved once in initialize()
  // rather than through a per-event ReadCondHandle.
  Gaudi::Property<std::string> m_hostDesignObjectName;
  const traccc::detector_design_description::host* m_hostDesign{nullptr};


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

  StringProperty m_stripRDOTimeBinStr;
  int m_stripRDOTimeBinBits[3]{-1, -1, -1};
  // decode the property string into bits
  StatusCode decodeTimeBins();
  // check if the time pattern matches the requirements of strip RDO "timeBins"
  bool passTiming(const std::bitset<3>& timePattern) const;
};

void sort_traccc_soa(
    traccc::edm::silicon_cell_collection::device & sorted_cells
  , traccc::edm::silicon_cell_collection::device const & cells
  );

} // namespace ActsTrk

#endif
