/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKALIGNGENTOOLS_MSCONSTRAINTTRACKSPROVIDER_H
#define TRKALIGNGENTOOLS_MSCONSTRAINTTRACKSPROVIDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonRecToolInterfaces/IMuonHitSummaryTool.h"
#include "TrkAlignInterfaces/ITrackCollectionProvider.h"
#include "TrkFitterUtils/FitterTypes.h"

#include "muonEvent/MuonContainer.h"


class TFile;
class TTree;

namespace Trk {
  class IGlobalTrackFitter;

  class MSConstraintTracksProvider : virtual public ITrackCollectionProvider, public AthAlgTool {

  public:
    MSConstraintTracksProvider(const std::string & type, const std::string & name, const IInterface * parent);
    virtual ~MSConstraintTracksProvider();
    
    virtual StatusCode initialize();
    virtual StatusCode finalize();
    
    virtual StatusCode trackCollection(const TrackCollection*& tracks);

    virtual void printSummary();

  private :

    bool combinedMuonSelection(const Analysis::Muon*);
    bool bookNtuple();  
    void initializeNtuple();
    void setNtuple(TFile* ntuple);
    StatusCode fillNtuple();

    ToolHandle<IGlobalTrackFitter> m_trackFitter
      {this, "TrackFitter", "Trk::GlobalChi2Fitter/InDetTrackFitter"};

    ToolHandle<Muon::IMuonHitSummaryTool> m_muonHitSummaryTool
      {this, "MuonHitSummaryTool", "Muon::MuonHitSummaryTool/MuonHitSummaryTool"};

    SG::ReadHandleKey<Analysis::MuonContainer> m_muonContainerKey
      {this, "InputMuonCollection", "MuidMuonCollection"};

    SG::ReadHandleKey<TrackCollection> m_trackContainerKey
      {this, "InputTracksCollection", "Tracks"};

    Gaudi::Property<RunOutlierRemoval> m_runOutlierRemoval
      {this, "RunOutlierRemoval", true, "run outlier removal in the GX2 fitter"};
    Gaudi::Property<bool> m_useMSConstraintTrkOnly
      {this, "UseMSConstraintTrkOnly", true};
    Gaudi::Property<bool> m_doTree{this, "DoTree", true};

    Gaudi::Property<double> m_minPt{this, "MinPt", 15.};
    Gaudi::Property<int> m_minPIXHits{this, "MinPIXHits", 1};
    Gaudi::Property<int> m_minSCTHits{this, "MinSCTHits", 6};
    Gaudi::Property<int> m_minTRTHits{this, "MinTRTHits", 0};
    Gaudi::Property<double> m_maxIDd0{this, "MaxIDd0", 500.};
    Gaudi::Property<double> m_maxIDz0{this, "MaxIDz0", 500.};
    Gaudi::Property<double> m_minIDPt{this, "MinIDPt", 10.};
    Gaudi::Property<int> m_minMDTHits{this, "MDTHits", 15};
    Gaudi::Property<int> m_minRPCPhiHits{this, "MinRPCPhiHits", 0};
    Gaudi::Property<int> m_minTGCPhiHits{this, "MinTGCPhiHits", 0};
    Gaudi::Property<double> m_maxMSd0{this, "MaxMSd0", 500.};
    Gaudi::Property<double> m_maxMSz0{this, "MaxMSz0", 500.};
    Gaudi::Property<double> m_minMSPt{this, "MinMSPt", 0.};
    Gaudi::Property<int> m_maxNumberOfSectors{this, "MaxNumberOfSectors", 1};
    Gaudi::Property<int> m_minNumberOfPhiLayers{this, "MinNumberOfPhiLayers", 2};
    Gaudi::Property<int> m_minStationLayers{this, "MinStationLayers", 3};

    int m_nCBMuonsFromSG = 0;
    int	m_nCBMuonsHasEXandID = 0;
    int m_nCBMuonsPassSelection = 0;
    int m_nCBMuonsFailedRefit = 0;
    int m_nCBMuonsSucRefit = 0;

    // ntuple variables
    TFile* m_ntuple = nullptr;
    TTree* m_tree = nullptr;
    int m_run{};
    int m_event{};
    double m_pID{};
    double m_pMS{};
    double m_ptID{};
    double m_ptMS{};
    int m_charge{};

    double m_combinedEta{};
    double m_IDEta{};
    double m_combinedPhi{};
    double m_IDPhi{};

    double m_pID_constrained{};
    double m_ptID_constrained{};
    double m_IDEta_constrained{};
    double m_IDPhi_constrained{};
    int m_charge_constrained{};

    int m_eBLhits{};
    int m_nBLhits{};

    int m_nPIXDS{};
    int m_nSCTDS{};

    int m_nPIXH{};
    int m_nSCTH{};

    int m_nPIXHits{};
    int m_nSCTHits{};
    int m_nTRTHits{};

    int m_sectors{};
    int m_phiLayers{};
    int m_stationLayers{};

    int m_sectorNum{};
    int m_phiLayerNum{};
    int m_stationLayerNum{};

  }; // end class

} // end namespace

#endif // TRKALIGNGENTOOLS_MSCONSTRAINTTRACKSPROVIDER_H
