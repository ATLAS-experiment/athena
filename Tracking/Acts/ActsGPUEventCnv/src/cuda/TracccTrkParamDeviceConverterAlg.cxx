/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccTrkParamDeviceConverterAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <memory>

namespace ActsTrk {

StatusCode TracccTrkParamDeviceConverterAlg::initialize()
{
  ATH_CHECK(m_mrTool.retrieve());
  ATH_CHECK(m_copiesTool.retrieve());
  ATH_CHECK(m_streamTool.retrieve());

  ATH_CHECK(m_inputTrackParamsKey.initialize());
  ATH_CHECK(m_inputSPKey.initialize());
  ATH_CHECK(m_inputSeedsKey.initialize());
  ATH_CHECK(m_outputPackedTrkParamsKey.initialize());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode TracccTrkParamDeviceConverterAlg::execute(const EventContext& ctx) const
{
  // ---- Read input
  auto trackParams = SG::makeHandle(m_inputTrackParamsKey, ctx);
  ATH_CHECK(trackParams.isValid());
  auto spacepoints = SG::makeHandle(m_inputSPKey, ctx);
  ATH_CHECK(spacepoints.isValid());
  auto seeds = SG::makeHandle(m_inputSeedsKey, ctx);
  ATH_CHECK(seeds.isValid());

  auto device_copy = m_copiesTool->deviceCopy(ctx);
  cudaStream_t cuda_stream = static_cast<cudaStream_t>(m_streamTool->stream(ctx));

  // The seed buffer is resizable, its size is only known on device
  const unsigned int n_seeds = device_copy->get_size(*seeds);
  const unsigned int n_params = device_copy->get_size(*trackParams);
  if (n_params != n_seeds) {
    ATH_MSG_ERROR("Got " << n_params << " track parameters in " << m_inputTrackParamsKey.key()
                  << " but " << n_seeds << " seeds in " << m_inputSeedsKey.key());
    return StatusCode::FAILURE;
  }

  // ---- Pack the track parameters on device
  auto packedTrkParams = std::make_unique<TracccTrkParamBuffer>(n_seeds, m_mrTool->mainMR());
  device_copy->setup(*packedTrkParams)->ignore();

  const cudaError_t launchError = packTrkParams(
    cuda_stream, n_seeds, *trackParams, *spacepoints, *seeds, *packedTrkParams);
  if (launchError != cudaSuccess) {
    ATH_MSG_ERROR("Failed to launch the track parameter packing kernel ("
                  << cudaGetErrorString(launchError) << ")");
    return StatusCode::FAILURE;
  }
  // Wait for the kernel to finish, so that the buffer can be used on any stream
  const cudaError_t syncError = cudaStreamSynchronize(cuda_stream);
  if (syncError != cudaSuccess) {
    ATH_MSG_ERROR("Failed to pack the track parameters ("
                  << cudaGetErrorString(syncError) << ")");
    return StatusCode::FAILURE;
  }

  // ---- Register to SG
  auto packedTrkParamsHandle = SG::makeHandle(m_outputPackedTrkParamsKey, ctx);
  ATH_CHECK(packedTrkParamsHandle.record(std::move(packedTrkParams)));

  ATH_MSG_DEBUG("Packed " << n_seeds << " packed track parameters into " << m_outputPackedTrkParamsKey.key());
  m_nSeeds += n_seeds;

  return StatusCode::SUCCESS;
}

StatusCode TracccTrkParamDeviceConverterAlg::finalize()
{
  ATH_MSG_DEBUG("Packed the track parameters of " << m_nSeeds << " seeds in total");
  return StatusCode::SUCCESS;
}

} // namespace ActsTrk
