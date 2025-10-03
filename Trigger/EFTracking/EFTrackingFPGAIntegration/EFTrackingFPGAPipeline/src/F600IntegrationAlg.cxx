/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EFTrackingFPGAPipeline/F600IntegrationAlg.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"
#include "AthenaKernel/Chrono.h"
#include "EFTrackingFPGAPipeline/DataPreparationPipeline.h"

namespace EFTrackingFPGAIntegration
{
    std::string F600IntegrationAlg::get_cu_name(const std::string& kernel_name, int cu) {
        std::string full_cu_name = kernel_name + ":{" + kernel_name + "_" + std::to_string(cu) + "}";
        ATH_MSG_DEBUG("LOADING " + full_cu_name);
        return full_cu_name;
    }

    void F600IntegrationAlg::outputHexData(size_t dataLen, uint64_t *data, const std::string& dataDescriptor) const {
        ATH_MSG_DEBUG("STARTING " << dataDescriptor << " words:");
        for (size_t i = 0; i < dataLen; i++) {
          ATH_MSG_DEBUG(std::hex << std::setw(16) << std::setfill('0') << data[i]);
        }
        ATH_MSG_DEBUG("ENDING " << dataDescriptor << " words");
    }

    StatusCode F600IntegrationAlg::initialize()
    {
        ATH_MSG_INFO("Running on the FPGA accelerator");

        ATH_CHECK(m_chronoSvc.retrieve());

        {
            Athena::Chrono chrono("Platform and device initlize", m_chronoSvc.get());
            ATH_CHECK(IntegrationBase::initialize());

            ATH_MSG_INFO("Initializing xrt::device");
            m_xrt_accelerator = xrt::opencl::get_xrt_device(m_accelerator.get());
            ATH_MSG_INFO("xrt::device bdf: " + m_xrt_accelerator.get_info<xrt::info::device::bdf>() + " with name: " + m_xrt_accelerator.get_info<xrt::info::device::name>());
        }

        {
            Athena::Chrono chrono("CL::loadProgram", m_chronoSvc.get());
            ATH_MSG_INFO("LOADING PROGRAM: " + m_xclbin);
            ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));
        }

        cl_int err = CL_SUCCESS;

        xrt::uuid loaded_xclbin_uuid = m_xrt_accelerator.get_xclbin_uuid();
        int cu = 1;

        // Pixel clustering
        m_pixelClusterL2GKernels.push_back(cl::Kernel(m_program, get_cu_name(m_pixelClusteringKernelName, cu).c_str(), &err));

        // Slicing Engine
        for (int i = 0; i < 1; ++i) {
            m_pixelFirstStageSlicingIPs.push_back(xrt::ip(m_xrt_accelerator, loaded_xclbin_uuid, get_cu_name(m_pixelFirstStageSlicingIPName, cu + i)));
            m_pixelFirstStageSlicingInputKernels.push_back(cl::Kernel(m_program, get_cu_name(m_pixelFirstStageInputKernelName, cu + i).c_str(), &err));
            m_pixelFirstStageSlicingOutputKernels.push_back(cl::Kernel(m_program, get_cu_name(m_pixelFirstStageOutputKernelName, cu + i).c_str(), &err));
        }

        m_insideOutInputKernels.push_back(cl::Kernel(m_program, get_cu_name(m_memReadKernelName, cu).c_str(), &err));
        m_insideOutOutputKernels.push_back(cl::Kernel(m_program, get_cu_name(m_memWriteKernelName, cu).c_str(), &err));

        m_queue = cl::CommandQueue(m_context, m_accelerator, CL_QUEUE_PROFILING_ENABLE , &err);
        
        if (err != CL_SUCCESS) {
          return StatusCode::FAILURE;
        }

        ATH_CHECK(m_xaodClusterMaker.retrieve());
        ATH_CHECK(m_testVectorTool.retrieve());
        ATH_CHECK(m_FPGADataFormatTool.retrieve());
        ATH_CHECK(m_FPGATrackKey.initialize());
        ATH_CHECK(m_outputConversionTool.retrieve());
        return StatusCode::SUCCESS;
    }

    int F600IntegrationAlg::readTVLength(std::string filepath) const {
        int n_words;
        int size = 0;
        ATH_MSG_DEBUG("Reading len of: " << filepath);
        size = std::filesystem::file_size(filepath);
        n_words = size / (16 + 1);
        ATH_MSG_DEBUG("Num TV words (64b): " << n_words);
        return n_words;
    }

    void F600IntegrationAlg::readTV(const std::string& filepath, int n_words, uint64_t *data) const {
        std::ifstream file_in;
        std::string line_in = "";

        std::fill(data, data + n_words, 0);

        ATH_MSG_DEBUG("Reading data from file: " + filepath);
        file_in.open(filepath);
        if (file_in.is_open()) {
            for(int i = 0; i < n_words; i++) {
                std::getline (file_in, line_in);
                data[i] = std::stoul(line_in.c_str(), NULL, 16);
            }
        } else {
            ATH_MSG_INFO("ERROR! Unable to open data file: " + filepath);
        }
        ATH_MSG_DEBUG("Closing " << filepath);
        file_in.close();
    }
    
    StatusCode F600IntegrationAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F600IntegrationAlg");

        int n_pixel_words = readTVLength(m_pixelClusterInputPath);
        int n_pixel_ghitz_words = readTVLength(m_pixelStageOneSlicingInputPath);
        int n_pixel_first_sliced_words = readTVLength(m_insideOutInputPath);
        int n_inside_out_words = 4 * 1024;

        ATH_MSG_DEBUG("Pixel Clustering TV words = " << n_pixel_words << " (64b)");
        ATH_MSG_DEBUG("Pixel GHITZ TV words = " << n_pixel_ghitz_words << " (64b)");
        ATH_MSG_DEBUG("Pixel Sliced TV words = " << n_pixel_first_sliced_words << " (64b)");

        uint64_t pixel_cls_input_data[n_pixel_words];
        uint64_t pixel_ghitz_data[n_pixel_ghitz_words];
        uint64_t pixel_first_sliced_data[n_pixel_first_sliced_words];

        readTV(m_pixelClusterInputPath, n_pixel_words, pixel_cls_input_data);
        readTV(m_pixelStageOneSlicingInputPath, n_pixel_ghitz_words, pixel_ghitz_data);
        readTV(m_insideOutInputPath, n_pixel_first_sliced_words, pixel_first_sliced_data);

        std::unique_lock lock(m_fpgaHandleMtx);

        cl_int err = 0;
        size_t pixel_size_bytes = n_pixel_words * sizeof(uint64_t);
        size_t pixel_ghitz_size_bytes = n_pixel_ghitz_words * sizeof(uint64_t);
        size_t pixel_first_sliced_size_bytes = n_pixel_first_sliced_words * sizeof(uint64_t);
        size_t inside_out_size_bytes = n_inside_out_words * sizeof(uint64_t);

        ATH_MSG_DEBUG("Allocating pxl cls input buffer");
        m_pixelClusterL2GInputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, pixel_size_bytes, NULL, &err));
        ATH_MSG_DEBUG("Allocating pxl cls out buffer");
        m_pixelClusterL2GOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, pixel_ghitz_size_bytes, NULL, &err));
        ATH_MSG_DEBUG("Allocating pxl cls EDM out buffer");
        m_pixelClusterL2GEDMOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, pixel_ghitz_size_bytes, NULL, &err));
        ATH_MSG_DEBUG("Allocating Input Buffer in Global Memory");
        m_pixelFirstStageSlicingInputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, pixel_ghitz_size_bytes, NULL, &err));
        ATH_MSG_DEBUG("Allocating Output Buffer in Global Memory");
        m_pixelFirstStageSlicingOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, pixel_first_sliced_size_bytes, NULL, &err));
        ATH_MSG_DEBUG("Allocating Inside PR Input Buffer in Global Memory");
        m_insideOutInputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, pixel_first_sliced_size_bytes, NULL, &err));
        ATH_MSG_DEBUG("Allocating Inside PR Output Buffer in Global Memory");
        m_insideOutOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, inside_out_size_bytes, NULL, &err));

        // Create Input/Output Kernel runs and set arguments
        ATH_MSG_DEBUG("Setting buffer for pxl cls input.");
        m_pixelClusterL2GKernels[0].setArg(0, m_pixelClusterL2GInputBufferList[0]);
        ATH_MSG_DEBUG("Setting buffer for pxl cls output.");
        m_pixelClusterL2GKernels[0].setArg(1, m_pixelClusterL2GOutputBufferList[0]);
        ATH_MSG_DEBUG("Setting buffer for pxl cls EDM output.");
        m_pixelClusterL2GKernels[0].setArg(2, m_pixelClusterL2GEDMOutputBufferList[0]);

        ATH_MSG_DEBUG("Setting buffer for Input kernel run.");
        m_pixelFirstStageSlicingInputKernels[0].setArg(0, m_pixelFirstStageSlicingInputBufferList[0]);
        ATH_MSG_DEBUG("Setting # words for Input kernel run.");
        m_pixelFirstStageSlicingInputKernels[0].setArg(1, n_pixel_ghitz_words);

        ATH_MSG_DEBUG("Setting buffer for Output kernel run.");
        m_pixelFirstStageSlicingOutputKernels[0].setArg(0, m_pixelFirstStageSlicingOutputBufferList[0]);
        ATH_MSG_DEBUG("Setting # words for Output kernel run.");
        m_pixelFirstStageSlicingOutputKernels[0].setArg(1, n_pixel_first_sliced_words);

        ATH_MSG_DEBUG("Setting buffer for Input kernel run INSIDE OUT.");
        m_insideOutInputKernels[0].setArg(0, m_insideOutInputBufferList[0]);

        ATH_MSG_DEBUG("Setting buffer for Output kernel run INSIDE OUT.");
        m_insideOutOutputKernels[0].setArg(0, m_insideOutOutputBufferList[0]);

        // Pixel Clustering
        // Write
        ATH_MSG_DEBUG("Loading input data to pixel clustering kernel...");
        m_queue.enqueueWriteBuffer(m_pixelClusterL2GInputBufferList[0], CL_TRUE, 0, pixel_size_bytes, &pixel_cls_input_data, NULL, NULL);

        // Verify write
        outputHexData(n_pixel_words, pixel_cls_input_data, "Pixel Clustering Input");
        
        // Execute
        ATH_MSG_DEBUG("Executing pixel clustering");
        m_queue.enqueueTask(m_pixelClusterL2GKernels[0]);
        m_queue.finish();

        // Read
        ATH_MSG_DEBUG("Reading pixel clustering output");
        uint64_t cls_out_data[n_pixel_ghitz_words];
        m_queue.enqueueReadBuffer(m_pixelClusterL2GOutputBufferList[0], CL_TRUE, 0, pixel_ghitz_size_bytes, &cls_out_data);

        outputHexData(n_pixel_ghitz_words, cls_out_data, "Real Pixel Clustering Output");

        // Read
        ATH_MSG_DEBUG("Reading pixel clustering EDM output");
        uint64_t cls_out_edm_data[n_pixel_ghitz_words];
        m_queue.enqueueReadBuffer(m_pixelClusterL2GEDMOutputBufferList[0], CL_TRUE, 0, pixel_ghitz_size_bytes, &cls_out_edm_data);

        outputHexData(n_pixel_ghitz_words, cls_out_edm_data, "Real Pixel Clustering EDM Output");


        ATH_MSG_DEBUG("Loading input data to Slicing Engine...");
        m_queue.enqueueWriteBuffer(m_pixelFirstStageSlicingInputBufferList[0], CL_TRUE, 0, pixel_ghitz_size_bytes, &pixel_ghitz_data, NULL, NULL);

        outputHexData(n_pixel_ghitz_words, pixel_ghitz_data, "Ideal Pixel Clustering Output");

        // Reset slicing engine event counters
        m_pixelFirstStageSlicingIPs[0].write_register(USER_CTRL_OFFSET, EVENT_COUNT_RST);

        ATH_MSG_DEBUG("------------------------------------------------------------------------------------------------");

        // Start Input/Output Kernel runs
        ATH_MSG_DEBUG("Starting Input kernel run.");
        m_queue.enqueueTask(m_pixelFirstStageSlicingInputKernels[0]);
        ATH_MSG_DEBUG("Starting Output kernel run.");
        m_queue.enqueueTask(m_pixelFirstStageSlicingOutputKernels[0]);
        m_queue.finish();

        // Get the output;
        ATH_MSG_DEBUG("Synchronize output buffer data from device global memory");
        uint64_t se_out_data[n_pixel_first_sliced_words];
        m_queue.enqueueReadBuffer(m_pixelFirstStageSlicingOutputBufferList[0], CL_TRUE, 0, pixel_first_sliced_size_bytes, &se_out_data);

        outputHexData(n_pixel_first_sliced_words, se_out_data, "Slicing Engine Output");

        ATH_MSG_DEBUG("Write SE output to inside out input (cross DDR banks)");
        m_queue.enqueueWriteBuffer(m_insideOutInputBufferList[0], CL_TRUE, 0, pixel_first_sliced_size_bytes, &pixel_first_sliced_data, NULL, NULL);

        ATH_MSG_DEBUG("Starting Inside Out Input kernel run.");
        m_queue.enqueueTask(m_insideOutInputKernels[0]);
        m_queue.finish();
        ATH_MSG_DEBUG("Starting Inside Out Output kernel run.");
        m_queue.enqueueTask(m_insideOutOutputKernels[0]);
        m_queue.finish();


        ATH_MSG_DEBUG("Slicing Engine System Done");
        ATH_MSG_DEBUG("------------------------------------------------------------------------------------------------");


        return StatusCode::SUCCESS;
    }
    
    StatusCode F600IntegrationAlg::finalize()
    {
      ATH_MSG_INFO("Average Kernel execution time: " << m_sum_kernelTime /m_num_Events /1e6 << " ms");
      return StatusCode::SUCCESS;
    }
}
