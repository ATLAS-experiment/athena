/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef EFTRACKING_FPGA_EDMCONVERSION_H
#define EFTRACKING_FPGA_EDMCONVERSION_H

// Athena include
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "EFTrackingFPGAUtility/xAODClusterMaker.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"

// STL include
#include <string>
#include <vector>

/**
 * @brief The class for enconding RDO to FPGA format. 
 * 
 */
namespace EFTrackingFPGAIntegration
{
    class F100EDMConversionAlg : public AthReentrantAlgorithm
    {
    public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;

        virtual StatusCode initialize() override;

        virtual StatusCode execute(const EventContext &ctx) const;


    protected:
        ToolHandle<xAODClusterMaker> m_xaodClusterMaker{
            this,
            "xAODClusterMaker",
            "xAODClusterMaker",
            "Tool for creating xAOD cluster containers"}; //!< Tool for creating xAOD containers
        
        
        SG::ReadHandleKey<std::vector<uint64_t>> m_FPGAPixelOutput{this, "FPGAOutputPixelKey", "FPGAPixelOutput", "Pixel output from FPGA"};
        SG::ReadHandleKey<std::vector<uint64_t>> m_FPGAStripOutput{this, "FPGAOutputStripKey", "FPGAStripOutput", "Strip output from FPGA"};


    };
}

#endif // EFTRACKING_FPGA_EDMCONVERSION_H
 