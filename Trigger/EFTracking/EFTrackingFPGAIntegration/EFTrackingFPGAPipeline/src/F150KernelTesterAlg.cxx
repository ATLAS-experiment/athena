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

    void F150KernelTesterAlg::dumpHexData(size_t dataLen, uint64_t *data, const std::string& dataDescriptor) const {
        ATH_MSG_DEBUG("STARTING " << dataDescriptor << " words:");
        std::ofstream outputFile(dataDescriptor);

        for (size_t i = 0; i < dataLen; i++) {
          outputFile << std::hex << std::setw(16) << std::setfill('0') << data[i] << std::endl;
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
        dumpHexData(pixelDataIN.size(), pixelDataIN.data(), "FPGATrackSim_slicingIn_pixel.txt");
        dumpHexData(stripDataIN.size(), stripDataIN.data(), "FPGATrackSim_slicingIn_strip.txt");

        ATH_MSG_DEBUG("Accessing SE Out data.");
        std::vector<uint64_t> dataPixelOut;
        std::vector<uint64_t> dataStripOut;
        SG::ReadHandle<FPGATrackSimHitCollection> outhitCollectionHandle(m_FPGASlicedHitKey, ctx);
        ATH_CHECK(m_FPGADataFormatTool->convertFPGASliceToFPGADataFormat(outhitCollectionHandle.cptr(), true, false, dataPixelOut, ctx));
        ATH_CHECK(m_FPGADataFormatTool->convertFPGASliceToFPGADataFormat(outhitCollectionHandle.cptr(), false, true, dataStripOut, ctx));
        dumpHexData(dataPixelOut.size(), dataPixelOut.data(), "FPGATrackSim_slicingOut_pixel.txt");
        dumpHexData(dataStripOut.size(), dataStripOut.data(), "FPGATrackSim_slicingOut_strip.txt");

        ATH_MSG_DEBUG("Accessing SE Out data.");
        std::vector<uint64_t> dataInsideOut;
        ATH_CHECK(m_FPGADataFormatTool->convertFPGATracksToFPGADataFormat(outTrackCollection.cptr(), dataInsideOut, ctx));
        dumpHexData(dataInsideOut.size(), dataInsideOut.data(), "FPGATrackSim_insideOut.txt");


        cl_int err = CL_SUCCESS;

        if (m_runSE) {
          ATH_MSG_DEBUG("Allocating SE buffers");

          size_t pixel_size_bytes = pixelDataIN.size() * sizeof(uint64_t);

          m_slicingEngineInputBuffer = cl::Buffer(m_context, CL_MEM_READ_ONLY, pixel_size_bytes, NULL, &err);
          m_slicingEngineOutputBuffer = cl::Buffer(m_context, CL_MEM_WRITE_ONLY, pixel_size_bytes, NULL, &err);

          m_slicingEngineInput.setArg(0, m_slicingEngineInputBuffer);
          m_slicingEngineInput.setArg(2, static_cast<unsigned long long>(pixelDataIN.size()));

          m_slicingEngineOutput.setArg(1, m_slicingEngineOutputBuffer);

          ATH_MSG_DEBUG("Transfering SE data");
          m_queue.enqueueWriteBuffer(m_slicingEngineInputBuffer, CL_FALSE, 0, sizeof(uint64_t) * pixelDataIN.size(), &pixelDataIN);
          m_queue.finish();
          
          // Execute
          ATH_MSG_DEBUG("Executing SE kernel");
          m_queue.enqueueTask(m_slicingEngineInput);
          m_queue.enqueueTask(m_slicingEngineOutput);
          m_queue.finish();

          // Read
          ATH_MSG_DEBUG("Reading output data from kernel");
          std::vector<uint64_t> out_data(pixelDataIN.size(), 0);
          m_queue.enqueueReadBuffer(m_slicingEngineOutputBuffer, CL_TRUE, 0, pixel_size_bytes, &out_data);
          m_queue.finish();

          dumpHexData(out_data.size(), &out_data[0], "HW_slicingOut_pixel.txt");
        }
        if (m_runIO) {
          ATH_MSG_DEBUG("Allocating IO buffers");
          size_t pixel_size_bytes = dataPixelOut.size() * sizeof(uint64_t);

          m_insideOutInputBuffer = cl::Buffer(m_context, CL_MEM_READ_ONLY, pixel_size_bytes, NULL, &err);
          m_insideOutOutputBuffer = cl::Buffer(m_context, CL_MEM_WRITE_ONLY, pixel_size_bytes, NULL, &err);

          ATH_MSG_DEBUG("Setting IO args");
          // TODO: FIX ARGS
          m_insideOutInput.setArg(0,  m_insideOutInputBuffer);
          m_insideOutOutput.setArg(0, m_insideOutOutputBuffer);

        
          ATH_MSG_DEBUG("Loading input data to IO input kernel");
          m_queue.enqueueWriteBuffer(m_insideOutInputBuffer, CL_TRUE, 0, pixel_size_bytes, &dataPixelOut);
          m_queue.finish();
          

          // Execute
          ATH_MSG_DEBUG("Executing IO kernel");
          m_queue.enqueueTask(m_insideOutInput);
          m_queue.enqueueTask(m_insideOutOutput);
          m_queue.finish();

          // Read
          ATH_MSG_DEBUG("Reading output data from kernel");
          std::vector<uint64_t> out_data(dataPixelOut.size(), 0);
          m_queue.enqueueReadBuffer(m_insideOutOutputBuffer, CL_TRUE, 0, pixel_size_bytes, &out_data);
          m_queue.finish();

          dumpHexData(out_data.size(), &out_data[0], "HW_insideOut.txt");
        }
        

        return StatusCode::SUCCESS;
    }
    
    StatusCode F150KernelTesterAlg::finalize()
    {
      return StatusCode::SUCCESS;
    }
}
