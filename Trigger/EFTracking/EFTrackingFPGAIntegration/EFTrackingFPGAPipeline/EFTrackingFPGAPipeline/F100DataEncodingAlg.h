/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef EFTRACKING_FPGA_DATAENCODING_H
#define EFTRACKING_FPGA_DATAENCODING_H

// Athena include
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "EFTrackingFPGAUtility/FPGADataFormatTool.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "GaudiKernel/ServiceHandle.h"
#include <TrigSteeringEvent/TrigRoiDescriptorCollection.h>
#include <IRegionSelector/IRegSelTool.h>

// STL include
#include <string>
#include <vector>

/**
 * @brief The class for enconding RDO to FPGA format. 
 * 
 */
namespace EFTrackingFPGAIntegration
{
    class F100DataEncodingAlg : public AthReentrantAlgorithm
    {
    public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;

        virtual StatusCode initialize() override;

        virtual StatusCode execute(const EventContext &ctx) const;


    protected:
        ToolHandle<FPGADataFormatTool> m_FPGADataFormatTool{this, "FPGADataFormatTool", "FPGADataFormatTool", "Tool for formatting FPGA data"}; //!< Tool for formatting FPGA data
        SG::ReadHandleKey<PixelRDO_Container> m_pixelRDOKey{this, "PixelRDO", "ITkPixelRDOs"};
        SG::ReadHandleKey<SCT_RDO_Container> m_stripRDOKey{this, "StripRDO", "ITkStripRDOs"};

        // For ROI running
        ToolHandle<IRegSelTool> m_regionPixelSelector {this, "RegPixelSelTool", "", "Pixel Region selector tool"};
        ToolHandle<IRegSelTool> m_regionStripSelector {this, "RegStripSelTool", "", "Strip Region selector tool"};

        SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey {this, "RoIs", "","RoIs to read in"};
        Gaudi::Property<bool> m_roiSeeded{this, "isRoI_Seeded", false, "Use RoI"};

        SG::WriteHandleKey<std::vector<uint64_t>> m_FPGAPixelRDO{this, "FPGAEncodedPixelKey", "FPGAEncodedPixelRDOs", "Pixel RDO converted to FPGA format"};
        SG::WriteHandleKey<std::vector<uint64_t>> m_FPGAStripRDO{this, "FPGAEncodedStripKey", "FPGAEncodedStripRDOs", "Strip RDO converted to FPGA format"};
        
        SG::WriteHandleKey<int> m_FPGAPixelRDOSize{this, "FPGAEncodedPixelSizeKey", "FPGAEncodedPixelSizeRDOs", "Size of the Pixel RDO converted to FPGA format"};
        SG::WriteHandleKey<int> m_FPGAStripRDOSize{this, "FPGAEncodedStripSizeKey", "FPGAEncodedStripSizeRDOs", "Size of the Strip RDO converted to FPGA format"};


    };
}

#endif // EFTRACKING_FPGA_DATAENCODING_H
 