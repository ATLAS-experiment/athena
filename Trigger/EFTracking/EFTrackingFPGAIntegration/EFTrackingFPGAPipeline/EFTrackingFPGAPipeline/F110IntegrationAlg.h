/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/F110IntegrationAlg.h
 */

#ifndef EFTRACKING_FPGA_INTEGRATION_F110IntegrationAlg_H
#define EFTRACKING_FPGA_INTEGRATION_F110IntegrationAlg_H

// EFTracking include
#include "EFTrackingFPGAPipeline/IntegrationBase.h"
#include "EFTrackingFPGAUtility/xAODClusterMaker.h"
#include "EFTrackingFPGAUtility/TestVectorTool.h"
#include "EFTrackingFPGAUtility/FPGADataFormatTool.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"

// Athena include
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IChronoSvc.h"
#include <TrigSteeringEvent/TrigRoiDescriptorCollection.h>
#include <IRegionSelector/IRegSelTool.h>

#include <mutex>

namespace EFTrackingFPGAIntegration
{
    /**
     * @brief This is the class for the benchmark algorithm specific to the FPGA integration and output conversion.
     *
     * This algorithm is used to benchmark and optimize the FPGA output memory migration
     * and output conversion. It expects the use of FPGA pass-through kernel.
     */
    class F110IntegrationAlg : public IntegrationBase
    {
    public:
        using IntegrationBase::IntegrationBase;
        virtual StatusCode initialize() override final;
        virtual StatusCode execute(const EventContext &ctx) const override final;
        virtual StatusCode finalize() override final;

    private:
        std::vector<cl::Event> getDepVector(std::vector<cl::Event> &endEvents, size_t cu) const;

        ServiceHandle<IChronoSvc> m_chronoSvc{"ChronoStatSvc", name()}; //!< Service for timing the algorithm

        SG::ReadHandleKey<std::vector<uint64_t>> m_FPGAPixelRDO{this, "FPGAEncodedPixelKey", "FPGAEncodedPixelRDOs", "Pixel RDO converted to FPGA format"};
        SG::ReadHandleKey<std::vector<uint64_t>> m_FPGAStripRDO{this, "FPGAEncodedStripKey", "FPGAEncodedStripRDOs", "Strip RDO converted to FPGA format"};

        SG::ReadHandleKey<int> m_FPGAPixelRDOSize{this, "FPGAEncodedPixelSizeKey", "FPGAEncodedPixelSizeRDOs", "Pixel RDO converted to FPGA format"};
        SG::ReadHandleKey<int> m_FPGAStripRDOSize{this, "FPGAEncodedStripSizeKey", "FPGAEncodedStripSizeRDOs", "Strip RDO converted to FPGA format"};

        SG::WriteHandleKey<std::vector<uint32_t>> m_FPGAPixelOutput{this, "FPGAOutputPixelKey", "FPGAPixelOutput", "Pixel output from FPGA format"};
        SG::WriteHandleKey<std::vector<uint32_t>> m_FPGAStripOutput{this, "FPGAOutputStripKey", "FPGAStripOutput", "Strip output from FPGA format"};

        Gaudi::Property<int> m_FPGAThreads{this, "FPGAThreads", 1, "number of FPGA threads to initialize"}; 
        
        Gaudi::Property<std::string> m_xclbin{this, "xclbin", "", "xclbin path and name"}; //!< Path and name of the xclbin file

        Gaudi::Property<std::string> m_pixelEdmKernelName{this, "PixelEDMPrepKernelName", "", "Name of the FPGA kernel"}; //!< Name of the FPGA kernel

        Gaudi::Property<std::string> m_stripEdmKernelName{this, "StripEDMPrepKernelName", "", "Name of the FPGA kernel"}; //!< Name of the FPGA kernel

        Gaudi::Property<std::string> m_pixelClusterKernelName{this, "PixelClusterKernelName", "", "Name of the pixel clustering kernel"}; //!< Name of the pixel clustering kernel

        Gaudi::Property<std::string> m_stripClusterKernelName{this, "StripClusterKernelName", "", "Name of the strip clustering kernel"}; //!< Name of the strip clustering kernel

        Gaudi::Property<std::string> m_stripL2GKernelName{this, "StripL2GKernelName", "", "Name of the strip L2G kernel"}; //!< Name of the strip L2G kernelS

        mutable std::mutex m_fpgaHandleMtx;

        mutable std::vector<cl::Event> m_stripClusterEndEvents ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Event> m_stripL2GEndEvents ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Event> m_stripEDMEndEvents ATLAS_THREAD_SAFE;

        mutable std::vector<cl::Event> m_pixelClusterEndEvents ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Event> m_pixelEDMEndEvents ATLAS_THREAD_SAFE;

        mutable std::vector<cl::Kernel> m_pixelClusterKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_pixelEDMKernels ATLAS_THREAD_SAFE;

        mutable std::vector<cl::Kernel> m_stripClusterKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_stripEDMKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_stripL2GKernels ATLAS_THREAD_SAFE;

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

        // Buffers for input
        mutable std::vector<cl::Buffer> m_pixelClusterInputBufferList ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Buffer> m_stripClusterInputBufferList ATLAS_THREAD_SAFE;
        // Buffers for Clustering
        mutable std::vector<cl::Buffer> m_stripClusterOutputBufferList ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Buffer> m_pixelClusterEDMOutputBufferList ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Buffer> m_stripClusterEDMOutputBufferList ATLAS_THREAD_SAFE;
        // L2G
        mutable std::vector<cl::Buffer> m_stripL2GOutputBufferList ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Buffer> m_stripL2GEDMOutputBufferList ATLAS_THREAD_SAFE;
        // EDMPrep
        mutable std::vector<cl::Buffer> m_edmPixelOutputBufferList ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Buffer> m_edmStripOutputBufferList ATLAS_THREAD_SAFE;

        // Command queue
        cl::CommandQueue m_acc_queue;


        void getListofCUs(std::vector<std::string>& cuNames);


    };
}

#endif // EFTRACKING_FPGA_INTEGRATION_F110IntegrationAlg_H
