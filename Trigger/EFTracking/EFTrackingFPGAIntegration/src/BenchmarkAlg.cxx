/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/BenchmarkAlg.h
 * @author zhaoyuan.cui@cern.ch
 * @date Feb. 25, 2025
 */

#include "BenchmarkAlg.h"
#include "EFTrackingTransient.h"
#include "AthenaKernel/Chrono.h"

namespace EFTrackingFPGAIntegration
{
    StatusCode BenchmarkAlg::initialize()
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

        ATH_CHECK(m_inputPixelClusterKey.initialize());
        ATH_CHECK(m_inputStripClusterKey.initialize());
        ATH_CHECK(m_pixelRDOKey.initialize());
        ATH_CHECK(m_stripRDOKey.initialize());

        ATH_CHECK(m_xaodClusterMaker.retrieve());
        ATH_CHECK(m_testVectorTool.retrieve());
        ATH_CHECK(m_FPGADataFormatTool.retrieve());
        return StatusCode::SUCCESS;
    }

    StatusCode BenchmarkAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing BenchmarkAlg");
        m_numEvents++;

        // Create host side output vectors
        std::vector<uint64_t> pixelOutput(EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE, 0);
        std::vector<uint64_t> stripOutput(EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE, 0);

        if (m_runPassThrough)
        {
            ATH_CHECK(runPassThrough(pixelOutput, stripOutput, ctx));
        }
        else // using the actual FPGA kernel chain
        {
            ATH_CHECK(runDataPrep(pixelOutput, stripOutput, ctx));
        }

        // use 64-bit pointer to access output
        uint64_t *stripClusters = stripOutput.data();
        uint64_t *pixelClusters = pixelOutput.data();

        unsigned int numStripClusters = stripClusters[0];
        ATH_MSG_DEBUG("numStripClusters: " << numStripClusters);

        unsigned int numPixelClusters = pixelClusters[0];
        ATH_MSG_DEBUG("numPixelClusters: " << numPixelClusters);

        std::unique_ptr<EFTrackingTransient::Metadata> metadata =
            std::make_unique<EFTrackingTransient::Metadata>();

        metadata->numOfStripClusters = numStripClusters;
        metadata->scRdoIndexSize = numStripClusters;
        metadata->numOfPixelClusters = numPixelClusters;
        metadata->pcRdoIndexSize = numPixelClusters;

        EFTrackingTransient::StripClusterAuxInput scAux;
        EFTrackingTransient::PixelClusterAuxInput pcAux;

        // Declare a few vairiables to be used in the loop
        int row = 0;
        uint64_t rdo;
        int rdoCounter = 0;

        // Make strip cluster aux input
        {
            Athena::Chrono chrono("Make strip cluster container", m_chronoSvc.get());
            for (unsigned int i = 0; i < numStripClusters; i++)
            {
                rdoCounter = 0;
                row = 0; // idhash
                scAux.idHash.push_back(stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
                row = 1; // id
                scAux.id.push_back(stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
                row = 2; // rdo w1
                rdo = stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8];
                if (rdo)
                {
                    scAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }
                row = 3; // rdo w2
                rdo = stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8];
                if (rdo)
                {
                    scAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }
                row = 4; // rdo w3
                rdo = stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8];
                if (rdo)
                {
                    scAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }
                row = 5; // rdo w4
                rdo = stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8];
                if (rdo)
                {
                    scAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }
                row = 6; // local x
                scAux.localPosition.push_back(*(double *)&stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
                row = 8; // local covariance xx
                scAux.localCovariance.push_back(*(double *)&stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
                row = 9; // global x
                scAux.globalPosition.push_back(*(double *)&stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
                row = 10; // global y
                scAux.globalPosition.push_back(*(double *)&stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
                row = 11; // global z
                scAux.globalPosition.push_back(*(double *)&stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
                row = 12; // channels in phi
                scAux.channelsInPhi.push_back(stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                metadata->scRdoIndex[i] = rdoCounter;
            }
            ATH_CHECK(m_xaodClusterMaker->makeStripClusterContainer(scAux, metadata.get(), ctx));
            // print out the strip cluster aux input
            if (msgLvl(MSG::DEBUG))
            {
                for (unsigned int i = 0; i < numStripClusters; i++)
                {
                    ATH_MSG_DEBUG("Strip cluster " << i << " idHash: " << scAux.idHash[i]);
                    ATH_MSG_DEBUG("Strip cluster " << i << " id: " << scAux.id[i]);
                    ATH_MSG_DEBUG("Strip cluster " << i << " localPosition x: " << scAux.localPosition[i]);
                    ATH_MSG_DEBUG("Strip cluster " << i << " localCovariance: " << scAux.localCovariance[i]);
                    ATH_MSG_DEBUG("Strip cluster " << i << " globalPosition x: " << scAux.globalPosition[i * 3]);
                    ATH_MSG_DEBUG("Strip cluster " << i << " globalPosition y: " << scAux.globalPosition[i * 3 + 1]);
                    ATH_MSG_DEBUG("Strip cluster " << i << " globalPosition z: " << scAux.globalPosition[i * 3 + 2]);
                    ATH_MSG_DEBUG("Strip cluster " << i << " channelsInPhi: " << scAux.channelsInPhi[i]);
                    ATH_MSG_DEBUG("Strip cluster " << i << " rdoList size: " << metadata->scRdoIndex[i]);
                }
            }
        }

        // Make pixel cluster aux input
        {
            Athena::Chrono chrono("Make pixel cluster container", m_chronoSvc.get());
            for (unsigned int i = 0; i < numPixelClusters; i++)
            {
                rdoCounter = 0;
                row = 0; // id hash
                pcAux.idHash.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 1; // id
                pcAux.id.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 2; // rdo w1
                rdo = pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8];
                if (rdo)
                {
                    pcAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }

                row = 3; // rdo w2
                rdo = pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8];
                if (rdo)
                {
                    pcAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }

                row = 4; // rdo w3
                rdo = pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8];
                if (rdo)
                {
                    pcAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }

                row = 5; // rdo w4
                rdo = pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8];
                if (rdo)
                {
                    pcAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }

                row = 6; // local x
                pcAux.localPosition.push_back(*(double *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 7; // local y
                pcAux.localPosition.push_back(*(double *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 8; // local covariance xx
                pcAux.localCovariance.push_back(*(double *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 9; // local covariance yy
                pcAux.localCovariance.push_back(*(double *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 10; // global x
                pcAux.globalPosition.push_back(*(double *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 11; // global y
                pcAux.globalPosition.push_back(*(double *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 12; // global z
                pcAux.globalPosition.push_back(*(double *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 13; // channels in phi
                pcAux.channelsInPhi.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 14; // channels in eta
                pcAux.channelsInEta.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 15; // width in eta
                pcAux.widthInEta.push_back(*(double *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 16; // omega x
                pcAux.omegaX.push_back(*(double *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 17; // omega y
                pcAux.omegaY.push_back(*(double *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                row = 18; // total ToT
                pcAux.totalToT.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

                metadata->pcRdoIndex[i] = rdoCounter;
            }

            ATH_CHECK(m_xaodClusterMaker->makePixelClusterContainer(pcAux, metadata.get(), ctx));

            // print out pixel cluster aux input
            if (msgLvl(MSG::DEBUG))
            {
                for (unsigned int i = 0; i < numPixelClusters; i++)
                {
                    ATH_MSG_DEBUG("Pixel cluster " << i << " idHash: " << pcAux.idHash[i]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " id: " << pcAux.id[i]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " localPosition x: " << pcAux.localPosition[i * 2]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " localPosition y: " << pcAux.localPosition[i * 2 + 1]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " localCovariance xx: " << pcAux.localCovariance[i * 2]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " localCovariance yy: " << pcAux.localCovariance[i * 2 + 1]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " globalPosition x: " << pcAux.globalPosition[i * 3]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " globalPosition y: " << pcAux.globalPosition[i * 3 + 1]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " globalPosition z: " << pcAux.globalPosition[i * 3 + 2]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " channelsInPhi: " << pcAux.channelsInPhi[i]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " channelsInEta: " << pcAux.channelsInEta[i]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " widthInEta: " << pcAux.widthInEta[i]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " omegaX: " << pcAux.omegaX[i]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " omegaY: " << pcAux.omegaY[i]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " totalToT: " << pcAux.totalToT[i]);
                    ATH_MSG_DEBUG("Pixel cluster " << i << " rdoList size: " << metadata->pcRdoIndex[i]);
                }
            }
        }

        return StatusCode::SUCCESS;
    }

    StatusCode BenchmarkAlg::runPassThrough(std::vector<uint64_t> &pixelChainOutput, std::vector<uint64_t> &stripChainOutput, const EventContext &ctx) const
    {
        cl_int err = 0;

        // Load the ITk clusters from SG
        SG::ReadHandle<xAOD::StripClusterContainer> scContainerHandle(m_inputStripClusterKey, ctx);
        if (!scContainerHandle.isValid())
        {
            ATH_MSG_ERROR("Failed to retrieve: " << m_inputStripClusterKey);
            return StatusCode::FAILURE;
        }

        SG::ReadHandle<xAOD::PixelClusterContainer> pcContainerHandle(m_inputPixelClusterKey, ctx);
        if (!pcContainerHandle.isValid())
        {
            ATH_MSG_ERROR("Failed to retrieve: " << m_inputPixelClusterKey);
            return StatusCode::FAILURE;
        }

        // Encode ITK clusters into byte stream
        std::vector<uint64_t> encodedStripClusters;
        std::vector<uint64_t> encodedPixelClusters;
        ATH_CHECK(m_testVectorTool->encodeStripL2G(scContainerHandle.get(), encodedStripClusters));
        ATH_CHECK(m_testVectorTool->encodePixelL2G(pcContainerHandle.get(), encodedPixelClusters));

        // Create local CL buffers and kernel object for pixel and strip
        cl::Buffer inputPixelBuffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        cl::Buffer inputStripBuffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        cl::Buffer outputPixelBuffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        cl::Buffer outputStripBuffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE * sizeof(uint64_t), NULL, &err);

        cl::Kernel kernel(m_program, m_edmKernelName.value().data());
        kernel.setArg<cl::Buffer>(0, inputPixelBuffer);
        kernel.setArg<cl::Buffer>(1, inputStripBuffer);
        kernel.setArg<cl::Buffer>(2, outputPixelBuffer);
        kernel.setArg<cl::Buffer>(3, outputStripBuffer);

        // Migrate the input test vectors to the accelerator
        cl::CommandQueue acc_queue(m_context, m_accelerator);
        acc_queue.enqueueWriteBuffer(inputPixelBuffer, CL_FALSE, 0, sizeof(uint64_t) * encodedPixelClusters.size(), encodedPixelClusters.data(), NULL, NULL);
        acc_queue.enqueueWriteBuffer(inputStripBuffer, CL_FALSE, 0, sizeof(uint64_t) * encodedStripClusters.size(), encodedStripClusters.data(), NULL, NULL);
        acc_queue.finish();

        // enqueue the kernel
        {
            Athena::Chrono chrono("EDMPrep kernel execution", m_chronoSvc.get());
            acc_queue.enqueueTask(kernel);
            acc_queue.finish();
        }

        // Read back the results
        {
            Athena::Chrono chrono("Read buffers", m_chronoSvc.get());
            acc_queue.enqueueReadBuffer(outputPixelBuffer, CL_FALSE, 0, sizeof(uint64_t) * pixelChainOutput.size(), pixelChainOutput.data(), NULL, NULL);
            acc_queue.enqueueReadBuffer(outputStripBuffer, CL_FALSE, 0, sizeof(uint64_t) * stripChainOutput.size(), stripChainOutput.data(), NULL, NULL);
            acc_queue.finish();
        }

        return StatusCode::SUCCESS;
    }

    StatusCode BenchmarkAlg::runDataPrep(std::vector<uint64_t> &pixelChainOutput, std::vector<uint64_t> &stripChainOutput, const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Running DataPrep on FPGA");
        cl_int err = 0;

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

        // Create local CL buffers
        // Clustering
        cl::Buffer pixelClusterInputBuffer(m_context, CL_MEM_READ_ONLY, sizeof(uint64_t) * encodedPixelRDO.size(), NULL, &err);
        cl::Buffer stripClusterInputBuffer(m_context, CL_MEM_READ_ONLY, sizeof(uint64_t) * encodedStripRDO.size(), NULL, &err);
        cl::Buffer pixelClusterOutputBuffer(m_context, CL_MEM_READ_WRITE, sizeof(uint64_t) * encodedPixelRDO.size(), NULL, &err); // Don't care in DataPrep
        cl::Buffer stripClusterOutputBuffer(m_context, CL_MEM_READ_WRITE, sizeof(uint64_t) * encodedStripRDO.size(), NULL, &err); // Don't care in DataPrep
        cl::Buffer pixelClusterEDMOutputBuffer(m_context, CL_MEM_READ_WRITE, sizeof(uint64_t) * encodedPixelRDO.size() * EFTrackingTransient::NUM_PIXEL_WORD, NULL, &err);
        cl::Buffer stripClusterEDMOutputBuffer(m_context, CL_MEM_READ_WRITE, sizeof(uint64_t) * encodedStripRDO.size() * EFTrackingTransient::NUM_STRIP_WORD, NULL, &err);
        // L2G
        cl::Buffer pixelL2GOutputBuffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err); // Don't care in DataPrep
        cl::Buffer stripL2GOutputBuffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err); // Don't care in DataPrep
        cl::Buffer pixelL2GEDMOutputBuffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        cl::Buffer stripL2GEDMOutputBuffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        // EDMPrep
        cl::Buffer edmPixelOutputBuffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        cl::Buffer edmStripOutputBuffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE * sizeof(uint64_t), NULL, &err);

        // Create local CL kernel objects
        // Clustering
        // Kernel names are hard-coded for the current development
        cl::Kernel pixelClusteringKernel(m_program, m_pixelClusterKernelName.value().data());
        pixelClusteringKernel.setArg<cl::Buffer>(0, pixelClusterInputBuffer);
        pixelClusteringKernel.setArg<cl::Buffer>(1, pixelClusterOutputBuffer);
        pixelClusteringKernel.setArg<cl::Buffer>(2, pixelClusterEDMOutputBuffer);

        cl::Kernel stripClusteringKernel(m_program, m_stripClusterKernelName.value().data());
        stripClusteringKernel.setArg<cl::Buffer>(0, stripClusterInputBuffer);
        stripClusteringKernel.setArg<cl::Buffer>(1, stripClusterOutputBuffer);
        stripClusteringKernel.setArg<cl::Buffer>(2, stripClusterEDMOutputBuffer);
        stripClusteringKernel.setArg<unsigned int>(3, encodedStripRDO.size());

        // L2G
        cl::Kernel pixelL2GKernel(m_program, m_pixelL2GKernelName.value().data());
        pixelL2GKernel.setArg<cl::Buffer>(0, pixelClusterOutputBuffer);
        pixelL2GKernel.setArg<cl::Buffer>(1, pixelClusterEDMOutputBuffer);
        pixelL2GKernel.setArg<cl::Buffer>(2, pixelL2GOutputBuffer);
        pixelL2GKernel.setArg<cl::Buffer>(3, pixelL2GEDMOutputBuffer);

        cl::Kernel stripL2GKernel(m_program, m_stripL2GKernelName.value().data());
        stripL2GKernel.setArg<cl::Buffer>(0, stripClusterOutputBuffer);
        stripL2GKernel.setArg<cl::Buffer>(1, stripClusterEDMOutputBuffer);
        stripL2GKernel.setArg<cl::Buffer>(2, stripL2GOutputBuffer);
        stripL2GKernel.setArg<cl::Buffer>(3, stripL2GEDMOutputBuffer);

        // Create EDMPrep kernel object and connect to buffers
        cl::Kernel edmPrepKernel(m_program, m_edmKernelName.value().data());
        edmPrepKernel.setArg<cl::Buffer>(0, pixelL2GEDMOutputBuffer);
        edmPrepKernel.setArg<cl::Buffer>(1, stripL2GEDMOutputBuffer);
        edmPrepKernel.setArg<cl::Buffer>(2, edmPixelOutputBuffer);
        edmPrepKernel.setArg<cl::Buffer>(3, edmStripOutputBuffer);

        cl::CommandQueue acc_queue(m_context, m_accelerator, CL_QUEUE_PROFILING_ENABLE, &err);

        cl::Event cl_evt_write_pixel_input;
        cl::Event cl_evt_write_strip_input;
        acc_queue.enqueueWriteBuffer(pixelClusterInputBuffer, CL_FALSE, 0, sizeof(uint64_t) * encodedPixelRDO.size(), encodedPixelRDO.data(), NULL, &cl_evt_write_pixel_input);
        acc_queue.enqueueWriteBuffer(stripClusterInputBuffer, CL_FALSE, 0, sizeof(uint64_t) * encodedStripRDO.size(), encodedStripRDO.data(), NULL, &cl_evt_write_strip_input);
        std::vector<cl::Event> cl_evt_vec_pixel_input{cl_evt_write_pixel_input};
        std::vector<cl::Event> cl_evt_vec_strip_input{cl_evt_write_strip_input};
        // Ideally, `finish` shouldn't be here because the kernels are invoked by event dependencies,
        // but we use this to temporarily enable kernel profiling.
        acc_queue.finish();

        cl::Event cl_evt_pixel_clustering;
        cl::Event cl_evt_strip_clustering;
        cl::Event cl_evt_pixel_l2g;
        cl::Event cl_evt_strip_l2g;
        cl::Event cl_evt_edm_prep;
        {
            Athena::Chrono chrono("Kernel execution", m_chronoSvc.get());
            acc_queue.enqueueTask(pixelClusteringKernel, &cl_evt_vec_pixel_input, &cl_evt_pixel_clustering);
            acc_queue.enqueueTask(stripClusteringKernel, &cl_evt_vec_strip_input, &cl_evt_strip_clustering);

            std::vector<cl::Event> cl_evt_vec_pixel_clustering{cl_evt_pixel_clustering};
            std::vector<cl::Event> cl_evt_vec_strip_clustering{cl_evt_strip_clustering};
            acc_queue.enqueueTask(pixelL2GKernel, &cl_evt_vec_pixel_clustering, &cl_evt_pixel_l2g);
            acc_queue.enqueueTask(stripL2GKernel, &cl_evt_vec_strip_clustering, &cl_evt_strip_l2g);
            std::vector<cl::Event> cl_evt_vec_l2g{cl_evt_pixel_l2g, cl_evt_strip_l2g};

            acc_queue.enqueueTask(edmPrepKernel, &cl_evt_vec_l2g, &cl_evt_edm_prep);
            // Ideally, `finish` shouldn't be here because the kernels are invoked by event dependencies,
            // but we use this to temporarily enable kernel profiling. CPU wall time
            acc_queue.finish();
        }

        cl::Event cl_evt_pixel_cluster_output;
        cl::Event cl_evt_strip_cluster_output;
        acc_queue.enqueueReadBuffer(edmPixelOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * pixelChainOutput.size(), pixelChainOutput.data(), NULL, &cl_evt_pixel_cluster_output);
        acc_queue.enqueueReadBuffer(edmStripOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * stripChainOutput.size(), stripChainOutput.data(), NULL, &cl_evt_strip_cluster_output);
        acc_queue.finish();

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

        // get the time of pixel L2G
        cl_ulong pixel_l2g_start = cl_evt_pixel_l2g.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong pixel_l2g_end = cl_evt_pixel_l2g.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        cl_ulong pixel_l2g_time = pixel_l2g_end - pixel_l2g_start;
        m_pixelL2GTime += pixel_l2g_time;
        ATH_MSG_DEBUG("Pixel L2G time: " << pixel_l2g_time / 1e6 << " ms");

        // get the time of strip L2G
        cl_ulong strip_l2g_start = cl_evt_strip_l2g.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong strip_l2g_end = cl_evt_strip_l2g.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        cl_ulong strip_l2g_time = strip_l2g_end - strip_l2g_start;
        m_stripL2GTime += strip_l2g_time;
        ATH_MSG_DEBUG("Strip L2G time: " << strip_l2g_time / 1e6 << " ms");

        // get the time of EDMPrep
        cl_ulong edm_prep_start = cl_evt_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong edm_prep_end = cl_evt_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        cl_ulong edm_prep_time = edm_prep_end - edm_prep_start;
        m_edmPrepTime += edm_prep_time;
        ATH_MSG_DEBUG("EDMPrep time: " << edm_prep_time / 1e6 << " ms");

        // get the time of the whole kernel execution
        cl_ulong kernel_start = cl_evt_pixel_clustering.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
        cl_ulong kernel_end = cl_evt_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>();
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

    StatusCode BenchmarkAlg::finalize()
    {
        if (!m_runPassThrough)
        {
            ATH_MSG_INFO("Finalizing BenchmarkAlg");
            ATH_MSG_INFO("Number of events: " << m_numEvents);
            ATH_MSG_INFO("Pixel input time: " << m_pixelInputTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Strip input time: " << m_stripInputTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Pixel clustering time: " << m_pixelClusteringTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Strip clustering time: " << m_stripClusteringTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Pixel L2G time: " << m_pixelL2GTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Strip L2G time: " << m_stripL2GTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("EDMPrep time: " << m_edmPrepTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Kernel execution time: " << m_kernelTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Pixel output time: " << m_pixelOutputTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Strip output time: " << m_stripOutputTime / m_numEvents / 1e6 << " ms");
        }

        return StatusCode::SUCCESS;
    }
} // namespace EFTrackingFPGAIntegration