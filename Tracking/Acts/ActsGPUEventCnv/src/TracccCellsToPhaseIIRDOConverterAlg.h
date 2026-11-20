/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCCELLSTOPHASEIIRDOCONVERTERALG_H
#define ACTSGPUEVENT_TRACCCCELLSTOPHASEIIRDOCONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "InDetRawData/PhaseIIPixelRawDataContainerMT.h"
#include "InDetRawData/PhaseIIStripRawDataContainer.h"
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "GaudiKernel/ServiceHandle.h"
#include "ActsGPUInterfaces/IDeviceDetectorDescriptionSvc.h"
#include <GaudiKernel/StatusCode.h>
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopiesTool.h"
#include "InDetIdentifier/PixelID.h"

namespace ActsTrk {

/*! Transfer Traccc cell collection from GPU and convert cells into Phase-II RDOs
 *
 * Together with ITkPixelDecodingTracccCellsCUDA, it provides a way to
 * decode the detector byte streams on GPU into the Phase-II RDOs.
 */

class TracccCellsToPhaseIIRDOConverterAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  SG::ReadHandleKey<traccc::edm::silicon_cell_collection::buffer> m_tracccCellsKey{this,
    "TracccCells", "TracccCells", "input traccc cell collection buffer"};
  SG::WriteHandleKey<PhaseIIPixelRawDataContainer> m_pixelRDOsKey{this,
    "PixelRDOs", "ITkPixelRDOs", "output pixel RDO container"};
  SG::WriteHandleKey<PhaseIIStripRawDataContainer> m_stripRDOsKey{this,
    "StripRDOs", "ITkStripRDOs", "output pixel RDO container"};

  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{this,
    "HostMR", "", "The host memory resource tool to use"};
  ToolHandle<AthDevice::ICopiesTool> m_copiesTool{this,
    "CopiesTool", "", "Tool that provides host and device copy objects"};

  ServiceHandle<ActsTrk::IDeviceDetectorDescriptionSvc> m_detDescSvc{this,
    "DeviceDetectorDescriptionSvc", "",
    "Service building the static device detector description"};
  const PixelID* m_pixelIdHelper{nullptr};

  using size_type = vecmem::edm::details::size_type;
  using TPixelRangeGuard = PhaseII::ContainerRangeGuard<PhaseII::DataRange, PhaseIIPixelRawDataContainerMT::ContainerPtr>;

  StatusCode registerCurrentModulePixel(
    PhaseIIPixelRawDataContainerMT & collection,
    PhaseIIPixelRawDataContainerMT::ContainerPtr & container,
    TPixelRangeGuard& range_guard, IdentifierHash id_hash, size_type & nb_rdos) const;
};

} // namespace ActsTrk

#endif // ACTSGPUEVENT_TRACCCCELLSTOPHASEIIRDOCONVERTERALG_H