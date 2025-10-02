/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "EFTrackingFPGAPipeline/PixelClustering.h"

#include "EFTrackingFPGAUtility/EFTrackingTransient.h"

#include <fstream>
#include <CL/cl_ext.h>

#define OCL_CHECK(err, call)								\
	call;										\
	if (err != 0) {									\
		ATH_MSG_DEBUG("Error calling " << #call << ", error code: " << err);	\
	}

StatusCode PixelClustering::initialize()
{
	ATH_CHECK(m_pixelRDOKey.initialize());

	ATH_CHECK(m_FPGADataFormatTool.retrieve());
	ATH_CHECK(m_xAODClusterMaker.retrieve());

	ATH_CHECK(IntegrationBase::precheck({m_xclbin, m_kernelName, m_inputTV, m_refTV}));
	ATH_CHECK(IntegrationBase::initialize());
	ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));

	return StatusCode::SUCCESS;
}

StatusCode PixelClustering::execute(const EventContext &ctx) const
{
	ATH_MSG_DEBUG("In execute(), event slot: " << ctx.slot());

	auto pixelRDOHandle = SG::makeHandle(m_pixelRDOKey, ctx);

	std::vector<uint64_t> inbufPixClustVec;
    std::vector<IdentifierHash> listOfIds;
	if(!m_FPGADataFormatTool->convertPixelHitsToFPGADataFormat(*pixelRDOHandle, inbufPixClustVec, listOfIds, ctx)) {
		return StatusCode::FAILURE;
	}

	unsigned int inbufPixClustSize = inbufPixClustVec.size();

	cl_int err;

	cl::CommandQueue queuePixClust{};
	OCL_CHECK(err, queuePixClust = cl::CommandQueue(m_context, m_accelerator, CL_QUEUE_PROFILING_ENABLE, &err));

	cl::Kernel kernelPixClust{};
	OCL_CHECK(err, kernelPixClust = cl::Kernel(m_program, "pixel_clustering_tool", &err));

	cl::Buffer clInbufPixClust{};
	std::unique_ptr<uint64_t, decltype(std::free) *> inbufPixClust(static_cast<uint64_t*>(aligned_alloc(64, sizeof(uint64_t) * inbufPixClustSize)), std::free);
	for (unsigned int i = 0; i < inbufPixClustSize; ++i) {
		inbufPixClust.get()[i] = inbufPixClustVec.at(i);
		ATH_MSG_DEBUG("inbufPixClust[" << std::setw(6) << i << "] = 0x" << std::hex << std::setfill('0') << std::setw(16) << inbufPixClust.get()[i] << std::setfill(' ') << std::dec);
	}
	// Clean inbufPixClustVec to save memory
	inbufPixClustVec.resize(1);
	inbufPixClustVec.clear();
	OCL_CHECK(err, clInbufPixClust = cl::Buffer(m_context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint64_t) * inbufPixClustSize, inbufPixClust.get(), &err));
	OCL_CHECK(err, err = kernelPixClust.setArg(0, clInbufPixClust));
	cl::Buffer clOutbufPixClust{};
	unsigned int outbufPixClustSize = inbufPixClustSize + 9164 + 6;
	std::unique_ptr<uint64_t, decltype(std::free) *> outbufPixClust(static_cast<uint64_t*>(aligned_alloc(64, sizeof(uint64_t) * outbufPixClustSize)), std::free);
	OCL_CHECK(err, clOutbufPixClust = cl::Buffer(m_context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint64_t) * outbufPixClustSize, outbufPixClust.get(), &err));
	OCL_CHECK(err, err = kernelPixClust.setArg(1, clOutbufPixClust));
	cl::Buffer clOutbufEDMPixClust{};
	unsigned int outbufEDMPixClustSize = inbufPixClustSize * 10 + 6;
	std::unique_ptr<uint64_t, decltype(std::free) *> outbufEDMPixClust(static_cast<uint64_t*>(aligned_alloc(64, sizeof(uint64_t) * outbufEDMPixClustSize)), std::free);
	OCL_CHECK(err, clOutbufEDMPixClust = cl::Buffer(m_context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint64_t) * outbufEDMPixClustSize, outbufEDMPixClust.get(), &err));
	OCL_CHECK(err, err = kernelPixClust.setArg(2, clOutbufEDMPixClust));

	ATH_MSG_INFO("Making clusters out of " << inbufPixClustSize - 6 << " channels over threshold");
	// Write data to accelerator
	auto write_start = std::chrono::high_resolution_clock::now();
	OCL_CHECK(err, err = queuePixClust.enqueueMigrateMemObjects({clInbufPixClust}, 0));
	OCL_CHECK(err, err = queuePixClust.finish());
	auto write_end = std::chrono::high_resolution_clock::now();

	// Run algorithm on accelerator
	OCL_CHECK(err, err = queuePixClust.enqueueTask(kernelPixClust));
	OCL_CHECK(err, err = queuePixClust.finish());
	auto compute_end = std::chrono::high_resolution_clock::now();

	// Read back result
	OCL_CHECK(err, err = queuePixClust.enqueueMigrateMemObjects({clOutbufPixClust}, CL_MIGRATE_MEM_OBJECT_HOST));
	OCL_CHECK(err, err = queuePixClust.enqueueMigrateMemObjects({clOutbufEDMPixClust}, CL_MIGRATE_MEM_OBJECT_HOST));
	OCL_CHECK(err, err = queuePixClust.finish());
	auto read_end = std::chrono::high_resolution_clock::now();

	auto total_latency = std::chrono::duration<float, std::micro>(read_end - write_start).count();
	auto write_latency = std::chrono::duration<float, std::micro>(write_end - write_start).count();
	auto compute_latency = std::chrono::duration<float, std::micro>(compute_end - write_end).count();
	auto read_latency = std::chrono::duration<float, std::micro>(read_end - compute_end).count();
	ATH_MSG_INFO("Total latency:" << total_latency << " us,  " << "Compute latency: " << compute_latency << " us");
	ATH_MSG_INFO("Write latency:" << write_latency << " us,  " << "Read latency: " << read_latency << " us");
	ATH_MSG_INFO("Clusters found, calculate coordinates");

	cl::CommandQueue queuePixCoords{};
	OCL_CHECK(err, queuePixCoords = cl::CommandQueue(m_context, m_accelerator, CL_QUEUE_PROFILING_ENABLE, &err));
	cl::Kernel kernelPixCoords{};
	OCL_CHECK(err, kernelPixCoords = cl::Kernel(m_program, "l2g_pixel_tool", &err));

	cl::Buffer clInbufPixCoords{};
	std::unique_ptr<uint64_t, decltype(std::free) *> inbufPixCoords(static_cast<uint64_t*>(aligned_alloc(64, sizeof(uint64_t) * outbufPixClustSize)), std::free);
	for (unsigned int i = 0; i < outbufPixClustSize; ++i) {
		ATH_MSG_DEBUG("outbufPixClust[" << std::setw(6) << i << "] = 0x" << std::hex << std::setfill('0') << std::setw(16) << outbufPixClust.get()[i] << std::setfill(' ') << std::dec);
		inbufPixCoords.get()[i] = outbufPixClust.get()[i];
	}
	OCL_CHECK(err, clInbufPixCoords = cl::Buffer(m_context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint64_t) * outbufPixClustSize, inbufPixCoords.get(), &err));
	OCL_CHECK(err, err = kernelPixCoords.setArg(0, clInbufPixCoords));

	cl::Buffer clInbufEDMPixCoords{};
	std::unique_ptr<uint64_t, decltype(std::free) *> inbufEDMPixCoords(static_cast<uint64_t*>(aligned_alloc(64, sizeof(uint64_t) * outbufEDMPixClustSize)), std::free);
	for (unsigned int i = 0; i < outbufEDMPixClustSize; ++i) {
		ATH_MSG_DEBUG("outbufEDMPixClust[" << std::setw(6) << i << "] = 0x" << std::hex << std::setfill('0') << std::setw(16) << outbufEDMPixClust.get()[i] << std::setfill(' ') << std::dec);
		inbufEDMPixCoords.get()[i] = outbufEDMPixClust.get()[i];
		// Resize EDM container to be not larger than neccessary
		if ((i > 2) && ((i - 3) % 10 == 0) && (((outbufEDMPixClust.get()[i] & (((1ULL << 8) - 1ULL) << 56)) >> 56) == 0xcd) && (((outbufEDMPixClust.get()[i - 1] & (((1ULL << 1) - 1ULL) << 25)) >> 25))) {
			outbufEDMPixClustSize = i + 3;
		}
	}
	OCL_CHECK(err, clInbufEDMPixCoords = cl::Buffer(m_context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint64_t) * outbufEDMPixClustSize, inbufEDMPixCoords.get(), &err));
	OCL_CHECK(err, err = kernelPixCoords.setArg(1, clInbufEDMPixCoords));

	cl::Buffer clOutbufPixCoords{};
	unsigned int outbufPixCoordsSize = inbufPixClustSize * 2 + 9164 + 6;
	std::unique_ptr<uint64_t, decltype(std::free) *> outbufPixCoords(static_cast<uint64_t*>(aligned_alloc(64, sizeof(uint64_t) * outbufPixCoordsSize)), std::free);
	OCL_CHECK(err, clOutbufPixCoords = cl::Buffer(m_context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint64_t) * outbufPixCoordsSize, outbufPixCoords.get(), &err));
	OCL_CHECK(err, err = kernelPixCoords.setArg(2, clOutbufPixCoords));
	cl::Buffer clOutbufEDMPixCoords{};
	std::unique_ptr<uint64_t, decltype(std::free) *> outbufEDMPixCoords(static_cast<uint64_t*>(aligned_alloc(64, sizeof(uint64_t) * outbufEDMPixClustSize)), std::free);
	OCL_CHECK(err, clOutbufEDMPixCoords = cl::Buffer(m_context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint64_t) * outbufEDMPixClustSize, outbufEDMPixCoords.get(), &err));
	OCL_CHECK(err, err = kernelPixCoords.setArg(3, clOutbufEDMPixCoords));

	// Write data to accelerator
	write_start = std::chrono::high_resolution_clock::now();
	OCL_CHECK(err, err = queuePixCoords.enqueueMigrateMemObjects({clInbufPixCoords}, 0));
	OCL_CHECK(err, err = queuePixCoords.enqueueMigrateMemObjects({clInbufEDMPixCoords}, 0));
	OCL_CHECK(err, err = queuePixCoords.finish());
	write_end = std::chrono::high_resolution_clock::now();

	// Run algorithm on accelerator
	OCL_CHECK(err, err = queuePixCoords.enqueueTask(kernelPixCoords));
	OCL_CHECK(err, err = queuePixCoords.finish());
	compute_end = std::chrono::high_resolution_clock::now();

	// Read back result
	OCL_CHECK(err, err = queuePixCoords.enqueueMigrateMemObjects({clOutbufPixCoords}, CL_MIGRATE_MEM_OBJECT_HOST));
	OCL_CHECK(err, err = queuePixCoords.enqueueMigrateMemObjects({clOutbufEDMPixCoords}, CL_MIGRATE_MEM_OBJECT_HOST));
	OCL_CHECK(err, err = queuePixCoords.finish());
	read_end = std::chrono::high_resolution_clock::now();

	total_latency = std::chrono::duration<float, std::micro>(read_end - write_start).count();
	write_latency = std::chrono::duration<float, std::micro>(write_end - write_start).count();
	compute_latency = std::chrono::duration<float, std::micro>(compute_end - write_end).count();
	read_latency = std::chrono::duration<float, std::micro>(read_end - compute_end).count();
	ATH_MSG_INFO("Total latency:" << total_latency << " us,  " << "Compute latency: " << compute_latency << " us");
	ATH_MSG_INFO("Write latency:" << write_latency << " us,  " << "Read latency: " << read_latency << " us");
	ATH_MSG_INFO("Coordinates calculated, transform to xAOD::PixelCluster compatible formats");

	for (unsigned int i = 0; i < outbufPixCoordsSize; ++i) {
		ATH_MSG_DEBUG("outbufPixCoords[" << std::setw(6) << i << "] = 0x" << std::hex << std::setfill('0') << std::setw(16) << outbufPixCoords.get()[i] << std::setfill(' ') << std::dec);
	}

	cl::CommandQueue queuePixEDMPrep{};
	OCL_CHECK(err, queuePixEDMPrep = cl::CommandQueue(m_context, m_accelerator, CL_QUEUE_PROFILING_ENABLE, &err));
	cl::Kernel kernelPixEDMPrep{};
	OCL_CHECK(err, kernelPixEDMPrep = cl::Kernel(m_program, "EDMPrep", &err));

	cl::Buffer clInbufEDMPixEDMPrep{};
	std::unique_ptr<uint64_t, decltype(std::free) *> inbufEDMPixEDMPrep(static_cast<uint64_t*>(aligned_alloc(64, sizeof(uint64_t) * outbufEDMPixClustSize)), std::free);
	for (unsigned int i = 0; i < outbufEDMPixClustSize; ++i) {
		inbufEDMPixEDMPrep.get()[i] = outbufEDMPixCoords.get()[i];
		ATH_MSG_DEBUG("inbufEDMPixEDMPrep[" << std::setw(6) << i << "] = 0x" << std::hex << std::setfill('0') << std::setw(16) << inbufEDMPixEDMPrep.get()[i] << std::setfill(' ') << std::dec);
	}
	OCL_CHECK(err, clInbufEDMPixEDMPrep = cl::Buffer(m_context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint64_t) * outbufEDMPixClustSize, inbufEDMPixEDMPrep.get(), &err));
	OCL_CHECK(err, err = kernelPixEDMPrep.setArg(0, clInbufEDMPixEDMPrep));

	cl::Buffer clOutbufEDMPixEDMPrep{};
	std::unique_ptr<uint64_t, decltype(std::free) *> outbufEDMPixEDMPrep(static_cast<uint64_t*>(aligned_alloc(512, sizeof(uint64_t) * EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE)), std::free);
	OCL_CHECK(err, clOutbufEDMPixEDMPrep = cl::Buffer(m_context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint64_t) * EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE, outbufEDMPixEDMPrep.get(), &err));
	OCL_CHECK(err, err = kernelPixEDMPrep.setArg(1, clOutbufEDMPixEDMPrep));

	// Write data to accelerator
	write_start = std::chrono::high_resolution_clock::now();
	OCL_CHECK(err, err = queuePixEDMPrep.enqueueMigrateMemObjects({clInbufEDMPixEDMPrep}, 0));
	OCL_CHECK(err, err = queuePixEDMPrep.finish());
	write_end = std::chrono::high_resolution_clock::now();

	// Run algorithm on accelerator
	OCL_CHECK(err, err = queuePixEDMPrep.enqueueTask(kernelPixEDMPrep));
	OCL_CHECK(err, err = queuePixEDMPrep.finish());
	compute_end = std::chrono::high_resolution_clock::now();

	// Read back result
	OCL_CHECK(err, err = queuePixEDMPrep.enqueueMigrateMemObjects({clOutbufEDMPixEDMPrep}, CL_MIGRATE_MEM_OBJECT_HOST));
	OCL_CHECK(err, err = queuePixEDMPrep.finish());
	read_end = std::chrono::high_resolution_clock::now();

	for (unsigned int i = 0; i < EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE; ++i)
		ATH_MSG_DEBUG("outbufEDMPixEDMPrep[" << std::setw(6) << i << "] = 0x" << std::hex << std::setfill('0') << std::setw(16) << outbufEDMPixEDMPrep.get()[i] << std::setfill(' ') << std::dec);

	total_latency = std::chrono::duration<float, std::micro>(read_end - write_start).count();
	write_latency = std::chrono::duration<float, std::micro>(write_end - write_start).count();
	compute_latency = std::chrono::duration<float, std::micro>(compute_end - write_end).count();
	read_latency = std::chrono::duration<float, std::micro>(read_end - compute_end).count();
	ATH_MSG_INFO("Total latency:" << total_latency << " us,  " << "Compute latency: " << compute_latency << " us");
	ATH_MSG_INFO("Write latency:" << write_latency << " us,  " << "Read latency: " << read_latency << " us");
	ATH_MSG_INFO("Received data for making xAOD::PixelCluster's");

	uint64_t* pixelClusters = (uint64_t*)outbufEDMPixEDMPrep.get();
	unsigned int numClusters = pixelClusters[0];
	ATH_MSG_INFO("Received " << numClusters << " clusters");

	write_start = std::chrono::high_resolution_clock::now();
	std::unique_ptr<EFTrackingTransient::Metadata> metadata = std::make_unique<EFTrackingTransient::Metadata>();

	metadata->numOfPixelClusters = numClusters;
	metadata->pcRdoIndexSize = numClusters;

	EFTrackingTransient::PixelClusterAuxInput pcAux;

	int rdoCounter, row;
	uint64_t rdo;
	for (unsigned int i = 0; i < numClusters; ++i) {
		ATH_MSG_DEBUG("New Cluster number " << i);
		rdoCounter = 0;
		row = 0;
		pcAux.idHash.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
		ATH_MSG_DEBUG("idHash = " << pcAux.idHash.back());

		row++;
		pcAux.id.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
		ATH_MSG_DEBUG("id = " << pcAux.id.back());

		row++;
		rdo = pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8];
		if (rdo) {
			pcAux.rdoList.push_back(rdo);
			rdoCounter++;
			ATH_MSG_DEBUG("rdo = " << pcAux.rdoList.back());
		}

		row++;
		rdo = pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8];
		if (rdo) {
			pcAux.rdoList.push_back(rdo);
			rdoCounter++;
			ATH_MSG_DEBUG("rdo = " << pcAux.rdoList.back());
		}

		row++;
		rdo = pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8];
		if (rdo) {
			pcAux.rdoList.push_back(rdo);
			rdoCounter++;
			ATH_MSG_DEBUG("rdo = " << pcAux.rdoList.back());
		}

		row++;
		rdo = pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8];
		if (rdo) {
			pcAux.rdoList.push_back(rdo);
			rdoCounter++;
			ATH_MSG_DEBUG("rdo = " << pcAux.rdoList.back());
		}

		row++;
		pcAux.localPosition.push_back(std::bit_cast<double>(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));
		ATH_MSG_DEBUG("local x = " << pcAux.localPosition.back());

		row++;
		pcAux.localPosition.push_back(std::bit_cast<double>(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));
		ATH_MSG_DEBUG("local y = " << pcAux.localPosition.back());

		row++;
		pcAux.localCovariance.push_back(std::bit_cast<double>(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));
		ATH_MSG_DEBUG("local covariance [0, 0] = " << pcAux.localCovariance.back());

		row++;
		pcAux.localCovariance.push_back(std::bit_cast<double>(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));
		ATH_MSG_DEBUG("local covariance [1, 1] = " << pcAux.localCovariance.back());

		row++;
		pcAux.globalPosition.push_back(std::bit_cast<double>(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));
		ATH_MSG_DEBUG("global x = " << pcAux.globalPosition.back());

		row++;
		pcAux.globalPosition.push_back(std::bit_cast<double>(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));
		ATH_MSG_DEBUG("global y = " << pcAux.globalPosition.back());

		row++;
		pcAux.globalPosition.push_back(std::bit_cast<double>(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));
		ATH_MSG_DEBUG("global z = " << pcAux.globalPosition.back());

		row++;
		pcAux.channelsInPhi.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
		ATH_MSG_DEBUG("channels in phi = " << pcAux.channelsInPhi.back());

		row++;
		pcAux.channelsInEta.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
		ATH_MSG_DEBUG("chanels in eta = " << pcAux.channelsInEta.back());

		row++;
		pcAux.widthInEta.push_back(std::bit_cast<double>(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]));
		ATH_MSG_DEBUG("width in eta = " << pcAux.widthInEta.back());

		row++;
		row++;
		row++;
		pcAux.totalToT.push_back(pixelClusters[row * EFTrackingTransient::MAX_NUM_CLUSTERS + i + 8]);
		ATH_MSG_DEBUG("total ToT = " << pcAux.totalToT.back());

		metadata->pcRdoIndex[i] = rdoCounter;
	}

	ATH_CHECK(m_xAODClusterMaker->makePixelClusterContainer(pcAux, metadata.get(), ctx));
	write_end = std::chrono::high_resolution_clock::now();
	write_latency = std::chrono::duration<float, std::micro>(write_end - write_start).count();
	ATH_MSG_INFO("xAOD::PixelClusterContainer made in " << write_latency << " us");

	return StatusCode::SUCCESS;
}

#undef OCL_CHECK
