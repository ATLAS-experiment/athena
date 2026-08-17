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

    ATH_CHECK(m_MRs.retrieve());
    ATH_CHECK(m_copy.retrieve());

    ATH_CHECK(m_detStore->retrieve(m_hostDetector, m_hostDetectorName));

    ATH_CHECK(m_detStore->retrieve(m_pixelID, m_pixelIdHelperName) );
    ATH_CHECK(m_detStore->retrieve(m_stripID, m_stripIdHelperName));
    ATH_CHECK(m_detStore->retrieve(m_pixelManager, "ITkPixel"));
    ATH_CHECK(m_detStore->retrieve(m_stripManager, "ITkStrip"));

    ATH_CHECK(m_writeHostCondKey.initialize());
    ATH_CHECK(m_writeDeviceCondKey.initialize());
    ATH_CHECK(m_stripPropertiesKey.initialize());
   
    ATH_CHECK(m_stripLorentzAngleTool.retrieve());
    ATH_CHECK(m_pixelLorentzAngleTool.retrieve());

    int nPix = 0, nStrip = 0, nRect = 0, nTrap = 0, nAnnu = 0, nEC = 0, nBar = 0;

    // ---- 1. Get ACTS Tracking Geometry, populate Athena<->ACTS maps and fill module design (segmentation) information ----
    // all of these are static upon construction through the run
    if (!m_trackingGeometrySvc.empty()) {
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        m_trackingGeometry = m_trackingGeometrySvc->trackingGeometry();

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

            thismod.isAnnulus = false;
            thismod.pixel = false;
            thismod.equidistant_binning = true;

            if (detElem->isPixel()) {
                ++nPix;
                athenaID = detElem->identify();
                const auto* p_design = static_cast<const InDetDD::PixelModuleDesign*>(&detElem->design());
                
                thismod.pixel = true;
                thismod.side = 2; // for pixel

                std::vector<traccc::scalar> row_centres;
                std::vector<traccc::scalar> column_centres;

                // pixels with cross design 
                // the middle four rows and columns are double pitch
                if ((m_pixelID->barrel_ec(athenaID) == 0 && m_pixelID->layer_disk(athenaID) > 0) ||
                    (m_pixelID->barrel_ec(athenaID) != 0 && m_pixelID->layer_disk(athenaID) > 1)) {

                    thismod.equidistant_binning = false;

                    for(int row = 0; row < p_design->rows(); row++){

                        std::array<InDetDD::PixelDiodeTree::CellIndexType, 2>
                            diode_idx = InDetDD::PixelDiodeTree::makeCellIndex(
                                row, 0);
                        InDetDD::PixelDiodeTree::DiodeProxyWithPosition si_param(
                            p_design->diodeProxyFromIdxCachePosition(diode_idx));

                        (row_centres).push_back(si_param.position()[0]-0.5*si_param.width()[0]);
                        if(row == p_design->rows()-1){
                            (row_centres).push_back(si_param.position()[0]+0.5*si_param.width()[0]);
                        }

                        float leading_edge  = si_param.position()[0] - 0.5f*si_param.width()[0];
                        float trailing_edge = si_param.position()[0] + 0.5f*si_param.width()[0];
                        float midpoint = (leading_edge + trailing_edge) / 2.f;
                        if(std::abs(midpoint - si_param.position()[0]) > 1e-6f){
                            ATH_MSG_ERROR("Row " << row << " edge midpoint " << midpoint
                                        << " != geometry centre " << si_param.position()[0]
                                        << " (diff=" << midpoint - si_param.position()[0] << ")");
                        }

                    }
                    for(int col = 0; col < p_design->columns(); col++){

                        std::array<InDetDD::PixelDiodeTree::CellIndexType, 2>
                            diode_idx = InDetDD::PixelDiodeTree::makeCellIndex(
                                0, col);
                        InDetDD::PixelDiodeTree::DiodeProxyWithPosition si_param(
                            p_design->diodeProxyFromIdxCachePosition(diode_idx));

                        (column_centres).push_back(si_param.position()[1]-0.5*si_param.width()[1]);
                        if(col == p_design->columns()-1){
                            (column_centres).push_back(si_param.position()[1]+0.5*si_param.width()[1]);
                        }

                        float leading_edge  = si_param.position()[1] - 0.5f*si_param.width()[1];
                        float trailing_edge = si_param.position()[1] + 0.5f*si_param.width()[1];
                        float midpoint = (leading_edge + trailing_edge) / 2.f;
                        if(std::abs(midpoint - si_param.position()[1]) > 1e-6f){
                            ATH_MSG_ERROR("Column " << col << " edge midpoint " << midpoint
                                        << " != geometry centre " << si_param.position()[1]
                                        << " (diff=" << midpoint - si_param.position()[1] << ")");
                        }

                    }

                }

                thismod.row_centres = row_centres;
                thismod.column_centres = column_centres;

                thismod.module_width = p_design->width();
                thismod.module_length = p_design->length();
                thismod.rows = p_design->rows();
                thismod.columns = p_design->columns();
               
            } else {
                ++nStrip;
                const Identifier moduleID = m_stripID->module_id(detElem->identify());
                const IdentifierHash moduleHash = m_stripID->wafer_hash(moduleID);
                const int side = m_stripID->side(detElem->identify());
                athenaID = m_stripID->wafer_id(moduleHash + side);

                thismod.pixel = false;
                thismod.side = side;
                thismod.columns = 1; // for strip

                if (m_stripID->barrel_ec(athenaID) == 0) {
                    ++nBar;
                    const auto* s_design = static_cast<const InDetDD::SCT_BarrelModuleSideDesign*>(&detElem->design());
                    auto boundsType = detElem->bounds().type();
                    if (boundsType == Trk::SurfaceBounds::Rectangle) {
                        ++nRect;
                        thismod.module_width = s_design->width();
                        thismod.module_length = s_design->length();
                        thismod.rows = s_design->cells();
                    }
                    if (boundsType == Trk::SurfaceBounds::Trapezoid) {
                        thismod.module_width = s_design->width();
                        thismod.module_length = s_design->length();
                        thismod.rows = s_design->cells();
                        nTrap++;
                    }
                    if (boundsType == Trk::SurfaceBounds::Annulus) {
                        ++nAnnu;
                        thismod.isAnnulus = true;
                        thismod.module_width = s_design->width();
                        thismod.module_length = s_design->length();
                        thismod.rows = s_design->cells();
                    }
                } else {
                    ++nEC;
                    const auto* annulus_design = static_cast<const InDetDD::StripStereoAnnulusDesign*>(&detElem->design());
                    const InDetDD::SiCellId annulus_cell = detElem->cellIdFromIdentifier(athenaID);
                    const double pitch_row = annulus_design->phiPitchPhi(annulus_cell);
                    const int nDiodes = annulus_design->diodesInRow(0.);
                    const double max_phi = nDiodes * pitch_row;
                    thismod.module_width = -max_phi;
                    // this could be the halfway point in radius, number is arbitrary
                    // const double radius = annulus_design->centreR();
                    thismod.module_length = 0.2; //radius;
                    thismod.rows = nDiodes;
                    thismod.isAnnulus = true;
                }
            }

            m_atlasModuleInfo[athenaID] = thismod;
            const auto geo_id = surface->geometryId();
            m_actsToAthena[geo_id] = athenaID;
        });
    }

    ATH_MSG_INFO(nPix << " Atlas Pixel modules found.");
    ATH_MSG_INFO(nStrip << " Atlas Strip modules found, " << nBar << " in barrel and " << nEC << " in the endcap");
    ATH_MSG_INFO("Out od those " << nRect << " are rectangular, " << nTrap << " are trapezoid and " << nAnnu << " are annulus.");
    ATH_MSG_INFO("Wrote segmentation info for " << m_atlasModuleInfo.size() << " modules");

    // ---- 2. Get Detray Tracking Geometry and populate Detray<->ACTS map ----
    ATH_MSG_INFO("Loading traccc detector");
    
    const auto& itkDetector = m_hostDetector->as<traccc::itk_detector>();

    int found_detray = 0; 
    int missing_detray = 0; 
    int missing_detray_passives = 0;

    for (const auto& surface : itkDetector.surfaces()) {
        const auto geo_id = surface.source;
        const Acts::GeometryIdentifier acts_geom_id{geo_id};
        auto sf = detray::tracking_surface{itkDetector, surface};
        const auto detray_id = sf.identifier().value();

        if (m_actsToAthena.contains(acts_geom_id)) {
            auto athena_id = m_actsToAthena.at(acts_geom_id);
            m_detrayToAthenaMap[detray_id] = athena_id;
            m_athenaToDetrayMap[athena_id] = detray_id;
            found_detray++;
        } else {
            ATH_MSG_VERBOSE("ACTS surface with key " << acts_geom_id << " was not traslated to detray geometry.");
            missing_detray++;
            if (surface.is_sensitive()) continue;
            ATH_MSG_VERBOSE("found this passive surface in detray: " << acts_geom_id);
            missing_detray_passives++;
        }
    }

    ATH_MSG_INFO("Traccc detector has " << found_detray << " surfaces matching ACTS and " << missing_detray << " additional sufaces, out of which " << missing_detray_passives << " are passive.");

    // ---- 3. Deduplicate the designs ----
    // there are only a handful of unique module designs, only store unique values
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

                const int nBinsX = thismod.rows;
                const int nBinsY = thismod.pixel ? thismod.columns : 1;

                auto edgesX = makeEquidistantEdges(0.5f * thismod.module_width, nBinsX);
                auto edgesY = makeEquidistantEdges(0.5f * thismod.module_length, nBinsY);

                if(!thismod.equidistant_binning){
                    edgesX = thismod.row_centres;
                    edgesY = thismod.column_centres;
                }
                
                designKey key{thismod.pixel, thismod.isAnnulus, edgesX, edgesY,
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
            }
        }else{
            ATH_MSG_ERROR("Could not find matching Athena module for detray surface with geometryId " << detray_id);
        }
        m_staticCondEntries[condIndex] = entry;
    }

    // ---- 4. Write detector design object ----

    auto hostDesign = std::make_unique<traccc::detector_design_description::host>(*m_MRs->hostMR());
    hostDesign->resize(designLookup.size());
    for (const auto& [key, id] : designLookup) {
        hostDesign->design_id()[id] = static_cast<int>(id);

        hostDesign->dimensions()[id] = key.pixel ? 2 : 1;
    

        if(!key.isAnnulus){
            hostDesign->bin_edges_x()[id].assign(key.edgesX.begin(), key.edgesX.end());
            hostDesign->bin_edges_y()[id].assign(key.edgesY.begin(), key.edgesY.end());
            hostDesign->subspace()[id] = std::array<detray::dindex_type<traccc::default_algebra>, 2u>{0u, 1u};
        }else{
            hostDesign->bin_edges_y()[id].assign(key.edgesX.begin(), key.edgesX.end());
            hostDesign->bin_edges_x()[id].assign(key.edgesY.begin(), key.edgesY.end());
            hostDesign->subspace()[id] = std::array<detray::dindex_type<traccc::default_algebra>, 2u>{1u, 0u};
        }
        
        
    }

    std::vector<unsigned int> designSizes(hostDesign->size());
    for (std::size_t i = 0; i < hostDesign->size(); ++i) {
        auto thisDesign = hostDesign->at(i);
        designSizes[i] = std::max(
            static_cast<unsigned int>(thisDesign.bin_edges_x().size()),
            static_cast<unsigned int>(thisDesign.bin_edges_y().size()));
    }

    auto initCopy = m_copy->copy(EventContext{});
    auto deviceDesign = std::make_unique<traccc::detector_design_description::buffer>(
        designSizes, m_MRs->mainMR(), m_MRs->hostMR(),
        vecmem::data::buffer_type::resizable);
    (*initCopy).setup(*deviceDesign)->wait();
    (*initCopy)(vecmem::get_data(*hostDesign), *deviceDesign)->wait();

    constexpr bool allowMods = false;
    ATH_CHECK(m_detStore->record(std::move(deviceDesign), m_deviceDesignObjectName.value(), allowMods));
    ATH_CHECK(m_detStore->record(std::move(hostDesign), m_hostDesignObjectName.value(), allowMods));

    ATH_MSG_DEBUG("Recorded host and device detector design description: " << m_hostDesignObjectName.value() << ", " << m_deviceDesignObjectName.value());

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

    auto hostCond = std::make_unique<traccc::detector_conditions_description::host>(*m_MRs->hostMR());
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
    // This object basically stores any information that we need per-module
    // like: detray id, acts id, index to module design, lorentz shift etc.
    // since lorentz shift is conditional, we need a valid event context
    // all the static info (like the id maps) have been pre-filled, here we just populate anything that might be conditions dependant

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
                shiftX = m_pixelLorentzAngleTool->getLorentzShift(pixelHash, ctx);
                shiftY = 0.f;
            } else {
                const IdentifierHash moduleHash = m_stripID->wafer_hash(
                    m_stripID->module_id(entry.athenaId));
                const int side = m_stripID->side(entry.athenaId);
                shiftX = m_stripLorentzAngleTool->getLorentzShift(moduleHash + side, ctx);
                shiftY = 0.f;   
            }
        }

        hostCond->measurement_translation()[condIndex] =
            traccc::vector2{static_cast<traccc::scalar>(shiftX), static_cast<traccc::scalar>(shiftY)};
    }

    auto copy = m_copy->copy(ctx);
    auto deviceCond = std::make_unique<traccc::detector_conditions_description::buffer>(
        static_cast<traccc::detector_conditions_description::buffer::size_type>(hostCond->size()),
        m_MRs->mainMR());
    (*copy).setup(*deviceCond)->wait();
    (*copy)(vecmem::get_data(*hostCond), *deviceCond)->wait();

    ATH_CHECK(writeHostHandle.record(std::move(hostCond)));
    ATH_CHECK(writeDeviceHandle.record(std::move(deviceCond)));

    ATH_MSG_DEBUG("Recorded host and device detector conditions description: " << m_writeHostCondKey.key() << ", " << m_writeDeviceCondKey.key());

    return StatusCode::SUCCESS;
}


} // namespace ActsTrk