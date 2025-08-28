/**
 * @brief Implementation of strip clustering using FPGA acceleration.
 */

#include "EFTrackingFPGAPipeline/FPGAStripClustering.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"
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
    std::vector<IdentifierHash> listOfIds;
    std::vector<uint64_t> outputData;
    if (!m_FPGADataFormatTool->convertStripHitsToFPGADataFormat(*stripRDOHandle, outputData, listOfIds, ctx)) {
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
    std::vector<uint64_t> kernelEDMOutput(outputData.size()*10, 0);
    std::vector<uint64_t> kernel_L2GOutput(outputData.size(), 0);
    std::vector<uint64_t> kernel_L2GEDMOutput(outputData.size()*10, 0);
    std::vector<uint32_t> kernel_EDMPrepOutputPixel(outputData.size(), 0);
    // Create host side output vectors
    std::vector<uint64_t> pixelOutput(EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE, 0);
    std::vector<uint64_t> stripOutput(EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE, 0);

    ATH_MSG_DEBUG("strip buffer size: "<<std::dec <<EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE);

    // Work with the FPGA accelerator
    cl_int err = 0;

    // Allocate buffers on the accelerator
    cl::Buffer inputBuffer(m_context, CL_MEM_READ_ONLY, sizeof(uint64_t) * outputData.size(), nullptr, &err);
    cl::Buffer outputBuffer(m_context, CL_MEM_READ_WRITE, sizeof(uint64_t) * kernelOutput.size(), nullptr, &err);
    cl::Buffer outputEDMBuffer(m_context, CL_MEM_READ_WRITE, sizeof(uint64_t) * kernelEDMOutput.size(), nullptr, &err);

    cl::Buffer output_L2GBuffer(m_context, CL_MEM_WRITE_ONLY, sizeof(uint64_t) * kernel_L2GOutput.size(), nullptr, &err);
    cl::Buffer output_L2GEDMBuffer(m_context, CL_MEM_READ_WRITE, sizeof(uint64_t) * kernel_L2GEDMOutput.size(), nullptr, &err);


    cl::Buffer inputBufferPixel(m_context, CL_MEM_READ_ONLY, sizeof(uint64_t) * outputData.size(), nullptr, &err);
    cl::Buffer output_EDMPrepBufferPixel(m_context, CL_MEM_WRITE_ONLY, sizeof(uint64_t) * kernel_EDMPrepOutputPixel.size(), nullptr, &err);
    
    cl::Buffer stripbuffer(m_context, CL_MEM_READ_WRITE, stripOutput.size() * sizeof(uint64_t), NULL, &err);

    // Prepare and configure the kernel
    cl::Kernel kernel(m_program, m_kernelName.value().c_str(), &err);
    kernel.setArg(0, inputBuffer);
    kernel.setArg(1, outputBuffer);
    kernel.setArg(2, outputEDMBuffer);
    kernel.setArg(3, static_cast<unsigned int>(outputData.size()));

    cl::Kernel kernel_L2G(m_program, "l2g_strip_tool", &err);
    kernel_L2G.setArg(0, outputBuffer);
    kernel_L2G.setArg(1, outputEDMBuffer);
    kernel_L2G.setArg(2, output_L2GBuffer);
    kernel_L2G.setArg(3, output_L2GEDMBuffer);

    cl::Kernel kernel_EMDPrep(m_program, "EDMPrep", &err);
    kernel_EMDPrep.setArg(0, inputBufferPixel);
    kernel_EMDPrep.setArg(1, output_L2GEDMBuffer);
    kernel_EMDPrep.setArg(2, output_EDMPrepBufferPixel);
    kernel_EMDPrep.setArg(3, stripbuffer);


    // Command queue for the accelerator
    cl::CommandQueue queue(m_context, m_accelerator, 0, &err);

    // Enqueue write, execute kernel, and read back results
    queue.enqueueWriteBuffer(inputBuffer, CL_TRUE, 0, sizeof(uint64_t) * outputData.size(), outputData.data());
    queue.enqueueTask(kernel);
    queue.finish();
    queue.enqueueTask(kernel_L2G);
    queue.finish();
    queue.enqueueTask(kernel_EMDPrep);
    queue.finish();
    queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, sizeof(uint64_t) * kernelOutput.size(), kernelOutput.data());
    queue.enqueueReadBuffer(outputEDMBuffer, CL_TRUE, 0, sizeof(uint64_t) * kernelEDMOutput.size(), kernelEDMOutput.data());
    queue.enqueueReadBuffer(output_L2GBuffer, CL_TRUE, 0, sizeof(uint64_t) * kernel_L2GOutput.size(), kernel_L2GOutput.data());
    queue.enqueueReadBuffer(output_L2GEDMBuffer, CL_TRUE, 0, sizeof(uint64_t) * kernel_L2GEDMOutput.size(), kernel_L2GEDMOutput.data());
    queue.enqueueReadBuffer(stripbuffer, CL_TRUE, 0, stripOutput.size() * sizeof(uint64_t), stripOutput.data());
    queue.finish();

    // Debug output data
    ATH_MSG_DEBUG("Kernel execution completed.");
    line = 0;
    for (const auto& var : kernelOutput) {
        if(var != 0) ATH_MSG_DEBUG("Clustering KernelOutput[" << std::dec << std::setw(4) << line << "] = 0x" 
                      << std::hex << std::setfill('0') << std::setw(16) << var << std::setfill(' '));
        line++;
    }
    line = 0;
    for (const auto& var : kernelEDMOutput) {
        if(var != 0) ATH_MSG_DEBUG("Clustering KernelEDMOutput[" << std::dec << std::setw(4) << line << "] = 0x" 
                      << std::hex << std::setfill('0') << std::setw(16) << var << std::setfill(' '));
        line++;
    }

    line = 0;
    for (const auto& var : kernel_L2GOutput) {
        if(var != 0) ATH_MSG_DEBUG("L2G KernelOutput[" << std::dec << std::setw(4) << line << "] = 0x" 
                      << std::hex << std::setfill('0') << std::setw(16) << var << std::setfill(' '));
        line++;
    }
    line = 0;
    for (const auto& var : kernel_L2GEDMOutput) {
        if(var != 0) ATH_MSG_DEBUG("L2g KernelEDMOutput[" << std::dec << std::setw(4) << line << "] = 0x" 
                      << std::hex << std::setfill('0') << std::setw(16) << var << std::setfill(' '));
        line++;
    }
    line = 0;
    for (const auto& var : stripOutput) {
        if(var != 0) ATH_MSG_DEBUG("EDM Prep kernel_EMDPrep[" << std::dec << std::setw(4) << line << "] = 0x" 
                      << std::hex << std::setfill('0') << std::setw(16) << var << std::setfill(' '));
        line++;
    }
    ATH_MSG_DEBUG("Total lines: "<<std::dec<<line<<std::endl);





    // use 32-bit point to access output
    uint64_t *stripClusters = (uint64_t *)stripOutput.data();

    unsigned int numStripClusters = stripClusters[0];
    ATH_MSG_DEBUG("numStripClusters: " << numStripClusters);


    std::unique_ptr<EFTrackingTransient::Metadata> metadata =
        std::make_unique<EFTrackingTransient::Metadata>();

    metadata->numOfStripClusters = numStripClusters;
    metadata->scRdoIndexSize = numStripClusters;

    EFTrackingTransient::StripClusterAuxInput scAux;

    // Declare a few vairiables to be used in the loop
    int row = 0;
    uint64_t rdo;
    int rdoCounter = 0;

    // Make strip cluster aux input
    {
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
         scAux.localPosition.push_back(std::bit_cast<double>(stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));

         ATH_MSG_DEBUG("Strip [" << std::dec << "] "<<row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8 << " " << std::hex << stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
         row = 8; // local covariance xx
         scAux.localCovariance.push_back(std::bit_cast<double>(stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));
         row = 9; // global x
         scAux.globalPosition.push_back(std::bit_cast<double>(stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));
         row = 10; // global y
         scAux.globalPosition.push_back(std::bit_cast<double>(stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));
         row = 11; // global z
         scAux.globalPosition.push_back(std::bit_cast<double>(stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));
         row = 12; // channels in phi
         scAux.channelsInPhi.push_back(stripClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);

         metadata->scRdoIndex[i] = rdoCounter;
     }
     // print out the strip cluster aux input
     if (msgLvl(MSG::DEBUG))
     {
         for (unsigned int i = 0; i < numStripClusters; i++)
         {
             ATH_MSG_DEBUG("Strip cluster " << i << " idHash: " << scAux.idHash[i]);
             ATH_MSG_DEBUG("Strip cluster " << i << std::hex << " id: " << scAux.id[i]);
             ATH_MSG_DEBUG("Strip cluster " << i << std::dec << " localPosition x: " << scAux.localPosition[i]);
             ATH_MSG_DEBUG("Strip cluster " << i << " localCovariance: " << scAux.localCovariance[i]);
             ATH_MSG_DEBUG("Strip cluster " << i << " globalPosition x: " << scAux.globalPosition[i * 3]);
             ATH_MSG_DEBUG("Strip cluster " << i << " globalPosition y: " << scAux.globalPosition[i * 3 + 1]);
             ATH_MSG_DEBUG("Strip cluster " << i << " globalPosition z: " << scAux.globalPosition[i * 3 + 2]);
             ATH_MSG_DEBUG("Strip cluster " << i << " channelsInPhi: " << scAux.channelsInPhi[i]);
             ATH_MSG_DEBUG("Strip cluster " << i << " rdoList size: " << metadata->scRdoIndex[i]);
         }
     }
 }




    return StatusCode::SUCCESS;
}
