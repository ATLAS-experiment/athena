/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/F110StreamIntegrationAlg.h
 */

#ifndef EFTRACKING_FPGA_INTEGRATION_F110StreamIntegrationAlg_H
#define EFTRACKING_FPGA_INTEGRATION_F110StreamIntegrationAlg_H

// EFTracking include
#include "EFTrackingFPGAPipeline/IntegrationBase.h"
#include "EFTrackingFPGAUtility/xAODClusterMaker.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"

// Athena include
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "GaudiKernel/ServiceHandle.h"        
#include "GaudiKernel/IChronoSvc.h"
#include <TrigSteeringEvent/TrigRoiDescriptorCollection.h>
#include <IRegionSelector/IRegSelTool.h>

namespace EFTrackingFPGAIntegration
{
    /**
     * @brief This is the class for the benchmark algorithm specific to the FPGA integration and output conversion.
     *
     * This algorithm is used to benchmark and optimize the FPGA output memory migration
     * and output conversion. It expects the use of FPGA pass-through kernel.
     */
    class F110StreamIntegrationAlg : public IntegrationBase
    {
    public:
        using IntegrationBase::IntegrationBase;
        virtual StatusCode initialize() override final;
        virtual StatusCode execute(const EventContext &ctx) const override final;
        virtual StatusCode finalize() override final;

    private:
        ServiceHandle<IChronoSvc> m_chronoSvc{"ChronoStatSvc", name()}; //!< Service for timing the algorithm

        // Input and output of the alg
        SG::ReadHandleKey<std::vector<uint64_t>> m_FPGAPixelRDO{this, "FPGAEncodedPixelKey", "FPGAEncodedPixelRDOs", "Pixel RDO converted to FPGA format"};
        SG::ReadHandleKey<std::vector<uint64_t>> m_FPGAStripRDO{this, "FPGAEncodedStripKey", "FPGAEncodedStripRDOs", "Strip RDO converted to FPGA format"};

        SG::ReadHandleKey<int> m_FPGAPixelRDOSize{this, "FPGAEncodedPixelSizeKey", "FPGAEncodedPixelSizeRDOs", "Pixel RDO converted to FPGA format"};
        SG::ReadHandleKey<int> m_FPGAStripRDOSize{this, "FPGAEncodedStripSizeKey", "FPGAEncodedStripSizeRDOs", "Strip RDO converted to FPGA format"};

        SG::WriteHandleKey<std::vector<uint32_t>> m_FPGAPixelOutput{this, "FPGAOutputPixelKey", "FPGAPixelOutput", "Pixel output from FPGA format"};
        SG::WriteHandleKey<std::vector<uint32_t>> m_FPGAStripOutput{this, "FPGAOutputStripKey", "FPGAStripOutput", "Strip output from FPGA format"};
        

        // properties
        Gaudi::Property<int> m_FPGAThreads{this, "FPGAThreads", 1, "number of FPGA threads to initialize"}; 
        
        Gaudi::Property<std::string> m_xclbin{this, "xclbin", "", "xclbin path and name"}; //!< Path and name of the xclbin file

        Gaudi::Property<std::string> m_pixelStartClusterKernelName{this, "PixelStartClusterKernelName", "", "Name of the pixel clustering start kernel"}; //!< Name of the pixel clustering kernel start
        Gaudi::Property<std::string> m_pixelEndClusterKernelName{this, "PixelEndClusterKernelName", "", "Name of the pixel clustering end kernel"}; //!< Name of the pixel clustering kernel start

        Gaudi::Property<std::string> m_stripStartClusterKernelName{this, "StripStartClusterKernelName", "", "Name of the strip clustering start kernel"}; //!< Name of the strip clustering kernel start
        Gaudi::Property<std::string> m_stripEndClusterKernelName{this, "StripEndClusterKernelName", "", "Name of the strip clustering end kernel"}; //!< Name of the strip clustering kernel start

        Gaudi::Property<std::string> m_pixelLUTKernelName{this, "PixelLUTKernelName", "", "Name of the pixel LUT loading kernel"}; //!< Name of the pixel lut loading kernel
        Gaudi::Property<std::string> m_stripLUTKernelName{this, "StripLUTKernelName", "", "Name of the strip LUT loading kernel"}; //!< Name of the pixel lut loading kernel

        Gaudi::Property<std::string> m_pixelLUTFilePath{this, "PixelLUTFilePath", "", "Path to the pixel LUT"}; 
        Gaudi::Property<std::string> m_stripLUTFilePath{this, "StripLUTFilePath", "", "Path to the strip LUT"}; 

        // to save information
        mutable std::atomic<ulonglong> m_numEvents{0};          //!< Number of events processed
        mutable std::atomic<cl_ulong> m_pixelInputTime{0};      //!< Time for pixel input buffer write
        mutable std::atomic<cl_ulong> m_stripInputTime{0};      //!< Time for strip input buffer write

        mutable std::atomic<cl_ulong> m_pixelPipelineTime{0};   //!< Time for pixel pipeline
        mutable std::atomic<cl_ulong> m_stripPipelineTime{0};   //!< Time for strip pipeline
        mutable std::atomic<cl_ulong> m_pixelOutputTime{0};     //!< Time for pixel output buffer read
        mutable std::atomic<cl_ulong> m_stripOutputTime{0};     //!< Time for strip output buffer read

        // Kernels
        // Clustering
        mutable std::vector<cl::Kernel> m_pixelStartClusteringKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_pixelEndClusteringKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_stripStartClusteringKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_stripEndClusteringKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_pixelLUTKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_stripLUTKernels ATLAS_THREAD_SAFE;

        // Buffers for input
        std::vector<cl::Buffer> m_pixelClusterInputBufferList;
        std::vector<cl::Buffer> m_stripClusterInputBufferList;

        // EDMPrep
        std::vector<cl::Buffer> m_edmPixelOutputBufferList;
        std::vector<cl::Buffer> m_edmStripOutputBufferList;

        // Command queue
        std::vector<cl::CommandQueue> m_acc_queues;
        void getListofCUs(std::vector<std::string>& cuNames);
        StatusCode readCalibfile(std::string inputFileName, std::vector<uint64_t>& data);


    };
}

#endif // EFTRACKING_FPGA_INTEGRATION_F110StreamIntegrationAlg_H
