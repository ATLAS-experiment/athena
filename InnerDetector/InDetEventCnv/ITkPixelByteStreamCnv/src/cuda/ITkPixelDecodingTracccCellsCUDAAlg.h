/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ITKPIXEL_DECODINGTRACCCCELLS_CUDA_ALG_H
#define ITKPIXEL_DECODINGTRACCCCELLS_CUDA_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthDeviceInterfaces/ICopiesTool.h"
#include "AthCUDAInterfaces/IStreamTool.h"
#include "ActsGPUEvent/GeometryIdMapping.h"
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "itksw/pix/endec/intf/Options.hpp"
#include <Gaudi/Property.h>

/*! \brief Decodes Pixel byte streams into Traccc cells hosted in GPU memory
 * 
 * - Aggregate Pixel byte stream data from the ROB data service provider
 * - Gather additional information required for decoding
 * - Transfer to GPU memory
 * - Decode the byte stream into Traccc cells in GPU memory
 * - Record the resulting traccc silicon cell collection to StoreGate
 *
 * \authors Roxani Lazaridou <roxani.lazaridou@cern.ch>
 *          Fabrice Le Goff <fabrice.le.goff@cern.ch>
 */
class ITkPixelDecodingTracccCellsCUDAAlg
    : public AthReentrantAlgorithm
{
public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

private:
    ServiceHandle<IROBDataProviderSvc> m_robDataProviderSvc{this,
        "ROBDataProvider", "ROBDataProviderSvc", "ROB data provider service"};
    SG::ReadCondHandleKey<ITkPixelCablingData> m_pixelCablingKey{this,
        "PixelCablingKey", "ITkPixelCablingData", "ITk Pixel cabling"};
    SG::WriteHandleKey<traccc::edm::silicon_cell_collection::buffer> m_tracccCellsKey{this,
        "TracccCells", "", "Output traccc cell collection buffer"};

    // Conversion of IDs between Athena, Acts, and Detray
    Gaudi::Property<std::string> m_geoIdMappingObjectName{
        this, "GeoIdMappingObjectName", "",
        "Detray/Acts/Athena geometry ID mapping"};
    const ActsTrk::GeometryIdMapping* m_geoIdMapping{nullptr};

    ToolHandle<AthDevice::IMemoryResourcesTool> m_MRs{this,
        "MemoryResources", "", "Athena device memory resources"};
    ToolHandle<AthDevice::ICopiesTool> m_copies{this,
        "CopiesTool", "", "Athena device copy tool"};
    ToolHandle<AthCUDA::IStreamTool> m_streamTool{this,
        "StreamTool", "", "Athena CUDA stream tool"};

    const itksw::pix::endec::intf::Options m_decodingOptions{.en_eos = false};

    Gaudi::Property<size_t> m_ExpectedDataSize{this,
        "ExpectedDataSize", 2000000,
        "Expected ITk Pixel data size, used to reserve memory"};
};

#endif // ITKPIXEL_DECODINGTRACCCCELLS_CUDA_ALG_H