/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////////////////////////////
//
//    TRT_TrackSegmentsMaker_BarrelCosmics.h, (c) ATLAS Detector software
//
//    TRT standalone segment finder for TRT barrel
//    specialized for cosmics and L2 trigger, plan to make it working in a magnetic field
//
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef TRT_TrackSegmentsMaker_BarrelCosmics_H
#define TRT_TrackSegmentsMaker_BarrelCosmics_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetPrepRawData/TRT_DriftCircleContainer.h"
#include "InDetRecToolInterfaces/ITRT_TrackSegmentsMaker.h"
#include "TrkEventUtils/EventDataBase.h"

#include "StoreGate/ReadHandleKey.h"
#include <vector>
#include <string>
#include <memory>
#include <iosfwd>

class MsgStream;
class TRT_ID;

namespace InDet{

  /** @class TRT_TrackSegmentsMaker_BarrelCosmics

      AlgTool that creates TrackSegments out of TRT Driftcircles
      for barrel TRT only, specializing for cosmic runs (to be used for LVL2 trigger)

      @author  Sasa.Fratina@cern.ch
   */

  class TRT_TrackSegmentsMaker_BarrelCosmics : virtual public ITRT_TrackSegmentsMaker, public AthAlgTool {

    public:

      TRT_TrackSegmentsMaker_BarrelCosmics(const std::string &, const std::string &, const IInterface*);
      virtual ~TRT_TrackSegmentsMaker_BarrelCosmics() {};

      ///////////////////////////////////////////////////////////////////////////////
      //  methods inherited from ITRT_TrackSegmentsMaker
      //  in each event, methods are called in this order:
      //  -  newEvent() in the offline, newRegion(...) for L2 (directed search)
      //  -  find() finds all segments
      //  -  next() many times (returns next segment), untill next() returns 0 pointer
      ///////////////////////////////////////////////////////////////////////////////

      virtual StatusCode               initialize() override;
      virtual StatusCode               finalize  () override;

      virtual std::unique_ptr<InDet::ITRT_TrackSegmentsMaker::IEventData> newEvent(const EventContext& ctx) const override;
      virtual std::unique_ptr<InDet::ITRT_TrackSegmentsMaker::IEventData> newRegion(const EventContext& ctx, const std::vector<IdentifierHash>&) const override;
      void endEvent(InDet::ITRT_TrackSegmentsMaker::IEventData &event_data) const override;

      virtual void find(const EventContext &ctx,
                        InDet::ITRT_TrackSegmentsMaker::IEventData &event_data,
                        InDet::TRT_DetElementLink_xk::TRT_DetElemUsedMap &used) const override;
      virtual Trk::TrackSegment *next(InDet::ITRT_TrackSegmentsMaker::IEventData
                                          &event_data) const override;

      virtual MsgStream&    dump          (MsgStream   & out) const override
      { return out; };
      virtual std::ostream& dump          (std::ostream& out) const override
      { return out; };


    protected:
      class EventData;
      class EventData : public Trk::EventDataBase<EventData,InDet::ITRT_TrackSegmentsMaker::IEventData>
      {
         friend class TRT_TrackSegmentsMaker_BarrelCosmics;
      public:
         EventData(const TRT_DriftCircleContainer *trtcontainer) :m_trtcontainer(trtcontainer) {}
         ~EventData() {}

         void clear() {
            m_listHits.clear();
            m_listHitCenter.clear();
            m_segmentDriftCircles.clear();
            m_segments.clear();
            m_segmentDriftCirclesCount = 0;
         }

      protected:
         const TRT_DriftCircleContainer                            *m_trtcontainer = nullptr;
         unsigned int                                               m_segmentDriftCirclesCount = 0;
         std::vector< const InDet::TRT_DriftCircle * >              m_listHits;
         std::vector< Amg::Vector3D >                               m_listHitCenter;

         std::vector< std::vector<const InDet::TRT_DriftCircle *> > m_segmentDriftCircles; // vector of DriftCircles associated to the segments
         std::vector< Trk::TrackSegment * >                         m_segments;            //!< List of found segments
      };

      ///////////////////////////////////////////////////////////////////
      // Protected data and methods
      ///////////////////////////////////////////////////////////////////



      SG::ReadHandleKey<InDet::TRT_DriftCircleContainer>     m_driftCirclesName{this,"TRT_ClustersContainer","TRT_DriftCircles","RHK to retrieve Drift Circles"} ;  //!< Container with TRT clusters
      StringProperty m_TRTManagerName{this, "TrtManagerLocation", "TRT",
        "Name of TRT det. manager"};

      const TRT_ID* m_trtid{nullptr};

    private:

      // other methods specific for this algorithm

      int findSeed(double xmin,
                   double xmax,
                   double phimin,
                   double phimax,
                   double *bestParameters,
                   TRT_TrackSegmentsMaker_BarrelCosmics::EventData &event_data) const;

      void findSeedInverseR(double *par, TRT_TrackSegmentsMaker_BarrelCosmics::EventData &event_data) const;

      // follow
      // convert lists of TRT circles (std::vector<const InDet::TRT_DriftCircle *>) to track segments (Trk::TrackSegment * -> fill m_segments):
      void convert(std::vector<const InDet::TRT_DriftCircle *> &hits,
                  double *trackpar,
                  TRT_TrackSegmentsMaker_BarrelCosmics::EventData &event_data) const;

      void findOld(TRT_TrackSegmentsMaker_BarrelCosmics::EventData &event_data) const;  // old function - need to debug an increased cpu consumption


      static void linearRegressionParabolaFit(double *mean, double *a); // input: elsewhere calculated mean, output: result a
      /** sort hits on segment such that they are ordered from larger y to smaller (from top down) */

      static bool sortHits( const InDet::TRT_DriftCircle *, const InDet::TRT_DriftCircle * );

      static void segFit(double *measx, double *measy, int nhits, double *residuals = 0, double *result = 0);

      // protect against high occupancy events
      // 21k corresponds to 20% occupancy
      // if set to 0, this requirement is not used. total number of TRT barrel straws is 105088
      IntegerProperty m_maxTotalHits{this, "MaxTotalNumberOfBarrelHits", 21000};

      IntegerProperty m_minHitsForSeed{this, "MinNumberOfHitsForSeed", -1};
      IntegerProperty m_minHitsForSegment{this, "MinimalNumberOfTRTHits", 20};
      IntegerProperty m_minHitsAboveTOT{this, "MinNumberOfHitsAboveTOT", -1};
      IntegerProperty m_nBinsInX{this, "NbinsInX", 100};
      IntegerProperty m_nBinsInPhi{this, "NbinsInPhi", 10};

      DoubleProperty m_minSeedTOT{this, "MinimalTOTForSeedSearch", 10.,
	"minimal time over threshold for seed search - default ~ 10 ns"};
      BooleanProperty m_magneticField{this, "IsMagneticFieldOn", true,
	"search for lines (if False) or circles (if True)"};
      BooleanProperty m_mergeSegments{this, "MergeSegments", false}; // default: False, should not be turned on yet

  };

} // end namespace InDet

#endif // TRT_TrackSegmentsMaker_BarrelCosmics_H
