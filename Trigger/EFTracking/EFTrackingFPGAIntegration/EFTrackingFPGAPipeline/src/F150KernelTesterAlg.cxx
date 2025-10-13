/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EFTrackingFPGAPipeline/F150KernelTesterAlg.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"
#include "AthenaKernel/Chrono.h"
#include "EFTrackingFPGAPipeline/DataPreparationPipeline.h"

#include <iostream>
#include <fstream> // Required for std::ofstream

namespace EFTrackingFPGAIntegration
{
    std::string F150KernelTesterAlg::get_cu_name(const std::string& kernel_name, int cu) {
        std::string full_cu_name = kernel_name + ":{" + kernel_name + "_" + std::to_string(cu) + "}";
        ATH_MSG_DEBUG("LOADING " + full_cu_name);
        return full_cu_name;
    }

    void F150KernelTesterAlg::dumpHexData(std::span<const uint64_t> data, const std::string& dataDescriptor) const {
        ATH_MSG_DEBUG("STARTING " << dataDescriptor << " words:");
        std::ofstream outputFile(dataDescriptor);

        for (uint64_t d : data) {
          outputFile << std::hex << std::setw(16) << std::setfill('0') << d << '\n';
        }
    
        // Write different data types
        outputFile.close();
    }

    StatusCode F150KernelTesterAlg::initialize()
    {
        ATH_MSG_INFO("Running on the FPGA accelerator");
        ATH_MSG_INFO("Testing Slicing Engine: " + m_runSE);
        ATH_MSG_INFO("Testing Inside Out: " + m_runIO);
        ATH_MSG_INFO("Testing Inside Out on Slicing Engine Output: " + m_runIOOnSE);

        ATH_CHECK(m_chronoSvc.retrieve());

        {
            Athena::Chrono chrono("Platform and device initialize", m_chronoSvc.get());
            ATH_CHECK(IntegrationBase::initialize());
        }

        {
            Athena::Chrono chrono("CL::loadProgram", m_chronoSvc.get());
            ATH_MSG_INFO("Loading Program: " + m_xclbin);
            ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));
        }

        cl_int err = CL_SUCCESS;

        int cu = 1;

        if (m_runSE) {
          m_slicingEngineInput = cl::Kernel(m_program, get_cu_name(m_slicingEngineInputName, cu).c_str(), &err);
          m_slicingEngineOutput = cl::Kernel(m_program, get_cu_name(m_slicingEngineOutputName, cu).c_str(), &err);

        }
        if (m_runIO) {
          m_insideOutInput = cl::Kernel(m_program, get_cu_name(m_insideOutInputName, cu).c_str(), &err);
          m_insideOutOutput = cl::Kernel(m_program, get_cu_name(m_insideOutOutputName, cu).c_str(), &err);
        }

        m_queue = cl::CommandQueue(m_context, m_accelerator, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
        
        if (err != CL_SUCCESS) {
          return StatusCode::FAILURE;
        }

        ATH_CHECK(m_xaodClusterMaker.retrieve());
        ATH_CHECK(m_testVectorTool.retrieve());
        ATH_CHECK(m_FPGADataFormatTool.retrieve());
        ATH_CHECK(m_outputConversionTool.retrieve());

        // Initialize track sim keys
        ATH_CHECK(m_FPGAHitKey.initialize());
        ATH_CHECK(m_FPGASlicedHitKey.initialize());
        ATH_CHECK(m_FPGATrackKey.initialize());

        ATH_CHECK(m_FPGATrackOutput.initialize());

        return StatusCode::SUCCESS;
    }

    StatusCode F150KernelTesterAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F150KernelTesterAlg");

        SG::WriteHandle<std::vector<uint64_t>> FPGATrackOutput(m_FPGATrackOutput, ctx);
        auto outputVec = std::make_unique<std::vector<uint64_t>>();

        SG::ReadHandle<FPGATrackSimTrackCollection> outTrackCollection(m_FPGATrackKey, ctx);
        ATH_CHECK(m_FPGADataFormatTool->convertFPGATracksToFPGADataFormat(outTrackCollection.cptr(), *outputVec, ctx));
        // Now record the filled vector
        ATH_CHECK(FPGATrackOutput.record(std::move(outputVec)));


        // Prepare the inputs for testing
        ATH_MSG_DEBUG("Accessing SE In data.");
        std::vector<uint64_t> pixelDataIN;
        std::vector<uint64_t> stripDataIN;
        SG::ReadHandle<FPGATrackSimHitCollection> hitCollectionHandle(m_FPGAHitKey, ctx);
        ATH_CHECK(m_FPGADataFormatTool->convertFPGAHitsToFPGADataFormat(hitCollectionHandle.cptr(), true, false, pixelDataIN, ctx));
        ATH_CHECK(m_FPGADataFormatTool->convertFPGAHitsToFPGADataFormat(hitCollectionHandle.cptr(), false, true, stripDataIN, ctx));
        dumpHexData(pixelDataIN, "FPGATrackSim_slicingIn_pixel.txt");
        dumpHexData(stripDataIN, "FPGATrackSim_slicingIn_strip.txt");

        ATH_MSG_DEBUG("Accessing SE Out data.");
        std::vector<uint64_t> dataPixelOut;
        std::vector<uint64_t> dataStripOut;
        SG::ReadHandle<FPGATrackSimHitCollection> outhitCollectionHandle(m_FPGASlicedHitKey, ctx);
        ATH_CHECK(m_FPGADataFormatTool->convertFPGASliceToFPGADataFormat(outhitCollectionHandle.cptr(), true, false, dataPixelOut, ctx));
        ATH_CHECK(m_FPGADataFormatTool->convertFPGASliceToFPGADataFormat(outhitCollectionHandle.cptr(), false, true, dataStripOut, ctx));
        dumpHexData(dataPixelOut, "FPGATrackSim_slicingOut_pixel.txt");
        dumpHexData(dataStripOut, "FPGATrackSim_slicingOut_strip.txt");

        ATH_MSG_DEBUG("Accessing SE Out data.");
        std::vector<uint64_t> dataInsideOut;
        ATH_CHECK(m_FPGADataFormatTool->convertFPGATracksToFPGADataFormat(outTrackCollection.cptr(), dataInsideOut, ctx));
        dumpHexData(dataInsideOut, "FPGATrackSim_insideOut.txt");


        cl_int err = CL_SUCCESS;

        // increment the event if there is data in this event
        if(pixelDataIN.size() > 6) m_numEvents++;

        if (m_runSE) {
          // Events (write → kSEInput → kSEOutput → read)
          cl::Event evtSEWriteIn;
          cl::Event evtSEKInputDone;
          cl::Event evtSEKOutputDone;
          cl::Event evtSEReadOut;

          ATH_MSG_DEBUG("Allocating SE buffers");
          const size_t pixel_size_bytes = pixelDataIN.size() * sizeof(uint64_t);

          m_slicingEngineInputBuffer  = cl::Buffer(m_context, CL_MEM_READ_ONLY,  pixel_size_bytes, nullptr, &err);
          m_slicingEngineOutputBuffer = cl::Buffer(m_context, CL_MEM_WRITE_ONLY, pixel_size_bytes, nullptr, &err);

          m_slicingEngineInput.setArg(0, m_slicingEngineInputBuffer);
          m_slicingEngineInput.setArg(2, static_cast<unsigned long long>(pixelDataIN.size())); 

          m_slicingEngineOutput.setArg(1, m_slicingEngineOutputBuffer);

          ATH_MSG_DEBUG("Transferring SE data");
          m_queue.enqueueWriteBuffer(m_slicingEngineInputBuffer, CL_TRUE, 0, pixel_size_bytes, pixelDataIN.data(), nullptr, &evtSEWriteIn);

          // Execute
          ATH_MSG_DEBUG("Executing SE kernels");
          std::vector<cl::Event> waitAfterSEWrite{evtSEWriteIn};
          m_queue.enqueueTask(m_slicingEngineInput,  &waitAfterSEWrite, &evtSEKInputDone);
          m_queue.enqueueTask(m_slicingEngineOutput, NULL, &evtSEKOutputDone);

          // Read
          ATH_MSG_DEBUG("Reading output data from kernel");
          std::vector<uint64_t> out_data(pixelDataIN.size(), 0);
          std::vector<cl::Event> waitForSERead{evtSEKOutputDone};
          m_queue.enqueueReadBuffer(m_slicingEngineOutputBuffer, /*blocking*/ CL_TRUE, 0, pixel_size_bytes,out_data.data(),&waitForSERead, &evtSEReadOut);

          // Optional explicit sync (blocking read already waits)
          cl::Event::waitForEvents({evtSEReadOut});

          dumpHexData(out_data, "HW_slicingOut_pixel.txt");

          m_SE_kernelTime += evtSEKOutputDone.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evtSEKInputDone.getProfilingInfo<CL_PROFILING_COMMAND_START>();

        }
        if (m_runIO) 
        {
          cl::Event evtWriteIn;
          cl::Event evtKInputDone;
          cl::Event evtKOutputDone;
          cl::Event evtReadOut;

          ATH_MSG_DEBUG("Allocating IO buffers");
          const size_t pixel_size_bytes = dataPixelOut.size() * sizeof(uint64_t);

          m_insideOutInputBuffer  = cl::Buffer(m_context, CL_MEM_READ_ONLY,  pixel_size_bytes, nullptr, &err);
          m_insideOutOutputBuffer = cl::Buffer(m_context, CL_MEM_WRITE_ONLY, pixel_size_bytes, nullptr, &err);

          ATH_MSG_DEBUG("Setting IO args");
          m_insideOutInput.setArg(0,  m_insideOutInputBuffer);
          m_insideOutOutput.setArg(0, m_insideOutOutputBuffer);

          ATH_MSG_DEBUG("Loading input data to IO input kernel");
          m_queue.enqueueWriteBuffer(m_insideOutInputBuffer, CL_TRUE, 0, pixel_size_bytes, dataPixelOut.data(), nullptr, &evtWriteIn);

          // Execute
          ATH_MSG_DEBUG("Executing IO kernels");
          std::vector<cl::Event> waitAfterWrite{evtWriteIn};
          m_queue.enqueueTask(m_insideOutInput,  &waitAfterWrite, &evtKInputDone);
          m_queue.enqueueTask(m_insideOutOutput, NULL, &evtKOutputDone);

          // Read
          ATH_MSG_ALWAYS("Reading output data from kernel");
          std::vector<uint64_t> out_data(dataPixelOut.size(), 0);
          std::vector<cl::Event> waitForRead{evtKOutputDone};
          m_queue.enqueueReadBuffer( m_insideOutOutputBuffer, CL_TRUE, 0, pixel_size_bytes, out_data.data(), &waitForRead, &evtReadOut);

          // Ensure completion (optional since read is blocking, but explicit is fine)
          cl::Event::waitForEvents({evtReadOut});
          dumpHexData(out_data, "HW_insideOut.txt");
          m_IO_kernelTime += evtKOutputDone.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evtKInputDone.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        }
        

        return StatusCode::SUCCESS;
    }
    
    StatusCode F150KernelTesterAlg::finalize()
    {
        ATH_MSG_INFO("Finalizing F150KernelTesterAlg");
        ATH_MSG_INFO("Number of events: " << m_numEvents);

        if(m_numEvents > 0){
            ATH_MSG_INFO("Inside out ave time: " << m_IO_kernelTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Slicing Engine ave time: " << m_SE_kernelTime / m_numEvents / 1e6 << " ms");
        }

      return StatusCode::SUCCESS;
    }
}
