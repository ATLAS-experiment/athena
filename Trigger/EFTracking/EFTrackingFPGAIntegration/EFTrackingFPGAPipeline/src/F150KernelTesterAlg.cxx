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

    void F150KernelTesterAlg::dumpHexData(std::span<const uint64_t> data, const std::string& dataDescriptor, const EventContext &ctx) const {

        if(!m_outputTextFile) return;
        auto withEvt = [&](const std::string& fname) {
            const auto evt = ctx.eventID().event_number(); // get current event number
            const auto dot = fname.rfind('.');
            if (dot == std::string::npos) {
                return fname + "_" + std::to_string(evt);
            }
            return fname.substr(0, dot) + "_" + std::to_string(evt) + fname.substr(dot);
        };


        ATH_MSG_DEBUG("STARTING " << dataDescriptor << " words:");
        std::ofstream outputFile(withEvt(dataDescriptor));

        for (uint64_t d : data) {
            outputFile << std::hex << std::setw(16) << std::setfill('0') << d << '\n';
        }

        // Write different data types
        outputFile.close();
    }

    void F150KernelTesterAlg::dumpHexData(std::span<const uint32_t> data, const std::string& dataDescriptor, const EventContext &ctx) const {

        if(!m_outputTextFile) return;
        auto withEvt = [&](const std::string& fname) {
            const auto evt = ctx.eventID().event_number(); // get current event number
            const auto dot = fname.rfind('.');
            if (dot == std::string::npos) {
                return fname + "_" + std::to_string(evt);
            }
            return fname.substr(0, dot) + "_" + std::to_string(evt) + fname.substr(dot);
        };


        ATH_MSG_DEBUG("STARTING " << dataDescriptor << " words:");
        std::ofstream outputFile(withEvt(dataDescriptor));

        for (uint64_t d : data) {
            outputFile << std::hex << std::setw(8) << std::setfill('0') << d << '\n';
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
        ATH_CHECK(m_FPGAStripRDO.initialize());
        ATH_CHECK(m_FPGAPixelRDO.initialize());
        ATH_CHECK(m_FPGAStripOutput.initialize(m_runFull150));
        ATH_CHECK(m_FPGAPixelOutput.initialize(m_runFull150));
        ATH_CHECK(m_FPGAPixelRDOSize.initialize(m_runFull150));
        ATH_CHECK(m_FPGAStripRDOSize.initialize(m_runFull150));

        return StatusCode::SUCCESS;
    }

    StatusCode F150KernelTesterAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F150KernelTesterAlg");


        SG::ReadHandle<FPGATrackSimTrackCollection> outTrackCollection(m_FPGATrackKey, ctx);
        // if not running the full IO, use the simulation to write the output to storegate
        if(!m_runIOOnSE && !m_runIO && !m_runFull150)
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

        int padLength = 8;
        int inputIOLength = pixelDataIN.size();
        auto remainder = inputIOLength % padLength;
        if (remainder != 0) {
            size_t to_add = padLength - remainder;
            pixelDataIN.insert(pixelDataIN.end(), to_add, 0); // append zeros
        }

        dumpHexData(pixelDataIN, "FPGATrackSim_slicingIn_pixel.txt", ctx);
        dumpHexData(stripDataIN, "FPGATrackSim_slicingIn_strip.txt", ctx);

        ATH_MSG_DEBUG("Accessing SE Out data.");
        std::vector<uint64_t> dataPixelOut;
        std::vector<uint64_t> dataStripOut;
        SG::ReadHandle<FPGATrackSimHitCollection> outhitCollectionHandle(m_FPGASlicedHitKey, ctx);
        ATH_CHECK(m_FPGADataFormatTool->convertFPGASliceToFPGADataFormat(outhitCollectionHandle.cptr(), true, false, dataPixelOut, ctx));
        ATH_CHECK(m_FPGADataFormatTool->convertFPGASliceToFPGADataFormat(outhitCollectionHandle.cptr(), false, true, dataStripOut, ctx));
        dumpHexData(dataPixelOut, "FPGATrackSim_slicingOut_pixel.txt", ctx);
        dumpHexData(dataStripOut, "FPGATrackSim_slicingOut_strip.txt", ctx);

        ATH_MSG_DEBUG("Accessing SE Out data.");
        std::vector<uint64_t> dataInsideOut;
        ATH_CHECK(m_FPGADataFormatTool->convertFPGATracksToFPGADataFormat(outTrackCollection.cptr(), dataInsideOut, ctx));
        dumpHexData(dataInsideOut, "FPGATrackSim_insideOut.txt", ctx);


        cl_int err = CL_SUCCESS;

        // increment the event if there is data in this event
        if(pixelDataIN.size() > 6) m_numEvents++;

        // initialize buffers
        m_pixelClusterInputBuffer = cl::Buffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::PIXEL_CONTAINER_INPUT_BUF_SIZE * sizeof(uint64_t), nullptr, &err);
        m_stripClusterInputBuffer = cl::Buffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::STRIP_CONTAINER_INPUT_BUF_SIZE * sizeof(uint64_t), nullptr, &err);

        // Clustering
        m_pixelClusterOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err);
        m_stripClusterOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err);
        m_pixelClusterEDMOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE,EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err);
        m_stripClusterEDMOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err);

        // L2G
        m_stripL2GInputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err);
        m_stripL2GEDMInputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err);

        m_stripL2GOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err);
        m_stripL2GEDMOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err);

        // EDMPrep
        m_edmPixelInputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err);
        m_edmStripInputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err);
        m_edmPixelOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE * sizeof(uint32_t), nullptr, &err);
        m_edmStripOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE * sizeof(uint32_t), nullptr, &err);


        if(m_runFull150) m_slicingEngineInputBuffer  = cl::Buffer(m_context, CL_MEM_READ_WRITE,  EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err);
        else m_slicingEngineInputBuffer  = cl::Buffer(m_context, CL_MEM_READ_WRITE,  pixelDataIN.size() * sizeof(uint64_t), nullptr, &err);
        m_slicingEngineOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t), nullptr, &err);

        if(m_runIOOnSE || m_runFull150) m_insideOutInputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t), nullptr, &err);
        else                            m_insideOutInputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, dataPixelOut.size() * sizeof(uint64_t), nullptr, &err);
        m_insideOutOutputBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t), nullptr, &err);


        if (m_runSE) {
            // Events (write → kSEInput → kSEOutput → read)
            cl::Event evtSEWriteIn;
            cl::Event evtSEKInputDone;
            cl::Event evtSEKOutputDone;
            cl::Event evtSEReadOut;

            m_slicingEngineInput.setArg(0, m_slicingEngineInputBuffer);
            m_slicingEngineInput.setArg(2, static_cast<unsigned long long>(inputIOLength)); 
            ATH_MSG_DEBUG("Setting NWords:" << static_cast<unsigned long long>(inputIOLength)<<" with size: "<<pixelDataIN.size());

            m_slicingEngineOutput.setArg(1, m_slicingEngineOutputBuffer);

            ATH_MSG_DEBUG("Transferring SE data");
            m_queue.enqueueWriteBuffer(m_slicingEngineInputBuffer, CL_FALSE, 0, pixelDataIN.size() * sizeof(uint64_t), pixelDataIN.data(), nullptr, &evtSEWriteIn);
            m_queue.finish();

            // Execute
            ATH_MSG_DEBUG("Executing SE Kernel");
            std::vector<cl::Event> waitAfterSEWrite{evtSEWriteIn};
            m_queue.enqueueTask(m_slicingEngineInput,  &waitAfterSEWrite, &evtSEKInputDone);
            m_queue.finish();
            ATH_MSG_DEBUG("Executing SE output Kernel");
            m_queue.enqueueTask(m_slicingEngineOutput, nullptr, &evtSEKOutputDone);
            m_queue.finish();

            // Read
            ATH_MSG_DEBUG("Reading output data from kernel");
            std::vector<uint64_t> out_data(EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE, 0);
            std::vector<cl::Event> waitForSERead{evtSEKOutputDone};
            m_queue.enqueueReadBuffer(m_slicingEngineOutputBuffer, /*blocking*/ CL_FALSE, 0, EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t) ,out_data.data(),&waitForSERead, &evtSEReadOut);

            // Optional explicit sync (blocking read already waits)
            cl::Event::waitForEvents({evtSEReadOut});

            dumpHexData(out_data, "HW_slicingOut_pixel.txt", ctx);

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
            m_queue.finish();
            // Execute
            ATH_MSG_DEBUG("Executing IO Kernel");
            std::vector<cl::Event> waitAfterWrite{evtWriteIn};
            m_queue.enqueueTask(m_insideOutInput,  &waitAfterWrite, &evtKInputDone);
            m_queue.enqueueTask(m_insideOutOutput, nullptr, &evtKOutputDone);
            m_queue.finish();

            // Read
            ATH_MSG_DEBUG("Reading output data from kernel");

            // output handles
            SG::WriteHandle<std::vector<uint64_t>> FPGATrackOutput(m_FPGATrackOutput, ctx);
            ATH_CHECK(FPGATrackOutput.record(std::make_unique<std::vector<uint64_t> >(EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE, 0)));

            std::vector<cl::Event> waitForRead{evtKOutputDone};
            m_queue.enqueueReadBuffer( m_insideOutOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*FPGATrackOutput).size(), (*FPGATrackOutput).data(), &waitForRead, &evtReadOut);

            // Ensure completion (optional since read is blocking, but explicit is fine)
            cl::Event::waitForEvents({evtReadOut});
            dumpHexData((*FPGATrackOutput), "HW_insideOut.txt", ctx);

            m_IO_kernelTime += evtKOutputDone.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evtKInputDone.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        }

        if (m_runIOOnSE) 
        {
            cl::Event evtSEWriteIn;
            cl::Event evtSEKInputDone;
            cl::Event evtSEKOutputDone;

            cl::Event evtBufferTransfer;


            cl::Event evtKInputDone;
            cl::Event evtKOutputDone;
            cl::Event evtReadOut;

            ATH_MSG_DEBUG("Allocating SE buffers");
            const size_t pixel_size_bytesIN = pixelDataIN.size() * sizeof(uint64_t);

            m_slicingEngineInput.setArg(0, m_slicingEngineInputBuffer);
            m_slicingEngineInput.setArg(2, static_cast<unsigned long long>(pixelDataIN.size())); 

            m_slicingEngineOutput.setArg(1, m_slicingEngineOutputBuffer);

            ATH_MSG_DEBUG("Setting IO args");
            m_insideOutInput.setArg(0,  m_insideOutInputBuffer);
            m_insideOutOutput.setArg(0, m_insideOutOutputBuffer);
            m_queue.finish();

            ATH_MSG_DEBUG("Transferring SE data");
            m_queue.enqueueWriteBuffer(m_slicingEngineInputBuffer, CL_FALSE, 0, pixel_size_bytesIN, pixelDataIN.data(), nullptr, &evtSEWriteIn);
            m_queue.finish();
            // Execute
            ATH_MSG_DEBUG("Executing SE Kernel");
            std::vector<cl::Event> waitAfterSEWrite{evtSEWriteIn};
            m_queue.enqueueTask(m_slicingEngineInput,  &waitAfterSEWrite, &evtSEKInputDone);
            m_queue.enqueueTask(m_slicingEngineOutput, nullptr, &evtSEKOutputDone);
            m_queue.finish();
            // Execute
            ATH_MSG_DEBUG("Executing IO Kernel");
            std::vector<cl::Event> waitAfterSE{evtSEKOutputDone};

            m_queue.enqueueCopyBuffer(m_slicingEngineOutputBuffer, m_insideOutInputBuffer, 0, 0, EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t), &waitAfterSE, &evtBufferTransfer);

            std::vector<cl::Event> waitAfterTransfer{evtBufferTransfer};
            m_queue.enqueueTask(m_insideOutInput,  &waitAfterTransfer, &evtKInputDone);
            m_queue.enqueueTask(m_insideOutOutput, NULL, &evtKOutputDone);
            m_queue.finish();
            // Read
            ATH_MSG_DEBUG("Reading output data from kernel");
            std::vector<cl::Event> waitForRead{evtKOutputDone};

            // output handles
            SG::WriteHandle<std::vector<uint64_t>> FPGATrackOutput(m_FPGATrackOutput, ctx);
            ATH_CHECK(FPGATrackOutput.record(std::make_unique<std::vector<uint64_t> >(EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE, 0)));

            m_queue.enqueueReadBuffer( m_insideOutOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*FPGATrackOutput).size(), (*FPGATrackOutput).data(), &waitForRead, &evtReadOut);

            // Ensure completion (optional since read is blocking, but explicit is fine)
            cl::Event::waitForEvents({evtReadOut});
            dumpHexData((*FPGATrackOutput), "HW_insideOut.txt", ctx);

            m_SE_kernelTime += evtSEKOutputDone.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evtSEKInputDone.getProfilingInfo<CL_PROFILING_COMMAND_START>();
            m_IO_kernelTime += evtKOutputDone.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evtKInputDone.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        }

        if(m_runFull150)
        {
            // === INPUT FETCH ===
            auto pixelInput = SG::get(m_FPGAPixelRDO, ctx);
            auto stripInput = SG::get(m_FPGAStripRDO, ctx);

            const int* pixelInputSize{nullptr}, *stripInputSize{nullptr};
            ATH_CHECK(SG::get(pixelInputSize, m_FPGAPixelRDOSize, ctx));
            ATH_CHECK(SG::get(stripInputSize, m_FPGAStripRDOSize, ctx));

            // === KERNEL ARG SETUP ===
            m_pixelClusteringKernel.setArg(0, m_pixelClusterInputBuffer);
            m_pixelClusteringKernel.setArg(1, m_pixelClusterOutputBuffer);
            m_pixelClusteringKernel.setArg(2, m_pixelClusterEDMOutputBuffer);

            m_stripClusteringKernel.setArg(0, m_stripClusterInputBuffer);
            m_stripClusteringKernel.setArg(1, m_stripClusterOutputBuffer);
            m_stripClusteringKernel.setArg(2, m_stripClusterEDMOutputBuffer);
            m_stripClusteringKernel.setArg(3, static_cast<unsigned int>(*stripInputSize));

            m_stripL2GKernel.setArg(0, m_stripL2GInputBuffer);
            m_stripL2GKernel.setArg(1, m_stripL2GEDMInputBuffer);
            m_stripL2GKernel.setArg(2, m_stripL2GOutputBuffer);
            m_stripL2GKernel.setArg(3, m_stripL2GEDMOutputBuffer);

            m_pixelEdmPrepKernel.setArg(0, m_edmPixelInputBuffer);
            m_pixelEdmPrepKernel.setArg(1, m_edmPixelOutputBuffer);
            m_stripEdmPrepKernel.setArg(0, m_edmStripInputBuffer);
            m_stripEdmPrepKernel.setArg(1, m_edmStripOutputBuffer);

            // === HOST->DEVICE INPUT WRITES ===
            cl::Event evt_pixel_input_write, evt_strip_input_write;

            m_queue.enqueueWriteBuffer(m_pixelClusterInputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*pixelInput).size(), (*pixelInput).data(), nullptr, &evt_pixel_input_write);
            m_queue.enqueueWriteBuffer(m_stripClusterInputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*stripInput).size(), (*stripInput).data(), nullptr, &evt_strip_input_write);

            std::vector<cl::Event> evts_pixel_input_write{evt_pixel_input_write};
            std::vector<cl::Event> evts_strip_input_write{evt_strip_input_write};

            // === CLUSTERING KERNELS ===
            cl::Event evt_pixel_clustering_done, evt_strip_clustering_done;

            m_queue.enqueueTask(m_pixelClusteringKernel, &evts_pixel_input_write, &evt_pixel_clustering_done);
            m_queue.enqueueTask(m_stripClusteringKernel, &evts_strip_input_write, &evt_strip_clustering_done);

            // === PCOPY STRIP CLUSTERS -> L2G INPUTS ===
            cl::Event evt_strip_l2g_input_copy_clusters, evt_strip_l2g_input_copy_edm;
            std::vector<cl::Event> evts_strip_clustering_done{evt_strip_clustering_done};

            m_queue.enqueueCopyBuffer(m_stripClusterOutputBuffer, m_stripL2GInputBuffer, 0, 0, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), &evts_strip_clustering_done, &evt_strip_l2g_input_copy_clusters);
            m_queue.enqueueCopyBuffer(m_stripClusterEDMOutputBuffer, m_stripL2GEDMInputBuffer, 0, 0, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), &evts_strip_clustering_done, &evt_strip_l2g_input_copy_edm);

            std::vector<cl::Event> evts_strip_l2g_input_copies{evt_strip_l2g_input_copy_clusters, evt_strip_l2g_input_copy_edm};

            // === STRIP L2G KERNEL ===
            cl::Event evt_strip_l2g_done;
            m_queue.enqueueTask(m_stripL2GKernel, &evts_strip_l2g_input_copies, &evt_strip_l2g_done);

            // === PHASE: COPY EDM INPUTS (PIXEL FROM CLUSTERING, STRIP FROM L2G) ===
            cl::Event evt_pixel_edm_input_copy, evt_strip_edm_input_copy;
            std::vector<cl::Event> evts_pixel_clustering_done{evt_pixel_clustering_done};
            std::vector<cl::Event> evts_strip_l2g_done{evt_strip_l2g_done};

            m_queue.enqueueCopyBuffer(m_pixelClusterEDMOutputBuffer, m_edmPixelInputBuffer, 0, 0, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), &evts_pixel_clustering_done, &evt_pixel_edm_input_copy);
            m_queue.enqueueCopyBuffer(m_stripL2GEDMOutputBuffer, m_edmStripInputBuffer, 0, 0, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), &evts_strip_l2g_done, &evt_strip_edm_input_copy);

            // === PHASE: EDM PREP KERNELS ===
            cl::Event evt_pixel_edm_prep_done, evt_strip_edm_prep_done;
            std::vector<cl::Event> evts_pixel_edm_input_copied{evt_pixel_edm_input_copy};
            std::vector<cl::Event> evts_strip_edm_input_copied{evt_strip_edm_input_copy};

            m_queue.enqueueTask(m_pixelEdmPrepKernel, &evts_pixel_edm_input_copied, &evt_pixel_edm_prep_done);
            m_queue.enqueueTask(m_stripEdmPrepKernel, &evts_strip_edm_input_copied, &evt_strip_edm_prep_done);

            // === PHASE: EDM READBACKS ===
            SG::WriteHandle<std::vector<uint32_t>> FPGAPixelOutput(m_FPGAPixelOutput, ctx);
            ATH_CHECK(FPGAPixelOutput.record(std::make_unique<std::vector<uint32_t>>(EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE, 0)));

            SG::WriteHandle<std::vector<uint32_t>> FPGAStripOutput(m_FPGAStripOutput, ctx);
            ATH_CHECK(FPGAStripOutput.record(std::make_unique<std::vector<uint32_t>>(EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE, 0)));

            cl::Event evt_pixel_edm_read_done, evt_strip_edm_read_done;
            std::vector<cl::Event> evts_pixel_edm_prep_done{evt_pixel_edm_prep_done};
            std::vector<cl::Event> evts_strip_edm_prep_done{evt_strip_edm_prep_done};

            m_queue.enqueueReadBuffer(m_edmPixelOutputBuffer, CL_FALSE, 0, sizeof(uint32_t) * (*FPGAPixelOutput).size(), (*FPGAPixelOutput).data(), &evts_pixel_edm_prep_done, &evt_pixel_edm_read_done);
            m_queue.enqueueReadBuffer(m_edmStripOutputBuffer, CL_FALSE, 0, sizeof(uint32_t) * (*FPGAStripOutput).size(), (*FPGAStripOutput).data(), &evts_strip_edm_prep_done, &evt_strip_edm_read_done);

            cl::Event::waitForEvents(std::vector<cl::Event>{evt_pixel_edm_read_done, evt_strip_edm_read_done});

            // === PHASE: POST-EDM GUARDS ===
            if (pixelInput->size() == 6) (*FPGAPixelOutput)[0] = 0;
            if (stripInput->size() == 6) (*FPGAStripOutput)[0] = 0;

            m_queue.finish();
            ATH_MSG_DEBUG("Done F100");

            // === PHASE: PIXEL CLUSTER RAW READBACK FOR SE ===
            cl::Event evt_pixel_cluster_output_read;
            std::vector<uint64_t> pixelClusterOut(EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE, 0);

            m_queue.enqueueReadBuffer(m_pixelClusterOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * pixelClusterOut.size(), pixelClusterOut.data(), &evts_pixel_clustering_done, &evt_pixel_cluster_output_read);
            m_queue.finish();

            // === PHASE: FOOTER SCAN & SANITIZE ===
            int nWords = static_cast<int>(pixelClusterOut.size()) - 1;
            for (; nWords >= 0; nWords--) 
            { 
                if (pixelClusterOut[nWords] == 0xcd00000000000000) break; 
            }
            // If footer not found, bail out safely
            if (nWords < 0) 
            { 
                ATH_MSG_ERROR("Footer 0xcd00000000000000 not found in pixelClusterOut; cannot determine nWords"); return StatusCode::FAILURE; 
            }

            // We have 3 footer words, account for that
            if (nWords > 0) nWords += 3;

            //clean the next 8 words to account for the input buffer read
            for (int i = 0; i < 8 && (nWords + i) < static_cast<int>(pixelClusterOut.size()); i++) 
            { 
                pixelClusterOut[nWords + i] = 0;
            }
            ATH_MSG_DEBUG("Got NWords:" << nWords);

            // === KERNEL ARG SETUP ===
            m_slicingEngineInput.setArg(0, m_slicingEngineInputBuffer);
            m_slicingEngineInput.setArg(2, static_cast<unsigned long long>(nWords));
            m_slicingEngineOutput.setArg(1, m_slicingEngineOutputBuffer);

            m_insideOutInput.setArg(0, m_insideOutInputBuffer);
            m_insideOutOutput.setArg(0, m_insideOutOutputBuffer);

            // === PHASE: WRITE BUFFER FOR SE ===
            cl::Event evt_se_input_write;

            m_queue.enqueueWriteBuffer(m_slicingEngineInputBuffer, CL_FALSE, 0, pixelClusterOut.size() * sizeof(uint64_t), pixelClusterOut.data(), nullptr, &evt_se_input_write);
            m_queue.finish();


            // === PHASE: SE RUNNING ===
            cl::Event evt_se_kernel_input_done, evt_se_kernel_output_done;
            std::vector<cl::Event> evts_after_se_input_write{evt_se_input_write};

            m_queue.enqueueTask(m_slicingEngineInput, &evts_after_se_input_write, &evt_se_kernel_input_done);
            m_queue.enqueueTask(m_slicingEngineOutput, nullptr, &evt_se_kernel_output_done);
            m_queue.finish();

            // === PHASE: SE->IO COPY ===
            cl::Event evt_io_input_transfer;
            std::vector<cl::Event> evts_after_se_output_done{evt_se_kernel_output_done};

            m_queue.enqueueCopyBuffer(m_slicingEngineOutputBuffer, m_insideOutInputBuffer, 0, 0, EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t), &evts_after_se_output_done, &evt_io_input_transfer);
            m_queue.finish();

            // === PHASE: IO KERNELS ===
            cl::Event evt_io_kernel_input_done, evt_io_kernel_output_done;
            std::vector<cl::Event> evts_after_io_input_transfer{evt_io_input_transfer};

            m_queue.enqueueTask(m_insideOutInput, &evts_after_io_input_transfer, &evt_io_kernel_input_done);
            m_queue.enqueueTask(m_insideOutOutput, nullptr, &evt_io_kernel_output_done);
            m_queue.finish();

            // === PHASE: IO READBACK ===
            SG::WriteHandle<std::vector<uint64_t>> FPGATrackOutput(m_FPGATrackOutput, ctx);
            ATH_CHECK(FPGATrackOutput.record(std::make_unique<std::vector<uint64_t>>(EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE, 0)));

            cl::Event evt_io_output_read;
            std::vector<cl::Event> evts_before_insideout_read{evt_io_kernel_output_done};
            m_queue.enqueueReadBuffer(m_insideOutOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*FPGATrackOutput).size(), (*FPGATrackOutput).data(), &evts_before_insideout_read, &evt_io_output_read);

            cl::Event::waitForEvents(std::vector<cl::Event>{evt_io_output_read});
            dumpHexData((*FPGATrackOutput), "HW_insideOut.txt", ctx);

            m_queue.finish();

            // === PHASE: PROFILING ACCUMULATION ===
            m_SE_kernelTime += evt_se_kernel_output_done.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_se_kernel_input_done.getProfilingInfo<CL_PROFILING_COMMAND_START>();
            m_IO_kernelTime += evt_io_kernel_output_done.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_io_kernel_input_done.getProfilingInfo<CL_PROFILING_COMMAND_START>();


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

