/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTMONITORINGRUN3RAW_ALG_H
#define TRTMONITORINGRUN3RAW_ALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "GaudiKernel/StatusCode.h"

// Data handles
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrkTrack/TrackCollection.h"
#include "xAODEventInfo/EventInfo.h"
#include "InDetRawData/InDetTimeCollection.h"
#include "InDetRawData/InDetRawDataCLASS_DEF.h"

// Tool interfaces
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "TrkToolInterfaces/ITrackHoleSearchTool.h"
#include "TRT_ConditionsServices/ITRT_CalDbTool.h"
#include "TRT_ConditionsServices/ITRT_StrawNeighbourSvc.h"

#include "InDetByteStreamErrors/TRT_BSErrContainer.h"
#include "TRT_ConditionsServices/ITRT_ByteStream_ConditionsSvc.h"

#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"

#include "CLHEP/Units/SystemOfUnits.h"

// STDLIB
#include <string>
#include <vector>
#include <set>

namespace InDetDD {
    class TRT_DetectorManager;
}
 
class AtlasDetectorID;
class TRT_ID;
class Identifier;
class ITRT_StrawStatusSummaryTool;
class ITRT_ByteStream_ConditionsSvc;

class TRTMonitoringRun3RAW_Alg : public AthMonitorAlgorithm {
public:
    TRTMonitoringRun3RAW_Alg( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~TRTMonitoringRun3RAW_Alg();
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

private:
    const AtlasDetectorID * m_idHelper{};
    const TRT_ID* m_pTRTHelper{};
    const InDetDD::TRT_DetectorManager *m_mgr{};

    std::vector<std::vector<unsigned char>> m_mat_chip_E{64, std::vector<unsigned char>(3840)};
    std::vector<std::vector<unsigned char>> m_mat_chip_B{64, std::vector<unsigned char>(1642)};

    static const int s_Straw_max[2];
    static const int s_iChip_max[2];

    static const int s_numberOfBarrelStacks;
    static const int s_numberOfEndCapStacks;

    BooleanProperty m_doStraws{this, "doStraws", true, ""};
    BooleanProperty m_doExpert{this, "doExpert", false, ""};
    BooleanProperty m_doChips{this, "doChips", true, ""};
    BooleanProperty m_doTracksMon{this, "doTracksMon", true, ""};
    BooleanProperty m_doRDOsMon{this, "doRDOsMon", true, ""};
    BooleanProperty m_doShift{this, "doShift", true, ""};
    BooleanProperty m_doMaskStraws{this, "doMaskStraws", true, ""};
    BooleanProperty m_useHoleFinder{this, "useHoleFinder", false, ""};
    BooleanProperty m_doHitsMon{this, "DoHitsMon", true, ""};
    FloatProperty m_DistToStraw{this, "DistanceToStraw", 0.4, ""};
    FloatProperty m_usedEvents{this, "totalEvents", -1, ""};

    BooleanProperty m_ArgonXenonSplitter{this, "doArgonXenonSeparation", true};

    FloatProperty m_longToTCut{this, "LongToTCut", 9.375};

    Gaudi::Property<std::vector<int>> m_strawMax {this,"strawMax", {-1, -1}};
    Gaudi::Property<std::vector<int>> m_iChipMax {this,"iChipMax", {-1, -1}};

    IntegerProperty m_min_si_hits{this, "min_si_hits", 1, ""};
    IntegerProperty m_min_pixel_hits{this, "min_pixel_hits", 0, ""};
    IntegerProperty m_min_sct_hits{this, "min_sct_hits", 0, ""};
    IntegerProperty m_min_trt_hits{this, "min_trt_hits", 10, ""};
    IntegerProperty m_minTRThits{this, "MinTRTHitCut", 10, ""};
    IntegerProperty m_every_xth_track{this, "every_xth_track", 1, ""};
    FloatProperty m_max_abs_d0{this, "max_abs_d0", 10  * CLHEP::mm, ""};
    FloatProperty m_max_abs_z0{this, "max_abs_z0", 300 * CLHEP::mm, ""};
    FloatProperty m_max_abs_eta{this, "max_abs_eta", 2.5, ""};
    FloatProperty m_minP{this, "MinTrackP", 0.0 * CLHEP::GeV, ""};
    FloatProperty m_min_pT{this, "min_pT", 0.5 * CLHEP::GeV, ""};

    StatusCode fillTRTRDOs(const EventContext& ctx,
                           const TRT_RDO_Container& rdoContainer,
	                       const xAOD::EventInfo& eventInfo,
	                       const InDetTimeCollection* trtBCIDCollection) const;
    StatusCode fillTRTEfficiency(const EventContext& ctx,
                                 const TrackCollection& combTrackCollection) const;
    StatusCode fillTRTHits(const EventContext& ctx,
                           const TrackCollection& trackCollection) const;       
    
    int chipToBoard(int chip) const;
    int chipToBoard_EndCap(int chip) const;
    StatusCode checkTRTReadoutIntegrity(const xAOD::EventInfo& eventInfo) const;
    std::vector<std::vector<std::vector<int>>>  initScaleVectors(const EventContext& ctx) const;
    bool checkEventBurst(const TRT_RDO_Container& rdoContainer) const;
    int strawNumberEndCap(int strawNumber, int strawLayerNumber, int LayerNumber, int phi_stack, int side) const;
    int strawNumber(int strawNumber, int strawlayerNumber, int LayerNumber) const;
    int strawLayerNumber(int strawLayerNumber, int LayerNumber) const;
    float radToDegrees(float radValue) const;
    int strawNumber_reverse(int inp_strawnumber,  int* strawNumber, int* strawlayerNumber, int* LayerNumber) const;
	int strawLayerNumber_reverse(int strawLayerNumInp,int* strawLayerNumber, int* LayerNumber) const;

    // Services
    ToolHandle<ITRT_StrawStatusSummaryTool> m_sumTool{this, "InDetTRTStrawStatusSummaryTool", "TRT_StrawStatusSummaryTool", ""};
    ServiceHandle<ITRT_StrawNeighbourSvc> m_TRTStrawNeighbourSvc{this, "StrawNeighbourSvc", "TRT_StrawNeighbourSvc", ""};
    ServiceHandle<ITRT_ByteStream_ConditionsSvc> m_BSSvc{this, "TRT_ByteStream_ConditionsSvc", "TRT_ByteStream_ConditionsSvc", ""};
    ToolHandle<InDet::IInDetTrackSelectionTool> m_trackSelTool{this, "TrackSelectionTool", "InDet::InDetTrackSelectionTool/TrackSelectionTool", ""};

    // Data handles
    SG::ReadHandleKey<TRT_RDO_Container>   m_rdoContainerKey{this, "TRTRawDataObjectName", "TRT_RDOs", "Name of TRT RDOs container"};
    SG::ReadHandleKey<InDetTimeCollection> m_TRT_BCIDCollectionKey{this, "TRTBCIDCollectionName", "TRT_BCID", "Name of TRT BCID collection"};
    SG::ReadHandleKey<TrackCollection>     m_combTrackCollectionKey{this, "track_collection_hole_finder", "CombinedInDetTracks", "Name of tracks container used for hole finder"};
    SG::ReadHandleKey<TrackCollection> m_trackCollectionKey{this, "TRTTracksObjectName", "CombinedInDetTracks", "Name of tracks container"};

    SG::ReadHandleKey<TRT_BSErrContainer> m_bsErrContKey{this,"ByteStreamErrors","TRT_ByteStreamErrs","SG key of TRT ByteStream Error container"};

    // Tools
    ToolHandle<Trk::ITrackHoleSearchTool>  m_trt_hole_finder{this, "trt_hole_search", "TRTTrackHoleSearchTool", "Track hole search tool name"};
    ToolHandle<Trk::ITrackSummaryTool>     m_TrackSummaryTool{this, "TrackSummaryTool", "InDetTrackSummaryTool", "Track summary tool name"};
    
    enum GasType{ Xe = 0, Ar = 1, Kr = 2 };
    //Deciphers status HT to  GasType Enumerator
	inline GasType Straw_Gastype(int stat) const {
		// getStatusHT returns enum {Undefined, Dead, Good, Xenon, Argon, Krypton}.
		// Our representation of 'GasType' is 0:Xenon, 1:Argon, 2:Krypton
		GasType Gas = Xe; // Xenon is default
		if (m_ArgonXenonSplitter) {
			//      int stat=m_sumSvc->getStatusHT(TRT_Identifier);
			if       ( stat==2 || stat==3 ) { Gas = Xe; } // Xe
			else if  ( stat==1 || stat==4 ) { Gas = Ar; } // Ar
			else if  ( stat==5 )            { Gas = Kr; } // Kr
			else if  ( stat==6 )            { Gas = Xe; } // emulate Ar (so treat as Xe here)
			else if  ( stat==7 )            { Gas = Xe; } // emulate Kr (so treat as Xe here)
			else { ATH_MSG_FATAL ("getStatusHT = " << stat << ", must be 'Good(2)||Xenon(3)' or 'Dead(1)||Argon(4)' or 'Krypton(5)!' or 6 or 7 for emulated types!");
				throw std::exception();
			}
		}
		return Gas;
	}
    
    BooleanProperty m_isCosmics{this, "IsCosmics", false};
    IntegerProperty m_EventBurstCut{this, "EventBurstCut", -1};
   
};
#endif
