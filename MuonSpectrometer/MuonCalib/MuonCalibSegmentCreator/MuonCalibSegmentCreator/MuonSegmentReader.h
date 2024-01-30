/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_MUONSEGMENTREADER_H
#define MUONCALIB_MUONSEGMENTREADER_H
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
//#include "xAODTracking/TrackParticleContainer.h"
#include <xAODEventInfo/EventInfo.h>
#include "MuonSegment/MuonSegment.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"

#include "TrkSegment/SegmentCollection.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"

//#include "LumiBlockData/LuminosityCondData.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
// #include "MdtCalibSvc/MdtCalibrationTool.h"
#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
// #include "MuonCalibEvent/MuonCalibPatternCollection.h"
#include "MuonCalibITools/IIdToFixedIdTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
//#include "MuonTester/MuonTesterBranch.h"

#include "TTree.h"

/** An example algorithm that reads and writes objects from the event store
using handles.*/
namespace Trk {
    //class IExtrapolator;
    class IResidualPullCalculator;
}  // namespace Trk

using namespace MuonVal ;
namespace MuonCalib {

// class MuonSegmentReader : public AthReentrantAlgorithm
//    {
//    public:
//    using AthReentrantAlgorithm::AthReentrantAlgorithm;
class MuonSegmentReader : public AthHistogramAlgorithm
   {
   public:
   using AthHistogramAlgorithm::AthHistogramAlgorithm;

   //virtual StatusCode initialize() override;
   virtual StatusCode initialize() override;
   // virtual StatusCode execute (const EventContext& ctx) const override;
   virtual StatusCode execute () override;
   virtual StatusCode finalize () override;
   
   unsigned int cardinality() const override final { return 1; } ;

   private:

        SG::ReadHandleKey<xAOD::EventInfo> m_evtKey{this, "EventInfoKey", "EventInfo", "xAOD::EventInfo ReadHandleKey"};

        SG::ReadHandleKey<TrackCollection> m_TrkKey {this, "MuonTrackLocations", "MuonSpectrometerTracks"};

        SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_MdtPrepData {this, "MdtPrepData", "MDT_DriftCircles"};

        // /** MuonDetectorManager from the conditions store */
        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                                 "Key of input MuonDetectorManager condition data"};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_MuonIdHelper{this, "MuonIdHelper", "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
                                                         "Handle to the MuonIdHelperSvc"};
       
         /** pointer to MdtCalibSvc */
        ToolHandle<IMdtCalibrationTool> m_calibrationTool{this, "CalibrationTool", "MdtCalibrationTool"};
        
        PublicToolHandle<Trk::IResidualPullCalculator> m_pullCalculator{this, "PullCalculator",
                                                                    "Trk::ResidualPullCalculator/ResidualPullCalculator"};

        ToolHandle<MuonCalib::IIdToFixedIdTool> m_idToFixedIdTool{this, "IdToFixedIdTool", "MuonCalib::IdToFixedIdTool/MuonCalib_IdToFixedIdTool"};

        MuonTesterTree m_tree{"Segments", "CALIBNTUPLESTREAM"};
        // book event_x branches
        ScalarBranch<int>& m_runNumber{m_tree.newScalar<int>("event_runNumber")};
        ScalarBranch<int>& m_eventNumber{m_tree.newScalar<int>("event_eventNumber")};
        ScalarBranch<int>& m_lumiBlock{m_tree.newScalar<int>("event_lumiBlock")};
        ScalarBranch<int>& m_bcId{m_tree.newScalar<int>("event_bcId")};
        ScalarBranch<int>& m_timeStamp{m_tree.newScalar<int>("event_timeStamp")};
        ScalarBranch<float>& m_pt{m_tree.newScalar<float>("event_pt")};
        //ScalarBranch<unsigned int>& m_eventTag{m_tree.newScalar<unsigned int>("event_eventTag")};
        //ScalarBranch<int>& m_nEvent{m_tree.newScalar<int>("event_nEvent")};

        // rawMDT hit branches
        ScalarBranch<int>& m_rawMdt_nRMdt{m_tree.newScalar<int>("rawMdt_nRMdt")};	//total number of MDT hits in the event
        VectorBranch<unsigned int>& m_rawMdt_id{m_tree.newVector<unsigned int>("rawMdt_id")};	//identifier of the raw MDT hit (given by MuonFixedId)
        VectorBranch<int>& m_rawMdt_tdc{m_tree.newVector<int>("rawMdt_tdc")}; 	//tdc counts of the raw MDT hit
        VectorBranch<int>& m_rawMdt_adc{m_tree.newVector<int>("rawMdt_adc")};  //adc counts of the raw MDT hit
        ThreeVectorBranch m_rawMdt_gPos{m_tree,"rawMdt_gPos"};

        // Muon Track branches
        ScalarBranch<int>& m_trk_nTracks{m_tree.newScalar<int>("trk_nTracks")};
        VectorBranch<float>& m_trk_d0{m_tree.newVector<float>("trk_d0")};        
        VectorBranch<float>& m_trk_z0{m_tree.newVector<float>("trk_z0")};        
        VectorBranch<float>& m_trk_phi{m_tree.newVector<float>("trk_phi")};        
        VectorBranch<float>& m_trk_theta{m_tree.newVector<float>("trk_theta")};        
        VectorBranch<float>& m_trk_eta{m_tree.newVector<float>("trk_eta")};        
        VectorBranch<float>& m_trk_qOverP{m_tree.newVector<float>("trk_qOverP")};        
        VectorBranch<float>& m_trk_pt{m_tree.newVector<float>("trk_pt")};        
        VectorBranch<float>& m_trk_chi2{m_tree.newVector<float>("trk_chi2")};        
        VectorBranch<int>& m_trk_ndof{m_tree.newVector<int>("trk_ndof")};   
        VectorBranch<int>& m_trk_author{m_tree.newVector<int>("trk_author")};  
        ThreeVectorBranch m_trk_perigee{m_tree,"trk_perigee"} ;  

        VectorBranch<int>& m_trk_nMdtHits{m_tree.newVector<int>("trk_nMdtHits")};  
        VectorBranch<int>& m_trk_nMdtGoodHits{m_tree.newVector<int>("trk_nMdtGoodHits")};  
        VectorBranch<int>& m_trk_nMdtHoles{m_tree.newVector<int>("trk_nMdtHoles")};  
        VectorBranch<int>& m_trk_nOutliersHits{m_tree.newVector<int>("trk_nOutliersHis")};  
        VectorBranch<int>& m_trk_nRpcPhiHits{m_tree.newVector<int>("trk_nRpcPhiHits")};  
        VectorBranch<int>& m_trk_nRpcEtaHits{m_tree.newVector<int>("trk_nRpcEtaHits")};  
        VectorBranch<int>& m_trk_nTgcPhiHits{m_tree.newVector<int>("trk_nTgcPhiHits")};  
        VectorBranch<int>& m_trk_nTgcEtaHits{m_tree.newVector<int>("trk_nTgcEtaHits")}; 

        //int m_trk_nTracks = 0;

        // Muon Track Hit branches
        ScalarBranch<int>& m_trkHit_nHits{m_tree.newScalar<int>("trkHit_nHits")};
        VectorBranch<int>& m_trkHit_trackIndex{m_tree.newVector<int>("trkHit_trackIndex")};        
        ThreeVectorBranch m_trkHit_gPos{m_tree,"trkHit_gPos"} ;
        ThreeVectorBranch m_trkHit_pos{m_tree,"trkHit_pos"} ;
        ThreeVectorBranch m_trkHit_closestApproach{m_tree,"trkHit_closestApproach"} ;
        ThreeVectorBranch m_trkHit_gClosestApproach{m_tree,"trkHit_gClosestApproach"} ;
        ThreeVectorBranch m_trkHit_center{m_tree,"trkHit_center"};  

        VectorBranch<int>& m_trkHit_adc{m_tree.newVector<int>("trkHit_adc")};        
        VectorBranch<int>& m_trkHit_type{m_tree.newVector<int>("trkHit_type")};        
        VectorBranch<int>& m_trkHit_tdc{m_tree.newVector<int>("trkHit_tdc")};        
        VectorBranch<float>& m_trkHit_resi{m_tree.newVector<float>("trkHit_resi")};        
        VectorBranch<float>& m_trkHit_pull{m_tree.newVector<float>("trkHit_pull")};   
        VectorBranch<unsigned int>& m_trkHit_FixedId{m_tree.newVector<unsigned int>("trkHit_FixedId")};       
        VectorBranch<float>& m_trkHit_driftRadius{m_tree.newVector<float>("trkHit_driftRadius")};        
        VectorBranch<float>& m_trkHit_rTrk{m_tree.newVector<float>("trkHit_rTrk")};        
        VectorBranch<float>& m_trkHit_driftTime{m_tree.newVector<float>("trkHit_driftTime")};        
        VectorBranch<float>& m_trkHit_distRO{m_tree.newVector<float>("trkHit_distRO")};  
        VectorBranch<float>& m_trkHit_localAngle{m_tree.newVector<float>("trkHit_localAngle")};  
        
        // branches with driftTime corrections
        VectorBranch<float>& m_trkHit_tubeT0{m_tree.newVector<float>("trkHit_tubeT0")};        
        VectorBranch<float>& m_trkHit_triggerTime{m_tree.newVector<float>("trkHit_triggerTime")};        
        VectorBranch<float>& m_trkHit_tubeMeanAdc{m_tree.newVector<float>("trkHit_tubeMeanAdc")};        
        VectorBranch<float>& m_trkHit_slewTime{m_tree.newVector<float>("trkHit_slewTime")};        
        VectorBranch<float>& m_trkHit_lorTime{m_tree.newVector<float>("trkHit_lorTime")};        
        VectorBranch<float>& m_trkHit_sagTime{m_tree.newVector<float>("trkHit_sagTime")};        
        VectorBranch<float>& m_trkHit_propTime{m_tree.newVector<float>("trkHit_propTime")};        
        VectorBranch<float>& m_trkHit_tempTime{m_tree.newVector<float>("trkHit_tempTime")};        
        VectorBranch<float>& m_trkHit_bkgTime{m_tree.newVector<float>("trkHit_bkgTime")};        
        VectorBranch<float>& m_trkHit_tof{m_tree.newVector<float>("trkHit_tof")}; 
        VectorBranch<int>& m_trkHit_calibStatus{m_tree.newVector<int>("trkHit_calibStatus")}; 
               

   };
   
   }  // namespace MuonCalib
#endif
