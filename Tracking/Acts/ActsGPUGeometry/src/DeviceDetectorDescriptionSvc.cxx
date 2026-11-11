/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceDetectorDescriptionSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "ActsPlugins/Detray/DetraySurfaceIdMap.hpp"

#include <cmath>
#include <optional>

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

StatusCode DeviceDetectorDescriptionSvc::initialize()
{
    ATH_MSG_DEBUG("Initializing  device detector description provider service ");

    ATH_CHECK(m_MRs.retrieve());
    ATH_CHECK(m_copy.retrieve());
    ATH_CHECK(m_detStore.retrieve());

    ATH_CHECK(m_detStore->retrieve(m_pixelID, m_pixelIdHelperName) );
    ATH_CHECK(m_detStore->retrieve(m_stripID, m_stripIdHelperName));
    ATH_CHECK(m_detStore->retrieve(m_pixelManager, "ITkPixel"));
    ATH_CHECK(m_detStore->retrieve(m_stripManager, "ITkStrip"));

    std::map<Acts::GeometryIdentifier, Identifier> actsToAthena;

    int nPix = 0, nStrip = 0, nEC = 0, nBar = 0;
    std::vector<Acts::GeometryIdentifier> actsHGTD;
    if (!m_trackingGeometrySvc.empty()) {

        // ---- 1. Get ACTS Tracking Geometry, populate Athena<->ACTS maps and fill module design (segmentation) information ----
        // all of these are static upon construction through the run

        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        m_trackingGeometry = m_trackingGeometrySvc->trackingGeometry();

        m_trackingGeometry->visitSurfaces([&](const Acts::Surface *surface) {
            if (!surface) return;
            const auto *actsElement = getActsDetectorElement(surface);
            if (!actsElement){
                ATH_MSG_DEBUG("Could not find matching Acts detector element for surface with geometryId " << surface->geometryId());
                return;
            }
            
            const ActsTrk::DetectorType detType = actsElement->detectorType();
            if (detType == ActsTrk::DetectorType::Hgtd) {
                // skip HGTD surfaces for now
                // currently no time info is being used in traccc
                ATH_MSG_VERBOSE("Found HGTD surface with geometryId " << surface->geometryId());
                actsHGTD.push_back(surface->geometryId());
                return;
            }
            if (detType != ActsTrk::DetectorType::Pixel && detType != ActsTrk::DetectorType::Sct) {
                ATH_MSG_DEBUG("Found non-pixel, non-strip detector element with geometryId " << surface->geometryId());
                return;
            }
            const auto *detElem = dynamic_cast<const InDetDD::SiDetectorElement*>(actsElement->upstreamDetectorElement());
            if (!detElem) {
                ATH_MSG_DEBUG("Could not find matching Athena silicon detector element for surface with geometryId " << surface->geometryId());
                return;
            }

            Identifier athenaID;
            moduleInfo thismod;
            // NOTE: lorentz_shift_x/y intentionally left at their default
            // (0.f) — they're not used for design-shape grouping and are
            // computed per-IOV by DeviceDetectorDescriptionCondAlg, so no
            // Lorentz-tool call happens here.

            thismod.isAnnulus = false;
            thismod.pixel = false;
            thismod.equidistant_binning = true;

            switch (detType) {
                case ActsTrk::DetectorType::Pixel: {
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

                    thismod.row_centres = std::move(row_centres);
                    thismod.column_centres = std::move(column_centres);

                    thismod.module_width = p_design->width();
                    thismod.module_length = p_design->length();
                    thismod.rows = p_design->rows();
                    thismod.columns = p_design->columns();
                    break;
                }
                case ActsTrk::DetectorType::Sct: {
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
                        
                        thismod.module_width = s_design->width();
                        thismod.module_length = s_design->length();
                        thismod.rows = s_design->cells();
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
                    break;
                }
                default:
                    // unreachable, see filter above
                    //coverity[DEADCODE]
                    return;
            }
            
            m_atlasModuleInfo[athenaID] = std::move(thismod);
            const auto geo_id = surface->geometryId();
            actsToAthena[geo_id] = athenaID;
            
        });

        ATH_MSG_DEBUG(nPix << " Atlas Pixel modules found.");
        ATH_MSG_DEBUG(nStrip << " Atlas Strip modules found, " << nBar << " in barrel and " << nEC << " in the endcap");
        ATH_MSG_DEBUG("Wrote segmentation info for " << m_atlasModuleInfo.size() << " modules");

        // ---- 2. Get the Detray geometry and the ACTS<->Detray surface map ----
        // Both are produced by the ACTS Detray plugin in the TrackingGeometrySvc
        ATH_MSG_INFO("Loading detray detector");

        const traccc::host_detector* hostDetector = m_trackingGeometrySvc->detrayGeometry();
        const ActsPlugins::DetraySurfaceIdMap* detraySurfaceIds = m_trackingGeometrySvc->detraySurfaceIdMap();
        if (!hostDetector || !detraySurfaceIds) {
            ATH_MSG_FATAL("No Detray geometry available from the TrackingGeometrySvc. "
                          "Set its BuildDetrayGeometry property to True.");
            return StatusCode::FAILURE;
        }
        const auto& itkDetector = hostDetector->as<traccc::itk_detector>();
        ATH_MSG_DEBUG("ACTS Detray plugin mapped " << detraySurfaceIds->size()
                      << " sensitive surfaces out of " << itkDetector.surfaces().size() << " Detray surfaces");

        // ---- 3. Deduplicate the designs ----
        // there are only a handful of unique module designs, only store unique values
        // and populate the detray<->acts<->athena geometry ID relationship map
        std::map<designKey, unsigned int> designLookup;
        unsigned int nextDesignId = 0;

        const std::size_t nSurfaces = itkDetector.surfaces().size();
        m_staticCondEntries.resize(nSurfaces);

        auto idMapping = std::make_unique<ActsTrk::GeometryIdMapping>();
        idMapping->reserve(nSurfaces);

        for (std::size_t condIndex = 0; condIndex < nSurfaces; ++condIndex) {
            const auto& surface = itkDetector.surfaces()[condIndex];
             if (!surface.is_sensitive()) {
                ATH_MSG_VERBOSE("Skipping passive surface with geometryId " << surface.source);
                continue;
            }
            const auto detray_id = surface.identifier().value();
            const auto actsIdOpt = detraySurfaceIds->fromDetray(surface.identifier());
            if (!actsIdOpt) {
                ATH_MSG_ERROR("Sensitive detray surface " << detray_id
                              << " is missing from the ACTS<->Detray surface map");
                return StatusCode::FAILURE;
            }
            const Acts::GeometryIdentifier acts_geom_id = *actsIdOpt;

            StaticCondEntry entry;
            entry.detrayGeometryId = surface.identifier();
            entry.actsGeometryId = acts_geom_id.value();

            auto athenaIt = actsToAthena.find(acts_geom_id);
            if (athenaIt == actsToAthena.end()) {
                if (std::find(actsHGTD.begin(), actsHGTD.end(), acts_geom_id) != actsHGTD.end()) {
                    ATH_MSG_VERBOSE("ACTS surface with key " << acts_geom_id << " is HGTD, detray id: " << detray_id);
                    continue;
                }
                ATH_MSG_ERROR("Could not find matching Athena module for detray surface " << detray_id
                              << " (ACTS geometryId " << acts_geom_id << ")");
                return StatusCode::FAILURE;
            }

            const std::optional<Identifier> athenaId = athenaIt->second;
            if (auto modIt = m_atlasModuleInfo.find(*athenaId); modIt != m_atlasModuleInfo.end()) {
                const moduleInfo& thismod = modIt->second;

                const int nBinsX = thismod.rows;
                const int nBinsY = thismod.pixel ? thismod.columns : 1;

                auto edgesX = makeEquidistantEdges(0.5f * thismod.module_width, nBinsX);
                auto edgesY = makeEquidistantEdges(0.5f * thismod.module_length, nBinsY);

                if(!thismod.equidistant_binning){
                    edgesX = thismod.row_centres;
                    edgesY = thismod.column_centres;
                }

                designKey key{thismod.pixel, thismod.isAnnulus, std::move(edgesX), std::move(edgesY),
                            std::abs(thismod.module_width), thismod.module_length};

                const auto [it, inserted] = designLookup.try_emplace(key, nextDesignId);
                entry.designId = it->second;
                if (inserted) ++nextDesignId;

                entry.athenaId = *athenaId;
                entry.hasAthenaModule = true;
                entry.isPixel = thismod.pixel;
            }

            idMapping->addEntry(detray_id, acts_geom_id.value(), athenaId);
            // condIndex is the row of this surface in the detector conditions description
            idMapping->addDetDescIndex(detray_id, static_cast<unsigned int>(condIndex));
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
        ATH_CHECK(m_detStore->record(std::move(idMapping), m_geoIdMappingObjectName.value(), allowMods));

        auto deviceDetector = std::make_unique<traccc::detector_buffer>();
        deviceDetector->set<traccc::itk_detector>(
            detray::get_buffer(itkDetector, m_MRs->mainMR(), const_cast<vecmem::copy&>(*initCopy)));
        ATH_CHECK(m_detStore->record(std::move(deviceDetector), m_deviceDetectorName.value(), allowMods));
        
        ATH_MSG_DEBUG("Recorded host and device detector design description: " << m_hostDesignObjectName.value() << ", " << m_deviceDesignObjectName.value());

    } else {
        ATH_MSG_FATAL("Tracking Geometry empty!!");
        return StatusCode::FAILURE;
    }


    return StatusCode::SUCCESS;
}

} // namespace ActsTrk
