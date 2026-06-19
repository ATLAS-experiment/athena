/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFP_CALIBRATION_AFP_PIXELHISTOFILLER_H
#define AFP_CALIBRATION_AFP_PIXELHISTOFILLER_H

// Framework includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODForward/AFPSiHitContainer.h"

#include "xAODForward/AFPTrackContainer.h"

// STL includes
#include <string>
#include <vector>
//ROOT includes
#include "TH2F.h"
#include "TProfile.h"
#include "TProfile2D.h"

/**
 * @class AFP_PixelHistoFiller
 **/

class AFP_PixelHistoFiller : public AthAlgorithm {
public:
  AFP_PixelHistoFiller(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~AFP_PixelHistoFiller() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

private:
    static const int m_nStations=4;
    static const int m_nLayers=4;
    int m_nPixelsX=0;
    int m_nPixelsY=0;
    
    std::vector<TH2F> m_pixelHits[m_nStations][m_nLayers];                  // 2D map of hits
    std::vector<TH2F> m_pixelCluster[m_nStations][m_nLayers];               // 2D map of clusters

    TProfile m_lb_xDistSiTrackCluster[m_nStations][m_nLayers];              // lumi block vs x-dist(track, cluster)
    TProfile m_lb_yDistSiTrackCluster[m_nStations][m_nLayers];              // lumi block vs y-dist(track, cluster)
    TProfile2D m_lb_xCluster_yDistSiTrackCluster[m_nStations][m_nLayers];   // lumi block vs x-cluster vs y-dist(track, cluster)
    TProfile2D m_lb_yCluster_xDistSiTrackCluster[m_nStations][m_nLayers];   // lumi block vs y-cluster vs x-dist(track, cluster)
    TProfile2D m_lb_zCluster_xDistSiTrackCluster[m_nStations][m_nLayers];   // lumi block vs z-cluster vs x-dist(track, cluster)
    TProfile2D m_lb_zCluster_yDistSiTrackCluster[m_nStations][m_nLayers];   // lumi block vs z-cluster vs y-dist(track, cluster)
    
    TProfile2D m_lb_yCluster_yDistSiTrackCluster[m_nStations][m_nLayers];   // lumi block vs cluster y-pos vs y-dist(track, cluster)
    TProfile2D m_lb_xCluster_xDistSiTrackCluster[m_nStations][m_nLayers];   // lumi block vs cluster x-pos vs x-dist(track, cluster)
    TProfile2D m_lb_sxTrack_xDistSiTrackCluster[m_nStations][m_nLayers];    // lumi block vs track x-slope vs x-dist(track, cluster)
    TProfile2D m_lb_syTrack_yDistSiTrackCluster[m_nStations][m_nLayers];    // lumi block vs track y-slope vs y-dist(track, cluster)
    TProfile2D m_lb_syTrack_xDistSiTrackCluster[m_nStations][m_nLayers];    // lumi block vs track y-slope vs x-dist(track, cluster)
    TProfile2D m_lb_sxTrack_yDistSiTrackCluster[m_nStations][m_nLayers];    // lumi block vs track x-slope vs y-dist(track, cluster)
    
    Gaudi::Property<int> m_LBRangeLength{this, "LBRangeLength",5, "How many lumiblocks should be merged together to have reasonable statistics"};
    
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey { this, "EventInfoKey", "EventInfo", "name of EventInfo container" };
    SG::ReadHandleKey<xAOD::AFPSiHitContainer> m_afpHitContainerKey{ this ,"AFPHitContainerKey", "AFPSiHitContainer", "name of AFPSiHitContainer" };
    SG::ReadHandleKey<xAOD::AFPTrackContainer> m_afpTrackContainerKey{ this, "AFPTrackContainerKey", "AFPTrackContainer", "name of AFPTrackContainer" };
};

#ifndef __CINT__
  CLASS_DEF( AFP_PixelHistoFiller , 161453345 , 1 )
#endif

#endif // AFP_CALIBRATION_AFP_PIXELHISTOFILLER_H
