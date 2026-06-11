// this is c++ file -*- c++ -*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// TRTStrawEfficiency.h
// author: Ryan D. Reece <ryan.reece@cern.ch>
// created: Nov 2009

#ifndef TRT_TrackHoleSearch_TRTStrawEfficiency_h
#define TRT_TrackHoleSearch_TRTStrawEfficiency_h

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "TrkToolInterfaces/IUpdator.h"
#include "TrigDecisionInterface/ITrigDecisionTool.h"
#include "TRT_ConditionsServices/ITRT_StrawNeighbourSvc.h"

#include "TrkTrack/TrackCollection.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"

#include "StoreGate/ReadHandleKey.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include <string>
#include <vector>

namespace Trk
{
  class ITrackHoleSearchTool;
  class TrackStateOnSurface;
}

class TRT_ID;

class TTree;

class TRTStrawEfficiency : public AthAlgorithm
{
  public:
        TRTStrawEfficiency(const std::string& name, ISvcLocator* pSvcLocator);

        StatusCode initialize();
        StatusCode finalize();
        StatusCode execute(const EventContext& ctx);

  private:
        // configurables
        //----------------------------------
        ToolHandle<Trk::ITrackHoleSearchTool> m_trt_hole_finder
          {this, "trt_hole_finder","TRTTrackHoleSearchTool"};
        FloatProperty m_max_abs_d0{this, "max_abs_d0", 600*CLHEP::mm};
        FloatProperty m_max_abs_z0{this, "max_abs_z0", 600*CLHEP::mm};
        FloatProperty m_min_pT{this, "min_pT", 1.0*CLHEP::GeV};
        FloatProperty m_min_p{this, "min_p", 2.0*CLHEP::GeV};
        FloatProperty m_max_abs_eta{this, "max_abs_eta", 2.5};
        IntegerProperty m_min_pixel_hits{this, "min_pixel_hits", 0};
        IntegerProperty m_min_sct_hits{this, "min_sct_hits", 2};
        IntegerProperty m_min_trt_hits{this, "min_trt_hits", 15};
        ServiceHandle<ITHistSvc> m_hist_svc{this, "hist_svc", "THistSvc"};
        ServiceHandle<ITRT_StrawNeighbourSvc> m_TRTStrawNeighbourSvc
          {this, "straw_neighbour_svc", "TRT_StrawNeighbourSvc"};
        StringProperty m_tree_name{this, "tree_name", "trt_eff"};
        StringProperty m_stream_name{this, "stream_name", "TRTEffStream"};
        StringProperty m_required_trigger{this, "required_trigger", ""};

        // private data
        //----------------------------------
        TTree* m_tree{nullptr};
        const TRT_ID* m_TRT_ID{nullptr};
        PublicToolHandle<Trk::IUpdator> m_updator
          {this,"KalmanUpdator","Trk::KalmanUpdator/TrkKalmanUpdator",""};
        PublicToolHandle<Trig::ITrigDecisionTool> m_trigDec
          {this,"ITrigDecisionTool","Trig::ITrigDecisionTool/TrigDecisionTool",""};

        // Data handles
        SG::ReadHandleKey<TrackCollection> m_tracksKey{this, "track_collection", "CombinedInDetTracks", "Tracks container key"};
        SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "event_info_key", "EventInfo", "Event info key"};
        SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerKey{this, "VertexContainerName", "PrimaryVertices", "Vertex container key"};

        unsigned int m_num_events{0};
        unsigned int m_num_tracks{0};
        unsigned int m_num_preselected_tracks{0};

        // ntuple branches
        unsigned int m_event_number{0};
        unsigned int m_run_number{0};
        unsigned int m_lumi_block{0};
        float m_track_pt{0.};
        float m_track_eta{0.};
        float m_track_phi{0.};
        float m_track_d0{0.};
        float m_track_z0{0.};
        int m_n_pixel_hits{0};
        int m_n_sct_hits{0};
        int m_n_trt_hits{0};
        std::vector<int> m_hit_bec;
        std::vector<int> m_hit_phi;
        std::vector<int> m_hit_layer;
        std::vector<int> m_hit_strawlayer;
        std::vector<int> m_hit_straw;
        std::vector<int> m_hit_chip;
        std::vector<int> m_hit_pad;
        std::vector<float> m_hit_x;
        std::vector<float> m_hit_y;
        std::vector<float> m_hit_z;
        std::vector<float> m_hit_center_x;
        std::vector<float> m_hit_center_y;
        std::vector<float> m_hit_center_z;
        std::vector<float> m_hit_R;
        std::vector<float> m_hit_locR;
        std::vector<int> m_hit_HL;
        std::vector<int> m_hit_det;
        std::vector<float> m_hit_ub_locR;
        std::vector<float> m_hit_ub_x;
        std::vector<float> m_hit_ub_y;
        std::vector<float> m_hit_ub_z;
        int m_n_pixel_holes{0};
        int m_n_sct_holes{0};
        int m_n_trt_holes{0};
        std::vector<int> m_hole_bec;
        std::vector<int> m_hole_phi;
        std::vector<int> m_hole_layer;
        std::vector<int> m_hole_strawlayer;
        std::vector<int> m_hole_straw;
        std::vector<int> m_hole_chip;
        std::vector<int> m_hole_pad;
        std::vector<float> m_hole_x;
        std::vector<float> m_hole_y;
        std::vector<float> m_hole_z;
        std::vector<float> m_hole_center_x;
        std::vector<float> m_hole_center_y;
        std::vector<float> m_hole_center_z;
        std::vector<float> m_hole_locR;
        std::vector<float> m_hole_locR_error;
        std::vector<int> m_hole_det;

        //---- branches added by dan -------
        std::vector<int> m_hit_tube_hit;
        int m_n_primary_vertex{0};
        int m_n_tube_hits{0};

        // private methods
        //----------------------------------
        void make_branches();
        void clear_branches();
        int fill_hit_data(const Trk::TrackStateOnSurface& hit);
        int fill_hole_data(const Trk::TrackStateOnSurface& hole);
};

#endif // TRT_TrackHoleSearch_TRTStrawEfficiency_h
