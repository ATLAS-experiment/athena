/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

/**
 * @file src/PixelClustering.cxx
 * @author zhaoyuan.cui@cern.ch
 * @date Sep. 10, 2024
 */

#include "PixelClustering.h"

#include <fstream>
#include <CL/cl_ext.h>

StatusCode PixelClustering::initialize()
{
    ATH_CHECK(m_pixelRDOKey.initialize());

    ATH_CHECK(m_FPGADataFormatTool.retrieve());

    ATH_CHECK(IntegrationBase::precheck({m_xclbin, m_kernelName, m_inputTV, m_refTV}));
    ATH_CHECK(IntegrationBase::initialize());
    ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));

    return StatusCode::SUCCESS;
}

StatusCode PixelClustering::execute(const EventContext &ctx) const
{
    ATH_MSG_DEBUG("In execute(), event slot: " << ctx.slot());

    auto pixelRDOHandle = SG::makeHandle(m_pixelRDOKey, ctx);

    std::vector<uint64_t> outputData;

    if(!m_FPGADataFormatTool->convertPixelHitsToFPGADataFormat(*pixelRDOHandle, outputData, ctx)) {
        return StatusCode::FAILURE;
    }

    // Prepare output vector (kernel will output 4096 word buffer)
    std::vector<uint64_t> kernelOutput(outputData.size(), 0);

    ATH_MSG_DEBUG("ITK pixel encoded data, size is: " << outputData.size());
    int line = 0;
    //if (outputData.size() < 4096) outputData.resize(4096);
    for(const auto& var: outputData) {
        ATH_MSG_DEBUG("EncodedData["<< std::dec << std::setw(4) << line <<"] = 0x" << std::hex << std::setfill('0') << std::setw(16) << var << std::setfill(' '));
        line++;
    }

    // Work with the accelerator
    cl_int err = 0;

    // Allocate buffers on accelerator
    cl::Buffer acc_inbuff(m_context, CL_MEM_READ_ONLY, sizeof(uint64_t) * outputData.size(), NULL, &err);
    cl::Buffer acc_outbuff(m_context, CL_MEM_READ_WRITE, sizeof(uint64_t) * kernelOutput.size(), NULL, &err);

    // Prepare kernel
    cl::Kernel acc_kernel(m_program, m_kernelName.value().data(), &err);
    acc_kernel.setArg(0, acc_inbuff);
    acc_kernel.setArg(1, acc_outbuff);

    // Make queue of commands
    cl::CommandQueue acc_queue(m_context, m_accelerator);

    acc_queue.enqueueWriteBuffer(acc_inbuff, CL_TRUE, 0, sizeof(uint64_t) * outputData.size(), outputData.data(), NULL, NULL);

    acc_queue.enqueueTask(acc_kernel);

    acc_queue.finish();

    // The current implementation of the kernel Read/Write the same buffer
    // So the line below reads the inbuff instead of outbuff
    //acc_queue.enqueueReadBuffer(acc_inbuff, CL_TRUE, 0, sizeof(uint64_t) * kernelOutput.size(), kernelOutput.data(), NULL, NULL);
    acc_queue.enqueueReadBuffer(acc_outbuff, CL_TRUE, 0, sizeof(uint64_t) * kernelOutput.size(), kernelOutput.data(), NULL, NULL);


    // Quick validation
    line = 0;
    bool is_inmod = false;
    uint64_t modid = 0;
    unsigned int foot_i = 0;
    for(const auto& var: kernelOutput) {
	if (var == 0x0000000000001d1e) {
	    line++;
	    continue;
	}

        ATH_MSG_DEBUG("kernelOutput["<< std::dec << std::setw(4) << line <<"] = 0x" << std::hex << std::setfill('0') << std::setw(16) << var << std::setfill(' ') << std::dec);
	line++;

#define SELECTBITS(len, startbit) (len == 64 ? UINTMAX_MAX : (((1ULL << len) - 1ULL) << startbit))

	if (((var & SELECTBITS(8, 56)) >> 56) == 0xab) {
	    continue;
	} else if (((var & SELECTBITS(8, 56)) >> 56) == 0xcd) {
	    foot_i++;
	} else if (((var & SELECTBITS(8, 56)) >> 56) == 0x55) {
	    is_inmod = true;
	    modid = ((var & SELECTBITS(32, 24)) >> 24);
	} else if (foot_i == 2) {
		break;
	} else {
	    if (is_inmod) {
	        auto colwidth = ((var & SELECTBITS(4, 59)) >> 59);
	        auto col = ((var & SELECTBITS(13, 46)) >> 46);
	        auto rowwidth = ((var & SELECTBITS(4, 42)) >> 42);
	        auto row = ((var & SELECTBITS(13, 29)) >> 29);
	        ATH_MSG_DEBUG("modid: " << std::hex << std::setfill('0') << std::setw(8) << modid << std::setfill(' ') << std::dec << " csv: " << col << "," << colwidth << "," << row << "," << rowwidth);
	    }
	}
#undef SELECTBITS
    }

    return StatusCode::SUCCESS;
}
