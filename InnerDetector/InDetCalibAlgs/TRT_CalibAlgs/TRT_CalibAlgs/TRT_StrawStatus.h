/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TRT_StrawStatus.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef INDETTRT_STRAWSTATUS_H
#define INDETTRT_STRAWSTATUS_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkToolInterfaces/IUpdator.h"

#include "xAODEventInfo/EventInfo.h"
#include "InDetRawData/TRT_RDO_Container.h"
#include "AthContainers/DataVector.h"
#include "TrkTrack/Track.h"
#include "xAODTracking/VertexContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrkToolInterfaces/ITrackHoleSearchTool.h"
#include "CxxUtils/checker_macros.h"
#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h"

#include "TRT_ConditionsServices/ITRT_StrawNeighbourSvc.h"
#include "TRT_ConditionsServices/ITRT_HWMappingSvc.h"
#include "TRT_ConditionsServices/ITRT_DCS_ConditionsSvc.h"

#include <cstdlib>
#include <string>
#include <vector>
#include <array>
#include <atomic>

class AtlasDetectorID;
class Identifier;

class TRT_ID;


namespace InDet 
{

  /** @class TRT_StrawStatus

      This algorithm finds dead or hot TRT straws / chips / boards based on  
      occupancy and hits on track.
      To be used in calibration stream (to mask off dead regions before processing)
            
      @author  Sasa Fratina <sasa.fratina@cern.ch>
  */  

  class ATLAS_NOT_THREAD_SAFE TRT_StrawStatus : public AthAlgorithm // A global variable (last_lumiBlock0) is read and written. Results should not be reproducible in multi-threading.
    {
    public:

       /** Standard Athena-Algorithm Constructor */
       TRT_StrawStatus(const std::string& name, ISvcLocator* pSvcLocator);
       /** Default Destructor */
       ~TRT_StrawStatus();

       /** standard Athena-Algorithm method */
       StatusCode          initialize();
       /** standard Athena-Algorithm method */
       StatusCode          execute();
       /** standard Athena-Algorithm method */
       StatusCode          finalize();

    private:

        // Decleare properties, servicies and tools
        ServiceHandle<ITRT_HWMappingSvc> m_mapSvc {this,"HWMapSvc","TRT_HWMappingSvc","" };
        ServiceHandle<ITRT_DCS_ConditionsSvc> m_DCSSvc {this,"InDetTRT_DCS_ConditionsSvc","TRT_DCS_ConditionsSvc","" };
        ServiceHandle<ITRT_StrawNeighbourSvc> m_TRTStrawNeighbourSvc {this,"TRT_StrawNeighbourSvc","TRT_StrawNeighbourSvc","retrieve barrel and end-cap straw number later on, as well as DTMROC" };
        ToolHandle<ITRT_StrawStatusSummaryTool> m_TRTStrawStatusSummaryTool {this, "TRT_StrawStatusSummaryTool", "ITRT_StrawStatusSummaryTool", ""};
        ToolHandle<Trk::ITrackHoleSearchTool>  m_trt_hole_finder {this, "trt_hole_finder", "Trk::ITrackHoleSearchTool", ""};
        PublicToolHandle<Trk::IUpdator> m_updator {this, "KalmanUpdator", "Trk::KalmanUpdator/TrkKalmanUpdator",""};

        Gaudi::Property<double> m_locR_cut {this, "locR_cut", 1.4, ""};
        Gaudi::Property<int> m_skipBusyEvents {this, "skipBusyEvents", 0, ""};
        Gaudi::Property<int> m_printDetailedInformation {this, "printDetailedInformation", 0, ""};
        Gaudi::Property<std::string> m_fileName {this, "outputFileName", "TRT_StrawStatusOutput", ""};
        
        // Declare Handles
        SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,"EventInfoKey","EventInfo","RHK to retrieve xAOD::EventInfo"};
        SG::ReadHandleKey<TRT_RDO_Container> m_rdoContainerKey{this,"RDO_ContainerKey","TRT_RDOs","RHK to retrieve TRT RDO's"};
        SG::ReadHandleKey<DataVector<Trk::Track>> m_tracksName{this,"tracksCollectionKey","CombinedInDetTracks","RHK to retrieve CombinedInDetTracks"};
        SG::ReadHandleKey<xAOD::VertexContainer> m_vxContainerKey{this,"VxContainerKey","PrimaryVertices","RHK to retrieve VX Primary candidates"};

      void clear();	
      StatusCode reportResults();
      void printDetailedInformation();
	
      /** function that returns straw index (in range 0-5481; 0-1641 for barrel, the rest for endcap) 
	      same convention as for TRT_monitoring (copied from there) */
      void myStrawIndex(Identifier id, int *index);
	    int barrelStrawNumber(int strawNumber, int strawlayerNumber, int LayerNumber);
	    int endcapStrawNumber( int strawNumber, int strawLayerNumber, int LayerNumber );
	  	  
      /** returns index of hardware units: board, chip, pad
		  private fix for now, will call TRTStrawNeighbourSvc when available 
		  number boards 0-9 barrel, 0-19 endcap (first 12 on A wheels, ordering from smaller to larger |z|)
		  number chips 0-103 barrel, 0-239 endcap
		  number pads: chips x 2 */
		  	  
      int m_nEvents; // count N of processed events, needed for normalization
      int m_runNumber;

      /** accumulate hits, last index: 0 - all hits, 1 - hits on track, 2 - all HT (TR) hits, 3 - HT (TR) hits on track */	 
      typedef std::array<std::array<std::array<std::array<int,6>,5482>,32>,2> ACCHITS_t;
      std::unique_ptr<ACCHITS_t> m_accumulateHits;

      const TRT_ID *m_TRTHelper;
      mutable std::atomic<int> m_printStatusCount{0};
    }; 
} // end of namespace

#endif 
