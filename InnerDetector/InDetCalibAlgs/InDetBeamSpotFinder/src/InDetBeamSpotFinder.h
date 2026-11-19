/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_INDETBEAMSPOTFINDER_H
#define INDET_INDETBEAMSPOTFINDER_H
//////////////////////////////
// Author: Brian Amadio
// Contact: btamadio@lbl.gov
// Main Algorithm: Determine the Beamspot position
//////////////////////////////

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "InDetBeamSpotFinder/IInDetBeamSpotTool.h" //for BeamSpot::Event
#include "InDetBeamSpotFinder/BeamSpotStatusCode.h"
#include "xAODEventInfo/EventInfo.h" //typedef, can't fwd declare
#include "xAODTracking/VertexContainer.h" //typedef, can't fwd declare
#include "xAODTracking/TrackingPrimitives.h" //for xAOD::VxType
#include "LumiBlockData/BunchCrossingCondData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "BeamSpotID.h"
#include <string>
#include <vector>
#include <map>

class IToolSvc;
class IInDetBeamSpotTool;
class TTree;

namespace InDet {
  class InDetBeamSpotFinder : public AthAlgorithm {

  public:
    //Standard constructor and methods
    using AthAlgorithm::AthAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;
    virtual StatusCode finalize() override;

  private:
    //Reorganize and clean up this section
    ServiceHandle<IToolSvc> m_toolSvc{this, "ToolSvc", "ToolSvc"};
    ServiceHandle<ITHistSvc> m_thistSvc{this, "THistSvc", "THistSvc"};
    PublicToolHandleArray<IInDetBeamSpotTool> m_beamSpotToolList{this, "BeamSpotToolList", {}};

    SG::ReadCondHandleKey<BunchCrossingCondData> m_bcDataKey
      {this, "BunchCrossingCondDataKey", "BunchCrossingData" ,"SG Key of BunchCrossing CDO"};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo
      {this, "EvtInfo", "EventInfo", "EventInfo name"};
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainer
      {this, "VertexContainer", "PrimaryVertices", "Vertex container name"};

    //Beamspot sorting options
    Gaudi::Property<unsigned int> m_maxRunsPerFit{this, "RunRange", 0};
    Gaudi::Property<unsigned int> m_maxEventsPerFit{this, "EventRange", 0};
    Gaudi::Property<unsigned int> m_maxLBsPerFit{this, "LumiRange", 0};
    //Event Selection
    Gaudi::Property<std::vector<unsigned int>> m_BCIDsToAccept{this, "UseBCID", {}};
    //Job Options
    Gaudi::Property<bool> m_writeVertexNtuple{this, "VertexNtuple", true};
    std::string m_beamSpotNtupleName;
    //Initialize some pointers
    TTree * m_root_bs=nullptr;
    TTree * m_root_vrt=nullptr;

    struct beamSpotNtuple_struct{
      int pileup{}, bcid{}, defectWord{}, fill{}, lbEnd{}, lbStart{};
      int nEvents{}, nValid{}, nVtxAll{}, nVtxPrim{};
      int run{}, separation{}, status{}, timeEnd{}, timeStart{}, runEnd{};
      std::map<std::string,double>paramMap{};
      std::map<std::string,double>covMap{};
    };

    struct vertexNtuple_struct{
      double x{},y{},z{},vxx{},vxy{},vyy{},vzz{};
      xAOD::VxType::VertexType vType{};
      unsigned int run{}, lb{}, bcid{}, pileup{}, nTracks{};
      unsigned long long eventNumber{}, eventTime{}, eventTime_NS{};
      bool passed{}, valid{};
    };

    beamSpotNtuple_struct m_beamSpotNtuple;
    vertexNtuple_struct m_root_vtx;
    BeamSpotStatusCode m_BeamStatusCode;
    
    Gaudi::Property<std::string> m_vertexTreeName{this, "VertexTreeName", "Vertices"};
    std::vector<BeamSpot::Event> m_eventList;
    std::map< BeamSpot::ID, std::vector<BeamSpot::Event> > m_eventMap;
    std::vector<std::vector<BeamSpot::Event>> m_sortedEventList;
    std::vector<std::vector<BeamSpot::VrtHolder>> m_sortedVertices;

    bool passEventSelection(const xAOD::EventInfo &);
    BeamSpot::Event readEvent(const xAOD::EventInfo & , const xAOD::VertexContainer &);
    void writeToVertexTree(BeamSpot::Event &, BeamSpot::VrtHolder &);
    void writeToBeamSpotTree(const IInDetBeamSpotTool *bs, std::vector<BeamSpot::Event> &, std::vector<BeamSpot::VrtHolder> &);
    void sortEvents();
    StatusCode setupVertexTree();
    StatusCode setupBeamSpotTree();
    StatusCode performFits();
    bool iequals(const std::string &, const std::string &);
    int min_lb( std::vector<BeamSpot::Event> & );
    int max_lb( std::vector<BeamSpot::Event> & );
    int min_run( std::vector<BeamSpot::Event> & );
    int max_run( std::vector<BeamSpot::Event> & );

    void convertVtxTypeNames();

    // Parameters for vertex selection criteria
    Gaudi::Property<int> m_minTrackNum{this, "MinTracksPerVtx", 5, "min Tracks in a vertex to be added"};
    Gaudi::Property<int> m_maxTrackNum{this, "MaxTracksPerVtx", 1000000, "max Tracks in a vertex to be added"};
    Gaudi::Property<double> m_maxChi2Vertex{this, "MaxVtxChi2", 10, "maxChi2 of vertex"};
    Gaudi::Property<double> m_maxTransverseError{this, "MaxTransverseErr", 1000000, "max transverse vertex resolution"};
    Gaudi::Property<double> m_maxAbsCorrelXY{this, "MaxAbsCorrelXY", 0.8, "max absolute XY correlation of a vertex to be added"};
    Gaudi::Property<double> m_minVtxProb{this, "MinVtxProb", 0.001, "probability cut on chi2/ndf"};
    Gaudi::Property<unsigned int> m_minVertexNum{this, "MinVtxNum", 100, "min vertex count for solution"};

    Gaudi::Property<std::vector<std::string>> m_vertexTypeNames{this, "VertexTypes", {}, "names of vertexTypes"};
    std::vector<xAOD::VxType::VertexType> m_vertexTypes{};

    IInDetBeamSpotTool * cloneTool( int );
    bool passVertexSelection(const xAOD::Vertex * );    
    //For forcing a specific run number

    //    bool m_groupFitsByBCID; // each bcid is fitted separately
    Gaudi::Property<bool> m_useFilledBCIDsOnly{this, "UseFilledBCIDsOnly", true, "Only use filled BCIDs not empty ones (data)"};
    Gaudi::Property<std::string> m_fitSortingKey{this, "GroupFitsBy", "none"};

    Gaudi::Property<bool> m_writeAllVertices{this, "WriteAllVertices", false};
    //std::map<unsigned int,long> m_nEvents;
    //std::map<BeamSpot::ID, long> m_nEvents;
   
    Gaudi::Property<unsigned long> m_secondsPerFit{this, "SecondsPerFit", 1};
   };
}//end namespace 

#endif

