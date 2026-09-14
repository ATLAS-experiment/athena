/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODToTracccSpacePointConverterAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "xAODInDetMeasurement/SpacePoint.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

#include <limits>
#include <memory>

namespace ActsTrk {

StatusCode xAODToTracccSpacePointConverterAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing.");

  if (m_inputSpacePointKeys.empty()) {
    ATH_MSG_FATAL("No input space point containers configured");
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_inputSpacePointKeys.initialize());
  ATH_CHECK(m_inputMeasToClusterKey.initialize());

  ATH_CHECK(m_outputSPKey.initialize());
  ATH_CHECK(m_outputSPContainerKey.initialize());
  ATH_CHECK(m_outputSPIndexKey.initialize());

  ATH_CHECK(m_hostMR.retrieve());
  ATH_CHECK(m_deviceMR.retrieve());
  ATH_CHECK(m_copiesTool.retrieve());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode xAODToTracccSpacePointConverterAlg::execute(const EventContext& ctx) const
{
  using size_type = traccc::edm::spacepoint_collection::buffer::size_type;
  constexpr unsigned int invalidIndex = std::numeric_limits<unsigned int>::max();

  auto measToCluster = SG::makeHandle(m_inputMeasToClusterKey, ctx);
  ATH_CHECK(measToCluster.isValid());

  std::vector<unsigned int> clusterToMeas;
  for (unsigned int measIdx = 0; measIdx < measToCluster->size(); ++measIdx) {
    const unsigned int clusterIdx = (*measToCluster)[measIdx];
    if (clusterIdx == invalidIndex) continue;
    if (clusterIdx >= clusterToMeas.size()) clusterToMeas.resize(clusterIdx + 1, invalidIndex);
    clusterToMeas[clusterIdx] = measIdx;
  }

  std::vector<SG::ReadHandle<xAOD::SpacePointContainer>> spacePointHandles =
      m_inputSpacePointKeys.makeHandles(ctx);
  size_type nSpacePoints = 0;
  for (auto& handle : spacePointHandles) {
    ATH_CHECK(handle.isValid());
    nSpacePoints += handle->size();
  }
  ATH_MSG_DEBUG("Found " << nSpacePoints << " space points in "
                << spacePointHandles.size() << " containers");

  auto hostCopy = m_copiesTool->hostCopy(ctx);
  traccc::edm::spacepoint_collection::buffer spHostBuffer{nSpacePoints, m_hostMR->mr()};
  hostCopy->setup(spHostBuffer)->wait();

  // Create a "device" collection around the buffer to work on it
  traccc::edm::spacepoint_collection::device spacepoints{spHostBuffer};

  auto spToContainer = std::make_unique<std::vector<unsigned int>>(nSpacePoints, invalidIndex);
  auto spToIndex = std::make_unique<std::vector<unsigned int>>(nSpacePoints, invalidIndex);

  // The traccc buffers are not default initialized: all members must be set.
  auto measurementIndexOf = [&](const xAOD::UncalibratedMeasurement* m) -> unsigned int {
    if (m == nullptr || m->index() >= clusterToMeas.size()) return invalidIndex;
    return clusterToMeas[m->index()];
  };

  xAOD::UncalibMeasType spacePointType = xAOD::UncalibMeasType::Other;
  size_type spIndex = 0;
  for (unsigned int c = 0; c < spacePointHandles.size(); ++c) {
    const xAOD::SpacePointContainer& container = *spacePointHandles[c];
    for (unsigned int j = 0; j < container.size(); ++j) {
      const xAOD::SpacePoint* xsp = container[j];
      const auto& xmeas = xsp->measurements();
      if (xmeas.empty() || xmeas.front() == nullptr) {
        ATH_MSG_FATAL("Space point " << j << " in '" << m_inputSpacePointKeys[c].key()
                      << "' has no associated measurements");
        return StatusCode::FAILURE;
      }

      const xAOD::UncalibMeasType type = xmeas.front()->type();
      if (spacePointType == xAOD::UncalibMeasType::Other) {
        spacePointType = type;
      } else if (type != spacePointType) {
        ATH_MSG_FATAL("Space point " << j << " in '" << m_inputSpacePointKeys[c].key()
                      << "' is of a different type than the previous space points, "
                      << "all input space points must be either pixel or strip");
        return StatusCode::FAILURE;
      }

      const unsigned int idx1 = measurementIndexOf(xmeas[0]);
      const unsigned int idx2 = (xmeas.size() > 1) ? measurementIndexOf(xmeas[1])
          : traccc::edm::spacepoint_collection::device::INVALID_MEASUREMENT_INDEX;
      if (idx1 == invalidIndex || (xmeas.size() > 1 && idx2 == invalidIndex)) {
        ATH_MSG_FATAL("Space point " << j << " in '" << m_inputSpacePointKeys[c].key()
                      << "' references a cluster not covered by '" << m_inputMeasToClusterKey.key() << "'");
        return StatusCode::FAILURE;
      }

      auto sp = spacepoints.at(spIndex);
      sp.measurement_index_1() = idx1;
      sp.measurement_index_2() = idx2;
      sp.global() = {xsp->x(), xsp->y(), xsp->z()};
      sp.z_variance() = xsp->varianceZ();
      sp.radius_variance() = xsp->varianceR();

      (*spToContainer)[spIndex] = c;
      (*spToIndex)[spIndex] = j;
      ++spIndex;
    }
  }

  auto deviceCopy = m_copiesTool->deviceCopy(ctx);
  auto spDeviceBuffer = std::make_unique<traccc::edm::spacepoint_collection::buffer>(
      spHostBuffer.capacity(), m_deviceMR->mr());

  // We ignore() the setup and wait() on the copy to allow parallelism.
  deviceCopy->setup(*spDeviceBuffer)->ignore();
  (*deviceCopy)(spHostBuffer, *spDeviceBuffer)->wait();

  auto spHandle = SG::makeHandle(m_outputSPKey, ctx);
  ATH_CHECK(spHandle.record(std::move(spDeviceBuffer)));
  auto spToContainerHandle = SG::makeHandle(m_outputSPContainerKey, ctx);
  ATH_CHECK(spToContainerHandle.record(std::move(spToContainer)));
  auto spToIndexHandle = SG::makeHandle(m_outputSPIndexKey, ctx);
  ATH_CHECK(spToIndexHandle.record(std::move(spToIndex)));

  m_nSpacePoints += nSpacePoints;

  ATH_MSG_DEBUG("Wrote " << nSpacePoints << " spacepoints to '"
                << m_outputSPKey.key() << "'");
  return StatusCode::SUCCESS;
}

StatusCode xAODToTracccSpacePointConverterAlg::finalize()
{
  ATH_MSG_DEBUG("Finalizing.");

  ATH_MSG_INFO("Converted total number of space points = " << m_nSpacePoints);

  return StatusCode::SUCCESS;
}

} // namespace ActsTrk
