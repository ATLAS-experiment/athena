/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


 #ifndef EFTRACKING_FPGA_INTEGRATION_F600INTEGRATIONALG_H
 #define EFTRACKING_FPGA_INTEGRATION_F600INTEGRATIONALG_H
 
 // EFTracking include
 #include "EFTrackingFPGAPipeline/IntegrationBase.h"
 #include "EFTrackingFPGAUtility/xAODClusterMaker.h"
 #include "EFTrackingFPGAUtility/TestVectorTool.h"
 #include "EFTrackingFPGAUtility/FPGADataFormatTool.h"
 #include "EFTrackingFPGAUtility/OutputConversionTool.h"
 #include "EFTrackingFPGAPipeline/DataPreparationPipeline.h"
 #include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"

 // Athena include
 #include "GaudiKernel/ServiceHandle.h"
 #include "GaudiKernel/IChronoSvc.h"

// XRT includes
#include "xrt/xrt_bo.h"
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"
#include <experimental/xrt_xclbin.h>
#include <experimental/xrt_ip.h>
#include <experimental/xrt_queue.h>

// XRT -> CL includes
#include "CL/cl2xrt.hpp"

#include <filesystem>
#include <fstream>

#define IP_START 0x1  // USER_CTRL[0]
#define IP_DONE 0x2  // USER_CTRL[1]
#define IP_IDLE 0x4  // USER_CTRL[2]
#define EVENT_COUNT_RST 0x80000000  // USER_CTRL[31]
// Addresses
#define USER_CTRL_OFFSET 0x10
#define EVENT_RX_COUNT_OFFSET 0x14
#define EVENT_TX_COUNT_OFFSET 0x1C
 
 namespace EFTrackingFPGAIntegration
 {
     class F600IntegrationAlg : public IntegrationBase
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
 
         Gaudi::Property<std::string> m_xclbin{
             this, "xclbin", "", "xclbin path and name"}; //!< Path and name of the xclbin file
 
        SG::ReadHandleKey<FPGATrackSimTrackCollection> m_FPGATrackKey{this, "FPGATrackSimTrack1stKey","FPGATracks_1st_reg34","FPGATrackSim Tracks 1st stage key"};

        // Tool for output conversion
        ToolHandle<OutputConversionTool> m_outputConversionTool{this, "OutputConversionTool", "OutputConversionTool", "tool for output conversion"};

        // XRT Kernels and IPs Name Properties
        // Data Prep
        Gaudi::Property<std::string> m_pixelClusteringKernelName{
            this, "PixelClusteringKernelName", "", "Name of the pixel clustering kernel (pxl cls)"};
        Gaudi::Property<std::string> m_processHitsKernelName{
            this, "ProcessHitsKernelName", "", "Name of the strip clustering kernel (strp cls)"};
        Gaudi::Property<std::string> m_pixelL2gKernelName{
            this, "PixelL2gKernelName", "", "Name of the pixel L2G transformation kernel"};
        Gaudi::Property<std::string> m_stripL2gKernelName{
            this, "StripL2gKernelName", "", "Name of the strip L2G transformation kernel"};
        Gaudi::Property<std::string> m_pixelEdmPrepKernelName{
            this, "PixelEdmPrepKernelName", "", "Name of the pixel EDM prep kernel"};
        Gaudi::Property<std::string> m_stripEdmPrepKernelName{
            this, "StripEdmPrepKernelName", "", "Name of the strip EDM prep kernel"};

        // Slicing Engine
        Gaudi::Property<std::string> m_pixelFirstStageSlicingIPName{
            this, "PixelFirstStageSlicingIPName", "", "Name of the slicing engine IP"};
        Gaudi::Property<std::string> m_pixelFirstStageInputKernelName{
            this, "PixelFirstStageInputKernelName", "", "Name of the input stage kernel"};
        Gaudi::Property<std::string> m_pixelFirstStageOutputKernelName{
            this, "PixelFirstStageOutputKernelName", "", "Name of the output stage kernel"};

        // Inside Out
        Gaudi::Property<std::string> m_memReadKernelName{
            this, "MemReadKernelName", "", "Name of the memory read kernel"};
        Gaudi::Property<std::string> m_memWriteKernelName{
            this, "MemWriteKernelName", "", "Name of the memory write kernel"};

        // Space Points
        Gaudi::Property<std::string> m_spacepointKernelName{
            this, "SpacepointKernelName", "", "Name of the spacepoint kernel"};

        // NN Pathfinder
        Gaudi::Property<std::string> m_loaderKernelName{
            this, "LoaderKernelName", "", "Name of the loader kernel"};
        Gaudi::Property<std::string> m_unloaderKernelName{
            this, "UnloaderKernelName", "", "Name of the unloader kernel"};

        // NN Classifier
        Gaudi::Property<std::string> m_nnOverlapDecoratorKernelName{
            this, "NnOverlapDecoratorKernelName", "", "Name of the NN overlap decorator kernel"};
        
        // Duplicate Remover
        Gaudi::Property<std::string> m_runnerKernelName{
            this, "RunnerKernelName", "", "Name of the duplicate remover runner kernel"};

        // NOTE: Temporary and for injecting TV data
        Gaudi::Property<std::string> m_pixelClusterInputPath{
          this, "PixelClusterInputPath", "", "Filepath for the pixel clustering input data"};
        Gaudi::Property<std::string> m_pixelStageOneSlicingInputPath{
          this, "PixelStageOneSlicingInputPath", "", "Filepath for the slicing engine (pixel first stage) input data"};
        Gaudi::Property<std::string> m_insideOutInputPath{
          this, "InsideOutInputPath", "", "Filepath for the inside out input data"};

        // NOTE WHA TI NEED: max size for IO for all the kernels. working SE other stages

        mutable std::atomic<cl_ulong> m_kernelTime{0};       //!< Time for kernel execution
        mutable std::atomic<cl_ulong> m_sum_kernelTime{0};  //!< Sum for the average time of the kernel execution
        mutable std::atomic<ulonglong> m_num_Events{0}; //!< Number of events for the average time of the kernel execution

        // For IP access through XRT
        xrt::device m_xrt_accelerator;

        mutable std::mutex m_fpgaHandleMtx ATLAS_THREAD_SAFE;

        mutable std::vector<cl::Event> m_pixelFirstStageSlicingInputEndEvents ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Event> m_pixelFirstStageSlicingOutputEndEvents ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Event> m_insideOutInputEndEvents ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Event> m_insideOutOutputEndEvents ATLAS_THREAD_SAFE;

        
        // Pixel CLS
        mutable std::vector<cl::Kernel> m_pixelClusterL2GKernels ATLAS_THREAD_SAFE;
        // Slicing Engine
        mutable std::vector<cl::Kernel> m_pixelFirstStageSlicingInputKernels ATLAS_THREAD_SAFE;
        mutable std::vector<xrt::ip> m_pixelFirstStageSlicingIPs ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_pixelFirstStageSlicingOutputKernels ATLAS_THREAD_SAFE;
        // Inside Out
        mutable std::vector<cl::Kernel> m_insideOutInputKernels ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Kernel> m_insideOutOutputKernels ATLAS_THREAD_SAFE;


        
        // Buffers for Clustering & L2G
        mutable std::vector<cl::Buffer> m_pixelClusterL2GInputBufferList ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Buffer> m_pixelClusterL2GOutputBufferList ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Buffer> m_pixelClusterL2GEDMOutputBufferList ATLAS_THREAD_SAFE;
        // Buffers for slicing engine
        mutable std::vector<cl::Buffer> m_pixelFirstStageSlicingInputBufferList ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Buffer> m_pixelFirstStageSlicingOutputBufferList ATLAS_THREAD_SAFE;
        // Buffers for inside out
        mutable std::vector<cl::Buffer> m_insideOutInputBufferList ATLAS_THREAD_SAFE;
        mutable std::vector<cl::Buffer> m_insideOutOutputBufferList ATLAS_THREAD_SAFE;

        // Command queue
        cl::CommandQueue m_queue;

        std::string get_cu_name(const std::string& kernel_name, int cu);

        // NOTE: These are temporary and are used to inject TVs in where kernels would otherwise go.
        // Once all kernels are working these will be removed.
        int readTVLength(std::string filepath) const;
        void readTV(const std::string &filepath, int n_words, uint64_t *data) const;
        
        void outputHexData(size_t dataLen, uint64_t *data, const std::string &dataDescriptor) const;
     };
 }
 
 #endif // EFTRACKING_FPGA_INTEGRATION_F600INTEGRATIONALG_H
 
