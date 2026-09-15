/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODToTracccMeasurementConverterAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <numeric>

namespace ActsTrk {

StatusCode xAODToTracccMeasurementConverterAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing.");

  ATH_CHECK(m_inputPixelClustersKey.initialize());
  ATH_CHECK(m_inputStripClustersKey.initialize());

  ATH_CHECK(m_outputMeasKey.initialize());
  ATH_CHECK(m_outputMeasToPixelKey.initialize());
  ATH_CHECK(m_outputMeasToStripKey.initialize());

  ATH_CHECK(m_MRs.retrieve());
  ATH_CHECK(m_copiesTool.retrieve());

  ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));
  ATH_CHECK(detStore()->retrieve(m_stripID, "SCT_ID"));
  ATH_CHECK(detStore()->retrieve(m_geoIdMapping, m_geoIdMappingObjectName.value()));
  ATH_CHECK(detStore()->retrieve(m_hostCond, m_hostCondObjectName.value()));
  ATH_CHECK(detStore()->retrieve(m_hostDesign, m_hostDesignObjectName.value()));

  // NOTE: m_detrayIdToCondIndex is built once here;
  // meaning it is only valid as long as the geometry does not change.
  const auto& gids = m_hostCond->geometry_id();
  m_detrayIdToCondIndex.reserve(gids.size());
  for (unsigned int i = 0; i < gids.size(); ++i) {
    m_detrayIdToCondIndex[gids[i].value()] = i;
  }
  ATH_MSG_INFO("Built detray→detcond map with "
      << m_detrayIdToCondIndex.size() << " entries");

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode xAODToTracccMeasurementConverterAlg::condIndexFor(
    const Identifier& moduleId, unsigned int& condIndex) const
{
  const auto detrayIdOpt = m_geoIdMapping->athenaToDetray(moduleId);
  if (!detrayIdOpt.has_value()) {
    ATH_MSG_ERROR("No detray id found for Athena identifier " << moduleId);
    return StatusCode::FAILURE;
  }
  const auto it = m_detrayIdToCondIndex.find(*detrayIdOpt);
  if (it == m_detrayIdToCondIndex.end()) {
    ATH_MSG_ERROR("No detray conditions entry found for detray id " << *detrayIdOpt);
    return StatusCode::FAILURE;
  }
  condIndex = it->second;
  return StatusCode::SUCCESS;
}

StatusCode xAODToTracccMeasurementConverterAlg::execute(const EventContext& ctx) const
{
  using size_type = traccc::edm::measurement_collection::buffer::size_type;
  constexpr unsigned int invalidIndex = std::numeric_limits<unsigned int>::max();

  auto pixelClusters = SG::makeHandle(m_inputPixelClustersKey, ctx);
  ATH_CHECK(pixelClusters.isValid());
  auto stripClusters = SG::makeHandle(m_inputStripClustersKey, ctx);
  ATH_CHECK(stripClusters.isValid());

  const unsigned int nPixel = pixelClusters->size();
  const unsigned int nStrip = stripClusters->size();
  const size_type nMeas = nPixel + nStrip;
  ATH_MSG_DEBUG("Found " << nPixel << " pixel clusters and " << nStrip
                << " strip clusters, total " << nMeas << " clusters");

  std::vector<MeasurementRecord> records;
  records.reserve(nMeas);

  for (const xAOD::PixelCluster* cl : *pixelClusters) {
    MeasurementRecord rec;
    rec.isPixel = true;
    rec.hostIndex = cl->index();
    const Identifier moduleId = m_pixelID->wafer_id(IdentifierHash{cl->identifierHash()});
    ATH_CHECK(condIndexFor(moduleId, rec.condIndex));
    rec.geometryId = m_hostCond->geometry_id()[rec.condIndex].value();
    const auto locPos = cl->localPosition<2>();
    const auto locCov = cl->localCovariance<2>();
    rec.localPosition = {locPos(0, 0), locPos(1, 0)};
    rec.localVariance = {locCov(0, 0), locCov(1, 1)};
    rec.diameter = cl->widthInEta();
    records.push_back(rec);
  }

  for (const xAOD::StripCluster* cl : *stripClusters) {
    MeasurementRecord rec;
    rec.isPixel = false;
    rec.hostIndex = cl->index();
    const Identifier moduleId = m_stripID->wafer_id(IdentifierHash{cl->identifierHash()});
    ATH_CHECK(condIndexFor(moduleId, rec.condIndex));
    rec.geometryId = m_hostCond->geometry_id()[rec.condIndex].value();

    // The measured coordinate is given by the module subspace, the other
    // coordinate is set to the centre of the module design
    const unsigned int designIdx = m_hostCond->module_to_design_id()[rec.condIndex];
    const unsigned int measuredAxis = static_cast<unsigned int>(m_hostDesign->subspace()[designIdx][0]);
    const unsigned int otherAxis = (measuredAxis == 0u) ? 1u : 0u;
    const auto& otherEdges = (otherAxis == 0u) ? m_hostDesign->bin_edges_x()[designIdx]
                                               : m_hostDesign->bin_edges_y()[designIdx];
    float otherCentre = 0.f;
    float otherVariance = 0.f;
    if (!otherEdges.empty()) {
      const float width = otherEdges.back() - otherEdges.front();
      otherCentre = 0.5f * (otherEdges.front() + otherEdges.back());
      otherVariance = width * width / 12.f;
    }

    const auto locPos = cl->localPosition<1>();
    const auto locCov = cl->localCovariance<1>();
    rec.localPosition[measuredAxis] = locPos(0, 0);
    rec.localVariance[measuredAxis] = locCov(0, 0);
    rec.localPosition[otherAxis] = otherCentre;
    rec.localVariance[otherAxis] = otherVariance;
    records.push_back(rec);
  }

  // The traccc track finding requires the measurements to be sorted by surface
  std::vector<unsigned int> order(records.size());
  std::iota(order.begin(), order.end(), 0u);
  std::sort(order.begin(), order.end(), [&records](unsigned int lhs, unsigned int rhs) {
    const MeasurementRecord& a = records[lhs];
    const MeasurementRecord& b = records[rhs];
    if (a.geometryId != b.geometryId) return a.geometryId < b.geometryId;
    if (a.localPosition[0] != b.localPosition[0]) return a.localPosition[0] < b.localPosition[0];
    return a.localPosition[1] < b.localPosition[1];
  });

  // Without a separate host memory resource the main memory resource is host accessible
  std::pmr::memory_resource* hostMR = m_MRs->hostMR();
  auto hostCopy = m_copiesTool->hostCopy(ctx);
  auto measHostBuffer = std::make_unique<traccc::edm::measurement_collection::buffer>(
      nMeas, hostMR ? *hostMR : m_MRs->mainMR());
  hostCopy->setup(*measHostBuffer)->wait();

  // Create a "device" collection around the buffer to work on it
  traccc::edm::measurement_collection::device measurements{*measHostBuffer};

  auto measToPixel = std::make_unique<std::vector<unsigned int>>(nMeas, invalidIndex);
  auto measToStrip = std::make_unique<std::vector<unsigned int>>(nMeas, invalidIndex);

  // The traccc buffers are not default initialized: all members must be set.
  for (size_type i = 0; i < nMeas; ++i) {
    const MeasurementRecord& rec = records[order[i]];
    const unsigned int designIdx = m_hostCond->module_to_design_id()[rec.condIndex];

    auto meas = measurements.at(i);
    meas.local_position() = rec.localPosition;
    meas.local_variance() = rec.localVariance;
    meas.dimensions() = static_cast<unsigned int>(m_hostDesign->dimensions()[designIdx]);
    meas.time() = 0.f;
    meas.diameter() = rec.diameter;
    meas.identifier() = i;
    meas.surface_link() = m_hostCond->geometry_id()[rec.condIndex];
    meas.set_subspace(m_hostDesign->subspace()[designIdx]);
    meas.cluster_index() = rec.hostIndex;

    if (rec.isPixel) {
      (*measToPixel)[i] = rec.hostIndex;
    } else {
      (*measToStrip)[i] = rec.hostIndex;
    }
  }

  auto measHandle = SG::makeHandle(m_outputMeasKey, ctx);
  if (hostMR) {
    auto deviceCopy = m_copiesTool->deviceCopy(ctx);
    auto measDeviceBuffer = std::make_unique<traccc::edm::measurement_collection::buffer>(
        measHostBuffer->capacity(), m_MRs->mainMR());

    // We ignore() the setup and wait() on the copy to allow parallelism.
    deviceCopy->setup(*measDeviceBuffer)->ignore();
    (*deviceCopy)(*measHostBuffer, *measDeviceBuffer)->wait();
    ATH_CHECK(measHandle.record(std::move(measDeviceBuffer)));
  } else {
    ATH_CHECK(measHandle.record(std::move(measHostBuffer)));
  }

  auto measToPixelHandle = SG::makeHandle(m_outputMeasToPixelKey, ctx);
  ATH_CHECK(measToPixelHandle.record(std::move(measToPixel)));
  auto measToStripHandle = SG::makeHandle(m_outputMeasToStripKey, ctx);
  ATH_CHECK(measToStripHandle.record(std::move(measToStrip)));

  m_nPixel += nPixel;
  m_nStrip += nStrip;

  ATH_MSG_DEBUG("Wrote " << nMeas << " measurements to '"
                << m_outputMeasKey.key() << "'");
  return StatusCode::SUCCESS;
}

StatusCode xAODToTracccMeasurementConverterAlg::finalize()
{
  ATH_MSG_DEBUG("Finalizing.");

  ATH_MSG_INFO("Converted total number of pixel clusters = " << m_nPixel
      << " and total number of strip clusters = " << m_nStrip);

  return StatusCode::SUCCESS;
}

} // namespace ActsTrk
