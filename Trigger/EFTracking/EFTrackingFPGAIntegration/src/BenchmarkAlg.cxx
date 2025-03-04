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

        ATH_CHECK(m_chronoSvc.retrieve());

        {
            Athena::Chrono chrono("Platform and device initlize", m_chronoSvc.get());
            ATH_CHECK(IntegrationBase::initialize());
        }

        {
            Athena::Chrono chrono("CL::loadProgram", m_chronoSvc.get());
            ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));
        }

        m_kernel = cl::Kernel(m_program, m_kernelName.value().data());

        {
            Athena::Chrono chrono("CL::BufferSetup", m_chronoSvc.get());
            ATH_CHECK(setupBuffers());
        }
        {
            Athena::Chrono chrono("CL::KernelArgsSetup", m_chronoSvc.get());
            ATH_CHECK(setupKernelArgs());
        }

        ATH_CHECK(m_pixelClusterTVKey.initialize());
        ATH_CHECK(m_stripClusterTVKey.initialize());

        ATH_CHECK(m_xaodContainerMaker.retrieve());
        return StatusCode::SUCCESS;
    }

    StatusCode BenchmarkAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing BenchmarkAlg");

        // Load the TV in the current event from SG
        SG::ReadHandle<std::vector<unsigned long>> pixleTV_stream(m_pixelClusterTVKey, ctx);
        if (!pixleTV_stream.isValid())
        {
            ATH_MSG_ERROR("Failed to retrieve: " << m_pixelClusterTVKey);
            return StatusCode::FAILURE;
        }

        SG::ReadHandle<std::vector<unsigned long>> stripTV_stream(m_stripClusterTVKey, ctx);
        if (!stripTV_stream.isValid())
        {
            ATH_MSG_ERROR("Failed to retrieve: " << m_stripClusterTVKey);
            return StatusCode::FAILURE;
        }

        // Migrate the input test vectors to the accelerator
        cl::CommandQueue acc_queue(m_context, m_accelerator);
        acc_queue.enqueueWriteBuffer(m_inPixelBuff, CL_TRUE, 0, sizeof(uint64_t) * pixleTV_stream->size(), pixleTV_stream->data(), NULL, NULL);
        acc_queue.enqueueWriteBuffer(m_inStripBuff, CL_TRUE, 0, sizeof(uint64_t) * stripTV_stream->size(), stripTV_stream->data(), NULL, NULL);

        // enqueue the kernel
        {
            Athena::Chrono chrono("Kernel execution", m_chronoSvc.get());
            acc_queue.enqueueTask(m_kernel);
            acc_queue.finish();
        }

        // Create host side output vectors
        std::vector<uint64_t> pixelOutput(EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE, 0);
        std::vector<uint64_t> stripOutput(EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE, 0);

        // Read back the results
        {
            Athena::Chrono chrono("Read buffers", m_chronoSvc.get());
            acc_queue.enqueueReadBuffer(m_outPixelBuff, CL_FALSE, 0, sizeof(uint64_t) * pixelOutput.size(), pixelOutput.data(), NULL, NULL);
            acc_queue.enqueueReadBuffer(m_outStripBuff, CL_FALSE, 0, sizeof(uint64_t) * stripOutput.size(), stripOutput.data(), NULL, NULL);
            acc_queue.finish();
        }

        // use 32-bit point to access output
        uint32_t *stripClusters = (uint32_t *)stripOutput.data();
        uint32_t *pixelClusters = (uint32_t *)pixelOutput.data();

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
                scAux.idHash.push_back(stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);
                row = 1; // id
                scAux.id.push_back(stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);
                row = 2; // rdo w1
                rdo = stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1];
                if (rdo)
                {
                    scAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }
                row = 3; // rdo w2
                rdo = stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1];
                if (rdo)
                {
                    scAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }
                row = 4; // rdo w3
                rdo = stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1];
                if (rdo)
                {
                    scAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }
                row = 5; // rdo w4
                rdo = stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1];
                if (rdo)
                {
                    scAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }
                row = 6; // local x
                scAux.localPosition.push_back(*(float *)&stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);
                row = 8; // local covariance xx
                scAux.localCovariance.push_back(*(float *)&stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);
                row = 9; // global x
                scAux.globalPosition.push_back(*(float *)&stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);
                row = 10; // global y
                scAux.globalPosition.push_back(*(float *)&stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);
                row = 11; // global z
                scAux.globalPosition.push_back(*(float *)&stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);
                row = 12; // channels in phi
                scAux.channelsInPhi.push_back(stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                metadata->scRdoIndex[i] = rdoCounter;
            }
            ATH_CHECK(m_xaodContainerMaker->makeStripClusterContainer(scAux, metadata.get(), ctx));
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
                pcAux.idHash.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 1; // id
                pcAux.id.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 2; // rdo w1
                rdo = pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1];
                if (rdo)
                {
                    pcAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }

                row = 3; // rdo w2
                rdo = pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1];
                if (rdo)
                {
                    pcAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }

                row = 4; // rdo w3
                rdo = pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1];
                if (rdo)
                {
                    pcAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }

                row = 5; // rdo w4
                rdo = pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1];
                if (rdo)
                {
                    pcAux.rdoList.push_back(rdo);
                    rdoCounter++;
                }

                row = 6; // local x
                pcAux.localPosition.push_back(*(float *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 7; // local y
                pcAux.localPosition.push_back(*(float *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 8; // local covariance xx
                pcAux.localCovariance.push_back(*(float *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 9; // local covariance yy
                pcAux.localCovariance.push_back(*(float *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 10; // global x
                pcAux.globalPosition.push_back(*(float *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 11; // global y
                pcAux.globalPosition.push_back(*(float *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 12; // global z
                pcAux.globalPosition.push_back(*(float *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 13; // channels in phi
                pcAux.channelsInPhi.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 14; // channels in eta
                pcAux.channelsInEta.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 15; // width in eta
                pcAux.widthInEta.push_back(*(float *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 16; // omega x
                pcAux.omegaX.push_back(*(float *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 17; // omega y
                pcAux.omegaY.push_back(*(float *)&pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                row = 18; // total ToT
                pcAux.totalToT.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 1]);

                metadata->pcRdoIndex[i] = rdoCounter;
            }

            ATH_CHECK(m_xaodContainerMaker->makePixelClusterContainer(pcAux, metadata.get(), ctx));

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

    StatusCode BenchmarkAlg::setupBuffers()
    {
        cl_int err = 0;

        // Assuming maximum 50,000 clusters, should be further discussed!
        // The EDM from L2G has 7 64-bit words per cluster

        m_inPixelBuff = cl::Buffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        if (err != CL_SUCCESS)
        {
            ATH_MSG_ERROR("Failed to allocate input buffer for pixel clustering. Error code: " << err);
            return StatusCode::FAILURE;
        }

        m_inStripBuff = cl::Buffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        if (err != CL_SUCCESS)
        {
            ATH_MSG_ERROR("Failed to allocate input buffer for strip clustering. Error code: " << err);
            return StatusCode::FAILURE;
        }

        m_outPixelBuff = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        if (err != CL_SUCCESS)
        {
            ATH_MSG_ERROR("Failed to allocate output buffer for pixel clustering. Error code: " << err);
            return StatusCode::FAILURE;
        }

        m_outStripBuff = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        if (err != CL_SUCCESS)
        {
            ATH_MSG_ERROR("Failed to allocate output buffer for strip clustering. Error code: " << err);
            return StatusCode::FAILURE;
        }

        return StatusCode::SUCCESS;
    }

    StatusCode BenchmarkAlg::setupKernelArgs()
    {
        cl_int err = 0;

        err = m_kernel.setArg<cl::Buffer>(0, m_inPixelBuff);
        if (err != CL_SUCCESS)
        {
            ATH_MSG_ERROR("Failed to set kernel argument 0: input pixel cluter. Error code: " << err);
            return StatusCode::FAILURE;
        }

        err = m_kernel.setArg<cl::Buffer>(1, m_inStripBuff);
        if (err != CL_SUCCESS)
        {
            ATH_MSG_ERROR("Failed to set kernel argument 1: input strip cluster. Error code: " << err);
            return StatusCode::FAILURE;
        }

        err = m_kernel.setArg<cl::Buffer>(2, m_outPixelBuff);
        if (err != CL_SUCCESS)
        {
            ATH_MSG_ERROR("Failed to set kernel argument 2: output pixel edm. Error code: " << err);
            return StatusCode::FAILURE;
        }

        err = m_kernel.setArg<cl::Buffer>(3, m_outStripBuff);
        if (err != CL_SUCCESS)
        {
            ATH_MSG_ERROR("Failed to set kernel argument 3: output strip edm. Error code: " << err);
            return StatusCode::FAILURE;
        }

        return StatusCode::SUCCESS;
    }
}
