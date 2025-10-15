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


        // Pixel clustering
        m_pixelClusteringKernel = cl::Kernel(m_program, get_cu_name(m_pixelClusterKernelName, cu).c_str(), &err);

        // Strip clustering
        m_stripClusteringKernel = cl::Kernel(m_program, get_cu_name(m_stripClusterKernelName, cu).c_str(), &err);

        // Strip L2G
        m_stripL2GKernel = cl::Kernel(m_program, get_cu_name(m_stripL2GKernelName, cu).c_str(), &err);

        // EDM prep
        m_pixelEdmPrepKernel = cl::Kernel(m_program, get_cu_name(m_pixelEdmKernelName, cu).c_str(), &err);
        m_stripEdmPrepKernel = cl::Kernel(m_program, get_cu_name(m_stripEdmKernelName, cu).c_str(), &err);

        // Slicing
        m_slicingEngineInput = cl::Kernel(m_program, get_cu_name(m_slicingEngineInputName, cu).c_str(), &err);
        m_slicingEngineOutput = cl::Kernel(m_program, get_cu_name(m_slicingEngineOutputName, cu).c_str(), &err);

        // inside out
        m_insideOutInput = cl::Kernel(m_program, get_cu_name(m_insideOutInputName, cu).c_str(), &err);
        m_insideOutOutput = cl::Kernel(m_program, get_cu_name(m_insideOutOutputName, cu).c_str(), &err);


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

        // Only needed if we are running the full F150
        ATH_CHECK(m_FPGAStripRDO.initialize(m_runFull150));
        ATH_CHECK(m_FPGAPixelRDO.initialize(m_runFull150));
        ATH_CHECK(m_FPGAStripOutput.initialize(m_runFull150));
        ATH_CHECK(m_FPGAPixelOutput.initialize(m_runFull150));


        return StatusCode::SUCCESS;
    }

    StatusCode F150KernelTesterAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F150KernelTesterAlg");


        SG::ReadHandle<FPGATrackSimTrackCollection> outTrackCollection(m_FPGATrackKey, ctx);
        // if not running the full IO, use the simulation to write the output to storegate
        if(!m_runIOOnSE)
        {
            SG::WriteHandle<std::vector<uint64_t>> FPGATrackOutput(m_FPGATrackOutput, ctx);
            auto outputVec = std::make_unique<std::vector<uint64_t>>();

            ATH_CHECK(m_FPGADataFormatTool->convertFPGATracksToFPGADataFormat(outTrackCollection.cptr(), *outputVec, ctx));
            // Now record the filled vector
            ATH_CHECK(FPGATrackOutput.record(std::move(outputVec)));
        }

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

        // initialize buffers
        m_pixelClusterInputBuffer = cl::Buffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::PIXEL_CONTAINER_INPUT_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        m_stripClusterInputBuffer = cl::Buffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::STRIP_CONTAINER_INPUT_BUF_SIZE * sizeof(uint64_t), NULL, &err);

        // Clustering
        m_pixelClusterOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE,EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        m_stripClusterOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        m_pixelClusterEDMOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE,EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        m_stripClusterEDMOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err);

        // L2G
        m_stripL2GOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        m_stripL2GEDMOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err);

        // EDMPrep
        m_edmPixelOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE * sizeof(uint64_t), NULL, &err);
        m_edmStripOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE * sizeof(uint64_t), NULL, &err);

        m_slicingEngineInputBuffer  = cl::Buffer(m_context, CL_MEM_READ_ONLY,  pixelDataIN.size() * sizeof(uint64_t), nullptr, &err);
        m_slicingEngineOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t), nullptr, &err);
        m_insideOutInputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, dataPixelOut.size() * sizeof(uint64_t), nullptr, &err);
        m_insideOutOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t), nullptr, &err);


        if (m_runSE) {
            // Events (write → kSEInput → kSEOutput → read)
            cl::Event evtSEWriteIn;
            cl::Event evtSEKInputDone;
            cl::Event evtSEKOutputDone;
            cl::Event evtSEReadOut;

            m_slicingEngineInput.setArg(0, m_slicingEngineInputBuffer);
            m_slicingEngineInput.setArg(2, static_cast<unsigned long long>(pixelDataIN.size())); 

            m_slicingEngineOutput.setArg(1, m_slicingEngineOutputBuffer);

            ATH_MSG_DEBUG("Transferring SE data");
            m_queue.enqueueWriteBuffer(m_slicingEngineInputBuffer, CL_TRUE, 0, pixelDataIN.size() * sizeof(uint64_t), pixelDataIN.data(), nullptr, &evtSEWriteIn);

            // Execute
            ATH_MSG_DEBUG("Executing SE Kernel");
            std::vector<cl::Event> waitAfterSEWrite{evtSEWriteIn};
            m_queue.enqueueTask(m_slicingEngineInput,  &waitAfterSEWrite, &evtSEKInputDone);
            m_queue.enqueueTask(m_slicingEngineOutput, NULL, &evtSEKOutputDone);

            // Read
            ATH_MSG_DEBUG("Reading output data from kernel");
            std::vector<uint64_t> out_data(EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t), 0);
            std::vector<cl::Event> waitForSERead{evtSEKOutputDone};
            m_queue.enqueueReadBuffer(m_slicingEngineOutputBuffer, /*blocking*/ CL_TRUE, 0, EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t) ,out_data.data(),&waitForSERead, &evtSEReadOut);

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

            ATH_MSG_DEBUG("Setting IO args");
            m_insideOutInput.setArg(0,  m_insideOutInputBuffer);
            m_insideOutOutput.setArg(0, m_insideOutOutputBuffer);

            ATH_MSG_DEBUG("Loading input data to IO input kernel");
            m_queue.enqueueWriteBuffer(m_insideOutInputBuffer, CL_TRUE, 0, dataPixelOut.size() * sizeof(uint64_t), dataPixelOut.data(), nullptr, &evtWriteIn);

            // Execute
            ATH_MSG_DEBUG("Executing IO Kernel");
            std::vector<cl::Event> waitAfterWrite{evtWriteIn};
            m_queue.enqueueTask(m_insideOutInput,  &waitAfterWrite, &evtKInputDone);
            m_queue.enqueueTask(m_insideOutOutput, NULL, &evtKOutputDone);

            // Read
            ATH_MSG_ALWAYS("Reading output data from kernel");
            std::vector<uint64_t> out_data(EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE, 0);
            std::vector<cl::Event> waitForRead{evtKOutputDone};
            m_queue.enqueueReadBuffer( m_insideOutOutputBuffer, CL_TRUE, 0, EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t), out_data.data(), &waitForRead, &evtReadOut);

            // Ensure completion (optional since read is blocking, but explicit is fine)
            cl::Event::waitForEvents({evtReadOut});
            dumpHexData(out_data, "HW_insideOut.txt");
            m_IO_kernelTime += evtKOutputDone.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evtKInputDone.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        }

        if (m_runIOOnSE) 
        {
            cl::Event evtSEWriteIn;
            cl::Event evtSEKInputDone;
            cl::Event evtSEKOutputDone;

            cl::Event evtKInputDone;
            cl::Event evtKOutputDone;
            cl::Event evtReadOut;

            ATH_MSG_DEBUG("Allocating SE buffers");
            const size_t pixel_size_bytesIN = pixelDataIN.size() * sizeof(uint64_t);

            m_slicingEngineInput.setArg(0, m_slicingEngineInputBuffer);
            m_slicingEngineInput.setArg(2, static_cast<unsigned long long>(pixelDataIN.size())); 

            m_slicingEngineOutput.setArg(1, m_slicingEngineOutputBuffer);

            ATH_MSG_DEBUG("Setting IO args");
            m_insideOutInput.setArg(0,  m_slicingEngineOutputBuffer);
            m_insideOutOutput.setArg(0, m_insideOutOutputBuffer);


            ATH_MSG_DEBUG("Transferring SE data");
            m_queue.enqueueWriteBuffer(m_slicingEngineInputBuffer, CL_TRUE, 0, pixel_size_bytesIN, pixelDataIN.data(), nullptr, &evtSEWriteIn);

            // Execute
            ATH_MSG_DEBUG("Executing SE Kernel");
            std::vector<cl::Event> waitAfterSEWrite{evtSEWriteIn};
            m_queue.enqueueTask(m_slicingEngineInput,  &waitAfterSEWrite, &evtSEKInputDone);
            m_queue.enqueueTask(m_slicingEngineOutput, NULL, &evtSEKOutputDone);

            // Execute
            ATH_MSG_DEBUG("Executing IO Kernel");
            std::vector<cl::Event> waitAfterSE{evtSEKOutputDone};
            m_queue.enqueueTask(m_insideOutInput,  &waitAfterSE, &evtKInputDone);
            m_queue.enqueueTask(m_insideOutOutput, NULL, &evtKOutputDone);

            // Read
            ATH_MSG_ALWAYS("Reading output data from kernel");
            std::vector<cl::Event> waitForRead{evtKOutputDone};

            // output handles
            SG::WriteHandle<std::vector<uint64_t>> FPGATrackOutput(m_FPGATrackOutput, ctx);
            ATH_CHECK(FPGATrackOutput.record(std::make_unique<std::vector<uint64_t> >(EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE, 0)));

            m_queue.enqueueReadBuffer( m_insideOutOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*FPGATrackOutput).size(), (*FPGATrackOutput).data(), &waitForRead, &evtReadOut);

            // Ensure completion (optional since read is blocking, but explicit is fine)
            cl::Event::waitForEvents({evtReadOut});

            dumpHexData((*FPGATrackOutput), "HW_insideOut.txt");

            m_SE_kernelTime += evtSEKOutputDone.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evtSEKInputDone.getProfilingInfo<CL_PROFILING_COMMAND_START>();
            m_IO_kernelTime += evtKOutputDone.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evtKInputDone.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        }

        if(m_runFull150)
        {
            // Get the input data
            auto pixelInput = SG::get(m_FPGAPixelRDO, ctx);
            auto stripInput = SG::get(m_FPGAStripRDO, ctx);

            // Set kernel arguments
            m_pixelClusteringKernel.setArg(0, m_pixelClusterInputBuffer);
            m_pixelClusteringKernel.setArg(1, m_pixelClusterEDMOutputBuffer);

            m_stripClusteringKernel.setArg(0, m_stripClusterInputBuffer);
            m_stripClusteringKernel.setArg(1, m_stripClusterOutputBuffer);
            m_stripClusteringKernel.setArg(2, m_stripClusterEDMOutputBuffer);
            m_stripClusteringKernel.setArg(3, static_cast<unsigned int>((*stripInput).size()));

            m_stripL2GKernel.setArg(0, m_stripClusterOutputBuffer);
            m_stripL2GKernel.setArg(1, m_stripClusterEDMOutputBuffer);
            m_stripL2GKernel.setArg(2, m_stripL2GOutputBuffer);
            m_stripL2GKernel.setArg(3, m_stripL2GEDMOutputBuffer);

            m_pixelEdmPrepKernel.setArg(0, m_pixelClusterEDMOutputBuffer);
            m_pixelEdmPrepKernel.setArg(1, m_edmPixelOutputBuffer);
            m_stripEdmPrepKernel.setArg(0, m_stripL2GEDMOutputBuffer);
            m_stripEdmPrepKernel.setArg(1, m_edmStripOutputBuffer);

            m_slicingEngineInput.setArg(0, m_pixelClusterInputBuffer); // TO change
            m_slicingEngineInput.setArg(3, static_cast<unsigned int>((*pixelInput).size()));

            m_slicingEngineOutput.setArg(1, m_slicingEngineOutputBuffer);

            m_insideOutInput.setArg(0,  m_slicingEngineOutputBuffer);
            m_insideOutOutput.setArg(0, m_insideOutOutputBuffer);

            // Start the transfers
            cl::Event evt_write_pixel_input;
            cl::Event evt_write_strip_input;

            m_queue.enqueueWriteBuffer(m_pixelClusterInputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*pixelInput).size(), (*pixelInput).data(), NULL, &evt_write_pixel_input);
            m_queue.enqueueWriteBuffer(m_stripClusterInputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*stripInput).size(), (*stripInput).data(), NULL, &evt_write_strip_input);
            std::vector<cl::Event> evt_vec_pixel_input{evt_write_pixel_input};
            std::vector<cl::Event> evt_vec_strip_input{evt_write_strip_input};

            cl::Event evt_pixel_clustering;
            cl::Event evt_strip_clustering;
            cl::Event evt_strip_l2g;
            cl::Event evt_edm_prep;
            cl::Event evt_pixel_edm_prep;
            cl::Event evt_strip_edm_prep;
            cl::Event evt_Slicing_InputDone;
            cl::Event evt_Slicing_OutputDone;
            cl::Event evt_Insideout_InputDone;
            cl::Event evt_Insideout_OutputDone;
            cl::Event evt_pixel_cluster_output;
            cl::Event evt_strip_cluster_output;
            cl::Event evt_track_output;
            
            m_queue.enqueueTask(m_pixelClusteringKernel, &evt_vec_pixel_input, &evt_pixel_clustering);
            m_queue.enqueueTask(m_stripClusteringKernel, &evt_vec_strip_input, &evt_strip_clustering);

            std::vector<cl::Event> evt_vec_strip_clustering{evt_strip_clustering};
            m_queue.enqueueTask(m_stripL2GKernel, &evt_vec_strip_clustering, &evt_strip_l2g);

            std::vector<cl::Event> evt_vec_pixelEDM {evt_pixel_clustering};
            std::vector<cl::Event> evt_vec_strip_l2g{evt_strip_l2g};
            m_queue.enqueueTask(m_pixelEdmPrepKernel, &evt_vec_pixelEDM, &evt_pixel_edm_prep);
            m_queue.enqueueTask(m_stripEdmPrepKernel, &evt_vec_strip_l2g, &evt_strip_edm_prep);
                        
            // Execute
            std::vector<cl::Event> evt_vec_pixelClustering{evt_pixel_clustering};
            m_queue.enqueueTask(m_slicingEngineInput,  &evt_vec_pixelClustering, &evt_Slicing_InputDone);
            m_queue.enqueueTask(m_slicingEngineOutput, NULL, &evt_Slicing_OutputDone);

            // Execute
            std::vector<cl::Event> evt_vec_slicing{evt_Slicing_OutputDone};
            m_queue.enqueueTask(m_insideOutInput,  &evt_vec_slicing, &evt_Insideout_InputDone);
            m_queue.enqueueTask(m_insideOutOutput, NULL, &evt_Insideout_OutputDone);

            // output handles
            SG::WriteHandle<std::vector<uint64_t>> FPGAPixelOutput(m_FPGAPixelOutput, ctx);
            ATH_CHECK(FPGAPixelOutput.record(std::make_unique<std::vector<uint64_t> >(EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE, 0)));

            SG::WriteHandle<std::vector<uint64_t>> FPGAStripOutput(m_FPGAStripOutput, ctx);
            ATH_CHECK(FPGAStripOutput.record(std::make_unique<std::vector<uint64_t> >(EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE, 0)));

            SG::WriteHandle<std::vector<uint64_t>> FPGATrackOutput(m_FPGATrackOutput, ctx);
            ATH_CHECK(FPGATrackOutput.record(std::make_unique<std::vector<uint64_t> >(EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE, 0)));

            std::vector<cl::Event> evt_vec_pixel_edm_prep{evt_pixel_edm_prep};
            std::vector<cl::Event> evt_vec_strip_edm_prep{evt_strip_edm_prep};
            std::vector<cl::Event> waitForInsideout{evt_Insideout_OutputDone};
            m_queue.enqueueReadBuffer(m_edmPixelOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*FPGAPixelOutput).size(), (*FPGAPixelOutput).data(), &evt_vec_pixel_edm_prep, &evt_pixel_cluster_output);
            m_queue.enqueueReadBuffer(m_edmStripOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*FPGAStripOutput).size(), (*FPGAStripOutput).data(), &evt_vec_strip_edm_prep, &evt_strip_cluster_output);
            m_queue.enqueueReadBuffer( m_insideOutOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*FPGATrackOutput).size(), (*FPGATrackOutput).data(), &waitForInsideout, &evt_track_output);

            std::vector<cl::Event> wait_for_reads = { evt_pixel_cluster_output, evt_strip_cluster_output, evt_track_output };
            cl::Event::waitForEvents(wait_for_reads);


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
