/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTS_PHYSVAL_TOOL_H
#define ACTS_PHYSVAL_TOOL_H

#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandleKey.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "HGTD_Identifier/HGTD_ID.h"
#include "src/PixelClusterValidationPlots.h"
#include "src/StripClusterValidationPlots.h"
#include "src/HgtdClusterValidationPlots.h"
#include "src/PixelSpacePointValidationPlots.h"
#include "src/StripSpacePointValidationPlots.h"

namespace ActsTrk {

  class PhysValTool : 
    public ManagedMonitorToolBase 
  {
  public:
    PhysValTool(const std::string & type, 
		const std::string& name, 
		const IInterface* parent);
    virtual ~PhysValTool() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode bookHistograms() override;
    virtual StatusCode fillHistograms() override;
    virtual StatusCode procHistograms() override;

  private:
    template<typename external_collection_t>
      StatusCode bookCollection(external_collection_t*);

    StatusCode fillPixelClusters(const EventContext& ctx,
				 float beamSpotWeight);
    StatusCode fillStripClusters(const EventContext& ctx,
                                 float beamSpotWeight);
    StatusCode fillHgtdClusters(const EventContext& ctx,
				float beamSpotWeight);

    StatusCode fillPixelSpacePoints(const EventContext& ctx,
				    float beamSpotWeight);
    StatusCode fillStripSpacePoints(const EventContext& ctx,
				    float beamSpotWeight);
    StatusCode fillStripOverlapSpacePoints(const EventContext& ctx,
					   float beamSpotWeight);
    
  private:
    SG::ReadHandleKey< xAOD::EventInfo > m_eventInfo {this, "EventInfo", "EventInfo", 
	"Event info key"};
    
    SG::ReadHandleKey< xAOD::PixelClusterContainer > m_pixelClusterContainerKey {this, "PixelClusterContainerKey", "ITkPixelClusters",
	"Key of input pixel clusters"};
    SG::ReadHandleKey< xAOD::StripClusterContainer > m_stripClusterContainerKey {this, "StripClusterContainerKey", "ITkStripClusters", 
	"Key of input pixel clusters"};
    SG::ReadHandleKey< xAOD::HGTDClusterContainer > m_hgtdClusterContainerKey {this, "HGTDClusterContainerKey", "HGTD_Clusters",
        "Key of input hgtd clusters"};
    
    SG::ReadHandleKey< xAOD::SpacePointContainer > m_pixelSpacePointContainerKey {this, "PixelSpacePointContainerKey", "ITkPixelSpacePoints",
	"Key of input pixel space points"};
    SG::ReadHandleKey< xAOD::SpacePointContainer > m_stripSpacePointContainerKey {this, "StripSpacePointContainerKey", "ITkStripSpacePoints",
	"Key of input strip space points"};
    SG::ReadHandleKey< xAOD::SpacePointContainer > m_stripOverlapSpacePointContainerKey {this, "StripOverlapSpacePointContainerKey", "ITkStripOverlapSpacePoints",
	"Key of input strip overlap space points"};

    SG::ReadCondHandleKey<InDetDD::HGTD_DetectorElementCollection> m_HGTDDetEleCollKey {this, "HGTDDetEleCollKey", "HGTD_DetectorElementCollection",
      "Key of HGTD_DetectorElementCollection for HGTD"};
    
    Gaudi::Property<bool> m_doPixelClusters {this, "doPixelClusters", false, "Analyse Pixel Clusters"};
    Gaudi::Property<bool> m_doStripClusters {this, "doStripClusters", false, "Analyse Strip Clusters"};
    Gaudi::Property<bool> m_doHgtdClusters {this, "doHgtdClusters", false, "Analyse HGTD Clusters"};    
    Gaudi::Property<bool> m_doPixelSpacePoints {this, "doPixelSpacePoints", false, "Analyse Pixel Space Points"};
    Gaudi::Property<bool> m_doStripSpacePoints {this, "doStripSpacePoints", false, "Analyse Strip Space Points"};
    Gaudi::Property<bool> m_doStripOverlapSpacePoints {this, "doStripOverlapSpacePoints", false, "Analyse Overlap Strip Space Points"};
    
    std::unique_ptr< ActsTrk::PixelClusterValidationPlots > m_pixelClusterValidationPlots;
    std::unique_ptr< ActsTrk::StripClusterValidationPlots > m_stripClusterValidationPlots;
    std::unique_ptr< ActsTrk::HgtdClusterValidationPlots > m_hgtdClusterValidationPlots;
    
    std::unique_ptr< ActsTrk::PixelSpacePointValidationPlots > m_pixelSpacePointValidationPlots;
    std::unique_ptr< ActsTrk::StripSpacePointValidationPlots > m_stripSpacePointValidationPlots;
    std::unique_ptr< ActsTrk::StripSpacePointValidationPlots > m_stripOverlapSpacePointValidationPlots;

    const PixelID *m_pixelID {};
    const SCT_ID *m_stripID {};
    const HGTD_ID *m_hgtdID {};
  };

  template<typename external_collection_t>
    StatusCode PhysValTool::bookCollection(external_collection_t* plot_collection)
    {
      std::vector<HistData> hists = plot_collection->retrieveBookedHistograms();
      for (auto& [histo, directory] : hists) {
	ATH_MSG_DEBUG ("Initializing " << histo << " " << histo->GetName() << " " << directory << "...");
	ATH_CHECK(regHist(histo, directory, all));
      }
      plot_collection->initialize();
      return StatusCode::SUCCESS;
    }
  
}

#endif
