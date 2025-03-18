/**
 * @file src/FPGAStripClustering.cxx
 * @brief Implementation of strip clustering using FPGA acceleration.
 */

#include "FPGAStripClustering.h"
#include <CL/cl_ext.h>
#include <iomanip>

StatusCode FPGAStripClustering::initialize() {
    ATH_CHECK(m_stripRDOKey.initialize());
    ATH_CHECK(m_FPGADataFormatTool.retrieve());

    ATH_CHECK(IntegrationBase::precheck({m_xclbin, m_kernelName, m_inputTV, m_refTV}));
    ATH_CHECK(IntegrationBase::initialize());
    ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));

    return StatusCode::SUCCESS;
}

StatusCode FPGAStripClustering::execute(const EventContext &ctx) const {
    ATH_MSG_DEBUG("Executing StripClustering for event slot: " << ctx.slot());

    // Handle input data
    auto stripRDOHandle = SG::makeHandle(m_stripRDOKey, ctx);
    if (!stripRDOHandle.isValid()) {
        ATH_MSG_ERROR("Failed to retrieve input data.");
        return StatusCode::FAILURE;
    }

    std::vector<uint64_t> outputData;
    if (!m_FPGADataFormatTool->convertStripHitsToFPGADataFormat(*stripRDOHandle, outputData, ctx)) {
        ATH_MSG_ERROR("Failed to convert input data to FPGA-compatible format.");
        return StatusCode::FAILURE;
    }

    // Debug input data
    ATH_MSG_DEBUG("Encoded strip data size: " << outputData.size());
    int line = 0;
    for (const auto& var : outputData) {
        ATH_MSG_DEBUG("EncodedData[" << std::dec << std::setw(4) << line << "] = 0x" 
                      << std::hex << std::setfill('0') << std::setw(16) << var << std::setfill(' '));
        line++;
    }

    // Prepare output vector (same size as input data)
    std::vector<uint64_t> kernelOutput(outputData.size(), 0);

    // Work with the FPGA accelerator
    cl_int err = 0;

    // Allocate buffers on the accelerator
    cl::Buffer inputBuffer(m_context, CL_MEM_READ_ONLY, sizeof(uint64_t) * outputData.size(), nullptr, &err);
    cl::Buffer outputBuffer(m_context, CL_MEM_WRITE_ONLY, sizeof(uint64_t) * kernelOutput.size(), nullptr, &err);

    // Prepare and configure the kernel
    cl::Kernel kernel(m_program, m_kernelName.value().c_str(), &err);
    kernel.setArg(0, inputBuffer);
    kernel.setArg(1, outputBuffer);
    kernel.setArg(2, static_cast<unsigned int>(outputData.size()));

    // Command queue for the accelerator
    cl::CommandQueue queue(m_context, m_accelerator, 0, &err);

    // Enqueue write, execute kernel, and read back results
    queue.enqueueWriteBuffer(inputBuffer, CL_TRUE, 0, sizeof(uint64_t) * outputData.size(), outputData.data());
    queue.enqueueTask(kernel);
    queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, sizeof(uint64_t) * kernelOutput.size(), kernelOutput.data());
    queue.finish();

    // Debug output data
    ATH_MSG_DEBUG("Kernel execution completed.");
    line = 0;
    for (const auto& var : kernelOutput) {
        ATH_MSG_DEBUG("KernelOutput[" << std::dec << std::setw(4) << line << "] = 0x" 
                      << std::hex << std::setfill('0') << std::setw(16) << var << std::setfill(' '));
        line++;
    }

    return StatusCode::SUCCESS;
}