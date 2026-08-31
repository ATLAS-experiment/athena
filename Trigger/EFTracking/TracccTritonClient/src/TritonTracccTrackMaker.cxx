/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
#include <fstream>
#include <sstream>

#include "TritonTracccTrackMaker.h"

#include "CxxUtils/StringUtils.h"

#include <chrono>

struct cell_order {
    bool operator()(const TracccCell& lhs,
                    const TracccCell& rhs) const
    {
        if (lhs.channel1 != rhs.channel1) {
            return (lhs.channel1 < rhs.channel1);
        } else {
            return (lhs.channel0 < rhs.channel0);
        }
    }
};

StatusCode TritonTracccTrackMaker::initialize()
{
    // input handles / tools
    ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));
    ATH_CHECK(m_pixelRDOKey.initialize());

    ATH_CHECK(detStore()->retrieve(m_stripID, "SCT_ID"));
    ATH_CHECK(m_stripRDOKey.initialize());

    ATH_CHECK(detStore()->retrieve(m_pixelManager));
    ATH_CHECK(detStore()->retrieve(m_stripManager));

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
    std::vector<TracccCell> cells;
    auto cell_start = std::chrono::high_resolution_clock::now();
    ATH_CHECK(read_cells(cells, ctx));
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
    ATH_CHECK(m_tracccTrackingTool->getTracks(cells, TracccTrackParams, TracccMeasurementsInfoInTracks));
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

std::vector<int> TritonTracccTrackMaker::map_index(int index,
                                                    int low_bound,
                                                    int high_bound,
                                                    int threshold,
                                                    int shift) const
{

    std::vector<int> result;

    if (index == low_bound) {  // 398 -> 398,399 ; 382 -> 382,383
        result.push_back(index);
        result.push_back(index + 1);
    } else if (index == low_bound + 1) {  // 399 -> 400,401 ; 383 -> 384,385
        result.push_back(index + 1);
        result.push_back(index + 2);
    } else if (index == low_bound + 2) {  // 400 -> 402,403 ; 384 -> 386,387
        result.push_back(index + 2);
        result.push_back(index + 3);
    } else if (index == high_bound) {  // 401 -> 404,405 ; 385 -> 388,390
        result.push_back(index + 3);
        result.push_back(index + 4);
    } else if (index > threshold) {
        result.push_back(index + shift);
    } else {
        result.push_back(index);
    }

    return result;
}

std::vector<std::pair<int, int>> TritonTracccTrackMaker::correct_indices(
    int phiIndex, int etaIndex, int rows,
    int columns) const
{
    // Traccc currently assumes a uniform pitch distributions on the modules
    // however, in some EC and barel layers (> 0 and >1, respectively), there
    // are 'quad' modules meaning the middle two rows and middle two columns are
    // double pitch: 0.05 mm -> 0.1 mm We solve this in G-200 by splitting these
    // cells into two contributions to ensure a uniform pitch across the module
    // and therefore the correct cluster position calculation When translating
    // the Athena cells to Traccc cells, the middle indices then need to get two
    // contributions, the indices below are unchanged, the indices above get
    // shifted by 4

    int middle_row = rows / 2;
    int middle_column = columns / 2;

    std::vector<std::pair<int, int>> index_vec;

    std::vector<int> mapped_x =
        map_index(phiIndex, middle_row - 2, middle_row + 1, middle_row + 1, 4);
    std::vector<int> mapped_y = map_index(
        etaIndex, middle_column - 2, middle_column + 1, middle_column + 1, 4);

    if ((middle_row - 2 <= phiIndex && phiIndex <= middle_row + 1) and
        (middle_column - 2 <= etaIndex && etaIndex <= middle_column + 1)) {
        ATH_MSG_DEBUG("mapping: " << phiIndex << "," << etaIndex);
    }

    for (auto [phi, eta] : std::views::cartesian_product(mapped_x, mapped_y)) {
        index_vec.emplace_back(phi, eta);
        if ((middle_row - 2 <= phiIndex && phiIndex <= middle_row + 1) and
            (middle_column - 2 <= etaIndex &&
                etaIndex <= middle_column + 1)) {
            ATH_MSG_DEBUG(phi << "," << eta);
        }
    }

    return index_vec;
}

StatusCode TritonTracccTrackMaker::read_cells(
    std::vector<TracccCell>& cells,
    const EventContext& evtcontext) const
{
    cells.clear();

    ATH_MSG_DEBUG("Reading pixel hits");

    const PixelRDO_Container* pixelRDOHandle{};
    ATH_CHECK(SG::get(pixelRDOHandle, m_pixelRDOKey, evtcontext));

    int nPix = 0;
    for (const InDetRawDataCollection<PixelRDORawData>* pixel_rdoCollection :
         *pixelRDOHandle) {

        for (const PixelRDORawData* pixelRawData : *pixel_rdoCollection) {

            Identifier rdoId = pixelRawData->identify();

            // get the det element from the det element collection
            const InDetDD::SiDetectorElement* sielement =
                m_pixelManager->getDetectorElement(rdoId);
            assert(sielement);
            const Identifier Pixel_ModuleID = sielement->identify();
            InDetDD::SiCellId id = sielement->cellIdFromIdentifier(rdoId);

            int layer_disk = m_pixelID->layer_disk(Pixel_ModuleID);
            int barrel_ec = m_pixelID->barrel_ec(Pixel_ModuleID);

            int64_t const geometry_id = Pixel_ModuleID.get_compact();

            ATH_MSG_DEBUG("Doing this module: " << geometry_id);

            if ((barrel_ec != 0 && layer_disk > 1) ||
                (barrel_ec == 0 && layer_disk > 0)) {
                const InDetDD::PixelModuleDesign* p_design =
                    static_cast<const InDetDD::PixelModuleDesign*>(
                        &sielement->design());
                std::vector<std::pair<int, int>> cell_vec =
                    correct_indices(id.phiIndex(), id.etaIndex(),
                                    p_design->rows(), p_design->columns());
                for (std::size_t c = 0; c < cell_vec.size(); c++) {

                    std::tuple<int, int> indices = cell_vec.at(c);
                    const int phiIndex = std::get<0>(indices);
                    const int etaIndex = std::get<1>(indices);

                    cells.push_back({
                        geometry_id, 0, static_cast<int64_t>(phiIndex),
                        static_cast<int64_t>(etaIndex),
                        static_cast<float>(pixelRawData->getToT()),
                        8  // timestamp is not used
                    });
                }
            } else {
                cells.push_back({
                    geometry_id, 0, static_cast<int64_t>(id.phiIndex()),
                    static_cast<int64_t>(id.etaIndex()),
                    static_cast<float>(pixelRawData->getToT()),
                    8  // timestamp is not used
                });
            }

            nPix++;
        }
    }
    ATH_MSG_DEBUG("Read " << nPix << " pixel hits");

    ATH_MSG_DEBUG("Reading strip hits");

    const SCT_RDO_Container* stripRDOHandle{};
    ATH_CHECK(SG::get(stripRDOHandle, m_stripRDOKey, evtcontext));

    int nStrip = 0;
    for (const InDetRawDataCollection<SCT_RDORawData>* strip_Collection :
         *stripRDOHandle) {
        if (strip_Collection == nullptr) {
            continue;
        }
        for (const SCT_RDORawData* stripRawData : *strip_Collection) {

            const Identifier rdoId = stripRawData->identify();
            const InDetDD::SiDetectorElement* sielement =
                m_stripManager->getDetectorElement(rdoId);

            const Identifier strip_moduleID = m_stripID->module_id(
                sielement->identify());  // from wafer id to module id
            const IdentifierHash Strip_ModuleHash =
                m_stripID->wafer_hash(strip_moduleID);

            // Extract the correct Strip_ModuleID
            int side = m_stripID->side(sielement->identify());
            const Identifier Strip_ModuleID =
                m_stripID->wafer_id(Strip_ModuleHash + side);

            InDetDD::SiCellId id = sielement->cellIdFromIdentifier(rdoId);

            int64_t const geometry_id = Strip_ModuleID.get_compact();

            if (m_stripID->barrel_ec(Strip_ModuleID) == 0) {

                // if we have barrel modules, then this is in cartesian
                // coordinates so phi, eta indices hold

                for (int i = 0; i < stripRawData->getGroupSize(); i++) {

                    cells.push_back({
                        geometry_id, 0,
                        static_cast<int64_t>(id.phiIndex() + i), 0, 1,
                        8  // timestamp is not used
                    });
                    nStrip++;
                }

            } else {

                // if we have annulus modules in the endcaps
                // then we want to make r,phi measurements in the strip local
                // frame but we cluster in the phi direction, so this has to be
                // coordinate y

                for (int i = 0; i < stripRawData->getGroupSize(); i++) {

                    cells.push_back({
                        geometry_id, 0, 0,
                        static_cast<int64_t>(id.phiIndex() + i), 1,
                        8  // timestamp is not used
                    });
                    nStrip++;
                }
            }
        }
    }
    ATH_MSG_DEBUG("Read " << nStrip << " strip hits");

    // Sort the cells. Deduplication or not, they do need to be sorted.
    std::sort(cells.begin(), cells.end(), ::cell_order());

    ATH_MSG_DEBUG("Sorted the cells container");

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
    
    return Acts::BoundTrackParameters(actsSurface, params, cov, hypothesis);
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
