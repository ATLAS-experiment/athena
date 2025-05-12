/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonLayerSegmentFinderTool.h"

#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "MuonLayerHough/MuonLayerHough.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "MuonSegment/MuonSegment.h"
#include "TrkSegment/SegmentCollection.h"
#include "TrkEventPrimitives/FitQuality.h"
#include <typeinfo>
using namespace xAOD::P4Helpers;
namespace {
    const float OneOverSqrt12 = 1. / std::sqrt(12);
}
namespace Muon {

    MuonLayerSegmentFinderTool::MuonLayerSegmentFinderTool(const std::string& type, const std::string& name, const IInterface* parent) :
        AthAlgTool(type, name, parent) {
        declareInterface<IMuonLayerSegmentFinderTool>(this);
    }

    StatusCode MuonLayerSegmentFinderTool::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_muonPRDSelectionTool.retrieve());
        ATH_CHECK(m_segmentMaker.retrieve());
        ATH_CHECK(m_csc2dSegmentFinder.retrieve(DisableTool{!m_idHelperSvc->hasCSC() || m_csc2dSegmentFinder.empty()}));
        ATH_CHECK(m_csc4dSegmentFinder.retrieve(DisableTool{!m_idHelperSvc->hasCSC() || m_csc4dSegmentFinder.empty()}));
        ATH_CHECK(m_patternSegs.initialize(!m_patternSegs.empty()));
        ATH_CHECK(m_segmentMatchingTool.retrieve(DisableTool{m_patternSegs.empty()}));       
        ATH_CHECK(m_houghDataPerSectorVecKey.initialize(!m_houghDataPerSectorVecKey.empty()));        
        ATH_CHECK(m_clusterSegMakerNSW.retrieve(DisableTool{m_clusterSegMakerNSW.empty()|| !m_patternSegs.empty()}));
      
        return StatusCode::SUCCESS;
    }

    void MuonLayerSegmentFinderTool::find(const EventContext& ctx, const MuonSystemExtension::Intersection& intersection, 
                                          const MuonLayerPrepRawData& layerPrepRawData, std::vector<std::shared_ptr<const Muon::MuonSegment> >& segments) const {
        ATH_MSG_VERBOSE(
            " Running segment finding in sector "
            << intersection.layerSurface.sector << " region " << MuonStationIndex::regionName(intersection.layerSurface.regionIndex)
            << " layer " << MuonStationIndex::layerName(intersection.layerSurface.layerIndex) << " intersection position: r "
            << intersection.trackParameters->position().perp() << " z " << intersection.trackParameters->position().z() << " locX "
            << intersection.trackParameters->parameters()[Trk::locX] << " locY " << intersection.trackParameters->parameters()[Trk::locY]
            << " phi " << intersection.trackParameters->position().phi());

        // run cluster hit based segment finding on PRDs
        findClusterSegments(ctx, intersection, layerPrepRawData, segments);
        ATH_MSG_VERBOSE(" findClusterSegments " << segments.size());

        // run standard MDT/Trigger hit segment finding either from Hough or hits
        findMdtSegments(intersection, layerPrepRawData, segments);
    }

    void MuonLayerSegmentFinderTool::findMdtSegments(const MuonSystemExtension::Intersection& intersection,
                                                     const MuonLayerPrepRawData& layerPrepRawData,
                                                     std::vector<std::shared_ptr<const Muon::MuonSegment>>& segments) const {
        // calibrate what is already there
        MuonLayerROTs layerROTs;
        if (!m_muonPRDSelectionTool->calibrateAndSelect(intersection, layerPrepRawData, layerROTs)) {
            ATH_MSG_WARNING("Failed to calibrate and select layer data");
            return;
        }

        // get hits
        MuonStationIndex::TechnologyIndex clusterTech =
            intersection.layerSurface.regionIndex == MuonStationIndex::Barrel ? MuonStationIndex::RPC : MuonStationIndex::TGC;
        const std::vector<const MdtDriftCircleOnTrack*>& mdts = layerROTs.getMdts();
        const std::vector<const MuonClusterOnTrack*>& clusters = layerROTs.getClusters(clusterTech);

        findMdtSegments(intersection, mdts, clusters, segments);
    }

    void MuonLayerSegmentFinderTool::findMdtSegments(const MuonSystemExtension::Intersection& intersection,
                                                     const std::vector<const MdtDriftCircleOnTrack*>& mdts,
                                                     const std::vector<const MuonClusterOnTrack*>& clusters,
                                                     std::vector<std::shared_ptr<const Muon::MuonSegment>>& segments) const {
        // require at least 2 MDT hits
        if (mdts.size() <= 2) return;
        // run segment finder
        std::unique_ptr<Trk::SegmentCollection> segColl = std::make_unique<Trk::SegmentCollection>(SG::VIEW_ELEMENTS);
        m_segmentMaker->find(intersection.trackParameters->position(), intersection.trackParameters->momentum(), mdts, clusters,
                             !clusters.empty(), segColl.get(), intersection.trackParameters->momentum().mag());

        for (Trk::Segment* tseg : *segColl) {
            MuonSegment* mseg = dynamic_cast<MuonSegment*>(tseg);
            ATH_MSG_DEBUG(" " << m_printer->print(*mseg));
            segments.emplace_back(mseg);
        }
    }

    void MuonLayerSegmentFinderTool::findClusterSegments(const EventContext& ctx, const MuonSystemExtension::Intersection& intersection,
                                                         const MuonLayerPrepRawData& layerPrepRawData,
                                                         std::vector<std::shared_ptr<const Muon::MuonSegment>>& segments) const {
        // if there are CSC hits run CSC segment finding
        if (!layerPrepRawData.cscs.empty()) findCscSegments(ctx, layerPrepRawData, segments);

        // No need to call the NSW segment finding
        if (layerPrepRawData.mms.empty() && layerPrepRawData.stgcs.empty()) return;

        // NSW segment finding
        MuonLayerROTs layerROTs;
        if (!m_muonPRDSelectionTool->calibrateAndSelect(intersection, layerPrepRawData, layerROTs)) {
            ATH_MSG_WARNING("Failed to calibrate and select layer data");
            return;
        }

        ATH_MSG_DEBUG(" MM prds " << layerPrepRawData.mms.size() << " STGC prds " << layerPrepRawData.stgcs.size());

        // get STGC and MM clusters
        const std::vector<const MuonClusterOnTrack*>& clustersSTGC = layerROTs.getClusters(MuonStationIndex::STGC);
        const std::vector<const MuonClusterOnTrack*>& clustersMM = layerROTs.getClusters(MuonStationIndex::MM);

        using NSWSegmentCache = Muon::IMuonNSWSegmentFinderTool::SegmentMakingCache;
        NSWSegmentCache cache{};

       
        if (!clustersSTGC.empty()) {
            ATH_MSG_DEBUG(" STGC clusters " << clustersSTGC.size());
            std::transform(clustersSTGC.begin(), clustersSTGC.end(), std::back_inserter(cache.inputClust), 
                            [](const Muon::MuonClusterOnTrack* cl){ return std::unique_ptr<Muon::MuonClusterOnTrack>{cl->clone()};});
           
        }
        if (!clustersMM.empty()) {
            ATH_MSG_DEBUG(" MM clusters " << clustersMM.size());
            std::transform(clustersMM.begin(), clustersMM.end(), std::back_inserter(cache.inputClust), 
                            [](const Muon::MuonClusterOnTrack* cl){ return  std::unique_ptr<Muon::MuonClusterOnTrack>{cl->clone()};});            
        }
        if (cache.inputClust.empty()) return;
        
        
        if (!m_patternSegs.empty()) {
            std::set<Identifier> needed_rios{};        
            for (std::unique_ptr<const MuonClusterOnTrack>& clus : cache.inputClust) {
                needed_rios.insert(clus->identify());
            }
            SG::ReadHandle<Trk::SegmentCollection>  input_segs{m_patternSegs, ctx};
            for (const Trk::Segment *trk_seg : *input_segs) {
                const MuonSegment *seg = dynamic_cast<const Muon::MuonSegment *>(trk_seg);
                // Initial check that the segments are on the same detector side
                if (intersection.trackParameters->associatedSurface().center().z() * seg->globalPosition().z() < 0) continue;
            
                /// Find whether the segment has some hits that are needed
                bool hasNSW{false};
                for (size_t n = 0; !hasNSW && n < seg->numberOfContainedROTs(); ++n) {
                    const MuonClusterOnTrack *clus = dynamic_cast<const MuonClusterOnTrack *>(seg->rioOnTrack(n));
                    hasNSW |=  (clus && needed_rios.count(clus->identify()));              
                }
                /// Nope
                if (!hasNSW) continue;            
                /// Segment is compatible
                if (m_segmentMatchingTool->match(ctx, intersection, *seg)) segments.emplace_back(seg->clone());
            }
            /// If no NSW was reconstructed thus far there's no hope that we'll do it later as well. Give up
            /// Lasst, die Ihr eintretet, alle Hoffnung fahren!
            return;
        }

        m_clusterSegMakerNSW->find(ctx, cache);
        
        for (std::unique_ptr<MuonSegment>& seg : cache.constructedSegs) {
            ATH_MSG_DEBUG(" NSW segment " << m_printer->print(*seg));
            segments.emplace_back(std::move(seg));
        }        
    }

    void MuonLayerSegmentFinderTool::findCscSegments(const EventContext& ctx, const MuonLayerPrepRawData& layerPrepRawData,
                                                     std::vector<std::shared_ptr<const Muon::MuonSegment>>& segments) const {
        // run 2d segment finder
        std::unique_ptr<MuonSegmentCombinationCollection> combi2D = m_csc2dSegmentFinder->find(layerPrepRawData.cscs, ctx);
        if (!combi2D) return;

        // run 4d segment finder
        std::unique_ptr<MuonSegmentCombinationCollection> combi4D = m_csc4dSegmentFinder->find(*combi2D, ctx);
        if (!combi4D) return;

        // extract segments and clean-up memory
        for (auto com : *combi4D) {
            const Muon::MuonSegmentCombination& combi = *com;
            unsigned int nstations = combi.numberOfStations();

            // loop over chambers in combi and extract segments
            for (unsigned int i = 0; i < nstations; ++i) {
                // loop over segments in station
                Muon::MuonSegmentCombination::SegmentVec* segs = combi.stationSegments(i);

                // check if not empty
                if (!segs || segs->empty()) continue;
                // loop over new segments, copy them into collection
                for (std::unique_ptr<MuonSegment>& seg_it : *segs) {
                    ATH_MSG_DEBUG(" " << m_printer->print(*seg_it));
                    segments.emplace_back(std::move(seg_it));
                }
            }
        }
    }
    void MuonLayerSegmentFinderTool::findMdtSegmentsFromHough(const EventContext& ctx,
                                                             const MuonSystemExtension::Intersection& intersection, 
                                                             std::vector<std::shared_ptr<const Muon::MuonSegment> >& segments) const {

        if(m_houghDataPerSectorVecKey.empty()) return;
        unsigned int                          nprevSegments = segments.size();  // keep track of what is already there
        int                                   sector        = intersection.layerSurface.sector;
        MuonStationIndex::DetectorRegionIndex regionIndex   = intersection.layerSurface.regionIndex;
        MuonStationIndex::LayerIndex          layerIndex    = intersection.layerSurface.layerIndex;

        // get hough data
        SG::ReadHandle houghDataPerSectorVec{m_houghDataPerSectorVecKey, ctx};
        if (!houghDataPerSectorVec.isValid()) {
            ATH_MSG_ERROR("Hough data per sector vector not found");
            return;
        }

        // sanity check
        if (static_cast<int>(houghDataPerSectorVec->vec.size()) <= sector - 1) {
            ATH_MSG_WARNING(" MuonLayerHoughTool::HoughDataPerSectorVec smaller than sector "
                            << houghDataPerSectorVec->vec.size() << " sector " << sector);
            return;
        }

        // get hough maxima in the layer
        unsigned int sectorLayerHash = MuonStationIndex::sectorLayerHash(regionIndex, layerIndex);
        const MuonLayerHoughTool::HoughDataPerSector& houghDataPerSector = houghDataPerSectorVec->vec[sector - 1];
        ATH_MSG_DEBUG(" findMdtSegmentsFromHough: sector "
                    << sector << " " << MuonStationIndex::regionName(regionIndex) << " "
                    << MuonStationIndex::layerName(layerIndex) << " sector hash " << sectorLayerHash << " houghData "
                    << houghDataPerSectorVec->vec.size() << " " << houghDataPerSector.maxVec.size());

        // sanity check
        if (houghDataPerSector.maxVec.size() <= sectorLayerHash) {
            ATH_MSG_WARNING(" houghDataPerSector.maxVec.size() smaller than hash " << houghDataPerSector.maxVec.size()
                                                                                << " hash " << sectorLayerHash);
            return;
        }
        const MuonLayerHoughTool::MaximumVec& maxVec = houghDataPerSector.maxVec[sectorLayerHash];

        // get local coordinates in the layer frame
        bool barrelLike = intersection.layerSurface.regionIndex == MuonStationIndex::Barrel;

        float phi = intersection.trackParameters->position().phi();

        // in the endcaps take the r in the sector frame from the local position of the extrapolation
        float r = barrelLike ? m_muonSectorMapping.transformRToSector(intersection.trackParameters->position().perp(), phi,
                                                                    intersection.layerSurface.sector, true)
                            : intersection.trackParameters->parameters()[Trk::locX];

        float z     = intersection.trackParameters->position().z();
        float errx  = Amg::error(*intersection.trackParameters->covariance(), Trk::locX);
        float x     = barrelLike ? r : z;
        float y     = barrelLike ? z : r;
        float theta = std::atan2(x, y);

        ATH_MSG_DEBUG("  Got Hough maxima " << maxVec.size() << " extrapolated position in Hough space (" << x << "," << y
                                            << ") error " << errx << " "
                                            << " angle " << theta);

        // lambda to handle calibration and selection of MDTs
        std::vector<std::unique_ptr<const Trk::MeasurementBase>> garbage;
        auto handleMdt = [this, intersection, &garbage](const MdtPrepData& prd, std::vector<const MdtDriftCircleOnTrack*>& mdts) {
            const MdtDriftCircleOnTrack* mdt = m_muonPRDSelectionTool->calibrateAndSelect(intersection, prd);
            if (!mdt) return;
            mdts.push_back(mdt);
            garbage.emplace_back(mdt);
        };


        // lambda to handle calibration and selection of clusters
        auto handleCluster = [this, intersection,&garbage](const MuonCluster&                      prd,
                                                std::vector<const MuonClusterOnTrack*>& clusters) {
            const MuonClusterOnTrack* cluster = m_muonPRDSelectionTool->calibrateAndSelect(intersection, prd);
            if (!cluster) return;
            clusters.push_back(cluster);
            garbage.emplace_back(cluster);
        };


        // loop over maxima and associate them to the extrapolation
        MuonLayerHoughTool::MaximumVec::const_iterator mit     = maxVec.begin();
        MuonLayerHoughTool::MaximumVec::const_iterator mit_end = maxVec.end();
        for (; mit != mit_end; ++mit) {
            const MuonHough::MuonLayerHough::Maximum& maximum       = **mit;
            float                                     residual      = maximum.pos - y;
            float                                     residualTheta = maximum.theta - theta;
            float refPos   = (maximum.hough != nullptr) ? maximum.hough->m_descriptor.referencePosition : 0;
            float maxwidth = (maximum.binposmax - maximum.binposmin);
            if (maximum.hough) maxwidth *= maximum.hough->m_binsize;
            float pull = residual / std::hypot(errx ,  maxwidth * OneOverSqrt12);

            
            ATH_MSG_DEBUG("   Hough maximum " << maximum.max << " position (" << refPos << "," << maximum.pos
                                            << ") residual " << residual << " pull " << pull << " angle " << maximum.theta
                                            << " residual " << residualTheta);

            // select maximum
            if (std::abs(pull) > 5) continue;

            // loop over hits in maximum and add them to the hit list
            std::vector<const MdtDriftCircleOnTrack*>    mdts;
            std::vector<const MuonClusterOnTrack*>       clusters;
            for (const auto& hit : maximum.hits) {

                // treat the case that the hit is a composite TGC hit
                if (hit->tgc) {
                    for (const auto& prd : hit->tgc->etaCluster) {
                        handleCluster(*prd, clusters);
                    }
                } else if (hit->prd) {
                    Identifier id = hit->prd->identify();
                    if (m_idHelperSvc->isMdt(id))
                        handleMdt(static_cast<const MdtPrepData&>(*hit->prd), mdts);
                    else
                        handleCluster(static_cast<const MuonCluster&>(*hit->prd), clusters);
                }
            }

            // get phi hits
            const MuonLayerHoughTool::PhiMaximumVec& phiMaxVec =
                houghDataPerSector.phiMaxVec[intersection.layerSurface.regionIndex];
            ATH_MSG_DEBUG("   Got Phi Hough maxima " << phiMaxVec.size() << " phi " << phi);

            // loop over maxima and associate them to the extrapolation
            MuonLayerHoughTool::PhiMaximumVec::const_iterator pit     = phiMaxVec.begin();
            MuonLayerHoughTool::PhiMaximumVec::const_iterator pit_end = phiMaxVec.end();
            for (; pit != pit_end; ++pit) {
                const MuonHough::MuonPhiLayerHough::Maximum& maximum  = **pit;
                const float residual = deltaPhi( maximum.pos, phi);
                
                ATH_MSG_DEBUG("     Phi Hough maximum " << maximum.max << " phi " << maximum.pos << ") angle "
                                                        << maximum.pos << " residual " << residual);

                for (const auto& phi_hit : maximum.hits) {
                    // treat the case that the hit is a composite TGC hit
                    if (phi_hit->tgc) {
                        Identifier id = phi_hit->tgc->phiCluster.front()->identify();
                        if (m_idHelperSvc->layerIndex(id) != intersection.layerSurface.layerIndex) continue;
                        for (const auto& prd : phi_hit->tgc->phiCluster) handleCluster(*prd, clusters);
                    } else if (phi_hit->prd) {
                        Identifier id = phi_hit->prd->identify();
                        if (m_idHelperSvc->layerIndex(id) != intersection.layerSurface.layerIndex) continue;
                        handleCluster(static_cast<const MuonCluster&>(*phi_hit->prd), clusters);
                    }
                }
            }

            // call segment finder
            ATH_MSG_DEBUG("    Got hits: mdts " << mdts.size() << " clusters " << clusters.size());
            findMdtSegments(intersection, mdts, clusters, segments);

            // clean-up memory
            garbage.clear();
            ATH_MSG_DEBUG("  Done maximum: new segments " << segments.size() - nprevSegments);
        }
        ATH_MSG_DEBUG("  Done with layer: new segments " << segments.size() - nprevSegments);
   }

}  // namespace Muon
