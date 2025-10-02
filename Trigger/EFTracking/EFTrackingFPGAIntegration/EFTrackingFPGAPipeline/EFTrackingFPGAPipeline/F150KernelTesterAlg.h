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

        SG::ReadHandleKey<FPGATrackSimHitCollection> m_FPGAHitKey {this, "FPGATrackSimHitKey","FPGAHits", "FPGATrackSim hits key"}; // Pixel CLS Output
        SG::ReadHandleKey<FPGATrackSimHitCollection> m_FPGASlicedHitKey{this, "FPGATrackSimHitKey_1st", "FPGAHits_1st_reg34", "FPGATrackSim Hits 1st stage key"}; // Slicing Engine Output
        SG::ReadHandleKey<FPGATrackSimTrackCollection> m_FPGATrackKey{this, "FPGATrackSimTrack1stKey","FPGATracks_1st_reg34","FPGATrackSim Tracks 1st stage key"}; // Inside Out Output

       // Tool for output conversion
       ToolHandle<OutputConversionTool> m_outputConversionTool{this, "OutputConversionTool", "OutputConversionTool", "tool for output conversion"};

       // XRT Kernels and IPs Name Properties
       Gaudi::Property<std::string> m_slicingEngineInputName{this, "SlicingEngineInputName", "", "Name of the slicing engine input kernel"};
       Gaudi::Property<std::string> m_slicingEngineName{this, "SlicingEngineName", "", "Name of the slicing engine kernel"};
       Gaudi::Property<std::string> m_slicingEngineOutputName{this, "SlicingEngineOutputName", "", "Name of the slicing engine output kernel"};
       Gaudi::Property<std::string> m_insideOutInputName{this, "InsideOutInputName", "", "Name of the inside out input kernel"};
       Gaudi::Property<std::string> m_insideOutOutputName{this, "InsideOutOutputName", "", "Name of the inside out output kernel"};

       mutable std::atomic<cl_ulong> m_kernelTime{0};       //!< Time for kernel execution
       mutable std::atomic<cl_ulong> m_sum_kernelTime{0};  //!< Sum for the average time of the kernel execution
       mutable std::atomic<ulonglong> m_num_Events{0}; //!< Number of events for the average time of the kernel execution
       // For IP access through XRT
       xrt::device m_xrt_accelerator;

       cl::Event m_slicingEngineInputEndEvent;
       cl::Event m_slicingEngineOutputEndEvent;
       cl::Event m_insideOutEndEvent;

       
       // Kernels
       mutable xrt::ip m_slicingEngineIP;
       mutable cl::Kernel m_slicingEngineInput;
       mutable cl::Kernel m_slicingEngineOutput;
       mutable cl::Kernel m_insideOutInput;
       mutable cl::Kernel m_insideOutOutput;

       
       // Buffers
       mutable cl::Buffer m_slicingEngineInputBuffer;
       mutable cl::Buffer m_slicingEngineOutputBuffer;
       mutable cl::Buffer m_insideOutInputBuffer;
       mutable cl::Buffer m_insideOutOutputBuffer;

       // Command queue
       cl::CommandQueue m_queue;

       std::string get_cu_name(const std::string& kernel_name, int cu);
       
       void dumpHexData(size_t dataLen, uint64_t *data, const std::string &dataDescriptor) const;
    };
}

#endif // EFTRACKING_FPGA_INTEGRATION_F150KERNELTESTERALG_H

