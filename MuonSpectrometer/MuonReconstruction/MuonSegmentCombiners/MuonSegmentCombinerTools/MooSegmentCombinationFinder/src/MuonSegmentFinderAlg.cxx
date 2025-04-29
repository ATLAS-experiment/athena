/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSegmentFinderAlg.h"

#include "MuonPattern/MuonPatternChamberIntersect.h"
#include "MuonPattern/MuonPatternCombination.h"
#include "MuonPattern/MuonPatternCombinationCollection.h"
#include "MuonPrepRawData/MuonCluster.h"
#include "MuonPrepRawData/MuonPrepDataCollection.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "MuonSegment/MuonSegment.h"
#include "MuonSegment/MuonSegmentCombination.h"
#include "MuonSegment/MuonSegmentCombinationCollection.h"
#include "TrkParameters/TrackParameters.h"
#include "MuonEDM_AssociationObjects/MuonSegPatAssMap.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"


StatusCode MuonSegmentFinderAlg::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_printer.retrieve());
    ATH_CHECK(m_segmentOverlapRemovalTool.retrieve());
    ATH_CHECK(m_segmentSelector.retrieve(DisableTool{m_segQuality<0}));
    /// MDT segments
    ATH_CHECK(m_patternCalibration.retrieve(DisableTool{!m_runMdtSegments}));
    ATH_CHECK(m_segmentMaker.retrieve(DisableTool{!m_runMdtSegments}));
    
    const bool doNSW = m_doSTgcSegments || m_doMMSegments;
    ATH_CHECK(m_clusterCreator.retrieve(DisableTool{!doNSW}));
    ATH_CHECK(m_clusterSegMakerNSW.retrieve(DisableTool{!doNSW}));   
    ATH_CHECK(m_csc2dSegmentFinder.retrieve(DisableTool{m_cscPrdsKey.empty()}));
    ATH_CHECK(m_csc4dSegmentFinder.retrieve(DisableTool{m_cscPrdsKey.empty()}));
    
    ATH_CHECK(m_segmentCollectionKey.initialize());
    /// Initialize the alignment container in the NSW
    ATH_CHECK(m_segmentNSWCollectionKey.initialize(doNSW && !m_segmentNSWCollectionKey.empty()));
    ATH_CHECK(m_cscPrdsKey.initialize(!m_cscPrdsKey.empty()));  // check for layouts without CSCs
    ATH_CHECK(m_patternCollKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode MuonSegmentFinderAlg::execute(const EventContext& ctx) const {
    
    NSWSegmentCache nswCache{};
    nswCache.buildQuads = !m_segmentNSWCollectionKey.empty();

    std::unique_ptr<Trk::SegmentCollection> segmentContainer = std::make_unique<Trk::SegmentCollection>();
    std::unique_ptr<Trk::SegmentCollection> nswSegmentContainer = !m_segmentNSWCollectionKey.empty() ? std::make_unique<Trk::SegmentCollection>()
                                                                                                     : nullptr;   
    
    
    const MuonPatternCombinationCollection* patternColl{nullptr};
    ATH_CHECK(SG::get(patternColl, m_patternCollKey, ctx));    
    ATH_MSG_DEBUG("Processing the pattern collections with  " << patternColl->size() << " Collections ");

    for (const Muon::MuonPatternCombination* patt :  *patternColl) {
        ATH_MSG_DEBUG("Working on pattern combination " << m_printer->print(*patt));
        // check the technology & call the corresponding segment finder
                
        ATH_CHECK(createSegmentsWithMDTs(ctx, patt, segmentContainer.get()));
        
       
        createNSWSegments(ctx, patt, nswCache);
        
        /// Move the segments into the output
        segmentContainer->insert(segmentContainer->end(), std::make_move_iterator(nswCache.constructedSegs.begin()),
                                                          std::make_move_iterator(nswCache.constructedSegs.end()));
        
        nswCache.constructedSegs.clear();

        if (!nswSegmentContainer) continue;
        nswSegmentContainer->insert(nswSegmentContainer->end(), std::make_move_iterator(nswCache.quadSegs.begin()),
                                                                std::make_move_iterator(nswCache.quadSegs.end()));
        nswCache.quadSegs.clear();
               
    }  // end loop on pattern combinations

    m_segmentOverlapRemovalTool->removeDuplicates(*segmentContainer);

    std::unique_ptr<MuonSegmentCombinationCollection> csc4dSegmentCombinations{};
    ATH_CHECK(createCscSegments(ctx, csc4dSegmentCombinations));
    appendSegmentsFromCombi(csc4dSegmentCombinations, segmentContainer.get());
    
    /// Get rid of all the duplicates in the segment container
    ATH_MSG_DEBUG("segments before overlap removal: " << segmentContainer->size());
    m_segmentOverlapRemovalTool->removeDuplicates(*segmentContainer);
    ATH_MSG_DEBUG(" Segments after overlap removal: " << segmentContainer->size());


    if (m_printSummary){
        ATH_MSG_INFO("Number of segments found " << segmentContainer->size());
        for (Trk::Segment* tseg : *segmentContainer) {
            const Muon::MuonSegment* mseg{dynamic_cast<Muon::MuonSegment*>(tseg)};
            ATH_MSG_INFO(m_printer->print(*mseg)<<std::endl<<m_printer->print(mseg->containedMeasurements())<<std::endl);
        }
    }


    SG::WriteHandle<Trk::SegmentCollection> handle(m_segmentCollectionKey, ctx);
    ATH_CHECK(handle.record(std::move(segmentContainer)));
    
    if (!m_segmentNSWCollectionKey.empty()) {
        m_segmentOverlapRemovalTool->removeDuplicates(*nswSegmentContainer);
        SG::WriteHandle<Trk::SegmentCollection> handle_segNSW(m_segmentNSWCollectionKey, ctx);
        ATH_CHECK(handle_segNSW.record(std::move(nswSegmentContainer)));  
    }
    return StatusCode::SUCCESS;
}  // execute

StatusCode MuonSegmentFinderAlg::createCscSegments(const EventContext& ctx, 
                                    std::unique_ptr<MuonSegmentCombinationCollection>& csc4dSegmentCombinations) const {
    
    const Muon::CscPrepDataContainer* cscPrds{nullptr};
    ATH_CHECK(SG::get(cscPrds, m_cscPrdsKey, ctx));
    if (!cscPrds) return StatusCode::SUCCESS;

    std::vector<const Muon::CscPrepDataCollection*> cscCols;
    std::copy_if(cscPrds->begin(),cscPrds->end(), std::back_inserter(cscCols), [](const Muon::CscPrepDataCollection* coll) {return !coll->empty();});
    ATH_MSG_DEBUG("Retrieved CscPrepDataContainer " << cscCols.size());
    if (cscCols.empty()) return StatusCode::SUCCESS;
    std::unique_ptr<MuonSegmentCombinationCollection> csc2dSegmentCombinations = m_csc2dSegmentFinder->find(cscCols, ctx);
    if (!csc2dSegmentCombinations) return StatusCode::SUCCESS;

    csc4dSegmentCombinations = m_csc4dSegmentFinder->find(*csc2dSegmentCombinations, ctx);
    return StatusCode::SUCCESS;
}
void MuonSegmentFinderAlg::appendSegmentsFromCombi(const std::unique_ptr<MuonSegmentCombinationCollection>& combi_coll, 
                                 Trk::SegmentCollection* segmentContainer) const {
    if (!combi_coll) return;
     /// Push back the output containers
    for (const Muon::MuonSegmentCombination* combi: *combi_coll) {
        if (!combi) {
            ATH_MSG_WARNING(" empty MuonSegmentCombination!!! ");
            continue;
        }
        const unsigned int nstations = combi->numberOfStations();
        const bool useEta = combi->useStripsInSegment(1);
        const bool usePhi = combi->useStripsInSegment(0);

        // loop over chambers in combi and extract segments
        for (unsigned int i = 0; i < nstations; ++i) {
            // loop over segments in station
            Muon::MuonSegmentCombination::SegmentVec* segments = combi->stationSegments(i);
            // check if not empty
            if (!segments || segments->empty()) continue;
            // loop over new segments, copy them into collection
            for (std::unique_ptr<Muon::MuonSegment>& seg : *segments) {                
                if(m_segQuality >=0 && !m_segmentSelector->select(*seg, false, m_segQuality, useEta, usePhi)) continue;                
                ATH_MSG_VERBOSE("Append segment "<<std::endl<<m_printer->print(seg->containedMeasurements()));
                segmentContainer->push_back(std::move(seg));
            }
                
        }
    }
}
void MuonSegmentFinderAlg::createNSWSegments(const EventContext& ctx, 
                           const Muon::MuonPatternCombination* patt, 
                           NSWSegmentCache& cache) const {
    // turn the PRD into MuonCluster
    if (!m_doSTgcSegments && !m_doMMSegments) return;
    std::map<int, std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>> > clustersPerSector;

    
    for (const Muon::MuonPatternChamberIntersect&  it :patt->chamberData()) {
        if (it.prepRawDataVec().empty()) continue;
        
        const Identifier id = it.prepRawDataVec().front()->identify();
        const int sector = m_idHelperSvc->sector(id);
        /// Constrain to NSW hits
        if (!m_idHelperSvc->isMM(id) && !m_idHelperSvc->issTgc(id)) continue;
        for (const Trk::PrepRawData* pit : it.prepRawDataVec()) {
            const Muon::MuonCluster* cl = dynamic_cast<const Muon::MuonCluster*>(pit);
            if (!cl) continue;           
            else if (!m_doMMSegments && m_idHelperSvc->isMM(cl->identify())) continue;
            else if (!m_doSTgcSegments && m_idHelperSvc->issTgc(cl->identify())) continue;
            else if (m_removeUsedNswHits && cache.usedHits.count(cl->identify())) continue;
            const Muon::MuonClusterOnTrack* newCluster = m_clusterCreator->createRIO_OnTrack(*cl, cl->globalPosition());
            if (!newCluster) continue;
            std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>& clusters = clustersPerSector[sector];
            clusters.emplace_back(newCluster);          
        }
    }    
    for (auto&[sector, clusters] :clustersPerSector) {
        ATH_MSG_VERBOSE("Run segment making on sector "<<sector);
        cache.inputClust = std::move(clusters);
        m_clusterSegMakerNSW->find(ctx, cache);
    }
}

StatusCode MuonSegmentFinderAlg::createSegmentsWithMDTs(const EventContext& ctx,
                                                        const Muon::MuonPatternCombination* patcomb, 
                                                        Trk::SegmentCollection* segs) const {
    
    if (!m_runMdtSegments) {
        ATH_MSG_DEBUG("Do not search segments in the Mdt part of the muon spectrometer");
        return StatusCode::SUCCESS;
    }
   
    bool hasPhiMeasurements = m_patternCalibration->checkForPhiMeasurements(*patcomb);
    Muon::IMuonPatternCalibration::ROTsPerRegion hitsPerRegion{};
    ATH_CHECK(m_patternCalibration->calibrate(ctx, *patcomb, hitsPerRegion));
    using MdtVec = Muon::IMuonPatternCalibration::MdtVec;
    using ROTRegion = Muon::IMuonPatternCalibration::ROTRegion; 
    for (const ROTRegion& region : hitsPerRegion) {
        for (const MdtVec& mdts : region.mdts()) {
            if (mdts.empty()) continue;
            ATH_MSG_VERBOSE("Calling segment finding for sector " << m_idHelperSvc->chamberNameString(mdts.front()->identify()));
            // fit the segments
            if (m_doFullFinder) {
                if (msgLvl(MSG::VERBOSE)){
                    std::vector<const Trk::MeasurementBase*> meas {};
                    meas.insert(meas.end(), mdts.begin(), mdts.end());
                    meas.insert(meas.end(), region.clusters().begin(), region.clusters().end());
                    ATH_MSG_VERBOSE("Call segment maker with road pos: "<<Amg::toString(region.regionPos)
                                                             << " road dir: "<<Amg::toString(region.regionDir) 
                                                             <<" nMdts: "<<mdts.size()<<" nClusters "<<region.clusters().size()         
                                                             <<std::endl<<m_printer->print(meas));
                
                }
                m_segmentMaker->find(region.regionPos, region.regionDir, mdts, region.clusters(), hasPhiMeasurements, segs,
                                    region.regionDir.mag());
            } else {
                std::vector<const Trk::RIO_OnTrack*> rios;
                rios.insert(rios.begin(), mdts.begin(), mdts.end());               
                m_segmentMaker->find(rios, segs);
            }
        }  // end loop on hits per region
    }     
    return StatusCode::SUCCESS;    
}
