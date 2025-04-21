/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MOOSEGMENTFINDERS_MUOSEGMENTFINDERALGS_H
#define MOOSEGMENTFINDERS_MUOSEGMENTFINDERALGS_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CscSegmentMakers/ICscSegmentFinder.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPattern/MuonPatternChamberIntersect.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonRecToolInterfaces/IMuonClusterOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonSegmentMaker.h"
#include "MuonSegment/MuonSegmentCombinationCollection.h"
#include "MuonSegmentMakerToolInterfaces/IMuonSegmentSelectionTool.h"
#include "MuonSegmentMakerToolInterfaces/IMuonNSWSegmentFinderTool.h"
#include "MuonSegmentMakerToolInterfaces/IMuonPatternCalibration.h"
#include "MuonSegmentMakerToolInterfaces/IMuonSegmentOverlapRemovalTool.h"
#include "TrkSegment/SegmentCollection.h"

class MuonSegmentFinderAlg : public AthReentrantAlgorithm {
public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual ~MuonSegmentFinderAlg() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this,
        "MuonIdHelperSvc",
        "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
    };

    PublicToolHandle<Muon::MuonEDMPrinterTool> m_printer{
        this,
        "EDMPrinter",
        "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool",
    };  //<! helper printer tool
    ToolHandle<Muon::IMuonPatternCalibration> m_patternCalibration{
        this,
        "MuonPatternCalibration",
        "Muon::MuonPatternCalibration/MuonPatternCalibration",
    };   
    ToolHandle<Muon::IMuonSegmentMaker> m_segmentMaker{
        this,
        "SegmentMaker",
        "Muon::DCMathSegmentMaker/DCMathSegmentMaker",
    };
    ToolHandle<Muon::IMuonSegmentOverlapRemovalTool> m_segmentOverlapRemovalTool{
        this,
        "MuonSegmentOverlapRemovalTool",
        "Muon::MuonSegmentOverlapRemovalTool/MuonSegmentOverlapRemovalTool",
    };
    ToolHandle<Muon::IMuonClusterOnTrackCreator> m_clusterCreator{
        this,
        "MuonClusterCreator",
        "Muon::MuonClusterOnTrackCreator/MuonClusterOnTrackCreator",
    };  //<! pointer to muon cluster rio ontrack creator
    ToolHandle<Muon::IMuonNSWSegmentFinderTool> m_clusterSegMakerNSW{
        this,
        "NSWSegmentMaker",
        "",
    };
    ToolHandle<ICscSegmentFinder> m_csc2dSegmentFinder{
        this,
        "Csc2dSegmentMaker",
        "Csc2dSegmentMaker/Csc2dSegmentMaker",
    };
    ToolHandle<ICscSegmentFinder> m_csc4dSegmentFinder{
        this,
        "Csc4dSegmentMaker",
        "Csc4dSegmentMaker/Csc4dSegmentMaker",
    };
    
    ToolHandle<Muon::IMuonSegmentSelectionTool> m_segmentSelector{this, "SegmentSelector",
                                                                "Muon::MuonSegmentSelectionTool/MuonSegmentSelectionTool"};

    // the following Trk::SegmentCollection MuonSegments are standard MuonSegments, the MuGirl segments are stored in MuonCreatorAlg.h
    SG::WriteHandleKey<Trk::SegmentCollection> m_segmentCollectionKey{
        this,
        "SegmentCollectionName",
        "TrackMuonSegments",
        "Muon Segments",
    };
    SG::WriteHandleKey<Trk::SegmentCollection> m_segmentNSWCollectionKey{ //this collection of segments are used to perform the alignment of the NSW
      this,
        "NSWSegmentCollectionName",
        "TrackMuonNSWSegments",
        "WriteHandleKey for NSW Segments",
    };
    SG::ReadHandleKey<Muon::CscPrepDataContainer> m_cscPrdsKey{
        this,
        "CSC_clusterkey",
        "CSC_Clusters",
        "CSC PRDs",
    };
    SG::ReadHandleKey<MuonPatternCombinationCollection> m_patternCollKey{
        this,
        "MuonLayerHoughCombisKey",
        "MuonLayerHoughCombis",
        "Hough combinations",
    };

    StatusCode createSegmentsWithMDTs(const EventContext& ctx, const Muon::MuonPatternCombination* patt, Trk::SegmentCollection* segs) const;
    
    
    using NSWSegmentCache = Muon::IMuonNSWSegmentFinderTool::SegmentMakingCache;
    void createNSWSegments(const EventContext& ctx, 
                           const Muon::MuonPatternCombination* patt, 
                           NSWSegmentCache& cache) const;
   
    /// Retrieve the raw outputs from the Csc segment makers for the curved combination
    StatusCode createCscSegments(const EventContext& ctx,
                                 std::unique_ptr<MuonSegmentCombinationCollection>& csc4dSegmentCombinations) const;

    void appendSegmentsFromCombi(const std::unique_ptr<MuonSegmentCombinationCollection>& combi_coll, 
                                 Trk::SegmentCollection* segments) const;


    Gaudi::Property<bool> m_printSummary{this, "PrintSummary", false};

    /// Run segment finding with eta / phi determination
    Gaudi::Property<bool> m_doFullFinder{this, "FullFinder", true}; 
    /// Run the Mdt segment maker (Switched of the NCB systems)
    Gaudi::Property<bool> m_runMdtSegments{this, "doMdtSegments", true};
    /// Run the NSW segment maker
    Gaudi::Property<bool> m_doSTgcSegments{this, "doStgcSegments", true};
    Gaudi::Property<bool> m_doMMSegments{this, "doMMSegments", true};
    /// If switched to true, hits that have been already successfully combined to a segment are removed from 
    /// future searches
    Gaudi::Property<bool> m_removeUsedNswHits{this, "removeUsedNSW", true};
    /// Apply a preselection on the segments
    Gaudi::Property<int> m_segQuality{this, "SegmentQuality", -1};
   
};

#endif
