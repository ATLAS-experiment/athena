/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EFTrackingFPGAPipeline/F150KernelTesterAlg.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"
#include "AthenaKernel/Chrono.h"
#include "EFTrackingFPGAPipeline/DataPreparationPipeline.h"

namespace EFTrackingFPGAIntegration
{
    std::string F150KernelTesterAlg::get_cu_name(const std::string& kernel_name, int cu) {
        std::string full_cu_name = kernel_name + ":{" + kernel_name + "_" + std::to_string(cu) + "}";
        ATH_MSG_DEBUG("LOADING " + full_cu_name);
        return full_cu_name;
    }

    void F150KernelTesterAlg::dumpHexData(size_t dataLen, uint64_t *data, const std::string& dataDescriptor) const {
        ATH_MSG_DEBUG("STARTING " << dataDescriptor << " words:");
        for (size_t i = 0; i < dataLen; i++) {
          ATH_MSG_DEBUG(std::hex << std::setw(16) << std::setfill('0') << data[i]);
        }
        ATH_MSG_DEBUG("ENDING " << dataDescriptor << " words");
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

          xrt::uuid loadedXclbinUUID = m_xrt_accelerator.get_xclbin_uuid();
          m_slicingEngineIP = xrt::ip(m_xrt_accelerator, loadedXclbinUUID, get_cu_name(m_slicingEngineName, cu).c_str());
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

        return StatusCode::SUCCESS;
    }

    StatusCode F150KernelTesterAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F150KernelTesterAlg");

        cl_int err = CL_SUCCESS;

        int n_pixel_words = 4;
        size_t pixel_size_bytes = n_pixel_words * sizeof(uint64_t);

        if (m_runSE) {
          ATH_MSG_DEBUG("Allocating SE buffers");
          m_slicingEngineInputBuffer = cl::Buffer(m_context, CL_MEM_READ_ONLY, pixel_size_bytes, NULL, &err);
          m_slicingEngineOutputBuffer = cl::Buffer(m_context, CL_MEM_WRITE_ONLY, pixel_size_bytes, NULL, &err);

          ATH_MSG_DEBUG("Setting SE args");
          // TODO: FIX ARGS
          m_slicingEngineInput.setArg(0, m_slicingEngineInputBuffer); // Input buffer
          m_slicingEngineInput.setArg(1, n_pixel_words); // Input words
          m_slicingEngineOutput.setArg(0, m_slicingEngineOutputBuffer); // Output buffer
          m_slicingEngineOutput.setArg(1, n_pixel_words); // Output words (TODO: how do we get this???)
                                                          
          uint64_t out_data[n_pixel_words];

          // Write
          ATH_MSG_DEBUG("Loading input data to SE input kernel");
          // TODO: Replace with real data to write
          m_queue.enqueueWriteBuffer(m_slicingEngineInputBuffer, CL_TRUE, 0, pixel_size_bytes, &out_data);
          m_queue.finish();
          
          // Execute
          ATH_MSG_DEBUG("Executing SE kernel");
          m_slicingEngineIP.write_register(USER_CTRL_OFFSET, EVENT_COUNT_RST);
          m_queue.enqueueTask(m_slicingEngineInput);
          m_queue.enqueueTask(m_slicingEngineOutput);
          m_queue.finish();

          // Read
          ATH_MSG_DEBUG("Reading output data from kernel");
          m_queue.enqueueReadBuffer(m_slicingEngineOutputBuffer, CL_TRUE, 0, pixel_size_bytes, &out_data);
          m_queue.finish();

          dumpHexData(n_pixel_words, out_data, "Real Slicing Engine Output");
        }
        if (m_runIO) {
          ATH_MSG_DEBUG("Allocating IO buffers");
          if (!m_runIOOnSE) {
            m_insideOutInputBuffer = cl::Buffer(m_context, CL_MEM_READ_ONLY, pixel_size_bytes, NULL, &err);
          }
          m_insideOutOutputBuffer = cl::Buffer(m_context, CL_MEM_WRITE_ONLY, pixel_size_bytes, NULL, &err);

          ATH_MSG_DEBUG("Setting IO args");
          // TODO: FIX ARGS
          m_insideOutInput.setArg(0, m_runIOOnSE ? m_slicingEngineOutputBuffer : m_insideOutInputBuffer);
          m_insideOutOutput.setArg(0, m_insideOutOutputBuffer);

          uint64_t out_data[n_pixel_words];

          if (!m_runIOOnSE) {
            // Write
            ATH_MSG_DEBUG("Loading input data to IO input kernel");
            // TODO: Replace with real data to write
            m_queue.enqueueWriteBuffer(m_insideOutInputBuffer, CL_TRUE, 0, pixel_size_bytes, &out_data);
            m_queue.finish();
          }

          // Execute
          ATH_MSG_DEBUG("Executing IO kernel");
          m_queue.enqueueTask(m_insideOutInput);
          m_queue.enqueueTask(m_insideOutOutput);
          m_queue.finish();

          // Read
          ATH_MSG_DEBUG("Reading output data from kernel");
          m_queue.enqueueReadBuffer(m_slicingEngineOutputBuffer, CL_TRUE, 0, pixel_size_bytes, &out_data);
          m_queue.finish();

          dumpHexData(n_pixel_words, out_data, "Real Inside Out Output");
        }


        return StatusCode::SUCCESS;
    }
    
    StatusCode F150KernelTesterAlg::finalize()
    {
      ATH_MSG_INFO("Average Kernel execution time: " << m_sum_kernelTime /m_num_Events /1e6 << " ms");
      return StatusCode::SUCCESS;
    }
}
