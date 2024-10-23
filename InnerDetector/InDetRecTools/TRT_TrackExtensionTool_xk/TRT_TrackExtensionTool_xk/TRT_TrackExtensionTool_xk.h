/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class TRT_TrackExtensionTool_xk
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
//  Concret implementation for base class ITRT_TrackExtensionTool
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 22/08/2005 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef TRT_TrackExtensionTool_xk_H
#define TRT_TrackExtensionTool_xk_H



#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetRecToolInterfaces/ITRT_TrackExtensionTool.h"
#include "InDetRecToolInterfaces/ITrtDriftCircleCutTool.h"
#include "InDetRecToolInterfaces/ITRT_DetElementsRoadMaker.h"
#include "TrkEventUtils/EventDataBase.h"
#include "TRT_TrackExtensionTool_xk/TRT_Trajectory_xk.h"
#include "InDetPrepRawData/TRT_DriftCircleContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include <iosfwd>
#include <vector>

// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"

class MsgStream;
class TRT_ID;

namespace InDet{

  /**
  @class TRT_TrackExtensionTool_xk

  InDet::TRT_TrackExtensionTool_xk is algorithm which produce track
  extension to TRT
  @author Igor.Gavrilenko@cern.ch
  */

  class TRT_TrackExtensionTool_xk :

    virtual public ITRT_TrackExtensionTool, public AthAlgTool
    {
      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////

    public:

      ///////////////////////////////////////////////////////////////////
      // Standard tool methods
      ///////////////////////////////////////////////////////////////////

      TRT_TrackExtensionTool_xk
	(const std::string&,const std::string&,const IInterface*);
      virtual ~TRT_TrackExtensionTool_xk();
      virtual StatusCode initialize() override;
      virtual StatusCode finalize  () override;

      ///////////////////////////////////////////////////////////////////
      // Main methods for track extension to TRT
      ///////////////////////////////////////////////////////////////////

      virtual std::vector<const Trk::MeasurementBase*>& extendTrack(
          const EventContext& ctx, const Trk::Track&,
          InDet::ITRT_TrackExtensionTool::IEventData& virt_event_data,
          InDet::TRT_DetElementLink_xk::TRT_DetElemUsedMap& used)
          const override;
      virtual std::vector<const Trk::MeasurementBase*>& extendTrack(
          const EventContext& ctx, const Trk::TrackParameters*,
          InDet::ITRT_TrackExtensionTool::IEventData& virt_event_data,
          InDet::TRT_DetElementLink_xk::TRT_DetElemUsedMap& used)
          const override;
      virtual Trk::TrackSegment* findSegment(
          const EventContext& ctx, const Trk::TrackParameters*,
          InDet::ITRT_TrackExtensionTool::IEventData& virt_event_data,
          InDet::TRT_DetElementLink_xk::TRT_DetElemUsedMap& used)
          const override;
      virtual Trk::Track* newTrack(const EventContext& ctx, const Trk::Track&,
                                   InDet::ITRT_TrackExtensionTool::IEventData& virt_event_data,
                                   InDet::TRT_DetElementLink_xk::TRT_DetElemUsedMap& used) const override;
      virtual std::unique_ptr<InDet::ITRT_TrackExtensionTool::IEventData>
      newEvent(const EventContext& ctx) const override;

      ///////////////////////////////////////////////////////////////////
      // Print internal tool parameters and status
      ///////////////////////////////////////////////////////////////////

      virtual MsgStream&    dump(MsgStream&    out) const override;
      virtual std::ostream& dump(std::ostream& out) const override;

    protected:

      class EventData;
      class EventData : public Trk::EventDataBase<EventData,InDet::ITRT_TrackExtensionTool::IEventData>
      {
         friend class TRT_TrackExtensionTool_xk;
      public:
         EventData(const TRT_DriftCircleContainer *trtcontainer,
                   double maxslope) : m_trtcontainer(trtcontainer),m_maxslope(maxslope) {}

         ~EventData() {}

      protected:
         const TRT_DriftCircleContainer           *m_trtcontainer;
         std::vector<const Trk::MeasurementBase*>  m_measurement;
         TRT_Trajectory_xk                         m_trajectory;
         double m_maxslope;
      };


      std::vector<const Trk::MeasurementBase*>& extendTrackFromParameters
      (const EventContext& ctx,
       const Trk::TrackParameters *,
       InDet::TRT_TrackExtensionTool_xk::EventData &event_data,
       InDet::TRT_DetElementLink_xk::TRT_DetElemUsedMap& used) const;

      ///////////////////////////////////////////////////////////////////
      // Protected Data
      ///////////////////////////////////////////////////////////////////

      const TRT_ID                          *m_trtid{};
      ToolHandle<ITRT_DetElementsRoadMaker>         m_roadtool
	{this, "RoadTool", "InDet::TRT_DetElementsRoadMaker_xk"}; // TRT road maker tool
      PublicToolHandle<Trk::IPatternParametersPropagator> m_proptool
	{this, "PropagatorTool", "Trk::RungeKuttaPropagator"};
      PublicToolHandle<Trk::IPatternParametersUpdator>    m_updatortool
	{this, "UpdatorTool", "Trk::KalmanUpdator_xk"};
      ToolHandle<ITrtDriftCircleCutTool>            m_selectortool
	{this, "DriftCircleCutTool", "InDet::InDetTrtDriftCircleCutTool"};// Segment selector tool
      ToolHandle<Trk::IRIO_OnTrackCreator>          m_riontrackD
	{this, "RIOonTrackToolYesDr", "InDet::TRT_DriftCircleOnTrackTool/TRT_DriftCircleOnTrackTool"};
      ToolHandle<Trk::IRIO_OnTrackCreator>          m_riontrackN
	{this, "RIOonTrackToolNoDr", "InDet::TRT_DriftCircleOnTrackNoDriftTimeTool/TRT_DriftCircleOnTrackNoDriftTimeTool"};

      SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};

      int                              m_outputlevel{}    ; // Print level

      IntegerProperty m_segmentFindMode{this, "SegmentFindMode", 3,
	"Method of segment find"};
      IntegerProperty m_minNumberDCs{this, "MinNumberDriftCircles", 9,
	"Min. number of DriftCircles"};
      IntegerProperty m_minNumberSCT{this, "MinNumberSCTclusters", 5,
	"Min. number SCT clusters for extension"};
      IntegerProperty m_minNumberPIX{this, "MinNumberPIXclusters", 2,
	"Min. number pixel clusters for extension"};
      DoubleProperty m_roadwidth{this, "RoadWidth", 10.,
	"Max width of the road"};
      DoubleProperty m_maxslope{this, "Maxslope", .00005};
      DoubleProperty m_zVertexWidth{this, "ZVertexHalfWidth", 150.,
	"Z-vertex half width"};
      DoubleProperty m_impact{this, "maxImpactParameter", 50,
	"max impact parameter"};
      BooleanProperty m_usedriftrad{this, "UseDriftRadius", true,
	"Use drift time ?"};
      BooleanProperty m_parameterization{this, "UseParameterization", true,
	"Use table of min number DCs"};
      DoubleProperty m_scale_error{this, "ScaleHitUncertainty", 2.,
	"Scalefactor for hit uncertainty"};
      StringProperty m_fieldmode{this, "MagneticFieldMode", "MapSolenoid",
	"Mode of magnetic field"};

      Trk::MagneticFieldProperties     m_fieldprop      ; // Magnetic field properties
      SG::ReadHandleKey<TRT_DriftCircleContainer> m_trtname {this,"TRT_ClustersContainer","TRT_DriftCircles","RHK to retrieve TRT_DriftCircleContainer"};

      DoubleProperty m_minTRTSegmentpT{this, "minTRTSegmentpT", 300.,
	"Min pT cut required for TRT Segments"};
      ///////////////////////////////////////////////////////////////////
      // Methods
      ///////////////////////////////////////////////////////////////////

      void       magneticFieldInit();
      StatusCode magneticFieldInit(IOVSVC_CALLBACK_ARGS);
      bool isGoodExtension(const EventContext& ctx,
                           const Trk::TrackParameters *,
                           InDet::TRT_TrackExtensionTool_xk::EventData &event_data,
                           InDet::TRT_DetElementLink_xk::TRT_DetElemUsedMap& used) const;
      bool numberPIXandSCTclustersCut(const Trk::Track&) const;

      MsgStream&    dumpConditions(MsgStream   & out) const;
    };

  MsgStream&    operator << (MsgStream&   ,const TRT_TrackExtensionTool_xk&);
  std::ostream& operator << (std::ostream&,const TRT_TrackExtensionTool_xk&);

} // end of name space

#endif // TRT_TrackExtensionTool_xk_H
