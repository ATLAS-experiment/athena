/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
   */

#include "EFTrackingFPGAPipeline/F110IntegrationAlg.h"
#include "AthenaKernel/Chrono.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_uuid.h>

namespace EFTrackingFPGAIntegration
{
    StatusCode F110IntegrationAlg::initialize()
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

        ATH_CHECK(m_FPGAPixelRDOSize.initialize());
        ATH_CHECK(m_FPGAStripRDOSize.initialize());

        ATH_CHECK(m_FPGAStripOutput.initialize());
        ATH_CHECK(m_FPGAPixelOutput.initialize());

        cl_int err = 0;

        // Get the list of CUs
        std::vector<std::string> listofCUs;
        getListofCUs(listofCUs);

        // Create kernels for each one of CUs that is inside device
        for (const auto& cuName: listofCUs)
        {
            // Pixel clustering
            if(cuName.find(m_pixelClusterKernelName.value()) != std::string::npos) m_pixelClusterKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));

            // Strip clustering
            else if(cuName.find(m_stripClusterKernelName.value()) != std::string::npos)  m_stripClusterKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));

            // Strip L2G
            else if(cuName.find(m_stripL2GKernelName.value()) != std::string::npos) m_stripL2GKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));

            // EDM prep
            else if(cuName.find(m_pixelEdmKernelName.value()) != std::string::npos) m_pixelEDMKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));

            else if(cuName.find(m_stripEdmKernelName.value()) != std::string::npos) m_stripEDMKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));

            else
            {
                ATH_MSG_WARNING("Do not recognize kernel name: "<<cuName);
            }

        }

        ATH_MSG_INFO(m_pixelClusterKernelName.value()<<" size: "<<m_pixelClusterKernels.size());
        ATH_MSG_INFO(m_stripClusterKernelName.value()<<" size: "<<m_stripClusterKernels.size());
        ATH_MSG_INFO(m_stripL2GKernelName.value()<<" size: "<<m_stripL2GKernels.size());
        ATH_MSG_INFO(m_pixelEdmKernelName.value()<<" size: "<<m_pixelEDMKernels.size());
        ATH_MSG_INFO(m_stripEdmKernelName.value()<<" size: "<<m_stripEDMKernels.size());


        // Strip
        //  Set vector size to be = to # of CUs
        m_stripClusterEndEvents.resize(m_stripClusterKernels.size());
        m_stripL2GEndEvents.resize(m_stripL2GKernels.size());
        m_stripEDMEndEvents.resize(m_stripEDMKernels.size());

        // Pixel
        m_pixelClusterEndEvents.resize(m_pixelClusterKernels.size());
        m_pixelEDMEndEvents.resize(m_pixelEDMKernels.size());

        unsigned int nthreads = m_FPGAThreads.value();

        if(m_FPGAThreads.value() < 1){
            nthreads = SG::getNSlots();
        }

        // create the buffers
        for(unsigned int i = 0; i < nthreads; i++)
        {
            // Input
            m_pixelClusterInputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::PIXEL_CONTAINER_INPUT_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            m_stripClusterInputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::STRIP_CONTAINER_INPUT_BUF_SIZE * sizeof(uint64_t), NULL, &err));

            m_stripClusterOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            m_pixelClusterEDMOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE,EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            m_stripClusterEDMOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err));

            m_stripL2GOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            m_stripL2GEDMOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            // EDMPrep
            m_edmPixelOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE * sizeof(uint64_t), NULL, &err));
            m_edmStripOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE * sizeof(uint64_t), NULL, &err));
        }



        m_acc_queue = cl::CommandQueue(m_context, m_accelerator, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
        
        if (err != 0) return StatusCode::FAILURE;
        return StatusCode::SUCCESS;
    }

    std::vector<cl::Event> F110IntegrationAlg::getDepVector(std::vector<cl::Event> &endEvents, size_t cu) const {
        std::vector<cl::Event> deps;

        cl::Event event = endEvents[cu];

        if (event() != NULL) 
        {
          // Event exists
          deps.push_back(event);
        }

        return deps;
    }


    StatusCode F110IntegrationAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F110IntegrationAlg");
        m_numEvents++;

        /// Input handles
        auto pixelInput = SG::get(m_FPGAPixelRDO, ctx);
        auto stripInput = SG::get(m_FPGAStripRDO, ctx);
   
        const int* pixelInputSize{nullptr}, *stripInputSize{nullptr};
        ATH_CHECK(SG::get(pixelInputSize, m_FPGAPixelRDOSize, ctx));
        ATH_CHECK(SG::get(stripInputSize, m_FPGAStripRDOSize, ctx));

        // logic
        unsigned int nthreads = m_FPGAThreads.value();

        if(m_FPGAThreads.value() < 1){
            nthreads = SG::getNSlots();
        }

        size_t bufferIndex = ctx.slot() % nthreads;

        // Get index for each of the kernels
        size_t pixelClusterIndex = ctx.slot() % m_pixelClusterKernels.size();
        size_t stripClusterIndex = ctx.slot() % m_stripClusterKernels.size();
        size_t stripL2GIndex     = ctx.slot() % m_stripL2GKernels.size();
        size_t pixelEDMIndex     = ctx.slot() % m_pixelEDMKernels.size();
        size_t stripEDMIndex     = ctx.slot() % m_stripEDMKernels.size();

        
        // Explicit mutex needed so we don't block multithreading from functioning properly but end the execute function in accordance with FPGA hardware resource utilization
        std::unique_lock lock(m_fpgaHandleMtx);


        //ATH_MSG_DEBUG("F100 Thread number "<<ctx.slot()<<" running on buffer "<<bufferIndex<<" pixelClusterIndex: "<< pixelClusterIndex<<" stripClusterIndex: "<< stripClusterIndex<<" stripL2GIndex: "<< stripL2GIndex<<" pixelEDMIndex: "<< pixelEDMIndex<<" stripEDMIndex: "<< stripEDMIndex);


        // Grab buffers
        cl::Buffer pixelClusterInputBuffer = m_pixelClusterInputBufferList[bufferIndex];
        cl::Buffer stripClusterInputBuffer = m_stripClusterInputBufferList[bufferIndex];
        cl::Buffer stripClusterOutputBuffer = m_stripClusterOutputBufferList[bufferIndex];
        cl::Buffer pixelClusterEDMOutputBuffer = m_pixelClusterEDMOutputBufferList[bufferIndex];
        cl::Buffer stripClusterEDMOutputBuffer = m_stripClusterEDMOutputBufferList[bufferIndex];
        cl::Buffer stripL2GOutputBuffer = m_stripL2GOutputBufferList[bufferIndex];
        cl::Buffer stripL2GEDMOutputBuffer = m_stripL2GEDMOutputBufferList[bufferIndex];
        cl::Buffer edmPixelOutputBuffer = m_edmPixelOutputBufferList[bufferIndex];
        cl::Buffer edmStripOutputBuffer = m_edmStripOutputBufferList[bufferIndex];


        // Grab kernels
        cl::Kernel &pixelClusteringKernel = m_pixelClusterKernels[pixelClusterIndex];
        cl::Kernel &pixelEdmPrepKernel = m_pixelEDMKernels[pixelEDMIndex];

        cl::Kernel &stripClusteringKernel = m_stripClusterKernels[stripClusterIndex];
        cl::Kernel &stripL2GKernel = m_stripL2GKernels[stripL2GIndex];
        cl::Kernel &stripEdmPrepKernel = m_stripEDMKernels[stripEDMIndex];


        // Set kernel args
        pixelClusteringKernel.setArg<cl::Buffer>(0, pixelClusterInputBuffer);
        pixelClusteringKernel.setArg<cl::Buffer>(1, pixelClusterEDMOutputBuffer);

        stripClusteringKernel.setArg<cl::Buffer>(0, stripClusterInputBuffer);
        stripClusteringKernel.setArg<cl::Buffer>(1, stripClusterOutputBuffer);
        stripClusteringKernel.setArg<cl::Buffer>(2, stripClusterEDMOutputBuffer);
        stripClusteringKernel.setArg<unsigned int>(3, *stripInputSize);

        stripL2GKernel.setArg<cl::Buffer>(0, stripClusterOutputBuffer);
        stripL2GKernel.setArg<cl::Buffer>(1, stripClusterEDMOutputBuffer);
        stripL2GKernel.setArg<cl::Buffer>(2, stripL2GOutputBuffer);
        stripL2GKernel.setArg<cl::Buffer>(3, stripL2GEDMOutputBuffer);

        pixelEdmPrepKernel.setArg<cl::Buffer>(0, pixelClusterEDMOutputBuffer);
        pixelEdmPrepKernel.setArg<cl::Buffer>(1, edmPixelOutputBuffer);
        stripEdmPrepKernel.setArg<cl::Buffer>(0, stripL2GEDMOutputBuffer);
        stripEdmPrepKernel.setArg<cl::Buffer>(1, edmStripOutputBuffer);
    

        // Start memory transfers while respecting event deps
        // Note that no explicit mutex is needed anymore due to the m_fpgaHandleMtx mutex
        std::vector<cl::Event> writePixelInputDeps = getDepVector(m_pixelClusterEndEvents, pixelClusterIndex);
        std::vector<cl::Event> writeStripInputDeps = getDepVector(m_stripClusterEndEvents, stripClusterIndex);

        cl::Event writePixelInputEvt;
        cl::Event writeStripInputEvt;
        m_acc_queue.enqueueWriteBuffer(pixelClusterInputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*pixelInput).size(), (*pixelInput).data(), &writePixelInputDeps, &writePixelInputEvt);
        m_acc_queue.enqueueWriteBuffer(stripClusterInputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*stripInput).size(), (*stripInput).data(), &writeStripInputDeps, &writeStripInputEvt);

        std::vector<cl::Event> pixelClusteringDeps = { writePixelInputEvt };
        std::vector<cl::Event> stripClusteringDeps = { writeStripInputEvt };
        
        cl::Event pixelClusteringEvt;
        cl::Event stripClusteringEvt;
        cl::Event pixelL2GEvt;
        cl::Event stripL2GEvt;
        cl::Event edmPrepEvt;
        cl::Event pixelEdmPrepEvt;
        cl::Event stripEdmPrepEvt;

        {
            Athena::Chrono chrono("Kernel execution", m_chronoSvc.get());

            // CLUSTERING
            m_acc_queue.enqueueTask(pixelClusteringKernel, &pixelClusteringDeps, &pixelClusteringEvt);
            m_acc_queue.enqueueTask(stripClusteringKernel, &stripClusteringDeps, &stripClusteringEvt);

            //    Track the clustering end events
            m_pixelClusterEndEvents[pixelClusterIndex] = pixelClusteringEvt;
            m_stripClusterEndEvents[stripClusterIndex] = stripClusteringEvt;

            std::vector<cl::Event> stripL2GDeps = getDepVector(m_stripL2GEndEvents, stripClusterIndex);
            stripL2GDeps.push_back(stripClusteringEvt);

            m_acc_queue.enqueueTask(stripL2GKernel, &stripL2GDeps, &stripL2GEvt);

            m_stripL2GEndEvents[stripClusterIndex] = stripL2GEvt;

            // EDM PREP
            std::vector<cl::Event> pixelEdmPrepDeps = getDepVector(m_pixelEDMEndEvents, pixelClusterIndex);
            pixelEdmPrepDeps.push_back(pixelClusteringEvt);
        
            // Run discrete EDM prep kernels for F110
            std::vector<cl::Event> stripEdmPrepDeps = getDepVector(m_stripEDMEndEvents, stripClusterIndex);
            stripEdmPrepDeps.push_back(stripL2GEvt);

            m_acc_queue.enqueueTask(stripEdmPrepKernel, &stripEdmPrepDeps, &stripEdmPrepEvt);
            m_acc_queue.enqueueTask(pixelEdmPrepKernel, &pixelEdmPrepDeps, &pixelEdmPrepEvt);
          
        }
        
        // READ OUTPUTS
        cl::Event readPixelOutputEvt;
        cl::Event readStripOutputEvt;
        std::vector<cl::Event> readPixelOutputDeps;
        std::vector<cl::Event> readStripOutputDeps;
       
        readPixelOutputDeps.push_back(pixelEdmPrepEvt);
        readStripOutputDeps.push_back(stripEdmPrepEvt);


        SG::WriteHandle<std::vector<uint64_t>> FPGAPixelOutput(m_FPGAPixelOutput, ctx);
        ATH_CHECK(FPGAPixelOutput.record(std::make_unique<std::vector<uint64_t> >(EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE, 0)));

        SG::WriteHandle<std::vector<uint64_t>> FPGAStripOutput(m_FPGAStripOutput, ctx);
        ATH_CHECK(FPGAStripOutput.record(std::make_unique<std::vector<uint64_t> >(EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE, 0)));

        m_acc_queue.enqueueReadBuffer(edmPixelOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*FPGAPixelOutput).size(), (*FPGAPixelOutput).data(), &readPixelOutputDeps, &readPixelOutputEvt);
        m_acc_queue.enqueueReadBuffer(edmStripOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * (*FPGAStripOutput).size(), (*FPGAStripOutput).data(), &readStripOutputDeps, &readStripOutputEvt);


        // Unlocks mutex so other events can handle their FPGA interactions while this event is waiting
        lock.unlock();

        
        // Wait for the reading to finish before terminating the event
        std::vector<cl::Event> terminationDeps = { readPixelOutputEvt, readStripOutputEvt };
        cl::Event::waitForEvents(terminationDeps);

        if(*pixelInputSize == 6) (*FPGAPixelOutput)[0] = 0; // if no pixel input, set the first element to 0
        if(*stripInputSize == 6) (*FPGAStripOutput)[0] = 0; // if no strip input, set the first element to 0


       // calculate the time for the kernel execution
        // get the time of writing pixel input buffer
        cl_ulong pixel_input_time = writePixelInputEvt.getProfilingInfo<CL_PROFILING_COMMAND_END>() - writePixelInputEvt.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_pixelInputTime += pixel_input_time;
        ATH_MSG_DEBUG("Pixel input buffer write time: " << pixel_input_time / 1e6 << " ms");

        // get the time of writing strip input buffer
        cl_ulong strip_input_time = writeStripInputEvt.getProfilingInfo<CL_PROFILING_COMMAND_END>() - writeStripInputEvt.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_stripInputTime += strip_input_time;
        ATH_MSG_DEBUG("Strip input buffer write time: " << strip_input_time / 1e6 << " ms");

        // get the time of pixel clustering
        cl_ulong pixel_clustering_time = pixelClusteringEvt.getProfilingInfo<CL_PROFILING_COMMAND_END>() - pixelClusteringEvt.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_pixelClusteringTime += pixel_clustering_time;
        ATH_MSG_DEBUG("Pixel clustering time: " << pixel_clustering_time / 1e6 << " ms");

        // get the time of strip clustering
        cl_ulong strip_clustering_time = stripClusteringEvt.getProfilingInfo<CL_PROFILING_COMMAND_END>() - stripClusteringEvt.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_stripClusteringTime += strip_clustering_time;
        ATH_MSG_DEBUG("Strip clustering time: " << strip_clustering_time / 1e6 << " ms");


        // get the time of strip L2G
        cl_ulong strip_l2g_time = stripL2GEvt.getProfilingInfo<CL_PROFILING_COMMAND_END>() - stripL2GEvt.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_stripL2GTime += strip_l2g_time;
        ATH_MSG_DEBUG("Strip L2G time: " << strip_l2g_time / 1e6 << " ms");

        // get the time of EDMPrep

        cl_ulong pixel_edm_prep_time = pixelEdmPrepEvt.getProfilingInfo<CL_PROFILING_COMMAND_END>() - pixelEdmPrepEvt.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong strip_edm_prep_time = stripEdmPrepEvt.getProfilingInfo<CL_PROFILING_COMMAND_END>() - stripEdmPrepEvt.getProfilingInfo<CL_PROFILING_COMMAND_START>();

        m_pixelEdmPrepTime += pixel_edm_prep_time;
        ATH_MSG_DEBUG("PixelEDMPrep time: " << pixel_edm_prep_time / 1e6 << " ms");

        m_stripEdmPrepTime += strip_edm_prep_time;
        ATH_MSG_DEBUG("StripEDMPrep time: " << strip_edm_prep_time / 1e6 << " ms");

        // get the time of the whole kernel execution
        cl_ulong kernel_start = pixelClusteringEvt.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
        cl_ulong kernel_end = std::max(pixelEdmPrepEvt.getProfilingInfo<CL_PROFILING_COMMAND_END>(), stripEdmPrepEvt.getProfilingInfo<CL_PROFILING_COMMAND_END>());
        m_kernelTime += (kernel_end - kernel_start);
        ATH_MSG_DEBUG("Kernel execution time: " << (kernel_end - kernel_start) / 1e6 << " ms");

        // get the time of reading pixel output buffer
        cl_ulong pixel_output_time = readPixelOutputEvt.getProfilingInfo<CL_PROFILING_COMMAND_END>() - readPixelOutputEvt.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_pixelOutputTime += pixel_output_time;
        ATH_MSG_DEBUG("Pixel output buffer read time: " << pixel_output_time / 1e6 << " ms");

        // get the time of reading strip output buffer
        cl_ulong strip_output_time = readStripOutputEvt.getProfilingInfo<CL_PROFILING_COMMAND_END>() - readStripOutputEvt.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_stripOutputTime += strip_output_time;
        ATH_MSG_DEBUG("Strip output buffer read time: " << strip_output_time / 1e6 << " ms");
        

        return StatusCode::SUCCESS;
    }

    StatusCode F110IntegrationAlg::finalize()
    {

        ATH_MSG_INFO("Finalizing F110IntegrationAlg");
        ATH_MSG_INFO("Number of events: " << m_numEvents);

        if(m_numEvents > 0){
            ATH_MSG_INFO("Pixel input ave time: " << m_pixelInputTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Strip input ave time: " << m_stripInputTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Pixel clustering ave time: " << m_pixelClusteringTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Strip clustering ave time: " << m_stripClusteringTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Strip L2G ave time: " << m_stripL2GTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("PixelEDMPrep ave time: " << m_pixelEdmPrepTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("StripEDMPrep ave time: " << m_stripEdmPrepTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Kernel execution ave time: " << m_kernelTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Pixel output ave time: " << m_pixelOutputTime / m_numEvents / 1e6 << " ms");
            ATH_MSG_INFO("Strip output ave time: " << m_stripOutputTime / m_numEvents / 1e6 << " ms");
        }

        return StatusCode::SUCCESS;
    }

    void F110IntegrationAlg::getListofCUs(std::vector<std::string>& cuNames)
    {
        xrt::xclbin xrt_xclbin(m_xclbin.value());

        ATH_MSG_INFO("xsa name: "<<xrt_xclbin.get_xsa_name());
        ATH_MSG_INFO("fpga name: "<<xrt_xclbin.get_fpga_device_name());
        ATH_MSG_INFO("uuid: "<<xrt_xclbin.get_uuid().to_string());

        for (const xrt::xclbin::kernel &kernel : xrt_xclbin.get_kernels()) {
            const std::string& kernelName = kernel.get_name();

            ATH_MSG_INFO("kernelName: "<<kernelName);


            for (const xrt::xclbin::ip &computeUnit : kernel.get_cus()) {
                const std::string& computeUnitName = computeUnit.get_name();
                const std::string computeUnitIsolatedName = computeUnitName.substr(kernelName.size() + 1);

                const std::string computeUnitUsableName = kernelName + ":{" + computeUnitIsolatedName + "}";

                ATH_MSG_INFO("CU name: "<<computeUnitUsableName);
                cuNames.push_back(computeUnitUsableName);
            }
        }

    }

} // namespace EFTrackingFPGAIntegration
