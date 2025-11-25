// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimTrackMonitor_H
#define FPGATrackSimTrackMonitor_H

/**
 * @file FPGATrackSimTrackMonitor.h
 * @author E.L.
 * @date August 28th, 2025
 * @brief This is the monitoring for the FPGATrackSimTrackMonitor
 * 
 **/

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"     ////service to book and manage histograms
#include "AthenaBaseComps/AthAlgTool.h"

class TH1D;
class TH2D;

//// detector hits
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
//// fittid track parameters 
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
//// truth track level MC information for efficiency/ purity checks
#include "FPGATrackSimObjects/FPGATrackSimTruthTrack.h"
//// include the header where FPGATrackSimRoad is defined
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
////// include the new header
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"

#include "AthenaMonitoringKernel/GenericMonitoringTool.h"


//// inherits from AthAlgTool (the standard Athena base for tools)
//// works inside Athena algorithms (e.g. track fitting, overlap removal) to record monitoring info
 class FPGATrackSimTrackMonitor : public AthAlgTool
 {
    public:

    ///////////////////////////////////////////////////////////////////////
    // AthAlgTool

    //// Standard Athena constructor pattern.
    FPGATrackSimTrackMonitor(const std::string &, const std::string &, const IInterface *);
    
    //// called once at the start of the job
    //// typically used to register histograms with ITHistSvc or connect to monitoring services
    virtual StatusCode initialize() override;

    //// called during event processing
    //// input: list of roads
    //// purpose: fill histograms/statistics based
    void fillRoad(const std::vector<std::shared_ptr<const FPGATrackSimRoad>>& roads,
                  const std::vector<FPGATrackSimTruthTrack>& truthTracks,
                  size_t nLogicalLayers);

    // track monitor
    void fillTrack(const std::vector<const FPGATrackSimTrack*>& tracks,
                     const std::vector<FPGATrackSimTruthTrack>& truthTracks,
                     float chi2Cut);
    

   private:
    ///////////////////////////////////////////////////////////////////////
    // Properties
    
    //// integrates with the Athena monitoning infrastructure
    ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool", "", "Monitoring tool"};

    //// defines where histograms will be stored in the ROOT output
    Gaudi::Property<std::string> m_dir{this, "dir", {"/TRACKMON/"}, "String name of output directory"};

    //// allows FPGATrsckSimTrackMonitor to book, register and output histograms automatically into the monitoring ROOT file
    //// ServiceHandle<ITHistSvc> is the bridge between the monitoring tool and Athena's central histogram service
    ServiceHandle<ITHistSvc> m_tHistSvc{this, "THistSvc", "THistSvc/THistSvc", "Histogramming service"};

 };

#endif // FPGATrackSimTrackMonitor_H
