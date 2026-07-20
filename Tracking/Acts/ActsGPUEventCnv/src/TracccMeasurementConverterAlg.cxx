/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccMeasurementConverterAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "AthAllocators/DataPool.h"
#include "InDetPrepRawData/SiWidth.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include "TrkSurfaces/Surface.h"

#include <stdexcept>

namespace ActsTrk {

StatusCode TracccMeasurementConverterAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing.");

  ATH_CHECK(m_hostMR.retrieve());
  ATH_CHECK(m_copy.retrieve());

  ATH_CHECK(m_inputMeasKey.initialize());
  ATH_CHECK(m_outputPixelKey.initialize());
  ATH_CHECK(m_outputStripKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_pixelID,      "PixelID"));
  ATH_CHECK(detStore()->retrieve(m_stripID,       "SCT_ID"));
  ATH_CHECK(detStore()->retrieve(m_pixelManager));
  ATH_CHECK(detStore()->retrieve(m_stripManager));

  m_detrayToAthena = &m_detDescSvc->detrayToAthenaMap();

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode TracccMeasurementConverterAlg::execute(const EventContext& ctx) const
{
  // ---- Read input ----
  auto measurements = SG::makeHandle(m_inputMeasKey, ctx);
  ATH_CHECK(measurements.isValid());

  auto copy = m_copy->copy(ctx);
  traccc::edm::measurement_collection::host meas_host{m_hostMR->mr()};
  (*copy)(*measurements, meas_host)->wait();
  ATH_MSG_DEBUG("Copied " << meas_host.size() << " measurements.");

  // ---- Count pixel vs strip ----
  int n_pixels = 0;
  int n_strips  = 0;
  for (std::size_t i = 0; i < meas_host.size(); ++i) {
    if (meas_host[i].dimensions() == 2u) ++n_pixels;
    else                                 ++n_strips;
  }

  // ---- Create output containers ----
  DataPool<xAOD::PixelCluster> pixel_pool(ctx);
  DataPool<xAOD::StripCluster> strip_pool(ctx);

  auto pixel_cont = std::make_unique<xAOD::PixelClusterContainer>(
      SG::VIEW_ELEMENTS, SG::ALWAYS_TRACK_INDICES);
  auto pixel_aux  = std::make_unique<xAOD::PixelClusterAuxContainer>();
  auto strip_cont = std::make_unique<xAOD::StripClusterContainer>(
      SG::VIEW_ELEMENTS, SG::ALWAYS_TRACK_INDICES);
  auto strip_aux  = std::make_unique<xAOD::StripClusterAuxContainer>();

  pixel_pool.reserve(n_pixels);
  strip_pool.reserve(n_strips);

  pixel_cont->push_new(n_pixels,
      [&pixel_pool]() { return pixel_pool.nextElementPtr(); });
  strip_cont->push_new(n_strips,
      [&strip_pool]() { return strip_pool.nextElementPtr(); });

  pixel_aux->resize(pixel_cont->size());
  strip_aux->resize(strip_cont->size());

  pixel_cont->setStore(pixel_aux.get());
  strip_cont->setStore(strip_aux.get());

  // ---- Convert ----
  auto pixItr   = pixel_cont->begin();
  auto stripItr = strip_cont->begin();

  size_t pixel_idx = 0;
  size_t strip_idx = 0;

  for (std::size_t i = 0; i < meas_host.size(); ++i) {
    const auto& meas = meas_host[i];
    const uint64_t detrayId = meas.surface_link().value();
    const Identifier athenaId = m_detrayToAthena->at(detrayId);

    if (meas.dimensions() == 2u) {
      // ---- Pixel ----
      xAOD::PixelCluster* xaod_pcl = *pixItr; ++pixItr;

      const IdentifierHash pixHash = m_pixelID->wafer_hash(athenaId);
      const InDetDD::SiDetectorElement* pDE =
          m_pixelManager->getDetectorElement(pixHash);

      const Amg::Vector2D localPos(meas.local_position()[0],
                                   meas.local_position()[1]);
      const Amg::Vector3D gp = pDE->globalPosition(localPos);

      Eigen::Matrix<float, 2, 1> localPosition(localPos.x(), localPos.y());
      Eigen::Matrix<float, 2, 2> localCovariance =
          Eigen::Matrix<float, 2, 2>::Zero();
      localCovariance(0, 0) = meas.local_variance()[0];
      localCovariance(1, 1) = meas.local_variance()[1];

      const Identifier hitId(athenaId.get_compact() + pixel_idx + strip_idx);

      xaod_pcl->setMeasurement<2>(pixHash, localPosition, localCovariance);
      xaod_pcl->globalPosition() =
          Eigen::Matrix<float, 3, 1>(gp.x(), gp.y(), gp.z());
      xaod_pcl->setIdentifier(hitId.get_compact());
      xaod_pcl->setWidthInEta(meas.diameter());

      ++pixel_idx;

    } else {
      // ---- Strip ----
      xAOD::StripCluster* xaod_scl = *stripItr; ++stripItr;

      const IdentifierHash stripHash = m_stripID->wafer_hash(athenaId);
      const InDetDD::SiDetectorElement* pDE =
          m_stripManager->getDetectorElement(stripHash);

      Eigen::Matrix<float, 1, 1> localPosition = Eigen::Matrix<float, 1, 1>::Zero();
      Eigen::Matrix<float, 1, 1> localCovariance = Eigen::Matrix<float, 1, 1>::Zero();

      if (pDE->isBarrel()) {
        localPosition(0, 0)  = meas.local_position()[0];
        localCovariance(0, 0) = meas.local_variance()[0];
      } else {
        localPosition(0, 0)  = meas.local_position()[1];
        localCovariance(0, 0) = meas.local_variance()[1];
      }

      const Identifier hitId(athenaId.get_compact() + pixel_idx + strip_idx);

      xaod_scl->setMeasurement<1>(stripHash, localPosition, localCovariance);
      xaod_scl->setIdentifierHash(stripHash);
      xaod_scl->setIdentifier(hitId.get_compact());
      ++strip_idx;
    }
  }

  ATH_MSG_DEBUG("Converted " << pixel_idx << " pixel clusters, "
                              << strip_idx  << " strip clusters");

  m_nMeas += meas_host.size();
  m_nPix  += pixel_idx;
  m_nStrip += strip_idx;

  // ---- Write outputs ----
  SG::WriteHandle<xAOD::PixelClusterContainer> pixelHandle{m_outputPixelKey, ctx};
  ATH_CHECK(pixelHandle.record(std::move(pixel_cont), std::move(pixel_aux)));

  SG::WriteHandle<xAOD::StripClusterContainer> stripHandle{m_outputStripKey, ctx};
  ATH_CHECK(stripHandle.record(std::move(strip_cont), std::move(strip_aux)));

  return StatusCode::SUCCESS;
}

StatusCode TracccMeasurementConverterAlg::finalize()
{
  ATH_MSG_DEBUG("Finalizing.");

  ATH_MSG_DEBUG("Received total number of measurements = " << m_nMeas
                  << ", of which pixel clusters = " << m_nPix
                  << " and strip clusters = " << m_nStrip);

  ATH_MSG_DEBUG("Successfully finalized");
  return StatusCode::SUCCESS;
}

} // namespace ActsTrk
