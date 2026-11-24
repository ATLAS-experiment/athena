/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ITKPIXEL_DECODINGTRACCCCELLS_CUDA_ALG_H
#define ITKPIXEL_DECODINGTRACCCCELLS_CUDA_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ITkPixelByteStreamCnv/ITkPixelDecodingTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthDeviceInterfaces/ICopiesTool.h"
#include "AthCUDAInterfaces/IStreamTool.h"
#include "ActsGPUEvent/GeometryIdMapping.h"
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "itksw/pix/endec/intf/Options.hpp"
#include <Gaudi/Property.h>
#include <cstdint>

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
    ToolHandle<ITkPixelDecodingTool> m_decodingTool{this,
        "PixelDecodingTool", "ITkPixelDecodingTool", "The decoding tool"};

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

    static constexpr itksw::pix::endec::intf::Options s_decodingOptions{
        .en_eos = false};
    const size_t m_frameSize{static_cast<size_t>(64 -
        itksw::pix::endec::intf::reserved_bits(s_decodingOptions))};
    StatusCode moduleIdToDetrayIndex(uint32_t module_id, uint32_t & detray_index) const;
};

#endif // ITKPIXEL_DECODINGTRACCCCELLS_CUDA_ALG_H