// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimLayerStudyAlg.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "FPGATrackSimBinning/FPGATrackSimBinUtil.h"
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimDataFlowInfo.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"

#include "FPGATrackSimAlgorithms/FPGATrackSimNNTrackTool.h"
#include "FPGATrackSimAlgorithms/FPGATrackSimOverlapRemovalTool.h"
#include "FPGATrackSimAlgorithms/FPGATrackSimTrackFitterTool.h"

#include "FPGATrackSimConfTools/FPGATrackSimRegionSlices.h"

#include "FPGATrackSimInput/FPGATrackSimRawToLogicalHitsTool.h"
#include "FPGATrackSimInput/FPGATrackSimReadRawRandomHitsTool.h"

#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"

#include "GaudiKernel/IEventProcessor.h"

constexpr bool enableBenchmark = 
#ifdef BENCHMARK_LAYERSTUDYALG
    true;
#else
    false;
#endif

///////////////////////////////////////////////////////////////////////////////
// Initialize

FPGATrackSimLayerStudyAlg::FPGATrackSimLayerStudyAlg (const std::string& name, ISvcLocator* pSvcLocator) :
    AthAlgorithm(name, pSvcLocator)
{
}


StatusCode FPGATrackSimLayerStudyAlg::initialize ATLAS_NOT_THREAD_SAFE()
{
    std::stringstream ss(m_description);
    std::string line;
    ATH_MSG_INFO("Tag config:");
    if (!m_description.empty()) {
        while (std::getline(ss, line, '\n')) {
            ATH_MSG_INFO('\t' << line);
        }
    }

    ATH_CHECK(m_hitBinningTool.retrieve());
    ATH_CHECK(m_binMonitoring.retrieve());
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    ATH_CHECK(m_evtSel.retrieve());

    // TODO add this here
    ATH_MSG_DEBUG("initialize() Instantiating output layer study tree");

    // Setup layer configuration if not already set from layerMap
    if (m_hitBinningTool->getNLayers()==0) {
        m_hitBinningTool->setNLayers(m_FPGATrackSimMapping->PlaneMap_1st(0)->getNLogiLayers());
    }

    // Set up the histograms. Use the event selection service to test whether or not this is single particle.
    // It may be better to not use a separate tool for this, and just fold it into this algorithm.
    // The reason to keep a separate tool would be is if we eventually want to migrate *all* of GenScanMonitoring -> BinMonitoring,
    // but that can't happen until GenScan is fully converted to use the new binning.
    ATH_CHECK(m_binMonitoring->registerHistograms(m_hitBinningTool.get(), (m_evtSel->getSampleType() == SampleType::skipTruth)));

    // Retrieve truth tracks.
    ATH_CHECK(m_FPGAHitKey.initialize());
    ATH_CHECK(m_FPGATruthTrackKey.initialize());

    ATH_CHECK(m_chrono.retrieve());
    ATH_MSG_DEBUG("initialize() Finished");

    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//                          MAIN EXECUTE ROUTINE                             //
///////////////////////////////////////////////////////////////////////////////

StatusCode FPGATrackSimLayerStudyAlg::execute ATLAS_NOT_THREAD_SAFE()
{
    const EventContext& ctx = getContext();

    // Get reference to hits from StoreGate.
    SG::ReadHandle<FPGATrackSimHitCollection> FPGAHits(m_FPGAHitKey, ctx);
    if (!FPGAHits.isValid()) {
        if (m_evt == 0) {
            ATH_MSG_WARNING("Didn't receive " << FPGAHits.key() << " on first event; assuming no input events.");
        }
        SmartIF<IEventProcessor> appMgr{service("ApplicationMgr")};
        if (!appMgr) {
            ATH_MSG_ERROR("Failed to retrieve ApplicationMgr as IEventProcessor");
            return StatusCode::FAILURE;
        }
        return appMgr->stopRun();
    }

    // Query the event selection service to make sure this event passed cuts.
    if (!m_evtSel->getSelectedEvent()) {
        ATH_MSG_DEBUG("Event skipped by: " << m_evtSel->name());
        return StatusCode::SUCCESS;
    }

    // Event passes cuts, count it. technically, DataPrep does this now.
    m_evt++;

    if constexpr (enableBenchmark) m_chrono->chronoStart("Layer Study: Split hits to 1st and 2nd stage");

    // It shouldn't matter which slice we use; there should really only be one here
    const FPGATrackSimPlaneMap* pmap_2nd = m_FPGATrackSimMapping->PlaneMap_2nd(0);

    std::vector<FPGATrackSimHit> hits;
    std::vector<std::shared_ptr<const FPGATrackSimHit>> phits;
    const FPGATrackSimRegionMap* rmap_1st = m_FPGATrackSimMapping->SubRegionMap();
    phits.reserve(FPGAHits->size());
    hits.reserve(FPGAHits->size());

    // For stage = 0, we'll run over all of them. Otherwise only look at first or second stage.
    for (const FPGATrackSimHit& hit : *FPGAHits) {
        // Acquire a non-constant copy of the hit, this is messy, but we have to map them.
        FPGATrackSimHit hitCopy = hit;
        pmap_2nd->map(hitCopy);
        hits.push_back(hitCopy);

        switch (m_stage) {
            case 0:
                phits.push_back(std::make_shared<FPGATrackSimHit>(hitCopy));
                break;
            case 1:
                if (rmap_1st->getRegions(hit).size() > 0) phits.push_back(std::make_shared<FPGATrackSimHit>(hitCopy));
                break;
            case 2:
                if (rmap_1st->getRegions(hit).size() == 0) phits.push_back(std::make_shared<FPGATrackSimHit>(hitCopy));
                break;
            default:
                ATH_MSG_FATAL("Unrecognized stage: " << m_stage << ", will exit layer study");
                break;
        }
    }

    if constexpr (enableBenchmark) m_chrono->chronoStop("Layer Study: Split hits to 1st and 2nd stage");

    // Get truth tracks from DataPrep as well.
    SG::ReadHandle<FPGATrackSimTruthTrackCollection> FPGATruthTracks(m_FPGATruthTrackKey, ctx);
    if (!FPGATruthTracks.isValid()) {
        ATH_MSG_ERROR("Could not find FPGA Truth Track Collection with key " << FPGATruthTracks.key());
        return StatusCode::FAILURE;
    }

    // Update truth information in output layer study tree.
    m_binMonitoring->parseTruthInfo(*FPGATruthTracks);
    m_hitBinningTool->getBinTool().binDesc()->setTruthBin(m_binMonitoring->truthBin());

    // Make hit level plots
    for (auto &hit : phits) {
        m_binMonitoring->fillHitLevelInput(hit.get());
    }

    // Bin the hits, depending on m_stage we either use phits_1st, phits_2nd, or all the hits.
    ATH_CHECK(m_hitBinningTool->fill(phits));
    m_binMonitoring->fillBinningSummary(phits);

    // scan over image building pairs for bins over threshold
    for (FPGATrackSimBinArray<FPGATrackSimBinnedHits::BinEntry>::ConstIterator &bin : m_hitBinningTool->lastStepBinnedHits()) {
        // Apply threshold, of course if threshold is 0 then use all bins
        if (bin.data().hitCnt < m_threshold) {
            continue;
        } else {
            if (FPGATrackSimBinUtil::IdxSet(bin.idx())==m_binMonitoring->truthBin(m_hitBinningTool->getBinTool().lastStep()->stepNum())) {
                ATH_MSG_DEBUG("Truth bin failed threshold " << bin.data().hitCnt << " thr=" << m_threshold << " " << bin.idx());}
        }
        ATH_MSG_DEBUG("Bin passes threshold " << bin.data().hitCnt << " " << bin.idx());

        // Monitor contents of bins passing threshold
        m_binMonitoring->fillBinLevelOutput(bin.idx(), bin.data());
    }


    // Reset the hit binning tool.
    m_hitBinningTool->resetBins();

    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// Finalize

StatusCode FPGATrackSimLayerStudyAlg::finalize()
{
    ATH_MSG_INFO("PRINTING FPGATRACKSIM SIMPLE STATS");
    ATH_MSG_INFO("========================================================================================");    
    ATH_MSG_INFO("Ran on events = " << m_evt);

    return StatusCode::SUCCESS;
}
