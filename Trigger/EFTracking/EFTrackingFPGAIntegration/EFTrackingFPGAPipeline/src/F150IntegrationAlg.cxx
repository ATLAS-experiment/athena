/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
   */

#include "EFTrackingFPGAPipeline/F150IntegrationAlg.h"
#include "AthenaKernel/Chrono.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_uuid.h>
#include <fstream>

namespace EFTrackingFPGAIntegration
{
    StatusCode F150IntegrationAlg::initialize()
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
        ATH_CHECK(m_FPGATrackOutput.initialize());

        std::vector<std::string> listofCUs;

        getListofCUs(listofCUs);

        cl_int err = 0;

        unsigned int nthreads = m_FPGAThreads.value();

        if(m_FPGAThreads.value() < 1){
            nthreads = SG::getNSlots();
        }

        // create the buffers
        for(unsigned int i = 0; i < nthreads; i++)
        {
            m_acc_queues.emplace_back(m_context, m_accelerator, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);

            // Input
            m_pixelClusterInputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::PIXEL_CONTAINER_INPUT_BUF_SIZE * sizeof(uint64_t), nullptr, &err));
            m_stripClusterInputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_ONLY, EFTrackingTransient::STRIP_CONTAINER_INPUT_BUF_SIZE * sizeof(uint64_t), nullptr, &err));

            
            m_pixelClusterOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err));
            m_stripClusterOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err));
            m_pixelClusterEDMOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE,EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err));
            m_stripClusterEDMOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err));
           
            m_stripL2GOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err));
            m_stripL2GEDMOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), nullptr, &err));
            // EDMPrep
            m_edmPixelOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE * sizeof(uint32_t), nullptr, &err));
            m_edmStripOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE * sizeof(uint32_t), nullptr, &err));


            m_slicingEngineOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t), nullptr, &err));
            m_insideOutOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE * sizeof(uint64_t), nullptr, &err));
        }

        // Create kernels for each one of CUs that is inside device
        for (const auto& cuName: listofCUs)
        {
            // Pixel clustering
            if(cuName.find(m_pixelClusterKernelName.value()) != std::string::npos) m_pixelClusteringKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));

            // Strip clustering
            else if(cuName.find(m_stripClusterKernelName.value()) != std::string::npos)  m_stripClusteringKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));
            // Strip L2G
            else if(cuName.find(m_stripL2GKernelName.value()) != std::string::npos) m_stripL2GKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));

            // EDM prep
            else if(cuName.find(m_pixelEdmKernelName.value()) != std::string::npos) m_pixelEdmPrepKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));
            else if(cuName.find(m_stripEdmKernelName.value()) != std::string::npos) m_stripEdmPrepKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));
            // Slicing
            else if(cuName.find(m_slicingEngineInputName.value()) != std::string::npos) m_slicingEngineInputKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));
            else if(cuName.find(m_slicingEngineOutputName.value()) != std::string::npos) m_slicingEngineOutputKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));
            // IO
            else if(cuName.find(m_insideOutInputName.value()) != std::string::npos) m_insideOutInputKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));
            else if(cuName.find(m_insideOutOutputName.value()) != std::string::npos) m_insideOutOutputKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));
            else
            {
                ATH_MSG_WARNING("Do not recognize kernel name: "<<cuName);
            }
        }

        ATH_MSG_INFO(m_pixelClusterKernelName.value()<<" size: "<<m_pixelClusteringKernels.size());
        ATH_MSG_INFO(m_stripClusterKernelName.value()<<" size: "<<m_stripClusteringKernels.size());
        ATH_MSG_INFO(m_stripL2GKernelName.value()<<" size: "<<m_stripL2GKernels.size());
        ATH_MSG_INFO(m_pixelEdmKernelName.value()<<" size: "<<m_pixelEdmPrepKernels.size());
        ATH_MSG_INFO(m_stripEdmKernelName.value()<<" size: "<<m_stripEdmPrepKernels.size());
        ATH_MSG_INFO(m_slicingEngineInputName.value()<<" size: "<<m_slicingEngineInputKernels.size());
        ATH_MSG_INFO(m_slicingEngineOutputName.value()<<" size: "<<m_slicingEngineOutputKernels.size());
        ATH_MSG_INFO(m_insideOutInputName.value()<<" size: "<<m_insideOutInputKernels.size());
        ATH_MSG_INFO(m_insideOutOutputName.value()<<" size: "<<m_insideOutOutputKernels.size());

        if(m_pixelClusteringKernels.size()==0){
            ATH_MSG_FATAL("No m_pixelClusteringKernels constructed");
            return StatusCode::FAILURE;
        }

        // monitoring
        if ( !m_monTool.empty() ) {
            ATH_CHECK(m_monTool.retrieve() );
        }
        else {
            ATH_MSG_INFO("Monitoring tool is empty");
        }

        return StatusCode::SUCCESS;
    }

    void F150IntegrationAlg::dumpHexData(std::span<const uint64_t> data, const std::string& dataDescriptor, const EventContext &ctx) const {

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
    

    StatusCode F150IntegrationAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F150IntegrationAlg");
        auto mnt_timer_Total = Monitored::Timer<std::chrono::milliseconds>("TIME_Total");
        auto monTime = Monitored::Group(m_monTool, mnt_timer_Total);

        mnt_timer_Total.start();

        m_numEvents++;

        /// Input handles
        const std::vector<uint64_t>* pixelInput{nullptr}, *stripInput{nullptr};
        ATH_CHECK(SG::get(pixelInput, m_FPGAPixelRDO, ctx));
        ATH_CHECK(SG::get(stripInput, m_FPGAStripRDO, ctx));  

    
        // logic
        unsigned int nthreads = m_FPGAThreads.value();

        if(m_FPGAThreads.value() < 1){
            nthreads = SG::getNSlots();
        }

        size_t bufferIndex = ctx.slot() % nthreads;

        // Get index for each of the kernels
        size_t pixelClusterIndex = ctx.slot() % m_pixelClusteringKernels.size();
        size_t stripClusterIndex = ctx.slot() % m_stripClusteringKernels.size();
        size_t stripL2GIndex = ctx.slot() % m_stripL2GKernels.size();
        size_t pixelEDMIndex = ctx.slot() % m_pixelEdmPrepKernels.size();
        size_t stripEDMIndex = ctx.slot() % m_stripEdmPrepKernels.size();
        size_t slicingOutIndex = ctx.slot() % m_slicingEngineOutputKernels.size();
        size_t insideOutInputIndex = ctx.slot() % m_insideOutInputKernels.size();
        size_t insideOutOutputIndex = ctx.slot() % m_insideOutOutputKernels.size();

        const cl::CommandQueue &acc_queue = m_acc_queues[bufferIndex];

        cl::Kernel &pixelClusteringKernel = m_pixelClusteringKernels[pixelClusterIndex];
        cl::Kernel &stripClusteringKernel = m_stripClusteringKernels[stripClusterIndex];
        cl::Kernel &stripL2GKernel  = m_stripL2GKernels[stripL2GIndex];
        cl::Kernel &pixelEdmPrepKernel = m_pixelEdmPrepKernels[pixelEDMIndex];
        cl::Kernel &stripEdmPrepKernel = m_stripEdmPrepKernels[stripEDMIndex];
        cl::Kernel &slicingEngineOutputKernel = m_slicingEngineOutputKernels[slicingOutIndex];
        cl::Kernel &insideOutInputKernel = m_insideOutInputKernels[insideOutInputIndex];
        cl::Kernel &insideOutOutputKernel = m_insideOutOutputKernels[insideOutOutputIndex];


        // Set kernel arguments
        pixelClusteringKernel.setArg(0, m_pixelClusterInputBufferList[bufferIndex]);
        pixelClusteringKernel.setArg(2, m_pixelClusterEDMOutputBufferList[bufferIndex]);
        
        stripClusteringKernel.setArg(0, m_stripClusterInputBufferList[bufferIndex]);
        stripClusteringKernel.setArg(1, m_stripClusterOutputBufferList[bufferIndex]);
        stripClusteringKernel.setArg(2, m_stripClusterEDMOutputBufferList[bufferIndex]);
        stripClusteringKernel.setArg(3, static_cast<unsigned int>((*stripInput).size()));

        stripL2GKernel.setArg(0, m_stripClusterOutputBufferList[bufferIndex]);
        stripL2GKernel.setArg(1, m_stripClusterEDMOutputBufferList[bufferIndex]);
        stripL2GKernel.setArg(2, m_stripL2GOutputBufferList[bufferIndex]);
        stripL2GKernel.setArg(3, m_stripL2GEDMOutputBufferList[bufferIndex]);

        pixelEdmPrepKernel.setArg(0, m_pixelClusterEDMOutputBufferList[bufferIndex]);
        pixelEdmPrepKernel.setArg(1, m_edmPixelOutputBufferList[bufferIndex]);
        stripEdmPrepKernel.setArg(0, m_stripL2GEDMOutputBufferList[bufferIndex]);
        stripEdmPrepKernel.setArg(1, m_edmStripOutputBufferList[bufferIndex]);

        slicingEngineOutputKernel.setArg(1, m_slicingEngineOutputBufferList[bufferIndex]);
        
        insideOutInputKernel.setArg(0,  m_slicingEngineOutputBufferList[bufferIndex]);
        insideOutOutputKernel.setArg(0, m_insideOutOutputBufferList[bufferIndex]);


        // Start the transfers
        cl::Event evt_write_pixel_input;
        cl::Event evt_write_strip_input;

        acc_queue.enqueueWriteBuffer(m_pixelClusterInputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint64_t) * (*pixelInput).size(), (*pixelInput).data(), nullptr, &evt_write_pixel_input);
        acc_queue.enqueueWriteBuffer(m_stripClusterInputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint64_t) * (*stripInput).size(), (*stripInput).data(), nullptr, &evt_write_strip_input);
        std::vector<cl::Event> evt_vec_pixel_input{evt_write_pixel_input};
        std::vector<cl::Event> evt_vec_strip_input{evt_write_strip_input};


        cl::Event evt_pixel_clustering;
        cl::Event evt_strip_clustering;
        cl::Event evt_strip_l2g;
        cl::Event evt_pixel_l2g;
        cl::Event evt_edm_prep;
        cl::Event evt_pixel_edm_prep;
        cl::Event evt_strip_edm_prep;
        cl::Event evt_slicing_done;
        cl::Event evt_insideoutInput_done;
        cl::Event evt_insideoutOutput_done;
        {
            Athena::Chrono chrono("Kernel execution", m_chronoSvc.get());
            acc_queue.enqueueTask(pixelClusteringKernel, &evt_vec_pixel_input, &evt_pixel_clustering);
            acc_queue.enqueueTask(stripClusteringKernel, &evt_vec_strip_input, &evt_strip_clustering);

            std::vector<cl::Event> evt_vec_strip_clustering{evt_strip_clustering};
            acc_queue.enqueueTask(stripL2GKernel, &evt_vec_strip_clustering, &evt_strip_l2g);

            std::vector<cl::Event> evt_vec_pixelEDM{evt_pixel_clustering};
            std::vector<cl::Event> evt_vec_strip_l2g{evt_strip_l2g};
            acc_queue.enqueueTask(pixelEdmPrepKernel, &evt_vec_pixelEDM, &evt_pixel_edm_prep);
            acc_queue.enqueueTask(stripEdmPrepKernel, &evt_vec_strip_l2g, &evt_strip_edm_prep);
            
            acc_queue.enqueueTask(slicingEngineOutputKernel, nullptr, &evt_slicing_done);
            
            // Execute
            ATH_MSG_DEBUG("Executing IO Kernel");
            std::vector<cl::Event> evt_vec_slicing{evt_slicing_done};
            acc_queue.enqueueTask(insideOutInputKernel,  &evt_vec_slicing, &evt_insideoutInput_done);
            acc_queue.enqueueTask(insideOutOutputKernel, nullptr, &evt_insideoutOutput_done);

        }

        cl::Event evt_pixel_cluster_output;
        cl::Event evt_strip_cluster_output;
        cl::Event evt_track_output;
        
        std::vector<cl::Event> evt_vec_pixel_edm_prep {evt_pixel_edm_prep};
        std::vector<cl::Event> evt_vec_strip_edm_prep {evt_strip_edm_prep};
        std::vector<cl::Event> evt_vec_insideout_output {evt_insideoutOutput_done};



        // output handles
        SG::WriteHandle<std::vector<uint32_t>> FPGAPixelOutput(m_FPGAPixelOutput, ctx);
        ATH_CHECK(FPGAPixelOutput.record(std::make_unique<std::vector<uint32_t> >(EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE, 0)));

        SG::WriteHandle<std::vector<uint32_t>> FPGAStripOutput(m_FPGAStripOutput, ctx);
        ATH_CHECK(FPGAStripOutput.record(std::make_unique<std::vector<uint32_t> >(EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE, 0)));

        SG::WriteHandle<std::vector<uint64_t>> FPGATrackOutput(m_FPGATrackOutput, ctx);
        ATH_CHECK(FPGATrackOutput.record(std::make_unique<std::vector<uint64_t> >(EFTrackingTransient::TRACK_CONTAINER_BUF_SIZE, 0)));

        acc_queue.enqueueReadBuffer(m_edmPixelOutputBufferList[bufferIndex],  CL_FALSE, 0, sizeof(uint32_t) * (*FPGAPixelOutput).size(), (*FPGAPixelOutput).data(), &evt_vec_pixel_edm_prep, &evt_pixel_cluster_output);
        acc_queue.enqueueReadBuffer(m_edmStripOutputBufferList[bufferIndex],  CL_FALSE, 0, sizeof(uint32_t) * (*FPGAStripOutput).size(), (*FPGAStripOutput).data(), &evt_vec_strip_edm_prep, &evt_strip_cluster_output);
        acc_queue.enqueueReadBuffer(m_insideOutOutputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint64_t) * (*FPGATrackOutput).size(), (*FPGATrackOutput).data(), &evt_vec_insideout_output, &evt_track_output);

        std::vector<cl::Event> wait_for_reads = { evt_pixel_cluster_output, evt_strip_cluster_output, evt_track_output };
        cl::Event::waitForEvents(wait_for_reads);


        mnt_timer_Total.stop();

        if(pixelInput->size() == 6) (*FPGAPixelOutput)[0] = 0; // if no pixel input, set the first element to 0
        if(stripInput->size() == 6) (*FPGAStripOutput)[0] = 0; // if no strip input, set the first element to 0


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

        // get the time of strip L2G
        cl_ulong strip_l2g_time = evt_strip_l2g.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_strip_l2g.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        m_stripL2GTime += strip_l2g_time;
        ATH_MSG_DEBUG("Strip L2G time: " << strip_l2g_time / 1e6 << " ms");

        cl_ulong pixel_edm_prep_time = evt_pixel_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_pixel_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong strip_edm_prep_time = evt_strip_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evt_strip_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_START>();

        m_pixelEdmPrepTime += pixel_edm_prep_time;
        ATH_MSG_DEBUG("PixelEDMPrep time: " << pixel_edm_prep_time / 1e6 << " ms");

        m_stripEdmPrepTime += strip_edm_prep_time;
        ATH_MSG_DEBUG("StripEDMPrep time: " << strip_edm_prep_time / 1e6 << " ms");


        // get the time of the whole kernel execution
        cl_ulong kernel_start = evt_pixel_clustering.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
        cl_ulong kernel_end = std::max(evt_pixel_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>(), evt_strip_edm_prep.getProfilingInfo<CL_PROFILING_COMMAND_END>());
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

    StatusCode F150IntegrationAlg::finalize()
    {

        ATH_MSG_INFO("Finalizing F150IntegrationAlg");
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

    void F150IntegrationAlg::getListofCUs(std::vector<std::string>& cuNames)
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
