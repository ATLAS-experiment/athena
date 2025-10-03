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

        SG::ReadHandleKey<std::vector<uint64_t>> m_FPGAPixelRDO{this, "FPGAEncodedPixelKey", "FPGAEncodedPixelRDOs", "Pixel RDO converted to FPGA format"};
        SG::ReadHandleKey<std::vector<uint64_t>> m_FPGAStripRDO{this, "FPGAEncodedStripKey", "FPGAEncodedStripRDOs", "Strip RDO converted to FPGA format"};

        SG::WriteHandleKey<std::vector<uint64_t>> m_FPGAPixelOutput{this, "FPGAOutputPixelKey", "FPGAPixelOutput", "Pixel output from FPGA format"};
        SG::WriteHandleKey<std::vector<uint64_t>> m_FPGAStripOutput{this, "FPGAOutputStripKey", "FPGAStripOutput", "Strip output from FPGA format"};

        Gaudi::Property<int> m_FPGAThreads{this, "FPGAThreads", 1, "number of FPGA threads to initialize"}; 
        
        Gaudi::Property<std::string> m_xclbin{this, "xclbin", "", "xclbin path and name"}; //!< Path and name of the xclbin file


        Gaudi::Property<std::string> m_pixelEdmKernelName{this, "PixelEDMPrepKernelName", "", "Name of the FPGA kernel"}; //!< Name of the FPGA kernel

        Gaudi::Property<std::string> m_stripEdmKernelName{this, "StripEDMPrepKernelName", "", "Name of the FPGA kernel"}; //!< Name of the FPGA kernel

        Gaudi::Property<std::string> m_pixelStartClusterKernelName{this, "PixelStartClusterKernelName", "", "Name of the pixel clustering start kernel"}; //!< Name of the pixel clustering kernel start
        Gaudi::Property<std::string> m_pixelEndClusterKernelName{this, "PixelEndClusterKernelName", "", "Name of the pixel clustering end kernel"}; //!< Name of the pixel clustering kernel start

        Gaudi::Property<std::string> m_stripStartClusterKernelName{this, "StripStartClusterKernelName", "", "Name of the strip clustering start kernel"}; //!< Name of the strip clustering kernel start
        Gaudi::Property<std::string> m_stripEndClusterKernelName{this, "StripEndClusterKernelName", "", "Name of the strip clustering end kernel"}; //!< Name of the strip clustering kernel start


        Gaudi::Property<std::string> m_stripL2GKernelName{this, "StripL2GKernelName", "", "Name of the strip L2G kernel"}; //!< Name of the strip L2G kernelS



        mutable std::atomic<ulonglong> m_numEvents{0};          //!< Number of events processed
        mutable std::atomic<cl_ulong> m_pixelInputTime{0};      //!< Time for pixel input buffer write
        mutable std::atomic<cl_ulong> m_stripInputTime{0};      //!< Time for strip input buffer write
        mutable std::atomic<cl_ulong> m_pixelClusteringTime{0}; //!< Time for pixel clustering
        mutable std::atomic<cl_ulong> m_stripClusteringTime{0}; //!< Time for strip clustering
        mutable std::atomic<cl_ulong> m_stripL2GTime{0};        //!< Time for strip L2G
        mutable std::atomic<cl_ulong> m_pixelEdmPrepTime{0};    //!< Time for pixel EDM preparation
        mutable std::atomic<cl_ulong> m_stripEdmPrepTime{0};    //!< Time for strip EDM preparation
        mutable std::atomic<cl_ulong> m_pixelOutputTime{0};     //!< Time for pixel output buffer read
        mutable std::atomic<cl_ulong> m_stripOutputTime{0};     //!< Time for strip output buffer read
        mutable std::atomic<cl_ulong> m_kernelTime{0};          //!< Time for kernel execution

        // Kernels
        // Clustering
        mutable std::vector<cl::Kernel> m_pixelStartClusteringKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_pixelEndClusteringKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_stripStartClusteringKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_stripEndClusteringKernels ATLAS_THREAD_SAFE;

        // L2G
        mutable std::vector<cl::Kernel> m_stripL2GKernels ATLAS_THREAD_SAFE;

        // EDM prep
        mutable std::vector<cl::Kernel> m_pixelEdmPrepKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_stripEdmPrepKernels ATLAS_THREAD_SAFE;

        // Buffers for input
        std::vector<cl::Buffer> m_pixelClusterInputBufferList;
        std::vector<cl::Buffer> m_stripClusterInputBufferList;
        // Buffers for Clustering
        std::vector<cl::Buffer> m_stripClusterOutputBufferList;
        std::vector<cl::Buffer> m_pixelClusterEDMOutputBufferList;
        std::vector<cl::Buffer> m_stripClusterEDMOutputBufferList;
        // L2G
        std::vector<cl::Buffer> m_stripL2GOutputBufferList;
        std::vector<cl::Buffer> m_stripL2GEDMOutputBufferList;
        // EDMPrep
        std::vector<cl::Buffer> m_edmPixelOutputBufferList;
        std::vector<cl::Buffer> m_edmStripOutputBufferList;

        // Command queue
        std::vector<cl::CommandQueue> m_acc_queues;
        void getListofCUs(std::vector<std::string>& cuNames);


    };
}

#endif // EFTRACKING_FPGA_INTEGRATION_F110StreamIntegrationAlg_H
