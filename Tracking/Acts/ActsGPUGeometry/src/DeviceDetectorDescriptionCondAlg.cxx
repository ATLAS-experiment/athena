/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceDetectorDescriptionCondAlg.h"
#include "StoreGate/StoreGateSvc.h"


namespace ActsTrk {

std::vector<traccc::scalar> makeEquidistantEdges(float halfWidth, int nBins) {
    std::vector<traccc::scalar> edges;
    edges.reserve(nBins + 1);
    const float min = -halfWidth;
    const float pitch = (2.f * halfWidth) / static_cast<float>(nBins);
    for (int i = 0; i <= nBins; ++i) {
        edges.push_back(static_cast<traccc::scalar>(min + static_cast<float>(i) * pitch));
    }
    return edges;
}

StatusCode DeviceDetectorDescriptionCondAlg::initialize()
{
    ATH_MSG_DEBUG("Initializing  device detector description provider service ");

    ATH_CHECK(m_hostMR.retrieve());
    ATH_CHECK(m_deviceMR.retrieve());
    ATH_CHECK(m_copy.retrieve());

    ATH_CHECK(m_detStore->retrieve(m_pixelID, m_pixelIdHelperName) );
    ATH_CHECK(m_detStore->retrieve(m_stripID, m_stripIdHelperName));
    ATH_CHECK(m_detStore->retrieve(m_pixelManager, "ITkPixel"));
    ATH_CHECK(m_detStore->retrieve(m_stripManager, "ITkStrip"));

    ATH_CHECK(m_writeHostCondKey.initialize());
    ATH_CHECK(m_writeDeviceCondKey.initialize());
    ATH_CHECK(m_stripPropertiesKey.initialize());
   

    ATH_CHECK(m_stripLorentzAngleTool.retrieve());
    ATH_CHECK(m_pixelLorentzAngleTool.retrieve());


    ATH_CHECK(m_writeHostCondKey.initialize());
    ATH_CHECK(m_writeDeviceCondKey.initialize());

    int nPix = 0, nStrip = 0, nPixel = 0, nRect1 = 0, nTrap1 = 0, nAnnu1 = 0, nnonbar = 0, nbar = 0;

    // ---- 1. Get ACTS Tracking Geometry, populate Athena<->ACTS maps and fill module design (segmentation) information ----
    // all of these aare static upon construction through the run
    if (!m_trackingGeometryTool.empty()) {
        ATH_CHECK(m_trackingGeometryTool.retrieve());
        m_trackingGeometry = m_trackingGeometryTool->trackingGeometry();

        m_trackingGeometry->visitSurfaces([&](const Acts::Surface *surface) {
            if (!surface) return;
            m_allsurfaces.push_back(std::const_pointer_cast<Acts::Surface>(surface->getSharedPtr()));
            const auto *actsElement = getActsDetectorElement(surface);
            if (!actsElement) return;
            const auto *geoElement = actsElement->upstreamDetectorElement();
            const auto *detElem = dynamic_cast<const InDetDD::SiDetectorElement*>(geoElement);
            if (!geoElement || !detElem) return;
            m_surfaces.push_back(std::const_pointer_cast<Acts::Surface>(surface->getSharedPtr()));

            Identifier athenaID;
            moduleInfo thismod;
            // NOTE: lorentz_shift_x/y intentionally left at their default
            // (0.f) — they're not used for design-shape grouping and are
            // recomputed per-IOV in execute(), so no Lorentz-tool call
            // happens here.

            if (detElem->isPixel()) {
                ++nPix; ++nPixel;
                athenaID = detElem->identify();
                const auto* p_design = static_cast<const InDetDD::PixelModuleDesign*>(&detElem->design());
                auto boundsType = detElem->bounds().type();
                thismod.pixel = true;
                thismod.side = 2;
                int index = 0;
                if ((m_pixelID->barrel_ec(athenaID) == 0 && m_pixelID->layer_disk(athenaID) > 0) ||
                    (m_pixelID->barrel_ec(athenaID) != 0 && m_pixelID->layer_disk(athenaID) > 1)) {
                    index = 4;
                }
                if (boundsType == Trk::SurfaceBounds::Rectangle) {
                    thismod.module_width = p_design->width();
                    thismod.module_length = p_design->length();
                    thismod.rows = p_design->rows() + index;
                    thismod.columns = p_design->columns() + index;
                }
                if (boundsType == Trk::SurfaceBounds::Trapezoid) {
                    thismod.module_width = p_design->width();
                    thismod.module_length = p_design->length();
                    thismod.rows = p_design->rows();
                    thismod.columns = p_design->columns();
                }
                if (boundsType == Trk::SurfaceBounds::Annulus) {
                    thismod.module_width = p_design->width();
                    thismod.module_length = p_design->length();
                    thismod.rows = p_design->rows();
                    thismod.columns = p_design->columns();
                }
            } else {
                ++nStrip;
                const Identifier moduleID = m_stripID->module_id(detElem->identify());
                const IdentifierHash moduleHash = m_stripID->wafer_hash(moduleID);
                const int side = m_stripID->side(detElem->identify());
                athenaID = m_stripID->wafer_id(moduleHash + side);
                thismod.pixel = false;
                thismod.side = side;

                if (m_stripID->barrel_ec(athenaID) == 0) {
                    ++nbar;
                    const auto* s_design = static_cast<const InDetDD::SCT_BarrelModuleSideDesign*>(&detElem->design());
                    auto boundsType = detElem->bounds().type();
                    if (boundsType == Trk::SurfaceBounds::Rectangle) {
                        ++nRect1;
                        thismod.module_width = s_design->width();
                        thismod.module_length = s_design->length();
                        thismod.rows = s_design->cells();
                    }
                    if (boundsType == Trk::SurfaceBounds::Trapezoid) {
                        thismod.module_width = s_design->width();
                        thismod.module_length = s_design->length();
                        thismod.rows = s_design->cells();
                    }
                    if (boundsType == Trk::SurfaceBounds::Annulus) {
                        ++nAnnu1;
                        thismod.isAnnulus = true;
                        thismod.module_width = s_design->width();
                        thismod.module_length = s_design->length();
                        thismod.rows = s_design->cells();
                    }
                } else {
                    ++nnonbar;
                    const auto* annulus_design = static_cast<const InDetDD::StripStereoAnnulusDesign*>(&detElem->design());
                    const InDetDD::SiCellId annulus_cell = detElem->cellIdFromIdentifier(athenaID);
                    const double pitch_row = annulus_design->phiPitchPhi(annulus_cell);
                    const double radius = annulus_design->centreR();
                    const int nStrips0 = annulus_design->diodesInRow(0.);
                    const double max_phi = nStrips0 * pitch_row;
                    thismod.module_width = -max_phi;
                    thismod.module_length = radius;
                    thismod.rows = nStrips0;
                    thismod.isAnnulus = true;
                }
            }

            m_atlasModuleInfo[athenaID] = thismod;
            const auto geo_id = surface->geometryId();
            m_actsToAthena[geo_id] = athenaID;
        });
    }

    ATH_MSG_INFO(nPixel << " Atlas Pixel modules found.");
    ATH_MSG_INFO(nStrip << " Atlas Strip modules found.");
    ATH_MSG_INFO("Wrote segmentation info for " << m_atlasModuleInfo.size() << " modules");

    // ---- 2. Get Detray Tracking Geometry and populate Detray<->ACTS map ----
    // this will in the future happen during ACTS geometry construction, but for now we load the geometry from file
    ATH_MSG_INFO("Loading traccc detector");
    m_detrayDetector = std::make_unique<traccc::host_detector>();
    traccc::io::read_detector(*m_detrayDetector, m_hostMR->mr(),
                               PathResolverFindCalibFile(m_geometryFile.value()));

    const auto& itkDetector = m_detrayDetector->as<traccc::itk_detector>();

    for (const auto& surface : itkDetector.surfaces()) {
        const auto geo_id = surface.source;
        const Acts::GeometryIdentifier acts_geom_id{geo_id};
        auto sf = detray::tracking_surface{itkDetector, surface};
        const auto detray_id = sf.identifier().value();

        if (m_actsToAthena.contains(acts_geom_id)) {
            auto athena_id = m_actsToAthena.at(acts_geom_id);
            m_detrayToAthenaMap[detray_id] = athena_id;
            m_athenaToDetrayMap[athena_id] = detray_id;
        } else {
            ATH_MSG_DEBUG("we did not save key " << acts_geom_id);
            if (surface.is_sensitive()) continue;
            ATH_MSG_DEBUG("found this passive surface in detray: " << acts_geom_id);
        }
    }

    // ---- 3. Deduplicate the designs ----
    // there are only a hanful of unique module designs, only store unique values
    std::map<designKey, unsigned int> designLookup;
    unsigned int nextDesignId = 0;

    const std::size_t nSurfaces = itkDetector.surfaces().size();
    m_staticCondEntries.resize(nSurfaces);

    for (std::size_t condIndex = 0; condIndex < nSurfaces; ++condIndex) {
        const auto& surface = itkDetector.surfaces()[condIndex];
        const auto geo_id = surface.source;
        const Acts::GeometryIdentifier acts_geom_id{geo_id};
        auto sf = detray::tracking_surface{itkDetector, surface};
        const auto detray_id = sf.identifier().value();

        StaticCondEntry entry;
        entry.geometryId = detray::geometry::identifier{detray_id};
        entry.actsGeometryId = acts_geom_id.value();

        auto detrayIt = m_detrayToAthenaMap.find(detray_id);
        if (detrayIt != m_detrayToAthenaMap.end()) {
            auto modIt = m_atlasModuleInfo.find(detrayIt->second);
            if (modIt != m_atlasModuleInfo.end()) {
                const moduleInfo& thismod = modIt->second;
                const int nBinsX = thismod.pixel ? thismod.columns : thismod.rows;
                const int nBinsY = thismod.pixel ? thismod.rows : 1;
                designKey key{thismod.pixel, thismod.isAnnulus, nBinsX, nBinsY,
                              std::abs(thismod.module_width), thismod.module_length};
                auto it = designLookup.find(key);
                if (it == designLookup.end()) {
                    entry.designId = nextDesignId;
                    designLookup.emplace(key, nextDesignId);
                    ++nextDesignId;
                } else {
                    entry.designId = it->second;
                }
                entry.athenaId = detrayIt->second;
                entry.hasAthenaModule = true;
                // no need to populate this until traccc implements neighbours
                // for strp modules
                // m_athenaToCondIndex[detrayIt->second] = condIndex;
            }
        }
        m_staticCondEntries[condIndex] = entry;
    }

    // ---- 4. Write detector design object ----

    auto hostDesign = std::make_unique<traccc::detector_design_description::host>(m_hostMR->mr());
    hostDesign->resize(designLookup.size());
    for (const auto& [key, id] : designLookup) {
        hostDesign->design_id()[id] = static_cast<int>(id);
        const auto edgesX = makeEquidistantEdges(0.5f * key.width, key.nBinsX);
        hostDesign->bin_edges_x()[id].assign(edgesX.begin(), edgesX.end());
        const auto edgesY = makeEquidistantEdges(0.5f * key.length, key.nBinsY);
        hostDesign->bin_edges_y()[id].assign(edgesY.begin(), edgesY.end());
        hostDesign->dimensions()[id] = key.pixel ? 2 : 1;
        hostDesign->subspace()[id] =
            key.isAnnulus
                ? std::array<detray::dindex_type<traccc::default_algebra>, 2u>{0u, 1u}
                : std::array<detray::dindex_type<traccc::default_algebra>, 2u>{1u, 0u};
    }

    std::vector<unsigned int> m_designSizes(hostDesign->size());
    for (std::size_t i = 0; i < hostDesign->size(); ++i) {
        auto thisDesign = hostDesign->at(i);
        m_designSizes[i] = std::max(
            static_cast<unsigned int>(thisDesign.bin_edges_x().size()),
            static_cast<unsigned int>(thisDesign.bin_edges_y().size()));
    }

    auto initCopy = m_copy->copy(EventContext{});
    auto deviceDesign = std::make_unique<traccc::detector_design_description::buffer>(
        m_designSizes, m_deviceMR->mr(), &(m_hostMR->mr()),
        vecmem::data::buffer_type::resizable);
    (*initCopy).setup(*deviceDesign)->wait();
    (*initCopy)(vecmem::get_data(*hostDesign), *deviceDesign)->wait();

    constexpr bool allowMods = false;
    ATH_CHECK(m_detStore->record(std::move(deviceDesign), m_deviceDesignObjectName.value(), allowMods));
    ATH_CHECK(m_detStore->record(std::move(hostDesign), m_hostDesignObjectName.value(), allowMods));

    return StatusCode::SUCCESS;
}

StatusCode DeviceDetectorDescriptionCondAlg::execute(const EventContext& ctx) const
{
    SG::WriteCondHandle<traccc::detector_conditions_description::host> writeHostHandle{m_writeHostCondKey, ctx};
    SG::WriteCondHandle<traccc::detector_conditions_description::buffer> writeDeviceHandle{m_writeDeviceCondKey, ctx};

    if (writeHostHandle.isValid() && writeDeviceHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandles " << writeHostHandle.fullKey() << " and "
                      << writeDeviceHandle.fullKey() << " are already valid.");
        return StatusCode::SUCCESS;
    }

    auto hostCond = std::make_unique<traccc::detector_conditions_description::host>(m_hostMR->mr());
    hostCond->resize(m_staticCondEntries.size());

    SG::ReadCondHandle<InDet::SiElementPropertiesTable> stripPropertiesHandle(m_stripPropertiesKey, ctx);
    const InDet::SiElementPropertiesTable* properties = stripPropertiesHandle.retrieve();
    if (properties == nullptr) {
        ATH_MSG_FATAL("Pointer of SiElementPropertiesTable (" << m_stripPropertiesKey.fullKey()
                      << ") could not be retrieved");
        return StatusCode::FAILURE;
    }

    writeHostHandle.addDependency(stripPropertiesHandle);
    writeDeviceHandle.addDependency(stripPropertiesHandle);

    // ---- 5. Write detector conditions object ----
    // This object basically stores any information that we neer per-module
    // like: detray id, acts id, index to module design, lorentz shift etc.
    // since lorentz shift is conditional, we need a valid event context
    // all the static info (like the id maps) have been pre-filled, here we just populate anything that might be conditions depenedant

    for (std::size_t condIndex = 0; condIndex < m_staticCondEntries.size(); ++condIndex) {
        const auto& entry = m_staticCondEntries[condIndex];

        hostCond->module_to_design_id()[condIndex] = entry.designId;
        hostCond->geometry_id()[condIndex] = entry.geometryId;
        hostCond->acts_geometry_id()[condIndex] = entry.actsGeometryId;

        float shiftX = 0.f, shiftY = 0.f;
        if (entry.hasAthenaModule) {
            auto modIt = m_atlasModuleInfo.find(entry.athenaId);
            const bool isPixel = (modIt != m_atlasModuleInfo.end()) && modIt->second.pixel;

            if (isPixel) {
                const IdentifierHash pixelHash = m_pixelID->wafer_hash(entry.athenaId);
                shiftY = m_pixelLorentzAngleTool->getLorentzShift(pixelHash, ctx);
                shiftX = 0.f;
            } else {
                const IdentifierHash moduleHash = m_stripID->wafer_hash(
                    m_stripID->module_id(entry.athenaId));
                const int side = m_stripID->side(entry.athenaId);
                const bool isAnnulus = (modIt != m_atlasModuleInfo.end()) && modIt->second.isAnnulus;
                if(isAnnulus){
                    shiftX = m_stripLorentzAngleTool->getLorentzShift(moduleHash + side, ctx);
                    shiftY = 0.f;
                } else {
                    shiftX = m_stripLorentzAngleTool->getLorentzShift(moduleHash + side, ctx);
                    shiftY = 0.f;
                }    
            }
        }

        hostCond->measurement_translation()[condIndex] =
            traccc::vector2{static_cast<traccc::scalar>(shiftX), static_cast<traccc::scalar>(shiftY)};
    }

    // ---- 6. Fill neighbour / backside indices for strip modules ----
    // This is not implemented yet on the traccc side (backside_id()/
    // neighbours() columns), left here for when that support lands, since
    // it will greatly help with strip space point formation.
    // for (const auto& [athenaId, condIndex] : m_athenaToCondIndex) {
    //     auto modIt = m_atlasModuleInfo.find(athenaId);
    //     if (modIt == m_atlasModuleInfo.end() || modIt->second.pixel) continue;

    //     // neighbours is only for front-side hashes, matching the usage in
    //     // StripSpacePointFormationTool
    //     if (modIt->second.side != 0) continue;

    //     const std::vector<IdentifierHash>* others =
    //         properties->neighbours(m_stripID->wafer_hash(athenaId));
    //     if (others == nullptr || others->empty()) continue;

    //     // Element 0 in `others` is always the opposite/stereo (backside)
    //     // module, per the ordering used in
    //     // StripSpacePointFormationTool::produceSpacePoints
    //     // (neigbourIndices = {ThisOne, Opposite, EtaMinus, EtaPlus,
    //     // PhiMinus, PhiPlus}).
    //     const Identifier backsideId = m_stripID->wafer_id(others->at(0));
    //     auto backIt = m_athenaToCondIndex.find(backsideId);
    //     if (backIt != m_athenaToCondIndex.end()) {
    //         hostCond->backside_id()[condIndex] =
    //             static_cast<unsigned int>(backIt->second);
    //     }

    //     auto& nbrs = hostCond->neighbours()[condIndex];
    //     for (std::size_t i = 1; i < others->size(); ++i) {
    //         const Identifier neighbourId = m_stripID->wafer_id(others->at(i));
    //         auto nbrIt = m_athenaToCondIndex.find(neighbourId);
    //         if (nbrIt != m_athenaToCondIndex.end()) {
    //             nbrs.push_back(static_cast<unsigned int>(nbrIt->second));
    //         }
    //     }
    // }

    auto copy = m_copy->copy(ctx);
    auto deviceCond = std::make_unique<traccc::detector_conditions_description::buffer>(
        static_cast<traccc::detector_conditions_description::buffer::size_type>(hostCond->size()),
        m_deviceMR->mr());
    (*copy).setup(*deviceCond)->wait();
    (*copy)(vecmem::get_data(*hostCond), *deviceCond)->wait();

    ATH_CHECK(writeHostHandle.record(std::move(hostCond)));
    ATH_CHECK(writeDeviceHandle.record(std::move(deviceCond)));

    ATH_MSG_DEBUG("Recorded host and device detector description conditions");
    return StatusCode::SUCCESS;
}


} // namespace ActsTrk