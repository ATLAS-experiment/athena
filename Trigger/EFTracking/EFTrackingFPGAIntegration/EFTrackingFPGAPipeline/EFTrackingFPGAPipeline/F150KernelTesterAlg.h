/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
   */


#ifndef EFTRACKING_FPGA_INTEGRATION_F150KERNELTESTERALG_H
#define EFTRACKING_FPGA_INTEGRATION_F150KERNELTESTERALG_H

// EFTracking include
#include "EFTrackingFPGAPipeline/IntegrationBase.h"
#include "EFTrackingFPGAUtility/xAODClusterMaker.h"
#include "EFTrackingFPGAUtility/TestVectorTool.h"
#include "EFTrackingFPGAUtility/FPGADataFormatTool.h"
#include "EFTrackingFPGAUtility/OutputConversionTool.h"
#include "EFTrackingFPGAPipeline/DataPreparationPipeline.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimHitCollection.h"

// XRT includes
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"
#include <experimental/xrt_ip.h>

// XRT -> CL includes
#include "CL/cl2xrt.hpp"

// Athena include
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IChronoSvc.h"

#include <filesystem>
#include <fstream>
#include <span>

#define EVENT_COUNT_RST 0x80000000
#define USER_CTRL_OFFSET 0x10

namespace EFTrackingFPGAIntegration
{
    class F150KernelTesterAlg : public IntegrationBase
    {
        public:
            using IntegrationBase::IntegrationBase;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext &ctx) const override final;
            virtual StatusCode finalize() override final;


        private:
            ServiceHandle<IChronoSvc> m_chronoSvc{
                "ChronoStatSvc", name()}; //!< Service for timing the algorithm

            ToolHandle<xAODClusterMaker> m_xaodClusterMaker{
                this,
                    "xAODClusterMaker",
                    "xAODClusterMaker",
                    "Tool for creating xAOD cluster containers"}; //!< Tool for creating xAOD containers

            ToolHandle<TestVectorTool> m_testVectorTool{
                this, "TestVectorTool", "TestVectorTool", "Tool for preparing test vectors"}; //!< Tool for preparing test vectors

            ToolHandle<FPGADataFormatTool> m_FPGADataFormatTool{
                this, "FPGADataFormatTool", "FPGADataFormatTool", "Tool for formatting FPGA data"}; //!< Tool for formatting FPGA data

            Gaudi::Property<std::string> m_xclbin{this, "xclbin", "", "xclbin path and name"}; //!< Path and name of the xclbin file

            Gaudi::Property<bool> m_runSE{this, "RunSlicing", "", "Whether to run slicing engine or not"}; //!<  Whether to run SE or not
            Gaudi::Property<bool> m_runIO{this, "RunInsideOut", "", "Whether to run inside out or not"}; //!<  Whether to run inside out or not
            Gaudi::Property<bool> m_runIOOnSE{this, "RunInsideOutOnSlicingEngine", "", "Whether to run inside out on the output of the slicing engine"}; //!<  Whether to run inside out on the output of slicing engine 
            Gaudi::Property<bool> m_runFull150{this, "RunFullF150", "", "Whether to run Full 150 chain"}; //!<  Whether to run the Full F150 include F100 on hy 
            Gaudi::Property<bool> m_outputTextFile{this, "outputTextFile", "", "Whether to output text file"}; //!<  Whether to run SE or not

            SG::ReadHandleKey<FPGATrackSimHitCollection> m_FPGAHitKey {this, "FPGATrackSimHitKey","FPGAHits", "FPGATrackSim hits key"}; // Pixel CLS Output
            SG::ReadHandleKey<FPGATrackSimHitCollection> m_FPGASlicedHitKey{this, "FPGATrackSimHitKey_1st", "FPGAHits_1st_reg34", "FPGATrackSim Hits 1st stage key"}; // Slicing Engine Output
            SG::ReadHandleKey<FPGATrackSimTrackCollection> m_FPGATrackKey{this, "FPGATrackSimTrack1stKey","FPGATracks_1st_reg34","FPGATrackSim Tracks 1st stage key"}; // Inside Out Output

            SG::WriteHandleKey<std::vector<uint64_t>> m_FPGATrackOutput{this, "FPGAOutputTrackKey", "FPGATrackOutput", "Track output from FPGA format"};

            // Tool for output conversion
            ToolHandle<OutputConversionTool> m_outputConversionTool{this, "OutputConversionTool", "OutputConversionTool", "tool for output conversion"};

            // XRT Kernels and IPs Name Properties
            Gaudi::Property<std::string> m_slicingEngineInputName{this, "SlicingEngineInputName", "", "Name of the slicing engine input kernel"};
            Gaudi::Property<std::string> m_slicingEngineOutputName{this, "SlicingEngineOutputName", "", "Name of the slicing engine output kernel"};
            Gaudi::Property<std::string> m_insideOutInputName{this, "InsideOutInputName", "", "Name of the inside out input kernel"};
            Gaudi::Property<std::string> m_insideOutOutputName{this, "InsideOutOutputName", "", "Name of the inside out output kernel"};


            Gaudi::Property<std::string> m_pixelEdmKernelName{this, "PixelEDMPrepKernelName", "", "Name of the FPGA kernel"}; //!< Name of the FPGA kernel
            Gaudi::Property<std::string> m_stripEdmKernelName{this, "StripEDMPrepKernelName", "", "Name of the FPGA kernel"}; //!< Name of the FPGA kernel
            Gaudi::Property<std::string> m_pixelClusterKernelName{this, "PixelClusterKernelName", "", "Name of the pixel clustering kernel"}; //!< Name of the pixel clustering kernel
            Gaudi::Property<std::string> m_stripClusterKernelName{this, "StripClusterKernelName", "", "Name of the strip clustering kernel"}; //!< Name of the strip clustering kernel
            Gaudi::Property<std::string> m_stripL2GKernelName{this, "StripL2GKernelName", "", "Name of the strip L2G kernel"}; //!< Name of the strip L2G kernelS


            mutable std::atomic<cl_ulong> m_IO_kernelTime{0};       //!< Time for kernel execution
            mutable std::atomic<cl_ulong> m_SE_kernelTime{0};  //!< Sum for the average time of the kernel execution
            mutable std::atomic<ulonglong> m_numEvents{0}; //!< Number of events for the average time of the kernel execution
            // For IP access through XRT
            xrt::device m_xrt_accelerator;

            cl::Event m_slicingEngineInputEndEvent ATLAS_THREAD_SAFE;
            cl::Event m_slicingEngineOutputEndEvent ATLAS_THREAD_SAFE;
            cl::Event m_insideOutEndEvent ATLAS_THREAD_SAFE;


            // Kernels
            mutable cl::Kernel m_slicingEngineInput ATLAS_THREAD_SAFE;
            mutable cl::Kernel m_slicingEngineOutput ATLAS_THREAD_SAFE;
            mutable cl::Kernel m_insideOutInput ATLAS_THREAD_SAFE;
            mutable cl::Kernel m_insideOutOutput ATLAS_THREAD_SAFE;


            // Buffers
            mutable cl::Buffer m_slicingEngineInputBuffer ATLAS_THREAD_SAFE;
            mutable cl::Buffer m_slicingEngineOutputBuffer ATLAS_THREAD_SAFE;
            mutable cl::Buffer m_insideOutInputBuffer ATLAS_THREAD_SAFE;
            mutable cl::Buffer m_insideOutOutputBuffer ATLAS_THREAD_SAFE;

            // F100 kernels
            // Clustering
            mutable cl::Kernel m_pixelClusteringKernel ATLAS_THREAD_SAFE;
            mutable cl::Kernel m_stripClusteringKernel ATLAS_THREAD_SAFE;
            // L2G
            mutable cl::Kernel m_stripL2GKernel ATLAS_THREAD_SAFE;
            // EDM prep
            mutable cl::Kernel m_pixelEdmPrepKernel ATLAS_THREAD_SAFE;
            mutable cl::Kernel m_stripEdmPrepKernel ATLAS_THREAD_SAFE;

            // Buffers for input
            mutable cl::Buffer m_pixelClusterInputBuffer ATLAS_THREAD_SAFE;
            mutable cl::Buffer m_stripClusterInputBuffer ATLAS_THREAD_SAFE;
            // Buffers for Clustering
            mutable cl::Buffer m_pixelClusterOutputBuffer ATLAS_THREAD_SAFE;
            mutable cl::Buffer m_stripClusterOutputBuffer ATLAS_THREAD_SAFE;
            mutable cl::Buffer m_pixelClusterEDMOutputBuffer ATLAS_THREAD_SAFE;
            mutable cl::Buffer m_stripClusterEDMOutputBuffer ATLAS_THREAD_SAFE;
            // L2G
            mutable cl::Buffer m_stripL2GOutputBuffer ATLAS_THREAD_SAFE;
            mutable cl::Buffer m_stripL2GEDMOutputBuffer ATLAS_THREAD_SAFE;
            // EDMPrep
            mutable cl::Buffer m_edmPixelOutputBuffer ATLAS_THREAD_SAFE;
            mutable cl::Buffer m_edmStripOutputBuffer ATLAS_THREAD_SAFE;

            SG::ReadHandleKey<std::vector<uint64_t>> m_FPGAPixelRDO{this, "FPGAEncodedPixelKey", "FPGAEncodedPixelRDOs", "Pixel RDO converted to FPGA format"};
            SG::ReadHandleKey<std::vector<uint64_t>> m_FPGAStripRDO{this, "FPGAEncodedStripKey", "FPGAEncodedStripRDOs", "Strip RDO converted to FPGA format"};

            SG::WriteHandleKey<std::vector<uint64_t>> m_FPGAPixelOutput{this, "FPGAOutputPixelKey", "FPGAPixelOutput", "Pixel output from FPGA format"};
            SG::WriteHandleKey<std::vector<uint64_t>> m_FPGAStripOutput{this, "FPGAOutputStripKey", "FPGAStripOutput", "Strip output from FPGA format"};

            // Command queue
            cl::CommandQueue m_queue;

            std::string get_cu_name(const std::string& kernel_name, int cu);

            void dumpHexData(std::span<const uint64_t> data, const std::string &dataDescriptor, const EventContext &ctx) const;
    };
}

#endif // EFTRACKING_FPGA_INTEGRATION_F150KERNELTESTERALG_H

