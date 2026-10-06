/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccMeasurementConverterAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "ActsGeometryInterfaces/GeometryDefs.h"
#include "AthAllocators/DataPool.h"
#include "InDetPrepRawData/SiWidth.h"
#include "ActsGeometryInterfaces/GeometryDefs.h"

#include "TrkSurfaces/Surface.h"

#include <stdexcept>

namespace ActsTrk {

StatusCode TracccMeasurementConverterAlg::initialize()
{
    ATH_MSG_DEBUG("Initializing.");

    ATH_CHECK(m_hostMR.retrieve());
    ATH_CHECK(m_copy.retrieve());

    ATH_CHECK(m_inputMeasKey.initialize());
    ATH_CHECK(m_inputClusterKey.initialize(m_convertClustersWithCells));
    ATH_CHECK(m_inputCellsKey.initialize(m_convertClustersWithCells));

    ATH_CHECK(m_outputPixelKey.initialize());
    ATH_CHECK(m_outputPixelSpacePointsKey.initialize());
    ATH_CHECK(m_outputMeasToPixelSPKey.initialize());
    ATH_CHECK(m_outputMeasToStripClKey.initialize());
    ATH_CHECK(m_outputStripKey.initialize());

    ATH_CHECK(detStore()->retrieve(m_pixelID, m_idHelperName) );
    ATH_CHECK(detStore()->retrieve(m_stripID,       "SCT_ID"));
    ATH_CHECK(detStore()->retrieve(m_pixelManager, "ITkPixel"));
    ATH_CHECK(detStore()->retrieve(m_stripManager, "ITkStrip"));

    ATH_CHECK(detStore()->retrieve(m_idMapping, m_geoIdMappingObjectName.value()));

    ATH_MSG_DEBUG("Successfully initialized");
    return StatusCode::SUCCESS;
}

/// Count the number of measurements 2D (pixel) vs 1D (strip)
std::pair<int, int> countPixelStrip(
  const traccc::edm::measurement_collection::const_device& measurements) {
  int n_pixels = 0;
  int n_strips = 0;
  for (std::size_t i = 0; i < measurements.size(); ++i) {
    if (measurements.at(i).dimensions() == 2u) {
      ++n_pixels;
    } else {
      ++n_strips;
    }
  }
  return {n_pixels, n_strips};
}

// Allocate `n` clusters from the pool into a view container + aux store.
template <typename ClusterT, typename AuxContainerT,
          typename ContainerT = DataVector<ClusterT>>
          std::pair<std::unique_ptr<ContainerT>, std::unique_ptr<AuxContainerT>>
          makeOutputContainer(const EventContext& ctx, int n) {

  DataPool<ClusterT> pool(ctx);

  auto cont = std::make_unique<ContainerT>(SG::VIEW_ELEMENTS,
                                           SG::ALWAYS_TRACK_INDICES);
  auto aux = std::make_unique<AuxContainerT>();

  pool.reserve(n);
  cont->push_new(n, [&pool]() { return pool.nextElementPtr(); });
  aux->resize(cont->size());
  cont->setStore(aux.get());

  return {std::move(cont), std::move(aux)};
}

template <typename MEAS>
void fillStripCluster(const MEAS& meas, const Identifier& athenaId,
                      const SCT_ID* stripID,
                      const InDetDD::SCT_DetectorManager* stripManager,
                      xAOD::StripCluster& xaod_scl,
                      std::size_t combinedIndex) {
  const IdentifierHash stripHash = stripID->wafer_hash(athenaId);
  const InDetDD::SiDetectorElement* pDE =
      stripManager->getDetectorElement(stripHash);

  Eigen::Matrix<float, 1, 1> localPosition = Eigen::Matrix<float, 1, 1>::Zero();
  Eigen::Matrix<float, 1, 1> localCovariance =
      Eigen::Matrix<float, 1, 1>::Zero();

  if (pDE->isBarrel()) {
    localPosition(0, 0) = meas.local_position()[0];
    localCovariance(0, 0) = meas.local_variance()[0];
  } else {
    localPosition(0, 0) = meas.local_position()[1];
    localCovariance(0, 0) = meas.local_variance()[1];
  }

  const Identifier hitId(athenaId.get_compact() + combinedIndex);

  xaod_scl.setMeasurement<1>(stripHash, localPosition, localCovariance);
  xaod_scl.setIdentifierHash(stripHash);
  xaod_scl.setIdentifier(hitId.get_compact());
}

template <typename MEAS>
void fillPixelCluster(const MEAS& meas, const Identifier& athenaId,
                      const PixelID* pixelID,
                      const InDetDD::PixelDetectorManager* pixelManager,
                      xAOD::PixelCluster& xaod_pcl,
                      std::size_t combinedIndex) {
  const IdentifierHash pixHash = pixelID->wafer_hash(athenaId);
  const InDetDD::SiDetectorElement* pDE =
      pixelManager->getDetectorElement(pixHash);

  const Amg::Vector2D localPos(meas.local_position()[0],
                               meas.local_position()[1]);
  const Amg::Vector3D gp = pDE->globalPosition(localPos);

  Eigen::Matrix<float, 2, 1> localPosition(localPos.x(), localPos.y());
  Eigen::Matrix<float, 2, 2> localCovariance =
      Eigen::Matrix<float, 2, 2>::Zero();
  localCovariance(0, 0) = meas.local_variance()[0];
  localCovariance(1, 1) = meas.local_variance()[1];

  const Identifier hitId(athenaId.get_compact() + combinedIndex);

  xaod_pcl.setIdentifierHash(pixHash);
  xaod_pcl.setMeasurement<2>(pixHash, localPosition, localCovariance);
  xaod_pcl.globalPosition() = Eigen::Matrix<float, 3, 1>(gp.x(), gp.y(), gp.z());
  xaod_pcl.setIdentifier(hitId.get_compact());
  xaod_pcl.setWidthInEta(meas.diameter());
}

StatusCode TracccMeasurementConverterAlg::execute(const EventContext& ctx) const
{
  // ---- Retrieve measurements (always needed) ----
  auto measurements = SG::makeHandle(m_inputMeasKey, ctx);
  ATH_CHECK(measurements.isValid());

  auto copy = m_copy->copy(ctx);

  traccc::edm::measurement_collection::buffer traccc_measurements_buffer{
      copy->get_size(*measurements), m_hostMR->mr()};
  copy->setup(traccc_measurements_buffer)->ignore();
  (*copy)(*measurements, traccc_measurements_buffer)->wait();

  traccc::edm::measurement_collection::const_device traccc_measurements(
      traccc_measurements_buffer);

  // ---- Optionally retrieve clusters and cells ----
  std::optional<traccc::edm::silicon_cluster_collection::buffer> traccc_clusters_buffer;
  std::optional<traccc::edm::silicon_cell_collection::buffer> traccc_cells_buffer;
  std::optional<traccc::edm::silicon_cluster_collection::const_device> traccc_clusters;
  std::optional<traccc::edm::silicon_cell_collection::const_device> traccc_cells;

  // ---- Create mapping from traccc measurement index to pixel spacepoint index ----
  // for pixel the index maps to cluster container or spacepoint containe
  // because every pixel measurement makes one spacepoint
  std::vector<unsigned int> measToPixelSP(traccc_measurements.size(),
                                         std::numeric_limits<unsigned int>::max());

  // ---- Create mapping from traccc measurement index to strip cluster index ----    
  // every strip measurement does not map to one strip spacepoint therefore this map 
  // is only valid between traccc measurements and strip clusters
  // The reason why we have two maps is that during traccc track conversion
  // we need to figure out if a track state is coming from pixel or strip
  // since we don't want to copy/write to SG the measurements twice
  // we can figure this out by creating two maps at conversion, 
  // then if the meas index is not in pixel map, has to be a strip meas                                     
  std::vector<unsigned int> measToStripCl(traccc_measurements.size(),
                                          std::numeric_limits<unsigned int>::max()); 
  if (m_convertClustersWithCells) {
    ATH_MSG_DEBUG("Read " << traccc_measurements.size() << " traccc measurements from " << m_inputMeasKey.key());
    ATH_MSG_DEBUG("Read traccc clusters from '"
                  << m_inputClusterKey.key()
                  << "' and will convert them with associated cells.");

    SG::ReadHandle<traccc::edm::silicon_cluster_collection::buffer> clusters{
        m_inputClusterKey, ctx};
    ATH_CHECK(clusters.isValid());
    SG::ReadHandle<traccc::edm::silicon_cell_collection::buffer> cells{
        m_inputCellsKey, ctx};
    ATH_CHECK(cells.isValid());

    traccc_clusters_buffer.emplace(copy->get_sizes(*clusters), m_hostMR->mr());
    copy->setup(*traccc_clusters_buffer)->wait();
    (*copy)(*clusters, *traccc_clusters_buffer)->wait();

    traccc_cells_buffer.emplace(copy->get_size(*cells), m_hostMR->mr());
    copy->setup(*traccc_cells_buffer)->wait();
    (*copy)(*cells, *traccc_cells_buffer)->wait();

    traccc_clusters.emplace(*traccc_clusters_buffer);
    ATH_MSG_DEBUG("Copied " << traccc_clusters->size() << " clusters.");

    traccc_cells.emplace(*traccc_cells_buffer);
    ATH_MSG_DEBUG("Copied " << traccc_cells->size() << " cells.");
  } else {
    ATH_MSG_DEBUG("Read traccc measurements from '"
                  << m_inputMeasKey.key()
                  << "' and will convert them without associated cells.");
  }

  // ---- Count pixel vs strip ----
  const auto [n_pixels, n_strips] = countPixelStrip(traccc_measurements);

  // ---- Create output containers ----
  auto [pixel_cont, pixel_aux] =
    makeOutputContainer<xAOD::PixelCluster, xAOD::PixelClusterAuxContainer>(ctx, n_pixels);
  auto [pixel_spacepoint_cont, pixel_spacepoint_aux] =
    makeOutputContainer<xAOD::SpacePoint, xAOD::SpacePointAuxContainer>(ctx, n_pixels);
  auto [strip_cont, strip_aux] =
    makeOutputContainer<xAOD::StripCluster, xAOD::StripClusterAuxContainer>(ctx, n_strips);

  // ---- Convert ----
  auto pixItr = pixel_cont->begin();
  auto spItr = pixel_spacepoint_cont->begin();
  auto stripItr = strip_cont->begin();

  std::size_t pixel_idx = 0;
  std::size_t strip_idx = 0;

  for (std::size_t i = 0; i < traccc_measurements.size(); ++i) {
    const auto& meas = traccc_measurements.at(i);
    const uint64_t detrayId = meas.surface_link().value();
    auto athenaIdOpt = m_idMapping->detrayToAthena(detrayId);
    if (!athenaIdOpt.has_value()) {
        ATH_MSG_FATAL("No Athena module found for detray id " << detrayId << " — skipping measurement.");
        return StatusCode::FAILURE;  
    }
    const Identifier athenaId = *athenaIdOpt;

    // ---- Pixel ----
    if (meas.dimensions() == 2u) {
      xAOD::PixelCluster* xaod_pcl = *pixItr;
      xAOD::SpacePoint* xaod_sp = *spItr;
      ++spItr;
      ++pixItr;

      // once seeding will come into play, we will need to keep track of the
      // mapping between traccc and xAOD clusters. The reason for this is:
      // 1. the traccc measurements are all in one collection, and the
      // xAOD clusters are in two separate collections (pixel and strip)
      // 2. the traccc spacepoints are not not created in the same order as the traccc measurements
      measToPixelSP[i] = pixel_idx;

      const IdentifierHash Pixel_ModuleHash =
        m_pixelID->wafer_hash(athenaId);
      const InDetDD::SiDetectorElement* pDE =
        m_pixelManager->getDetectorElement(Pixel_ModuleHash);
      const InDetDD::PixelModuleDesign* design(
        dynamic_cast<const InDetDD::PixelModuleDesign*>(
            &pDE->design()));
      if(design == nullptr){
        ATH_MSG_FATAL("Could not retrieve module design for pixel module hash " << Pixel_ModuleHash);
        return StatusCode::FAILURE;
      }

      fillPixelCluster(meas, athenaId, m_pixelID, m_pixelManager, *xaod_pcl,
                         pixel_idx + strip_idx);

      if (m_convertClustersWithCells) {
        const auto cluster = traccc_clusters->at(meas.cluster_index());

        std::vector<Identifier> rdoList;
        rdoList.reserve(cluster.cell_indices().size());
        int phiIndicesMax = -1;
        int etaIndicesMax = -1;
        int phiIndicesMin = std::numeric_limits<int>::max();
        int etaIndicesMin = std::numeric_limits<int>::max();

        for (const unsigned int cell_idx : cluster.cell_indices()) {
            const auto& cell = traccc_cells->at(cell_idx);

            int const phiIndex = cell.channel0();
            int const etaIndex = cell.channel1();

            phiIndicesMax = std::max(phiIndicesMax, phiIndex);
            etaIndicesMax = std::max(etaIndicesMax, etaIndex);
            phiIndicesMin = std::min(phiIndicesMin, phiIndex);
            etaIndicesMin = std::min(etaIndicesMin, etaIndex);

            Identifier const hit_id =
                m_pixelID->pixel_id(athenaId, phiIndex, etaIndex);
            rdoList.push_back(hit_id);
        }

        double const colWidth = static_cast<double>((etaIndicesMax - etaIndicesMin) + 1);
        double const rowWidth = static_cast<double>((phiIndicesMax - phiIndicesMin) + 1);
        double const etaWidth =
            design->widthFromColumnRange(etaIndicesMin, etaIndicesMax);
        double const phiWidth = design->widthFromRowRange(phiIndicesMin, phiIndicesMax);

        InDet::SiWidth siWidth(Amg::Vector2D(rowWidth, colWidth),
                                Amg::Vector2D(phiWidth, etaWidth));

        float width0 = phiWidth / rowWidth;
        float width1 = etaWidth / colWidth;
        Eigen::Matrix<float, 2, 2> localCovariance =
            Eigen::Matrix<float, 2, 2>::Zero();
        localCovariance(0, 0) = width0 * width0 / 12.0f;
        localCovariance(1, 1) = width1 * width1 / 12.0f;

        Eigen::Matrix<float, 2, 1> localPosition(meas.local_position()[0], meas.local_position()[1]);
        xaod_pcl->setMeasurement<2>(Pixel_ModuleHash, localPosition,
                                        localCovariance);

        std::sort(rdoList.begin(), rdoList.end());
        xaod_pcl->setIdentifier(rdoList.front().get_compact());
        xaod_pcl->setRDOlist(std::move(rdoList));
        xaod_pcl->setChannelsInPhiEta(siWidth.colRow()[0],
                                    siWidth.colRow()[1]);
        float width_phiRZ = static_cast<float>(siWidth.widthPhiRZ()[1]);
        xaod_pcl->setWidthInEta(width_phiRZ);

        // using width to scale the cluster covariance for space points
        float covTerm = width_phiRZ * width_phiRZ * (1/12.0f);
        
        if( covTerm < localCovariance(1, 1) )
            covTerm = localCovariance(1, 1);

        const Amg::Transform3D& Tp = pDE->surface().transform();
        float const cov_z =
            6.f * covTerm *
            static_cast<float>(Tp(0, 2) * Tp(0, 2) + Tp(1, 2) * Tp(1, 2));
        float const cov_r = 6.f * covTerm *
                            static_cast<float>(Tp(2, 2) * Tp(2, 2));
        xaod_sp->setSpacePoint(xaod_pcl->identifierHash(),
                               xaod_pcl->globalPosition(), cov_r, cov_z,
                               {xaod_pcl});

      }
      ++pixel_idx;

    // ---- Strip ----
    } else {
      xAOD::StripCluster* xaod_scl = *stripItr;
      ++stripItr;

      // see comment above for pixel clusters, the same applies here
      // when we eventually have strip spacepoints
      
      const IdentifierHash Strip_ModuleHash =
        m_stripID->wafer_hash(athenaId);
      const InDetDD::SiDetectorElement* pDE =
        m_stripManager->getDetectorElement(Strip_ModuleHash);

      fillStripCluster(meas, athenaId, m_stripID, m_stripManager, *xaod_scl,
                         pixel_idx + strip_idx);

      if (m_convertClustersWithCells) {
        const auto cluster = traccc_clusters->at(meas.cluster_index());

        std::vector<Identifier> rdoList;
        rdoList.reserve(cluster.cell_indices().size());
        int phiIndicesMax = -1;
        int phiIndicesMin = std::numeric_limits<int>::max();

        for (const unsigned int cell_idx : cluster.cell_indices()) {
            const auto& cell = traccc_cells->at(cell_idx);

            // (m_stripID->barrel_ec(athenaId) != 0) <=> endcap strip
            // => we are working with polar coordinates (r,phi)
            // and we have to swap the indices
            int const phiIndex = (m_stripID->barrel_ec(athenaId) != 0)
                                    ? cell.channel1()
                                    : cell.channel0();

            phiIndicesMax = std::max(phiIndicesMax, phiIndex);
            phiIndicesMin = std::min(phiIndicesMin, phiIndex);
            Identifier const hit_id =
                m_stripID->strip_id(athenaId, int(phiIndex));
            rdoList.push_back(hit_id);
        }

        std::sort(rdoList.begin(), rdoList.end());
        const int firstStrip = m_stripID->strip(rdoList.front());
        const int lastStrip = m_stripID->strip(rdoList.back());

        const InDetDD::SCT_ModuleSideDesign* design;
        if (pDE->isBarrel()) {
            design = (static_cast<const InDetDD::SCT_ModuleSideDesign*>(
                &pDE->design()));
        } else {
            design = (static_cast<const InDetDD::StripStereoAnnulusDesign*>(
                &pDE->design()));
        }

        const int row = m_stripID->row(rdoList.front());
        const int firstStrip1D = design->strip1Dim(firstStrip, row);
        const int lastStrip1D = design->strip1Dim(lastStrip, row);
        const InDetDD::SiCellId cell1(firstStrip1D);
        const InDetDD::SiCellId cell2(lastStrip1D);
        const InDetDD::SiLocalPosition firstStripPos(
            pDE->rawLocalPositionOfCell(cell1));
        const InDetDD::SiLocalPosition lastStripPos(
            pDE->rawLocalPositionOfCell(cell2));
        const InDetDD::SiLocalPosition centre(
              (firstStripPos + lastStripPos) * 0.5);
        const std::pair<InDetDD::SiLocalPosition, InDetDD::SiLocalPosition>
              ends(design->endsOfStrip(centre));
        const double stripLength(
              std::abs(ends.first.xEta() - ends.second.xEta()));
        const double width =
              design->stripPitch() * (lastStrip - firstStrip + 1);

        double const phiWidth = static_cast<double>((phiIndicesMax - phiIndicesMin) + 1);
        InDet::SiWidth siWidth(Amg::Vector2D(phiWidth, 1),
                                  Amg::Vector2D(width, stripLength));

        xaod_scl->setIdentifier(rdoList.front().get_compact());
        xaod_scl->setRDOlist(std::move(rdoList));
        xaod_scl->setChannelsInPhi(siWidth.colRow()[0]);

      }
      measToStripCl[i] = strip_idx;
      ++strip_idx;
    }
  }

  ATH_MSG_DEBUG("Converted " << pixel_idx << " pixel clusters, " << strip_idx
                             << " strip clusters");

  m_nMeas += traccc_measurements.size();
  m_nPix += pixel_idx;
  m_nStrip += strip_idx;

  // ---- Write outputs ----
  SG::WriteHandle<xAOD::PixelClusterContainer> pixelHandle{m_outputPixelKey,
                                                           ctx};
  ATH_CHECK(pixelHandle.record(std::move(pixel_cont), std::move(pixel_aux)));

  SG::WriteHandle<xAOD::SpacePointContainer> spacePointHandle{m_outputPixelSpacePointsKey,
                                                              ctx};
  ATH_CHECK(spacePointHandle.record(std::move(pixel_spacepoint_cont), std::move(pixel_spacepoint_aux)));

  SG::WriteHandle<std::vector<unsigned int>> measToPixelSPHandle{m_outputMeasToPixelSPKey, ctx};
  ATH_CHECK(measToPixelSPHandle.record(std::make_unique<std::vector<unsigned int>>(std::move(measToPixelSP))));

  SG::WriteHandle<std::vector<unsigned int>> measToStripClHandle{m_outputMeasToStripClKey, ctx};
  ATH_CHECK(measToStripClHandle.record(std::make_unique<std::vector<unsigned int>>(std::move(measToStripCl))));

  SG::WriteHandle<xAOD::StripClusterContainer> stripHandle{m_outputStripKey,
                                                           ctx};
  ATH_CHECK(stripHandle.record(std::move(strip_cont), std::move(strip_aux)));

  ATH_MSG_DEBUG("Wrote clusters to " << m_outputPixelKey.key() << " and  " << m_outputStripKey.key());

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
