/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
   */

#include "EFTrackingFPGAPipeline/F100IntegrationAlg.h"
#include "AthenaKernel/Chrono.h"

namespace EFTrackingFPGAIntegration
{
    StatusCode F100IntegrationAlg::initialize()
    {
        ATH_MSG_INFO("Running on the FPGA accelerator");

        ATH_CHECK(IntegrationBase::precheck({m_xclbin}));

        ATH_CHECK(m_chronoSvc.retrieve());

        {
            Athena::Chrono chrono("Platform and device initlize", m_chronoSvc.get());
            ATH_CHECK(IntegrationBase::initialize());
        }

        {
            Athena::Chrono chrono("CL::loadProgram", m_chronoSvc.get());
            ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));
        }
        ATH_MSG_INFO("loading "<<m_xclbin);

        
        ATH_CHECK(m_FPGAStripRDO.initialize());
        ATH_CHECK(m_FPGAPixelRDO.initialize());

        ATH_CHECK(m_FPGAStripOutput.initialize());
        ATH_CHECK(m_FPGAPixelOutput.initialize());

        cl_int err = 0;

        // create the buffers
        for(int i = 0; i < m_FPGAThreads.value(); i++)
        {
            m_acc_queues.emplace_back(m_context, m_accelerator, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);

            // Input
            m_pixelClusterInputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::PIXEL_CONTAINER_INPUT_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            m_stripClusterInputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::STRIP_CONTAINER_INPUT_BUF_SIZE * sizeof(uint64_t), NULL, &err));

            // Clustering
            if (!m_doF110) {
                m_pixelClusterOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            }
            m_stripClusterOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            m_pixelClusterEDMOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE,EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            m_stripClusterEDMOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            // L2G
            if (!m_doF110) {
                m_pixelL2GOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err)); 
                m_pixelL2GEDMOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            }
            m_stripL2GOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err)); 
            m_stripL2GEDMOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            // EDMPrep
            m_edmPixelOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            m_edmStripOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE * sizeof(uint64_t), NULL, &err));
        }

        for (int i = 0; i < m_FPGAThreads.value(); ++i) {
            // Indexing CUs
            size_t pixelCU = (i % m_NpixelCU.value()) + 1;
            size_t stripCU = (i % m_NstripCU.value()) + 1;

            // Pixel clustering
            std::string pixelClustName = std::string(m_pixelClusterKernelName.value()) + ":{" + std::string(m_pixelClusterKernelName.value()) + "_" + std::to_string(pixelCU) + "}";
            m_pixelClusteringKernels.emplace_back(cl::Kernel(m_program, pixelClustName.c_str()));

            // Strip clustering
            std::string stripClustName = std::string(m_stripClusterKernelName.value()) + ":{" + std::string(m_stripClusterKernelName.value()) + "_" + std::to_string(stripCU) + "}";
            m_stripClusteringKernels.emplace_back(cl::Kernel(m_program, stripClustName.c_str()));

            if (!m_doF110) {
                // Pixel L2G
                std::string pixelL2GName = std::string(m_pixelL2GKernelName.value()) + ":{" + std::string(m_pixelL2GKernelName.value()) + "_" + std::to_string(pixelCU) + "}";
                m_pixelL2GKernels.emplace_back(cl::Kernel(m_program, pixelL2GName.c_str()));
            }

            // Strip L2G
            std::string stripL2GName = std::string(m_stripL2GKernelName.value()) + ":{" + std::string(m_stripL2GKernelName.value()) + "_" + std::to_string(stripCU) + "}";
            m_stripL2GKernels.emplace_back(cl::Kernel(m_program, stripL2GName.c_str()));

            // EDM prep
            if (m_doF110) {
                std::string pixelEdmName = std::string(m_pixelEdmKernelName.value()) + ":{" + std::string(m_pixelEdmKernelName.value()) + "_" + std::to_string(pixelCU) + "}";
                m_pixelEdmPrepKernels.emplace_back(cl::Kernel(m_program, pixelEdmName.c_str()));

                std::string stripEdmName = std::string(m_stripEdmKernelName.value()) + ":{" + std::string(m_stripEdmKernelName.value()) + "_" + std::to_string(stripCU) + "}";
                m_stripEdmPrepKernels.emplace_back(cl::Kernel(m_program, stripEdmName.c_str()));
            } else {
                std::string edmPrepName = std::string(m_edmKernelName.value()) + ":{" + std::string(m_edmKernelName.value()) + "_" + std::to_string(stripCU) + "}";
                m_edmPrepKernels.emplace_back(cl::Kernel(m_program, edmPrepName.c_str()));
            }
        }



        return StatusCode::SUCCESS;
    }

    StatusCode F100IntegrationAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F100IntegrationAlg");
        m_numEvents++;

        /// Input handles
        auto pixelInput = SG::get(m_FPGAPixelRDO, ctx);
        auto stripInput = SG::get(m_FPGAStripRDO, ctx);

    
        // logic

        size_t bufferIndex = ctx.slot() % m_FPGAThreads.value();

        size_t pixelKernelIndex = ctx.slot() % m_NpixelCU.value() + 1;
        size_t stripKernelIndex = ctx.slot() % m_NstripCU.value() + 1;
        
        const cl::CommandQueue &acc_queue = m_acc_queues[bufferIndex];


        ATH_MSG_DEBUG("Thread number "<<ctx.slot()<<" running on buffer "<<bufferIndex<<" pixelKernelIndex: "<< pixelKernelIndex<<" stripKernelIndex: "<< stripKernelIndex);

        cl::Kernel &pixelClusteringKernel = m_pixelClusteringKernels[bufferIndex];
        cl::Kernel &stripClusteringKernel = m_stripClusteringKernels[bufferIndex];
        cl::Kernel &stripL2GKernel = m_stripL2GKernels[bufferIndex];

        cl::Kernel *pixelL2GKernel = m_doF110 ? nullptr : &m_pixelL2GKernels[bufferIndex];
        cl::Kernel *edmPrepKernel = nullptr;
        cl::Kernel *pixelEdmPrepKernel = nullptr;
        cl::Kernel *stripEdmPrepKernel = nullptr;

        if (m_doF110) {
            pixelEdmPrepKernel = &m_pixelEdmPrepKernels[bufferIndex];
            stripEdmPrepKernel = &m_stripEdmPrepKernels[bufferIndex];
        } else {
            edmPrepKernel = &m_edmPrepKernels[bufferIndex];
        }

        // Set kernel arguments
        pixelClusteringKernel.setArg(0, m_pixelClusterInputBufferList[bufferIndex]);
        if (m_doF110) {
            pixelClusteringKernel.setArg(1, m_pixelClusterEDMOutputBufferList[bufferIndex]);
        } else {
            pixelClusteringKernel.setArg(1, m_pixelClusterOutputBufferList[bufferIndex]);
            pixelClusteringKernel.setArg(2, m_pixelClusterEDMOutputBufferList[bufferIndex]);
        }

        stripClusteringKernel.setArg(0, m_stripClusterInputBufferList[bufferIndex]);
        stripClusteringKernel.setArg(1, m_stripClusterOutputBufferList[bufferIndex]);
        stripClusteringKernel.setArg(2, m_stripClusterEDMOutputBufferList[bufferIndex]);
        stripClusteringKernel.setArg(3, static_cast<unsigned int>((*stripInput).size()));

        if (!m_doF110) {
            pixelL2GKernel->setArg(0, m_pixelClusterOutputBufferList[bufferIndex]);
            pixelL2GKernel->setArg(1, m_pixelClusterEDMOutputBufferList[bufferIndex]);
            pixelL2GKernel->setArg(2, m_pixelL2GOutputBufferList[bufferIndex]);
            pixelL2GKernel->setArg(3, m_pixelL2GEDMOutputBufferList[bufferIndex]);
        }

        stripL2GKernel.setArg(0, m_stripClusterOutputBufferList[bufferIndex]);
        stripL2GKernel.setArg(1, m_stripClusterEDMOutputBufferList[bufferIndex]);
        stripL2GKernel.setArg(2, m_stripL2GOutputBufferList[bufferIndex]);
        stripL2GKernel.setArg(3, m_stripL2GEDMOutputBufferList[bufferIndex]);

        if (m_doF110) {
            pixelEdmPrepKernel->setArg(0, m_pixelClusterEDMOutputBufferList[bufferIndex]);
            pixelEdmPrepKernel->setArg(1, m_edmPixelOutputBufferList[bufferIndex]);
            stripEdmPrepKernel->setArg(0, m_stripL2GEDMOutputBufferList[bufferIndex]);
            stripEdmPrepKernel->setArg(1, m_edmStripOutputBufferList[bufferIndex]);
        } else {
            edmPrepKernel->setArg(0, m_pixelL2GEDMOutputBufferList[bufferIndex]);
            edmPrepKernel->setArg(1, m_stripL2GEDMOutputBufferList[bufferIndex]);
            edmPrepKernel->setArg(2, m_edmPixelOutputBufferList[bufferIndex]);
            edmPrepKernel->setArg(3, m_edmStripOutputBufferList[bufferIndex]);
        }



        // Start the transfers
        cl::Event evt_write_pixel_input;
        cl::Event evt_write_strip_input;

        acc_queue.enqueueWriteBuffer(m_pixelClusterInputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint64_t) * (*pixelInput).size(), (*pixelInput).data(), NULL, &evt_write_pixel_input);
        acc_queue.enqueueWriteBuffer(m_stripClusterInputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint64_t) * (*stripInput).size(), (*stripInput).data(), NULL, &evt_write_strip_input);
        std::vector<cl::Event> evt_vec_pixel_input{evt_write_pixel_input};
        std::vector<cl::Event> evt_vec_strip_input{evt_write_strip_input};


        cl::Event evt_pixel_clustering;
        cl::Event evt_strip_clustering;
        cl::Event evt_strip_l2g;
        cl::Event evt_pixel_l2g;
        cl::Event evt_edm_prep;
        cl::Event evt_pixel_edm_prep;
        cl::Event evt_strip_edm_prep;
        {
            Athena::Chrono chrono("Kernel execution", m_chronoSvc.get());
            acc_queue.enqueueTask(pixelClusteringKernel, &evt_vec_pixel_input, &evt_pixel_clustering);
            acc_queue.enqueueTask(stripClusteringKernel, &evt_vec_strip_input, &evt_strip_clustering);

            std::vector<cl::Event> evt_vec_pixel_clustering{evt_pixel_clustering};
            std::vector<cl::Event> evt_vec_strip_clustering{evt_strip_clustering};
            if (!m_doF110) 
            {
                acc_queue.enqueueTask(*pixelL2GKernel, &evt_vec_pixel_clustering, &evt_pixel_l2g);
            }
            acc_queue.enqueueTask(stripL2GKernel, &evt_vec_strip_clustering, &evt_strip_l2g);

            if (!m_doF110) 
            {
                std::vector<cl::Event> evt_vec_l2g{evt_pixel_l2g, evt_strip_l2g};
                acc_queue.enqueueTask(*edmPrepKernel, &evt_vec_l2g, &evt_edm_prep);
            } 
            else {
                std::vector<cl::Event> evt_vec_strip_l2g{evt_strip_l2g};
                acc_queue.enqueueTask(*pixelEdmPrepKernel, &evt_vec_pixel_clustering, &evt_pixel_edm_prep);
                acc_queue.enqueueTask(*stripEdmPrepKernel, &evt_vec_strip_l2g, &evt_strip_edm_prep);
            }
        }

        cl::Event evt_pixel_cluster_output;
        cl::Event evt_strip_cluster_output;
        
        std::vector<cl::Event> evt_vec_pixel_edm_prep;
        std::vector<cl::Event> evt_vec_strip_edm_prep;
        if(m_doF110)
        {
            evt_vec_pixel_edm_prep.push_back(evt_pixel_edm_prep);
            evt_vec_strip_edm_prep.push_back(evt_strip_edm_prep);
        }
        else
        {
            evt_vec_pixel_edm_prep.push_back(evt_edm_prep);
            evt_vec_strip_edm_prep.push_back(evt_edm_prep);
        }

        // output handles

        SG::WriteHandle<std::vector<uint64_t>> FPGAPixelOutput(m_FPGAPixelOutput, ctx);
        ATH_CHECK(FPGAPixelOutput.record(std::make_unique<std::vector<uint64_t> >(EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE, 0)));

        SG::WriteHandle<std::vector<uint64_t>> FPGAStripOutput(m_FPGAStripOutput, ctx);
        ATH_CHECK(FPGAStripOutput.record(std::make_unique<std::vector<uint64_t> >(EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE, 0)));

        acc_queue.enqueueReadBuffer(m_edmPixelOutputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint64_t) * (*FPGAPixelOutput).size(), (*FPGAPixelOutput).data(), &evt_vec_pixel_edm_prep, &evt_pixel_cluster_output);
        acc_queue.enqueueReadBuffer(m_edmStripOutputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint64_t) * (*FPGAStripOutput).size(), (*FPGAStripOutput).data(), &evt_vec_strip_edm_prep, &evt_strip_cluster_output);

        std::vector<cl::Event> wait_for_reads = { evt_pixel_cluster_output, evt_strip_cluster_output };
        cl::Event::waitForEvents(wait_for_reads);

        // calculate the time for the kernel execution
        // get the time of writing pixel input buffer
        cl_ulong pixel_input_time = evt_write_pixel_input.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_write_pixel_input.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_pixelInputTime += pixel_input_time;
        ATH_MSG_DEBUG("Pixel input buffer write time: " << pixel_input_time / 1e6 << " ms");

        // get the time of writing strip input buffer
        cl_ulong strip_input_time = evt_write_strip_input.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_write_strip_input.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_stripInputTime += strip_input_time;
        ATH_MSG_DEBUG("Strip input buffer write time: " << strip_input_time / 1e6 << " ms");

        // get the time of pixel clustering
        cl_ulong pixel_clustering_time = evt_pixel_clustering.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_pixel_clustering.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_pixelClusteringTime += pixel_clustering_time;
        ATH_MSG_DEBUG("Pixel clustering time: " << pixel_clustering_time / 1e6 << " ms");

        // get the time of strip clustering
        cl_ulong strip_clustering_time = evt_strip_clustering.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_strip_clustering.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_stripClusteringTime += strip_clustering_time;
        ATH_MSG_DEBUG("Strip clustering time: " << strip_clustering_time / 1e6 << " ms");

        if (!m_doF110) {
            // get the time of pixel L2G
            cl_ulong pixel_l2g_time = evt_pixel_l2g.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_pixel_l2g.getProfilingInfo<CL_PROFILING_COMMAND_START>();
            m_pixelL2GTime += pixel_l2g_time;
            ATH_MSG_DEBUG("Pixel L2G time: " << pixel_l2g_time / 1e6 << " ms");
        }

        // get the time of strip L2G
        cl_ulong strip_l2g_time = evt_strip_l2g.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_strip_l2g.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_stripL2GTime += strip_l2g_time;
        ATH_MSG_DEBUG("Strip L2G time: " << strip_l2g_time / 1e6 << " ms");

        // get the time of EDMPrep
        if (m_doF110) {
            cl_ulong pixel_edm_prep_time = evt_pixel_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_pixel_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_START>();
            cl_ulong strip_edm_prep_time = evt_strip_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_strip_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_START>();

            m_pixelEdmPrepTime += pixel_edm_prep_time;
            ATH_MSG_DEBUG("PixelEDMPrep time: " << pixel_edm_prep_time / 1e6 << " ms");

            m_stripEdmPrepTime += strip_edm_prep_time;
            ATH_MSG_DEBUG("StripEDMPrep time: " << strip_edm_prep_time / 1e6 << " ms");
        } 
        else {
            cl_ulong edm_prep_time = evt_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_START>();

            m_edmPrepTime += edm_prep_time;
            ATH_MSG_DEBUG("EDMPrep time: " << edm_prep_time / 1e6 << " ms");
        }

        // get the time of the whole kernel execution
        cl_ulong kernel_start = evt_pixel_clustering.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
        cl_ulong kernel_end = m_doF110 ?
        std::max(evt_pixel_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>(), evt_strip_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>()) :
        evt_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        m_kernelTime += (kernel_end - kernel_start);
        ATH_MSG_DEBUG("Kernel execution time: " << (kernel_end - kernel_start) / 1e6 << " ms");

        // get the time of reading pixel output buffer
        cl_ulong pixel_output_time = evt_pixel_cluster_output.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_pixel_cluster_output.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_pixelOutputTime += pixel_output_time;
        ATH_MSG_DEBUG("Pixel output buffer read time: " << pixel_output_time / 1e6 << " ms");

        // get the time of reading strip output buffer
        cl_ulong strip_output_time = evt_strip_cluster_output.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_strip_cluster_output.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_stripOutputTime += strip_output_time;
        ATH_MSG_DEBUG("Strip output buffer read time: " << strip_output_time / 1e6 << " ms");

        return StatusCode::SUCCESS;
    }

    StatusCode F100IntegrationAlg::finalize()
    {

        ATH_MSG_INFO("Finalizing F100IntegrationAlg");
        ATH_MSG_INFO("Number of events: " << m_numEvents);
        ATH_MSG_INFO("Pixel input ave time: " << m_pixelInputTime / m_numEvents / 1e6 << " ms");
        ATH_MSG_INFO("Strip input ave time: " << m_stripInputTime / m_numEvents / 1e6 << " ms");
        ATH_MSG_INFO("Pixel clustering ave time: " << m_pixelClusteringTime / m_numEvents / 1e6 << " ms");
        ATH_MSG_INFO("Strip clustering ave time: " << m_stripClusteringTime / m_numEvents / 1e6 << " ms");
        if (!m_doF110) {
            ATH_MSG_INFO("Pixel L2G ave time: " << m_pixelL2GTime / m_numEvents / 1e6 << " ms");
        }
        ATH_MSG_INFO("Strip L2G ave time: " << m_stripL2GTime / m_numEvents / 1e6 << " ms");
        if (!m_doF110) {
            ATH_MSG_INFO("EDMPrep ave time: " << m_edmPrepTime / m_numEvents / 1e6 << " ms");
        } else {
            ATH_MSG_INFO("PixelEDMPrep ave time: " << m_pixelEdmPrepTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("StripEDMPrep ave time: " << m_stripEdmPrepTime / m_numEvents / 1e6 << " ms");
        }
        ATH_MSG_INFO("Kernel execution ave time: " << m_kernelTime / m_numEvents / 1e6 << " ms");
        ATH_MSG_INFO("Pixel output ave time: " << m_pixelOutputTime / m_numEvents / 1e6 << " ms");
        ATH_MSG_INFO("Strip output ave time: " << m_stripOutputTime / m_numEvents / 1e6 << " ms");

        return StatusCode::SUCCESS;
    }
} // namespace EFTrackingFPGAIntegration
