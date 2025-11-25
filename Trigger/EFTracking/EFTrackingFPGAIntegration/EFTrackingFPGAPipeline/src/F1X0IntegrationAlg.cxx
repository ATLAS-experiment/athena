/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
   */

#include "EFTrackingFPGAPipeline/F1X0IntegrationAlg.h"
#include "AthenaKernel/Chrono.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_uuid.h>

namespace EFTrackingFPGAIntegration
{
    StatusCode F1X0IntegrationAlg::initialize()
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
        ATH_CHECK(m_FPGAPixelRDOSize.initialize());
        ATH_CHECK(m_FPGAStripRDOSize.initialize());

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
            m_edmPixelOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE * sizeof(uint32_t), NULL, &err));
            m_edmStripOutputBufferList.push_back(cl::Buffer(m_context, CL_MEM_READ_WRITE, EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE * sizeof(uint32_t), NULL, &err));
        }

        // Create kernels for each one of CUs that is inside device
        for (const auto& cuName: listofCUs)
        {
            // Pixel clustering
            if(cuName.find(m_pixelClusterKernelName.value()) != std::string::npos) m_pixelClusteringKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));

            // Strip clustering
            else if(cuName.find(m_stripClusterKernelName.value()) != std::string::npos)  m_stripClusteringKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));

            // Pixel L2G
            else if(!m_doF110 && cuName.find(m_pixelL2GKernelName.value()) != std::string::npos) m_pixelL2GKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));

            // Strip L2G
            else if(cuName.find(m_stripL2GKernelName.value()) != std::string::npos) m_stripL2GKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));

            // EDM prep
            else if(cuName.find(m_pixelEdmKernelName.value()) != std::string::npos) m_pixelEdmPrepKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));

            else if(cuName.find(m_stripEdmKernelName.value()) != std::string::npos) m_stripEdmPrepKernels.emplace_back(cl::Kernel(m_program, cuName.c_str()));
            else
            {
                ATH_MSG_WARNING("Do not recognize kernel name: "<<cuName);
            }
        }

        ATH_MSG_INFO(m_pixelClusterKernelName.value()<<" size: "<<m_pixelClusteringKernels.size());
        ATH_MSG_INFO(m_stripClusterKernelName.value()<<" size: "<<m_stripClusteringKernels.size());
        ATH_MSG_INFO(m_pixelL2GKernelName.value()<<" size: "<<m_pixelL2GKernels.size());
        ATH_MSG_INFO(m_stripL2GKernelName.value()<<" size: "<<m_stripL2GKernels.size());
        ATH_MSG_INFO(m_pixelEdmKernelName.value()<<" size: "<<m_pixelEdmPrepKernels.size());
        ATH_MSG_INFO(m_stripEdmKernelName.value()<<" size: "<<m_stripEdmPrepKernels.size());

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

    StatusCode F1X0IntegrationAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F1X0IntegrationAlg");
        auto mnt_timer_Total = Monitored::Timer<std::chrono::milliseconds>("TIME_Total");
        auto monTime = Monitored::Group(m_monTool, mnt_timer_Total);

        mnt_timer_Total.start();

        m_numEvents++;

        /// Input handles
        const std::vector<uint64_t>* pixelInput{nullptr}, *stripInput{nullptr};
        ATH_CHECK(SG::get(pixelInput, m_FPGAPixelRDO, ctx));
        ATH_CHECK(SG::get(stripInput, m_FPGAStripRDO, ctx));  

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
        size_t pixelClusterIndex = ctx.slot() % m_pixelClusteringKernels.size();
        size_t stripClusterIndex = ctx.slot() % m_stripClusteringKernels.size();
        size_t stripL2GIndex = ctx.slot() % m_stripL2GKernels.size();
        size_t pixelL2GIndex = m_pixelL2GKernels.size() ? ctx.slot() % m_pixelL2GKernels.size() : 0;
        size_t pixelEDMIndex = m_pixelEdmPrepKernels.size() ? ctx.slot() % m_pixelEdmPrepKernels.size() : 0;
        size_t stripEDMIndex = m_stripEdmPrepKernels.size() ? ctx.slot() % m_stripEdmPrepKernels.size() : 0;

        const cl::CommandQueue &acc_queue = m_acc_queues[bufferIndex];

        cl::Kernel &pixelClusteringKernel = m_pixelClusteringKernels[pixelClusterIndex];
        cl::Kernel &stripClusteringKernel = m_stripClusteringKernels[stripClusterIndex];
        cl::Kernel &stripL2GKernel = m_stripL2GKernels[stripL2GIndex];
        cl::Kernel &pixelEdmPrepKernel = m_pixelEdmPrepKernels[pixelEDMIndex];
        cl::Kernel &stripEdmPrepKernel = m_stripEdmPrepKernels[stripEDMIndex];
        cl::Kernel *pixelL2GKernel = nullptr;

        if (!m_doF110)  pixelL2GKernel = &m_pixelL2GKernels[pixelL2GIndex];

        // Set kernel arguments
        pixelClusteringKernel.setArg(0, m_pixelClusterInputBufferList[bufferIndex]);
        if (m_doF110) {
            pixelClusteringKernel.setArg(1, m_pixelClusterEDMOutputBufferList[bufferIndex]);
        } else {
            pixelClusteringKernel.setArg(1, m_pixelClusterOutputBufferList[bufferIndex]);
            pixelClusteringKernel.setArg(2, m_pixelClusterEDMOutputBufferList[bufferIndex]);

            uint32_t hit_vector_size_bytes = sizeof(uint64_t) * (*pixelInput).size();
            uint32_t cluster_vector_size_bytes = sizeof(uint64_t) * (*pixelInput).size();
            uint32_t edm_vector_size_bytes = sizeof(uint64_t) * (*pixelInput).size() * 8;


            pixelClusteringKernel.setArg(3, hit_vector_size_bytes);
            pixelClusteringKernel.setArg(4, cluster_vector_size_bytes);
            pixelClusteringKernel.setArg(5, edm_vector_size_bytes);
        }

        stripClusteringKernel.setArg(0, m_stripClusterInputBufferList[bufferIndex]);
        stripClusteringKernel.setArg(1, m_stripClusterOutputBufferList[bufferIndex]);
        stripClusteringKernel.setArg(2, m_stripClusterEDMOutputBufferList[bufferIndex]);
        stripClusteringKernel.setArg(3, static_cast<unsigned int>(*stripInputSize));

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


        pixelEdmPrepKernel.setArg(0, m_pixelClusterEDMOutputBufferList[bufferIndex]);
        pixelEdmPrepKernel.setArg(1, m_edmPixelOutputBufferList[bufferIndex]);
        stripEdmPrepKernel.setArg(0, m_stripL2GEDMOutputBufferList[bufferIndex]);
        stripEdmPrepKernel.setArg(1, m_edmStripOutputBufferList[bufferIndex]);



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

            std::vector<cl::Event> evt_vec_strip_clustering{evt_strip_clustering};
            if (!m_doF110) 
            {
                std::vector<cl::Event> evt_vec_pixel_clustering{evt_pixel_clustering};
                acc_queue.enqueueTask(*pixelL2GKernel, &evt_vec_pixel_clustering, &evt_pixel_l2g);
            }
            acc_queue.enqueueTask(stripL2GKernel, &evt_vec_strip_clustering, &evt_strip_l2g);

            std::vector<cl::Event> evt_vec_pixelEDM;
            if(m_doF110) evt_vec_pixelEDM.push_back(evt_pixel_clustering);
            else evt_vec_pixelEDM.push_back(evt_pixel_l2g);
            std::vector<cl::Event> evt_vec_strip_l2g{evt_strip_l2g};
            acc_queue.enqueueTask(pixelEdmPrepKernel, &evt_vec_pixelEDM, &evt_pixel_edm_prep);
            acc_queue.enqueueTask(stripEdmPrepKernel, &evt_vec_strip_l2g, &evt_strip_edm_prep);
            
        }

        cl::Event evt_pixel_cluster_output;
        cl::Event evt_strip_cluster_output;
        
        std::vector<cl::Event> evt_vec_pixel_edm_prep;
        std::vector<cl::Event> evt_vec_strip_edm_prep;

        evt_vec_pixel_edm_prep.push_back(evt_pixel_edm_prep);
        evt_vec_strip_edm_prep.push_back(evt_strip_edm_prep);


        // output handles

        SG::WriteHandle<std::vector<uint32_t>> FPGAPixelOutput(m_FPGAPixelOutput, ctx);
        ATH_CHECK(FPGAPixelOutput.record(std::make_unique<std::vector<uint32_t> >(EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE, 0)));

        SG::WriteHandle<std::vector<uint32_t>> FPGAStripOutput(m_FPGAStripOutput, ctx);
        ATH_CHECK(FPGAStripOutput.record(std::make_unique<std::vector<uint32_t> >(EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE, 0)));

        acc_queue.enqueueReadBuffer(m_edmPixelOutputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint32_t) * (*FPGAPixelOutput).size(), (*FPGAPixelOutput).data(), &evt_vec_pixel_edm_prep, &evt_pixel_cluster_output);
        acc_queue.enqueueReadBuffer(m_edmStripOutputBufferList[bufferIndex], CL_FALSE, 0, sizeof(uint32_t) * (*FPGAStripOutput).size(), (*FPGAStripOutput).data(), &evt_vec_strip_edm_prep, &evt_strip_cluster_output);

        std::vector<cl::Event> wait_for_reads = { evt_pixel_cluster_output, evt_strip_cluster_output };
        cl::Event::waitForEvents(wait_for_reads);

        mnt_timer_Total.stop();

        if(*pixelInputSize == 6) (*FPGAPixelOutput)[0] = 0; // if no pixel input, set the first element to 0
        if(*stripInputSize == 6) (*FPGAStripOutput)[0] = 0; // if no strip input, set the first element to 0

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

    StatusCode F1X0IntegrationAlg::finalize()
    {

        ATH_MSG_INFO("Finalizing F1X0IntegrationAlg");
        ATH_MSG_INFO("Number of events: " << m_numEvents);

        if(m_numEvents > 0){
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
        }

        return StatusCode::SUCCESS;
    }

    void F1X0IntegrationAlg::getListofCUs(std::vector<std::string>& cuNames)
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
