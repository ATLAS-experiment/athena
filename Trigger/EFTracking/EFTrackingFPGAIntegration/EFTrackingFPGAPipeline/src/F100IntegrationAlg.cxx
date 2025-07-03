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

        ATH_CHECK(m_pixelRDOKey.initialize());
        ATH_CHECK(m_stripRDOKey.initialize());

        ATH_CHECK(m_xaodClusterMaker.retrieve());
        ATH_CHECK(m_testVectorTool.retrieve());
        ATH_CHECK(m_FPGADataFormatTool.retrieve());


        cl_int err = 0;

        m_acc_queue = cl::CommandQueue(m_context, m_accelerator, CL_QUEUE_PROFILING_ENABLE, &err);

        // create the buffers
        for(int i = 0; i < m_FPGAThreads.value(); i++)
        {
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

        return StatusCode::SUCCESS;
    }

    StatusCode F100IntegrationAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F100IntegrationAlg");
        m_numEvents++;

        // Create host side output vectors
        std::vector<uint64_t> pixelOutput(EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE, 0);
        std::vector<uint64_t> stripOutput(EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE, 0);

        ATH_CHECK(runDataPrep(pixelOutput, stripOutput, ctx));

        // use 64-bit pointer to access output
        uint64_t *stripClusters = stripOutput.data();
        uint64_t *pixelClusters = pixelOutput.data();

        unsigned int numStripClusters = stripClusters[0];
        ATH_MSG_DEBUG("numStripClusters: " << numStripClusters);

        unsigned int numPixelClusters = pixelClusters[0];
        ATH_MSG_DEBUG("numPixelClusters: " << numPixelClusters);

        std::unique_ptr<EFTrackingTransient::Metadata> metadata = std::make_unique<EFTrackingTransient::Metadata>();

        metadata->numOfStripClusters = numStripClusters;
        metadata->scRdoIndexSize = numStripClusters;
        metadata->numOfPixelClusters = numPixelClusters;
        metadata->pcRdoIndexSize = numPixelClusters;

        // make strip cluster
        ATH_CHECK(m_xaodClusterMaker->makeStripClusterContainer(stripClusters, metadata.get(), ctx));

        // Make pixel cluster
        ATH_CHECK(m_xaodClusterMaker->makePixelClusterContainer(pixelClusters, metadata.get(), ctx));

        return StatusCode::SUCCESS;
    }

    StatusCode F100IntegrationAlg::runDataPrep(std::vector<uint64_t> &pixelChainOutput, std::vector<uint64_t> &stripChainOutput, const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Running DataPrep on FPGA");

        // Get the RDOs from the SG
        auto pixelRDOHandle = SG::makeHandle(m_pixelRDOKey, ctx);
        auto stripRDOHandle = SG::makeHandle(m_stripRDOKey, ctx);

        // Encode RDO into byte stream
        std::vector<uint64_t> encodedPixelRDO;
        std::vector<uint64_t> encodedStripRDO;

        // Encode RDOs into byte stream
        ATH_CHECK(m_FPGADataFormatTool->convertPixelHitsToFPGADataFormat(*pixelRDOHandle, encodedPixelRDO, ctx));
        ATH_CHECK(m_FPGADataFormatTool->convertStripHitsToFPGADataFormat(*stripRDOHandle, encodedStripRDO, ctx));

        for (unsigned int i = 0; i < encodedPixelRDO.size(); i++)
        {
            ATH_MSG_DEBUG("Pixel RDO[" << i << "]: " << std::hex << encodedPixelRDO[i] << std::dec);
        }
        for (unsigned int i = 0; i < encodedStripRDO.size(); i++)
        {
            ATH_MSG_DEBUG("Strip RDO[" << i << "]: " << std::hex << encodedStripRDO[i] << std::dec);
        }

        size_t bufferIndex = ctx.slot() % m_FPGAThreads.value();

        ATH_MSG_INFO("Thread number "<<ctx.slot()<<" running on buffer "<<bufferIndex);

        // Connect kernels to the buffer
        // Kernel names are hard-coded for the current development
        // Clustering
        cl::Kernel pixelClusteringKernel(m_program, m_pixelClusterKernelName.value().data());
        pixelClusteringKernel.setArg<cl::Buffer>(0, m_pixelClusterInputBufferList[bufferIndex]);
        if (m_doF110) {
            pixelClusteringKernel.setArg<cl::Buffer>(1, m_pixelClusterEDMOutputBufferList[bufferIndex]);
        } else {
            pixelClusteringKernel.setArg<cl::Buffer>(1, m_pixelClusterOutputBufferList[bufferIndex]);
            pixelClusteringKernel.setArg<cl::Buffer>(2, m_pixelClusterEDMOutputBufferList[bufferIndex]);
        }

        cl::Kernel stripClusteringKernel(m_program, m_stripClusterKernelName.value().data());
        stripClusteringKernel.setArg<cl::Buffer>(0, m_stripClusterInputBufferList[bufferIndex]);
        stripClusteringKernel.setArg<cl::Buffer>(1, m_stripClusterOutputBufferList[bufferIndex]);
        stripClusteringKernel.setArg<cl::Buffer>(2, m_stripClusterEDMOutputBufferList[bufferIndex]);
        stripClusteringKernel.setArg<unsigned int>(3, encodedStripRDO.size());

        // L2G
        cl::Kernel pixelL2GKernel(m_program, m_pixelL2GKernelName.value().data());
        if (!m_doF110) {
            pixelL2GKernel.setArg<cl::Buffer>(0, m_pixelClusterOutputBufferList[bufferIndex]);
            pixelL2GKernel.setArg<cl::Buffer>(1, m_pixelClusterEDMOutputBufferList[bufferIndex]);
            pixelL2GKernel.setArg<cl::Buffer>(2, m_pixelL2GOutputBufferList[bufferIndex]);
            pixelL2GKernel.setArg<cl::Buffer>(3, m_pixelL2GEDMOutputBufferList[bufferIndex]);
        }

        cl::Kernel stripL2GKernel(m_program, m_stripL2GKernelName.value().data());
        stripL2GKernel.setArg<cl::Buffer>(0, m_stripClusterOutputBufferList[bufferIndex]);
        stripL2GKernel.setArg<cl::Buffer>(1, m_stripClusterEDMOutputBufferList[bufferIndex]);
        stripL2GKernel.setArg<cl::Buffer>(2, m_stripL2GOutputBufferList[bufferIndex]);
        stripL2GKernel.setArg<cl::Buffer>(3, m_stripL2GEDMOutputBufferList[bufferIndex]);

        // Create EDMPrep kernel object and connect to buffers
        cl::Kernel edmPrepKernel(m_program, m_edmKernelName.value().data());
        cl::Kernel pixelEdmPrepKernel;
        cl::Kernel stripEdmPrepKernel;
        if (m_doF110) {
            pixelEdmPrepKernel = cl::Kernel(m_program, m_pixelEdmKernelName.value().data());
            stripEdmPrepKernel = cl::Kernel(m_program, m_stripEdmKernelName.value().data());
            pixelEdmPrepKernel.setArg<cl::Buffer>(0, m_pixelClusterEDMOutputBufferList[bufferIndex]);
            pixelEdmPrepKernel.setArg<cl::Buffer>(1, m_edmPixelOutputBufferList[bufferIndex]);
            stripEdmPrepKernel.setArg<cl::Buffer>(0, m_stripL2GEDMOutputBufferList[bufferIndex]);
            stripEdmPrepKernel.setArg<cl::Buffer>(1, m_edmStripOutputBufferList[bufferIndex]);
        } else {
            edmPrepKernel.setArg<cl::Buffer>(0, m_pixelL2GEDMOutputBufferList[bufferIndex]);
            edmPrepKernel.setArg<cl::Buffer>(1, m_stripL2GEDMOutputBufferList[bufferIndex]);
            edmPrepKernel.setArg<cl::Buffer>(2, m_edmPixelOutputBufferList[bufferIndex]);
            edmPrepKernel.setArg<cl::Buffer>(3, m_edmStripOutputBufferList[bufferIndex]);
        }



        // Start the transfers
        cl::Event cl_evt_write_pixel_input;
        cl::Event cl_evt_write_strip_input;
        m_acc_queue.enqueueWriteBuffer(m_pixelClusterInputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint64_t) * encodedPixelRDO.size(), encodedPixelRDO.data(), NULL, &cl_evt_write_pixel_input);
        m_acc_queue.enqueueWriteBuffer(m_stripClusterInputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint64_t) * encodedStripRDO.size(), encodedStripRDO.data(), NULL, &cl_evt_write_strip_input);
        std::vector<cl::Event> cl_evt_vec_pixel_input{cl_evt_write_pixel_input};
        std::vector<cl::Event> cl_evt_vec_strip_input{cl_evt_write_strip_input};


        cl::Event cl_evt_pixel_clustering;
        cl::Event cl_evt_strip_clustering;
        cl::Event cl_evt_strip_l2g;
        cl::Event cl_evt_pixel_l2g;
        cl::Event cl_evt_edm_prep;
        cl::Event cl_evt_pixel_edm_prep;
        cl::Event cl_evt_strip_edm_prep;
        {
            Athena::Chrono chrono("Kernel execution", m_chronoSvc.get());
            m_acc_queue.enqueueTask(pixelClusteringKernel, &cl_evt_vec_pixel_input, &cl_evt_pixel_clustering);
            m_acc_queue.enqueueTask(stripClusteringKernel, &cl_evt_vec_strip_input, &cl_evt_strip_clustering);

            std::vector<cl::Event> cl_evt_vec_pixel_clustering{cl_evt_pixel_clustering};
            std::vector<cl::Event> cl_evt_vec_strip_clustering{cl_evt_strip_clustering};
            if (!m_doF110) {
                m_acc_queue.enqueueTask(pixelL2GKernel, &cl_evt_vec_pixel_clustering, &cl_evt_pixel_l2g);
            }
            m_acc_queue.enqueueTask(stripL2GKernel, &cl_evt_vec_strip_clustering, &cl_evt_strip_l2g);
            if (!m_doF110) {
                std::vector<cl::Event> cl_evt_vec_l2g{cl_evt_pixel_l2g, cl_evt_strip_l2g};
                m_acc_queue.enqueueTask(edmPrepKernel, &cl_evt_vec_l2g, &cl_evt_edm_prep);
            } else {
                std::vector<cl::Event> cl_evt_vec_strip_l2g{cl_evt_strip_l2g};
                m_acc_queue.enqueueTask(pixelEdmPrepKernel, &cl_evt_vec_pixel_clustering, &cl_evt_pixel_edm_prep);
                m_acc_queue.enqueueTask(stripEdmPrepKernel, &cl_evt_vec_strip_l2g, &cl_evt_strip_edm_prep);
            }
        }

        cl::Event cl_evt_pixel_cluster_output;
        cl::Event cl_evt_strip_cluster_output;
        m_acc_queue.enqueueReadBuffer(m_edmPixelOutputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint64_t) * pixelChainOutput.size(), pixelChainOutput.data(), NULL, &cl_evt_pixel_cluster_output);
        m_acc_queue.enqueueReadBuffer(m_edmStripOutputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint64_t) * stripChainOutput.size(), stripChainOutput.data(), NULL, &cl_evt_strip_cluster_output);
        m_acc_queue.finish();

        // calculate the time for the kernel execution
        // get the time of writing pixel input buffer
        cl_ulong pixel_input_start = cl_evt_write_pixel_input.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong pixel_input_end = cl_evt_write_pixel_input.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        cl_ulong pixel_input_time = pixel_input_end - pixel_input_start;
        m_pixelInputTime += pixel_input_time;
        ATH_MSG_DEBUG("Pixel input buffer write time: " << pixel_input_time / 1e6 << " ms");

        // get the time of writing strip input buffer
        cl_ulong strip_input_start = cl_evt_write_strip_input.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong strip_input_end = cl_evt_write_strip_input.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        cl_ulong strip_input_time = strip_input_end - strip_input_start;
        m_stripInputTime += strip_input_time;
        ATH_MSG_DEBUG("Strip input buffer write time: " << strip_input_time / 1e6 << " ms");

        // get the time of pixel clustering
        cl_ulong pixel_clustering_start = cl_evt_pixel_clustering.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong pixel_clustering_end = cl_evt_pixel_clustering.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        cl_ulong pixel_clustering_time = pixel_clustering_end - pixel_clustering_start;
        m_pixelClusteringTime += pixel_clustering_time;
        ATH_MSG_DEBUG("Pixel clustering time: " << pixel_clustering_time / 1e6 << " ms");

        // get the time of strip clustering
        cl_ulong strip_clustering_start = cl_evt_strip_clustering.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong strip_clustering_end = cl_evt_strip_clustering.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        cl_ulong strip_clustering_time = strip_clustering_end - strip_clustering_start;
        m_stripClusteringTime += strip_clustering_time;
        ATH_MSG_DEBUG("Strip clustering time: " << strip_clustering_time / 1e6 << " ms");

        if (!m_doF110) {
            // get the time of pixel L2G
            cl_ulong pixel_l2g_start = cl_evt_pixel_l2g.getProfilingInfo<CL_PROFILING_COMMAND_START>();
            cl_ulong pixel_l2g_end = cl_evt_pixel_l2g.getProfilingInfo<CL_PROFILING_COMMAND_END>();
            cl_ulong pixel_l2g_time = pixel_l2g_end - pixel_l2g_start;
            m_pixelL2GTime += pixel_l2g_time;
            ATH_MSG_DEBUG("Pixel L2G time: " << pixel_l2g_time / 1e6 << " ms");
        }

        // get the time of strip L2G
        cl_ulong strip_l2g_start = cl_evt_strip_l2g.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong strip_l2g_end = cl_evt_strip_l2g.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        cl_ulong strip_l2g_time = strip_l2g_end - strip_l2g_start;
        m_stripL2GTime += strip_l2g_time;
        ATH_MSG_DEBUG("Strip L2G time: " << strip_l2g_time / 1e6 << " ms");

        // get the time of EDMPrep
        cl_ulong pixel_edm_prep_end;
        cl_ulong strip_edm_prep_end;
        if (m_doF110) {
            cl_ulong pixel_edm_prep_start = cl_evt_pixel_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_START>();
            pixel_edm_prep_end = cl_evt_pixel_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>();
            cl_ulong pixel_edm_prep_time = pixel_edm_prep_end - pixel_edm_prep_start;
            m_pixelEdmPrepTime += pixel_edm_prep_time;
            ATH_MSG_DEBUG("PixelEDMPrep time: " << pixel_edm_prep_time / 1e6 << " ms");
            cl_ulong strip_edm_prep_start = cl_evt_strip_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_START>();
            strip_edm_prep_end = cl_evt_strip_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>();
            cl_ulong strip_edm_prep_time = strip_edm_prep_end - strip_edm_prep_start;
            m_stripEdmPrepTime += strip_edm_prep_time;
            ATH_MSG_DEBUG("StripEDMPrep time: " << strip_edm_prep_time / 1e6 << " ms");
        } else {
            cl_ulong edm_prep_start = cl_evt_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_START>();
            cl_ulong edm_prep_end = cl_evt_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>();
            cl_ulong edm_prep_time = edm_prep_end - edm_prep_start;
            m_edmPrepTime += edm_prep_time;
            ATH_MSG_DEBUG("EDMPrep time: " << edm_prep_time / 1e6 << " ms");
        }

        // get the time of the whole kernel execution
        cl_ulong kernel_start = cl_evt_pixel_clustering.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
        cl_ulong kernel_end;
        if (m_doF110) {
            kernel_end = pixel_edm_prep_end > strip_edm_prep_end ? pixel_edm_prep_end : strip_edm_prep_end;
        } else {
            kernel_end = cl_evt_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        }
        cl_ulong kernel_time = kernel_end - kernel_start;
        m_kernelTime += kernel_time;
        ATH_MSG_DEBUG("Kernel execution time: " << kernel_time / 1e6 << " ms");

        // get the time of reading pixel output buffer
        cl_ulong pixel_output_start = cl_evt_pixel_cluster_output.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong pixel_output_end = cl_evt_pixel_cluster_output.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        cl_ulong pixel_output_time = pixel_output_end - pixel_output_start;
        m_pixelOutputTime += pixel_output_time;
        ATH_MSG_DEBUG("Pixel output buffer read time: " << pixel_output_time / 1e6 << " ms");

        // get the time of reading strip output buffer
        cl_ulong strip_output_start = cl_evt_strip_cluster_output.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong strip_output_end = cl_evt_strip_cluster_output.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        cl_ulong strip_output_time = strip_output_end - strip_output_start;
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
