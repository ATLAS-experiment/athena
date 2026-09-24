/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
#include <fstream>
#include <sstream>

#include "TritonTracccTrackMaker.h"

#include "CxxUtils/StringUtils.h"

#include <chrono>

StatusCode TritonTracccTrackMaker::initialize()
{
    // input handles / tools
    ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));
    ATH_CHECK(detStore()->retrieve(m_stripID, "SCT_ID"));

    ATH_CHECK(m_tracccCellsKey.initialize());

    // output container
    ATH_CHECK(m_ActsTracccTrackContainerKey.initialize());

    ATH_CHECK(m_tracksBackendHandlesHelper.initialize(
        ActsTrk::prefixFromTrackContainerName(
            m_ActsTracccTrackContainerKey.key())));

    // geometry helper for output
    if (!m_trackingGeometrySvc.empty()) {
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        m_trackingGeometry = m_trackingGeometrySvc->trackingGeometry();
        if (!m_trackingGeometry) {
            ATH_MSG_DEBUG("Null Acts::TrackingGeometry from tracking geometry service");
            return StatusCode::FAILURE;
        }

        m_detEleToGeoIdMap = m_trackingGeometrySvc->surfaceIdMap();
        if (!m_detEleToGeoIdMap) {
            ATH_MSG_DEBUG("Null detector element to Acts geometry ID map from "
                          "tracking geometry service");
            return StatusCode::FAILURE;
        }
    }

    // tools
    ATH_CHECK(m_tracccTrackingTool.retrieve());

    m_featureNamesVec = CxxUtils::tokenize(m_featureNames, ",");

    // Initialize condition handle keys
    ATH_CHECK(m_pixelDetEleCollKey.initialize());
    ATH_CHECK(m_stripDetEleCollKey.initialize());

    // Initialize cluster container keys
    ATH_CHECK(m_inputPixelClusterContainerKey.initialize());
    ATH_CHECK(m_inputStripClusterContainerKey.initialize());
    ATH_CHECK(m_xAODPixelClusterFromInDetClusterKey.initialize());
    ATH_CHECK(m_xAODStripClusterFromInDetClusterKey.initialize());
    ATH_CHECK(m_xAODSpacepointFromInDetClusterKey.initialize());

    // Initialize truth association keys if needed
    ATH_CHECK(m_pixelClustersToTruth.initialize(m_doTruth));
    ATH_CHECK(m_stripClustersToTruth.initialize(m_doTruth));

    ATH_CHECK(m_ctxProvider.initialize());

    return StatusCode::SUCCESS;
}

StatusCode TritonTracccTrackMaker::execute(const EventContext& ctx) const
{

    // fill cells struct for sending to traccc
    auto cell_start = std::chrono::high_resolution_clock::now();

    // Read the cells produced by RDOtoTracccCellConverterAlg.
    auto cells_handle = SG::makeHandle(m_tracccCellsKey, ctx);
    ATH_CHECK(cells_handle.isValid());

    traccc::edm::silicon_cell_collection::const_device cells{*cells_handle};

    std::vector<uint8_t> cells_buffer;
    ATH_CHECK(serializeCells(cells, cells_buffer));

    auto cell_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> cell_time = cell_end - cell_start;
    ATH_MSG_INFO("Cell reading time: " << cell_time.count() << " ms");  

    std::unordered_map<int64_t, int> cluster_map;
    if (m_doTruth)
    {
        cluster_map = readAndConvertClusters(ctx);
    }

    // create output containers
    std::vector<TracccTrackParameters> TracccTrackParams;
    std::vector<LocalMeasurementInfoInTracks> TracccMeasurementsInfoInTracks;

    // Run the inference 
    auto traccc_start = std::chrono::high_resolution_clock::now();
    ATH_CHECK(m_tracccTrackingTool->getTracks(cells_buffer, TracccTrackParams, TracccMeasurementsInfoInTracks));
    auto traccc_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> traccc_time = traccc_end - traccc_start;
    ATH_MSG_INFO("Traccc total inference time: " << traccc_time.count() << " ms");

    // convert to ACTs tracks
    unsigned int nb_output_tracks = 0;
    auto convert_start = std::chrono::high_resolution_clock::now();
    ATH_CHECK(convertTracks(ctx, TracccTrackParams, TracccMeasurementsInfoInTracks, 
                            cluster_map, nb_output_tracks));
    auto convert_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> convert_time = convert_end - convert_start;
    ATH_MSG_INFO("Traccc total conversion time: " << convert_time.count() << " ms");

    return StatusCode::SUCCESS;
}

// Serialize the traccc::cells object to a raw uint8 buffer
inline void appendBytes(std::vector<uint8_t>& out, const uint64_t& value)
{
    const auto* p = reinterpret_cast<const uint8_t*>(&value);
    out.insert(out.end(), p, p + sizeof(value));
}
inline void appendBytes(std::vector<uint8_t>& out, const uint32_t& value)
{
    const auto* p = reinterpret_cast<const uint8_t*>(&value);
    out.insert(out.end(), p, p + sizeof(value));
}
inline void appendBytes(std::vector<uint8_t>& out, const float& value)
{
    const auto* p = reinterpret_cast<const uint8_t*>(&value);
    out.insert(out.end(), p, p + sizeof(value));
}

StatusCode TritonTracccTrackMaker::serializeCells(
    const traccc::edm::silicon_cell_collection::const_device& cells,
    std::vector<uint8_t>& out) const
{
    const uint64_t nCells = cells.size();

    // Reserve: 8-byte header + 5 columns * (4 or 4-byte scalars) per cell.
    out.clear();
    out.reserve(sizeof(uint64_t) + nCells * (3 * sizeof(uint32_t) + 2 * sizeof(float)));

    // Header: number of cells.
    appendBytes(out, nCells);

    // SoA column blocks, in silicon_cell_collection column order.
    for (uint64_t i = 0; i < nCells; ++i) {
        appendBytes(out, static_cast<uint32_t>(cells.channel0()[i]));
    }
    for (uint64_t i = 0; i < nCells; ++i) {
        appendBytes(out, static_cast<uint32_t>(cells.channel1()[i]));
    }
    for (uint64_t i = 0; i < nCells; ++i) {
        appendBytes(out, static_cast<float>(cells.activation()[i]));
    }
    for (uint64_t i = 0; i < nCells; ++i) {
        appendBytes(out, static_cast<float>(cells.time()[i]));
    }
    for (uint64_t i = 0; i < nCells; ++i) {
        appendBytes(out, static_cast<uint32_t>(cells.module_index()[i]));
    }

    ATH_MSG_DEBUG("Serialized " << nCells << " traccc cells into "
                  << out.size() << " bytes");

    return StatusCode::SUCCESS;
}

std::unordered_map<int64_t, int> TritonTracccTrackMaker::readAndConvertClusters(
    const EventContext& eventContext) const
{
    int nPix = 0;
    int nStrip = 0;
    std::unordered_map<int64_t, int> traccc_to_xaod_cluster_map;

    ATH_MSG_INFO("Converting InDet clusters to xAOD");
    SG::WriteHandle<xAOD::PixelClusterContainer>
        xAODPixelContainerFromInDetClusters(
            m_xAODPixelClusterFromInDetClusterKey, eventContext);
    if ((xAODPixelContainerFromInDetClusters.record(
             std::make_unique<xAOD::PixelClusterContainer>(),
             std::make_unique<xAOD::PixelClusterAuxContainer>()))
            .isFailure()) {
        ATH_MSG_FATAL(
            "Could not record xAOD Pixel container from InDet Clusters");
        throw std::runtime_error(
            "creation of xAOD Pixel container from InDet Clusters failed");
    }

    SG::WriteHandle<xAOD::StripClusterContainer>
        xAODStripContainerFromInDetClusters(
            m_xAODStripClusterFromInDetClusterKey, eventContext);
    if ((xAODStripContainerFromInDetClusters.record(
             std::make_unique<xAOD::StripClusterContainer>(),
             std::make_unique<xAOD::StripClusterAuxContainer>()))
            .isFailure()) {
        ATH_MSG_FATAL(
            "Could not record xAOD Strip container from InDet Clusters");
        throw std::runtime_error(
            "creation of xAOD Strip container from InDet Clusters failed");
    }

    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(
        m_pixelDetEleCollKey, eventContext);

    const InDetDD::SiDetectorElementCollection* pixElements{};
    if (SG::get(pixElements, m_pixelDetEleCollKey, eventContext).isFailure() ||
        pixElements == nullptr) {
        ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
        std::ostringstream errMsg;
        errMsg << m_pixelDetEleCollKey.fullKey() << " is not available.";
        throw std::runtime_error(errMsg.str());
    }

    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleHandle(
        m_stripDetEleCollKey, eventContext);
    const InDetDD::SiDetectorElementCollection* stripElements(
        *stripDetEleHandle);
    if (not stripDetEleHandle.isValid() or stripElements == nullptr) {
        ATH_MSG_FATAL(m_stripDetEleCollKey.fullKey() << " is not available.");
        std::ostringstream errMsg;
        errMsg << m_stripDetEleCollKey.fullKey() << " is not available.";
        throw std::runtime_error(errMsg.str());
    }

    SG::WriteHandle<xAOD::SpacePointContainer>
        xAODSpacepointContainerFromInDetClusters(
            m_xAODSpacepointFromInDetClusterKey, eventContext);
    if (xAODSpacepointContainerFromInDetClusters
            .record(std::make_unique<xAOD::SpacePointContainer>(),
                    std::make_unique<xAOD::SpacePointAuxContainer>())
            .isFailure()) {
        throw std::runtime_error(
            "creation of InDet spacepoint containers failed");
    }

    ATH_MSG_DEBUG("Reading clusters");
    
    const InDet::PixelClusterContainer* inputPixelClusterContainer{};
    if (SG::get(inputPixelClusterContainer, m_inputPixelClusterContainerKey,
                eventContext)
            .isFailure() ||
        inputPixelClusterContainer == nullptr) {
        ATH_MSG_FATAL(m_inputPixelClusterContainerKey.fullKey()
                      << " is not available.");
        std::ostringstream errMsg;
        errMsg << m_inputPixelClusterContainerKey.fullKey()
               << " is not available.";
        throw std::runtime_error(errMsg.str());
    }

    for (const auto* const clusterCollection : *inputPixelClusterContainer) {
        if (!clusterCollection)
            continue;
        for (const auto* const theCluster : *clusterCollection) {

            const InDetDD::SiDetectorElement* element =
                theCluster->detectorElement();
            const Identifier Pixel_ModuleID = element->identify();

            auto pixelCl = xAODPixelContainerFromInDetClusters->push_back(
                                        std::make_unique<xAOD::PixelCluster>());
            if ((convertInDetToXaodCluster(*theCluster, *element, *pixelCl))
                    .isFailure()) {
                ATH_MSG_FATAL("Could not convert InDet pixel cluster to xAOD");
                throw std::runtime_error(
                    "conversion of InDet pixel cluster to xAOD failed");
            }

            auto xaod_sp = xAODSpacepointContainerFromInDetClusters->push_back(
                                            std::make_unique<xAOD::SpacePoint>());
            const IdentifierHash Pixel_ModuleHash =
                m_pixelID->wafer_hash(Pixel_ModuleID);

            xAOD::MeasVector<2> globalVariance(xAOD::MeasVector<2>::Zero());
            xAOD::MeasVector<3> globalPosition{xAOD::toStorage(theCluster->globalPosition())};

            xaod_sp->setSpacePoint(Pixel_ModuleHash, globalPosition,
                                   globalVariance(0, 0), globalVariance(1, 0),
                                   {pixelCl});

            size_t index = xAODPixelContainerFromInDetClusters->size() - 1;
            traccc_to_xaod_cluster_map[Pixel_ModuleID.get_compact()] = index;

            nPix++;
        }
    }
    ATH_MSG_DEBUG("Read " << nPix << " pixel clusters");

    const InDet::SCT_ClusterContainer* inputStripClusterContainer{};
    if (SG::get(inputStripClusterContainer, m_inputStripClusterContainerKey,
                eventContext)
            .isFailure() ||
        inputStripClusterContainer == nullptr) {
        ATH_MSG_FATAL(m_inputStripClusterContainerKey.fullKey()
                      << " is not available.");
        std::ostringstream errMsg;
        errMsg << m_inputStripClusterContainerKey.fullKey()
               << " is not available.";
        throw std::runtime_error(errMsg.str());
    }

    for (const auto* const clusterCollection : *inputStripClusterContainer) {
        if (!clusterCollection)
            continue;
        for (const auto* const theCluster : *clusterCollection) {

            const InDetDD::SiDetectorElement* element =
                theCluster->detectorElement();
            const Identifier Strip_ModuleID = element->identify();

            xAOD::StripCluster* stripCl = new xAOD::StripCluster();
            xAODStripContainerFromInDetClusters->push_back(stripCl);
            if ((convertInDetToXaodCluster(*theCluster, *element, *stripCl))
                    .isFailure()) {
                ATH_MSG_FATAL("Could not convert InDet strip cluster to xAOD");
                throw std::runtime_error(
                    "conversion of InDet strip cluster to xAOD failed");
            }
            size_t index = xAODStripContainerFromInDetClusters->size() - 1;
            traccc_to_xaod_cluster_map[Strip_ModuleID.get_compact()] = index;

            nStrip++;
        }
    }

    ATH_MSG_DEBUG("Read " << nStrip << " strip clusters");

    return traccc_to_xaod_cluster_map;
}

StatusCode TritonTracccTrackMaker::convertInDetToXaodCluster(
    const InDet::PixelCluster& indetCluster,
    const InDetDD::SiDetectorElement& element, xAOD::PixelCluster& xaodCluster) const
{
    IdentifierHash idHash = element.identifyHash();

    auto localPos = indetCluster.localPosition();
    auto localCov = indetCluster.localCovariance();

    xAOD::MeasVector<2> localPosition = xAOD::toStorage(localPos);

    xAOD::MeasMatrix<2> localCovariance;
    localCovariance.setZero();
    localCovariance(0, 0) = localCov(0, 0);
    localCovariance(1, 1) = localCov(1, 1);

    auto globalPos = indetCluster.globalPosition();
    Eigen::Matrix<float, 3, 1> globalPosition(globalPos.x(), globalPos.y(),
                                              globalPos.z());

    const auto& RDOs = indetCluster.rdoList();
    const auto& ToTs = indetCluster.totList();
    const auto& charges = indetCluster.chargeList();
    const auto& width = indetCluster.width();
    //coverity[UNINIT]
    xaodCluster.setMeasurement<2>(idHash, localPosition, localCovariance);
    xaodCluster.setIdentifier(indetCluster.identify().get_compact());
    xaodCluster.setRDOlist(RDOs);
    xaodCluster.globalPosition() = globalPosition;
    xaodCluster.setToTlist(ToTs);
    xaodCluster.setChargelist(charges);
    xaodCluster.setLVL1A(indetCluster.LVL1A());
    xaodCluster.setChannelsInPhiEta(width.colRow()[0], width.colRow()[1]);
    xaodCluster.setWidthInEta(static_cast<float>(width.widthPhiRZ()[1]));

    return StatusCode::SUCCESS;
}

StatusCode TritonTracccTrackMaker::convertInDetToXaodCluster(
    const InDet::SCT_Cluster& indetCluster,
    const InDetDD::SiDetectorElement& element, xAOD::StripCluster& xaodCluster) const
{
    constexpr double one_over_twelve = 1. / 12.;
    IdentifierHash idHash = element.identifyHash();

    auto localPos = indetCluster.localPosition();

    xAOD::MeasVector<1> localPosition;
    xAOD::MeasMatrix<1> localCovariance;
    localCovariance.setZero();

    if (element.isBarrel()) {
        localPosition(0, 0) = localPos.x();
        localCovariance(0, 0) =
            element.phiPitch() * element.phiPitch() * one_over_twelve;
    } else {
        InDetDD::SiCellId cellId = element.cellIdOfPosition(localPos);
        const InDetDD::StripStereoAnnulusDesign* design =
            dynamic_cast<const InDetDD::StripStereoAnnulusDesign*>(
                &element.design());
        if (design == nullptr) {
            return StatusCode::FAILURE;
        }
        InDetDD::SiLocalPosition localInPolar =
            design->localPositionOfCellPC(cellId);
        localPosition(0, 0) = localInPolar.xPhi();
        localCovariance(0, 0) =
            design->phiPitchPhi() * design->phiPitchPhi() * one_over_twelve;
    }

    auto globalPos = indetCluster.globalPosition();
    Eigen::Matrix<float, 3, 1> globalPosition(globalPos.x(), globalPos.y(),
                                              globalPos.z());

    const auto& RDOs = indetCluster.rdoList();
    const auto& width = indetCluster.width();
    //coverity[UNINIT]
    xaodCluster.setMeasurement<1>(idHash, localPosition, localCovariance);
    xaodCluster.setIdentifier(indetCluster.identify().get_compact());
    xaodCluster.setRDOlist(RDOs);
    xaodCluster.globalPosition() = globalPosition;
    xaodCluster.setChannelsInPhi(width.colRow()[0]);

    return StatusCode::SUCCESS;
}

const Acts::Surface* TritonTracccTrackMaker::actsSurfaceFromAtlasId(
    const Identifier& atlasID) const
{
    const bool isPixel = m_pixelID->is_pixel(atlasID);
    const IdentifierHash hash = isPixel ? m_pixelID->wafer_hash(atlasID)
                                        : m_stripID->wafer_hash(atlasID);
    const auto measType = isPixel ? xAOD::UncalibMeasType::PixelClusterType
                                  : xAOD::UncalibMeasType::StripClusterType;

    const auto geoKey = ActsTrk::makeDetectorElementKey(
        measType, static_cast<unsigned int>(hash));
    const auto it = m_detEleToGeoIdMap->find(geoKey);
    if (it != m_detEleToGeoIdMap->end()) {
        const Acts::Surface* surface = m_trackingGeometry->findSurface(
            ActsTrk::DetectorElementToActsGeometryIdMap::getValue(*it));
        if (surface) {
            return surface;
        }
    }
    ATH_MSG_DEBUG("No Acts surface corresponding to this ATLAS id: " << atlasID);
    return nullptr;
}

Acts::BoundMatrix TritonTracccTrackMaker::buildBoundCovariance(
    const LocalMeasurementInfoInTracks& state) const
{
    // traccc's model output is in Acts-native units
    Acts::BoundMatrix cov = Acts::BoundMatrix::Zero();
    for (unsigned int i = 0; i < 5; i++) {
        for (unsigned int j = 0; j < 5; j++) {
            size_t index = i * 5 + j;
            cov(i, j) = state.covariances[index];
        }
    }

    // traccc does not fit the time parameter (yet). Give it a
    // large placeholder uncertainty instead of leaving it at zero.
    constexpr double kUnconstrainedTimeVariance = 1e6;
    cov(Acts::eBoundTime, Acts::eBoundTime) = kUnconstrainedTimeVariance;

    return cov;
}

std::optional<Acts::BoundTrackParameters>
    TritonTracccTrackMaker::convertToActsParameters(
            const LocalMeasurementInfoInTracks& state) const
{
    using namespace Acts::UnitLiterals;
    std::shared_ptr<const Acts::Surface> actsSurface;
    Acts::BoundVector params{};

    Identifier const atlas_ID(static_cast<Identifier::value_type>(state.athena_id[0]));

    // get the associated surface
    const Acts::Surface* surface = actsSurfaceFromAtlasId(atlas_ID);
    if (!surface) {
        return std::nullopt;
    }
    actsSurface = surface->getSharedPtr();

    // Construct track parameters
    ATH_MSG_VERBOSE("Constructing track parameters for this state");
    params << state.local_x[0], state.local_y[0],
        state.phi[0], state.theta[0], state.qop[0], state.time[0];

    Acts::BoundMatrix const cov = buildBoundCovariance(state);

    Acts::ParticleHypothesis hypothesis{Acts::ParticleHypothesis::pion()};
    
    return Acts::BoundTrackParameters(std::move(actsSurface), params, cov, hypothesis);
}

StatusCode TritonTracccTrackMaker::convertTracks(
    EventContext const& eventContext,
    std::vector<TracccTrackParameters>& trackParams,
    std::vector<LocalMeasurementInfoInTracks>& measInfo,
    const std::unordered_map<int64_t, int>& cluster_map,
    unsigned& nb_output_tracks) const
{
    nb_output_tracks = 0;

    Acts::VectorTrackContainer track_backend;
    Acts::VectorMultiTrajectory track_state_backend;
    ActsTrk::MutableTrackContainer track_container(
        std::move(track_backend), std::move(track_state_backend));

    SG::WriteHandle<ActsTrk::TrackContainer> trackContainerHandle(
        m_ActsTracccTrackContainerKey, eventContext);

    Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(eventContext);

    if (m_doTruth) {
        ATH_MSG_DEBUG("Will map truth");
        ATH_MSG_DEBUG("retrieving cluster container keys: "
                        << m_xAODPixelClusterFromInDetClusterKey.key() << ","
                        << m_xAODStripClusterFromInDetClusterKey.key());
    }

    // Debug stats
    float chi2_min = std::numeric_limits<float>::max();
    float chi2_max = std::numeric_limits<float>::min();

    float ndf_min = std::numeric_limits<float>::max();
    float ndf_max = std::numeric_limits<float>::min();

    unsigned meas_min = std::numeric_limits<unsigned>::max();
    unsigned meas_max = std::numeric_limits<unsigned>::min();

    int excluded_ndf = 0;
    int excluded_no_sp = 0;
    int excluded_weird_state = 0;

    for (std::size_t i = 0; i < trackParams.size(); i++) {
        auto fit_res = trackParams.at(i);
        auto& states = measInfo.at(i);
        if (states.local_x.size() < 1) {
            excluded_no_sp += 1;
            continue;
        }

        // In Acts ndf, aka nDoF, is unsigned int. This makes sure the number is
        // safe to cast; exclude the track otherwise.
        if (fit_res.ndf >
                static_cast<float>(std::numeric_limits<unsigned int>::max()) ||
            fit_res.ndf <
                static_cast<float>(std::numeric_limits<unsigned int>::min())) {
            excluded_ndf += 1;
            continue;
        }

        // Create the MutableTrack and add the parameters
        auto actsTrack = track_container.makeTrack();
        enum TrackValidity { VALID, INVALID_STATE, INVALID_GLOBAL };
        TrackValidity track_validity = VALID;


        actsTrack.chi2() = fit_res.chi2;
        actsTrack.nDoF() = fit_res.ndf;

        Acts::TrackStatePropMask const mask =
            Acts::TrackStatePropMask::Smoothed;

        bool first_state = true;
        for (size_t j = 0; j < states.local_x.size(); ++j) {
            auto actsTSOS = actsTrack.appendTrackState(mask);

            // Build a measurement struct for this state
            LocalMeasurementInfoInTracks singleState;
            singleState.local_x = {states.local_x[j]};
            singleState.local_y = {states.local_y[j]};
            singleState.phi = {states.phi[j]};
            singleState.theta = {states.theta[j]};
            singleState.qop = {states.qop[j]};
            singleState.time = {states.time[j]};
            singleState.covariances = {};
            for (size_t k = 0; k < 25; ++k) {
                singleState.covariances.push_back(states.covariances[j * 25 + k]);
            }
            singleState.athena_id = {states.athena_id[j]};

            std::optional<Acts::BoundTrackParameters> params_opt =
                convertToActsParameters(singleState);
            if (!params_opt.has_value()) {
                // if the measurement was "buggy", the whole track is dismissed.
                ATH_MSG_DEBUG(
                    "convertToActsParameters failed: track state is weird");
                track_validity = INVALID_STATE;
                break;

            }

            Acts::BoundTrackParameters const& parameters = params_opt.value();
            ATH_MSG_DEBUG("Track parameters: " << parameters.parameters());

            if (m_doTruth) {
                SG::ReadHandle<xAOD::PixelClusterContainer> pixelClustersHandle(
                    m_xAODPixelClusterFromInDetClusterKey.key(), eventContext);
                ATH_CHECK(pixelClustersHandle.isValid());
                const xAOD::PixelClusterContainer* inputPixelClusters =
                    pixelClustersHandle.cptr();

                SG::ReadHandle<xAOD::StripClusterContainer> stripClustersHandle(
                    m_xAODStripClusterFromInDetClusterKey.key(), eventContext);
                ATH_CHECK(stripClustersHandle.isValid());
                const xAOD::StripClusterContainer* inputStripClusters =
                    stripClustersHandle.cptr();

                // match measurement to the cluster container
                int cl_index = -1;
                if (auto it = cluster_map.find(states.athena_id[j]);
                    it != cluster_map.end()) {
                    cl_index = it->second;
                } else {
                    return StatusCode::FAILURE;
                }

                // Determine detector type from athena_id
                Identifier id(static_cast<Identifier::value_type>(states.athena_id[j]));
                bool isPixel = m_pixelID->is_pixel(id);

                const xAOD::UncalibratedMeasurement* umeas = nullptr;
                if (isPixel) {
                    umeas = inputPixelClusters->at(cl_index);
                } else {
                    umeas = inputStripClusters->at(cl_index);
                }

                actsTSOS.setUncalibratedSourceLink(
                    ActsTrk::detail::MeasurementCalibratorBase::pack(umeas));
            }

            // This is the conversion of global track parameters
            // Because Traccc does not do backpropagation yet,
            // we do not have the reference surface ie the perigee.
            // So for now we will set the global track params with
            // the reference surface being the surface of first measurement
            // and enable back propagation during ACTS->xAOD conversion
            // this will find the pergee and re-set the global trk params.
            if (first_state) {
                // This is the first track state, so we need to set the track
                // global parameters

                std::optional<Acts::BoundTrackParameters> params_gl =
                    convertToActsParameters(singleState);

                if (!params_gl.has_value()) {
                    // if the params are "buggy", the whole track is dismissed.
                    ATH_MSG_DEBUG(
                        "convertToActsParameters failed: track state is weird");
                    track_validity = INVALID_GLOBAL;
                    break;
                }

                Acts::BoundTrackParameters const& parameters_gl =
                    params_gl.value();

                ATH_MSG_VERBOSE("First state of track.");
                actsTrack.parameters() = parameters_gl.parameters();
                actsTrack.covariance() = *parameters_gl.covariance();
                actsTrack.setReferenceSurface(
                    parameters_gl.referenceSurface().getSharedPtr());
                first_state = false;

            }
            
            actsTSOS.setReferenceSurface(
                parameters.referenceSurface().getSharedPtr());
            actsTSOS.smoothed() = parameters.parameters();
            actsTSOS.smoothedCovariance() = *parameters.covariance();
            // Mark this state as a measurement for temporary efficiency matching
            actsTSOS.typeFlags().setIsMeasurement();
            if (!(actsTSOS.hasSmoothed() &&
                    actsTSOS.hasReferenceSurface())) {
                ATH_MSG_INFO(
                    "TrackState does not have smoothed state ["
                    << actsTSOS.hasSmoothed()
                    << "] or reference surface ["
                    << actsTSOS.hasReferenceSurface() << "].");
            } else {
                ATH_MSG_DEBUG(
                    "TrackState has smoothed state and reference "
                    "surface.");
            }
        }

        // ATH_MSG_INFO("Done with states of this track.");
    if (track_validity == INVALID_STATE) {
            ATH_MSG_INFO("excluding track " << i << " for weird state");
            excluded_weird_state += 1;
            track_container.removeTrack(actsTrack.index());
        } else if (track_validity == INVALID_GLOBAL) {
            ATH_MSG_INFO("excluding track " << i
                                             << " for weird global params");
            track_container.removeTrack(actsTrack.index());
        }


        // Debug stats
        chi2_min = std::min(fit_res.chi2, chi2_min);
        chi2_max = std::max(fit_res.chi2, chi2_max);
        ndf_min = std::min(fit_res.ndf, ndf_min);
        ndf_max = std::max(fit_res.ndf, ndf_max);
        meas_min = std::min<unsigned>(states.local_x.size(), meas_min);
        meas_max = std::max<unsigned>(states.local_x.size(), meas_max);
    }

    nb_output_tracks = track_container.size();
    ATH_MSG_DEBUG("Wrote out "<< nb_output_tracks << " tracks from " << trackParams.size() << " candidates"
      << ", excluded:"
      << " no sp: " << excluded_no_sp
      << ", weird sp: " << excluded_weird_state
      << ", ndf: " << excluded_ndf
    );

    Acts::ConstVectorTrackContainer ctrack_backend(
        std::move(track_container.container()));
    Acts::ConstVectorMultiTrajectory ctrack_state_backend(
        std::move(track_container.trackStateContainer()));
    std::unique_ptr<ActsTrk::TrackContainer> ctrack_container =
        std::make_unique<ActsTrk::TrackContainer>(
            std::move(ctrack_backend), std::move(ctrack_state_backend));

    ATH_CHECK(trackContainerHandle.record(std::move(ctrack_container)));

    return StatusCode::SUCCESS;
}
