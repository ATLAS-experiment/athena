/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EFTrackingFPGAPipeline/F600IntegrationAlg.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"
#include "AthenaKernel/Chrono.h"
#include "EFTrackingFPGAPipeline/DataPreparationPipeline.h"

namespace EFTrackingFPGAIntegration
{
    StatusCode F600IntegrationAlg::initialize()
    {
        ATH_MSG_INFO("Running on the FPGA accelerator");

        // ATH_CHECK(IntegrationBase::precheck({m_xclbin}));

        ATH_CHECK(m_chronoSvc.retrieve());

        {
            Athena::Chrono chrono("Platform and device initlize", m_chronoSvc.get());
            ATH_CHECK(IntegrationBase::initialize());
        }

        {
            Athena::Chrono chrono("CL::loadProgram", m_chronoSvc.get());
            ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));
        }

        ATH_CHECK(m_xaodClusterMaker.retrieve());
        ATH_CHECK(m_testVectorTool.retrieve());
        ATH_CHECK(m_FPGADataFormatTool.retrieve());
        ATH_CHECK(m_FPGATrackKey.initialize());
        ATH_CHECK(m_outputConversionTool.retrieve());
        return StatusCode::SUCCESS;
    }
    
    StatusCode F600IntegrationAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F600IntegrationAlg");
	    std::vector<uint64_t> encodedData;
        auto FPGATrackHandle = SG::makeHandle(m_FPGATrackKey, ctx);
        
        ATH_CHECK(m_FPGADataFormatTool->convertFPGATracksToFPGADataFormat(&(*FPGATrackHandle), encodedData, ctx));
        
        std::unique_ptr<EFTrackingTransient::Metadata> metadata =
        std::make_unique<EFTrackingTransient::Metadata>();
        	            
        cl_int err = 0;
        cl::Buffer scoringInputBuffer(m_context, CL_MEM_READ_ONLY, sizeof(uint64_t) * encodedData.size(), NULL, &err);
        cl::Buffer scoringOutputBuffer(m_context, CL_MEM_READ_WRITE, sizeof(uint64_t) * encodedData.size(), NULL, &err);

        cl::Kernel NNOverlapDecorator_kernel(m_program, "NNOverlapDecorator_kernel");
        NNOverlapDecorator_kernel.setArg<cl::Buffer>(0, scoringInputBuffer);
        NNOverlapDecorator_kernel.setArg<cl::Buffer>(1, scoringOutputBuffer);
        NNOverlapDecorator_kernel.setArg<unsigned int>(2, encodedData.size());

        cl::CommandQueue acc_queue(m_context, m_accelerator, CL_QUEUE_PROFILING_ENABLE, &err);

        cl::Event cl_evt_write_scoring_input;
        acc_queue.enqueueWriteBuffer(scoringInputBuffer, CL_FALSE, 0, sizeof(uint64_t) * encodedData.size(), encodedData.data(), NULL, &cl_evt_write_scoring_input);
        std::vector<cl::Event> cl_evt_vec_scoring_input{cl_evt_write_scoring_input};
        acc_queue.finish();


        cl::Event cl_evt_scoring_kernel;

        {
            Athena::Chrono chrono("Kernel execution", m_chronoSvc.get());
            acc_queue.enqueueTask(NNOverlapDecorator_kernel, &cl_evt_vec_scoring_input, &cl_evt_scoring_kernel);
            acc_queue.finish();
        }

        cl::Event cl_evt_scoring_output;
        std::vector<uint64_t> scoringOutputData(encodedData.size(),0);
      
        acc_queue.enqueueReadBuffer(scoringOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * scoringOutputData.size(), scoringOutputData.data(), NULL, &cl_evt_scoring_output);
        acc_queue.finish();
    
	    ATH_CHECK(m_outputConversionTool->decodeFPGAoutput(scoringOutputData, metadata.get(), nullptr, nullptr, OutputConversion::FSM::GTracks)); 
	    cl_ulong track_input_start = cl_evt_write_scoring_input.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	    cl_ulong track_input_end = cl_evt_write_scoring_input.getProfilingInfo<CL_PROFILING_COMMAND_END>();
	    cl_ulong track_input_time = track_input_end - track_input_start;
	    m_kernelTime += track_input_time;
	    m_num_Events +=1;
	    m_sum_kernelTime +=track_input_time;
	    ATH_MSG_INFO("Kernel execution time: " <<track_input_time / 1e6 << " ms");

	    //Overlap Removal Kernel
	    cl::Buffer overlapRemovalInputBuffer(m_context, CL_MEM_READ_ONLY, sizeof(uint64_t) * encodedData.size(), NULL, &err);
	    cl::Buffer overlapRemovalOutputBuffer(m_context, CL_MEM_READ_WRITE, sizeof(uint64_t) * encodedData.size(), NULL, &err);
	    
	    cl::Kernel DuplicateRemoverPackage_Top(m_program, "DuplicateRemoverPackage_Top");
	    DuplicateRemoverPackage_Top.setArg<cl::Buffer>(0, overlapRemovalInputBuffer);
	    DuplicateRemoverPackage_Top.setArg<cl::Buffer>(1, overlapRemovalOutputBuffer);
	    DuplicateRemoverPackage_Top.setArg<unsigned int>(2, encodedData.size());

	    cl::Event cl_evt_write_overlapRemoval_input;
	    acc_queue.enqueueWriteBuffer(overlapRemovalInputBuffer, CL_FALSE, 0, sizeof(uint64_t) * encodedData.size(), encodedData.data(), NULL, &cl_evt_write_overlapRemoval_input);
	    std::vector<cl::Event> cl_evt_vec_overlapRemoval_input{cl_evt_write_overlapRemoval_input};
	    acc_queue.finish();

	    cl::Event cl_evt_overlapRemoval_kernel;
	    
	    {
	      Athena::Chrono chrono("Kernel execution", m_chronoSvc.get());
	      acc_queue.enqueueTask(DuplicateRemoverPackage_Top, &cl_evt_vec_overlapRemoval_input, &cl_evt_overlapRemoval_kernel);
	      acc_queue.finish();
	    }

	    cl::Event cl_evt_overlapRemoval_output;
	    std::vector<uint64_t> overlapRemovalOutputData(encodedData.size(),0);
	    
	    acc_queue.enqueueReadBuffer(overlapRemovalOutputBuffer, CL_FALSE, 0, sizeof(uint64_t) * overlapRemovalOutputData.size(), overlapRemovalOutputData.data(), NULL, &cl_evt_overlapRemoval_output);
	    acc_queue.finish();

        return StatusCode::SUCCESS;
    }


    
     StatusCode F600IntegrationAlg::finalize()
    {
      ATH_MSG_INFO("Average Kernel execution time: " << m_sum_kernelTime /m_num_Events /1e6 << " ms");
      return StatusCode::SUCCESS;
    }



}
