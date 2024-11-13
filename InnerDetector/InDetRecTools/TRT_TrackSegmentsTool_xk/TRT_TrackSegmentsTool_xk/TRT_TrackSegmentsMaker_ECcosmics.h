/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////////////////
// TRT_TrackSegments.h
//    Header file for class TRT_TrackSegmentsMaker_ECcosmics
///////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
//
// Based on TRT_TrackSegmentsMaker_CTBxk from I.Gavrilenko
///////////////////////////////////////////////////////////////////////////////
// Christian.Schmitt@cern.ch
///////////////////////////////////////////////////////////////////////////////


#ifndef TRT_TrackSegmentsMaker_ECcosmics_H
#define TRT_TrackSegmentsMaker_ECcosmics_H


#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "InDetRecToolInterfaces/ITRT_TrackSegmentsMaker.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"

#include "StoreGate/ReadHandleKey.h"
#include "TrkEventUtils/PRDtoTrackMap.h"
#include "TrkEventUtils/EventDataBase.h"

#include <iosfwd>
#include <list>

class MsgStream;

class TF1;

class TRT_ID;
namespace InDet{
  class TRT_DriftCircle;
}

namespace InDetDD{
  class TRT_DetectorManager;
}

namespace Trk{
  class TrackSegment;
  class Segment;
}

class PRD_MultiTruthCollection;


namespace InDet{

  /** @class TRT_TrackSegmentsMaker_ECcosmics

      AlgTool that creates TrackSegments out of TRT Driftcircles
      in the special case of cosmic runs in SR1

      @author Christian.Schmitt@cern.ch
   */
  class TRT_TrackSegmentsMaker_ECcosmics : // This class read and write gErrorIgnoreLevel without a global lock for ROOT.
    virtual public ITRT_TrackSegmentsMaker, public AthAlgTool
    {
      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////
      
    public:
      
      ///////////////////////////////////////////////////////////////////
      // Standard tool methods
      ///////////////////////////////////////////////////////////////////

      /** Constructor with parameters */
      TRT_TrackSegmentsMaker_ECcosmics
	(const std::string&,const std::string&,const IInterface*);
      virtual ~TRT_TrackSegmentsMaker_ECcosmics();
      virtual StatusCode               initialize() override;
      virtual StatusCode               finalize  () override;

      ///////////////////////////////////////////////////////////////////
      // Methods to initialize tool for new event or region
      ///////////////////////////////////////////////////////////////////

      virtual std::unique_ptr<InDet::ITRT_TrackSegmentsMaker::IEventData> newEvent(const EventContext& ctx) const override;
      virtual std::unique_ptr<InDet::ITRT_TrackSegmentsMaker::IEventData> newRegion(const EventContext& ctx, const std::vector<IdentifierHash>&) const override;
      void endEvent(InDet::ITRT_TrackSegmentsMaker::IEventData &event_data) const override;

      ///////////////////////////////////////////////////////////////////
      // Methods of seeds production without vertex constraint
      ///////////////////////////////////////////////////////////////////

      virtual void find(const EventContext &ctx,
                        InDet::ITRT_TrackSegmentsMaker::IEventData &event_data,
                        InDet::TRT_DetElementLink_xk::TRT_DetElemUsedMap &used) const override;

      ///////////////////////////////////////////////////////////////////
      // Iterator through seeds pseudo collection produced accordingly
      // methods find    
      ///////////////////////////////////////////////////////////////////
      
      virtual Trk::TrackSegment* next(InDet::ITRT_TrackSegmentsMaker::IEventData &event_data) const override;
  
      ///////////////////////////////////////////////////////////////////
      // Print internal tool parameters and status
      ///////////////////////////////////////////////////////////////////

      virtual MsgStream&    dump          (MsgStream   & out) const override;
      virtual std::ostream& dump          (std::ostream& out) const override;

    protected:
      class EventData;
      class EventData : public Trk::EventDataBase<EventData,InDet::ITRT_TrackSegmentsMaker::IEventData>
      {
         friend class InDet::TRT_TrackSegmentsMaker_ECcosmics;
      public:
         EventData()  {
            m_segiterator   = m_segments.begin();
         }

         ~EventData() {
            std::vector<std::vector<const InDet::TRT_DriftCircle*> *>::iterator sit,sitE;
            sit= m_seeds.begin();
            sitE= m_seeds.end();
            for(;sit!=sitE;++sit){
               (*sit)->clear();
               delete(*sit);
            }
         }

      protected:
         std::list<const InDet::TRT_DriftCircle*> m_noiseHits; //!< List containing potentially noise hits
         std::list<const InDet::TRT_DriftCircle*> m_goodHits; //!< List containing potenitally good hits


         std::list<const InDet::TRT_DriftCircle*> m_sectors[2][20][16]; //!< Divide into two endcaps and each endcap into 16 sectors in phi and 20 in z

         std::vector<std::vector<const InDet::TRT_DriftCircle*> *> m_seeds; //!<Vector of seeds
         std::list<Trk::TrackSegment*>            m_segments    ; //!< List of found segments
         std::list<Trk::TrackSegment*>::iterator  m_segiterator ; //!< Iterator over found segments

         double m_a_z[1000];
         double m_a_phi[1000];
         double m_a_phi_err[1000];
         double m_a_tan[1000];
         double m_a_tan_err[1000];
         double m_a_z_err[1000];
         double m_a_steigung[1000];

         // Fitting functions
         TF1 *m_fitf_ztanphi=nullptr; //!< analytic function to fit tan(phi) vs. z
         TF1 *m_fitf_zphi=nullptr; //!< analytic function to fit phi vs. z

         TF1 *m_fitf_ztanphi_approx=nullptr; //!< approx function to fit tan(phi) vs. z
         TF1 *m_fitf_zphi_approx=nullptr; //!< anpprox function to fit phi vs. z

      };

      ///////////////////////////////////////////////////////////////////
      // Protected data and methods
      ///////////////////////////////////////////////////////////////////

      BooleanProperty m_phaseMode{this, "Phase", false,
	"Switch to destinguish between phase calculation and full reco"};
      const TRT_ID*                            m_trtid{}       ; 

      SG::ReadHandleKey<InDet::TRT_DriftCircleContainer> m_trtname{this,"TRT_ClustersContainer","TRT_DriftCircles","RHK to retrieve TRT_DriftCircles"}; //!< TRTs   container 
      SG::ReadHandleKey<Trk::PRDtoTrackMap>       m_prdToTrackMap
         {this,"PRDtoTrackMap",""};

      ToolHandle<Trk::IRIO_OnTrackCreator> m_riomakerD
	{this, "RIOonTrackToolYesDr",
	 "InDet::TRT_DriftCircleOnTrackTool/TRT_DriftCircleOnTrackTool",
	 "RI0_onTrack creator with drift information"};
      ToolHandle<Trk::IRIO_OnTrackCreator> m_riomakerN
	{this, "RIOonTrackToolNoDr",
	 "InDet::TRT_DriftCircleOnTrackNoDriftTimeTool/TRT_DriftCircleOnTrackNoDriftTimeTool",
	 "RI0_onTrack creator without drift information"};

      BooleanProperty m_useDriftTime{this, "UseDriftTime", false,
	"Shall the drifttime be used or only tube hits?"};
      DoubleProperty m_scaleTube{this, "ScaleFactorTube", 2.,
	"Scalefactor for uncertainty of tube hits"};
      DoubleProperty m_scaleFactorDrift{this, "ScaleFactorDrift", 1.,
	"Scalefactor for uncertainty of drifttime hits"};
      DoubleProperty m_scaleTubeNoise{this, "ScaleFactorTubeNoise", 1.,
	"Scalefactor for uncertainty of tube hits flagged as noise"};
      DoubleProperty m_cutToTLoose{this, "ToTCutLoose", 7.,
	"Loose cut on ToT (preselection)"};
      DoubleProperty m_cutToTTight{this, "ToTCutTight", 18.,
	"Hard cut on ToT (preselection)"};
      DoubleProperty m_cutToTUpper{this, "ToTCutUpper", 32.,
	"Upper cut on ToT (preselection)"};
      IntegerProperty m_minDCSeed{this, "MinDCperSeed", 7,
	"Minimum number of driftcircles to form a seed"};
      IntegerProperty m_hitLimit{this, "HitLimit", 2000,
	"Maximum number of good hits (i.e. after noise cut) in endcap"};

      static std::mutex s_fitMutex;  // @TODO not sufficient need a global root lock

      ///////////////////////////////////////////////////////////////////
      // Protected methods
      ///////////////////////////////////////////////////////////////////
      
      /** sort hits into good/noise lists
       */
      void retrieveHits(TRT_TrackSegmentsMaker_ECcosmics::EventData &event_data) const;

      /** Find seed in the given sector/zslice/endcap
       */
      bool find_seed(int endcap, int zslice, int sector,TRT_TrackSegmentsMaker_ECcosmics::EventData &event_data) const;

      /** Evaluate how many dc match this seed
       */
      int evaluate_seed(int endcap,int zslice,int sector,const double *p, TRT_TrackSegmentsMaker_ECcosmics::EventData &event_data) const;

      /** Create segment out of a seed
       */
      void create_segment(std::vector<const InDet::TRT_DriftCircle*> *seed,
                          TRT_TrackSegmentsMaker_ECcosmics::EventData &event_data) const;

      void setFitFunctions(TRT_TrackSegmentsMaker_ECcosmics::EventData &event_data) const;

      /** Perform the fit and return a function that provides the fitted phi information
       */
      TF1 *perform_fit(int count,TRT_TrackSegmentsMaker_ECcosmics::EventData &event_data) const;

      /** checks if a hit that matches the segment looks suspicious (i.e. isolated)
       */
      bool is_suspicious(const InDet::TRT_DriftCircle* dc, std::vector<const InDet::TRT_DriftCircle*> *seed) const;

      /** provide the proper subtraction of two phi values
       */
      static double phidiff(double a,double b);
      
      /** is the hit accepted? 
      */
      bool accepted(const std::list<const InDet::TRT_DriftCircle*>::iterator compareIt, 
        std::list<const InDet::TRT_DriftCircle*> & container,
        double phiLimit, double dzLimit) const;

    };

} // end of name space


inline double InDet::TRT_TrackSegmentsMaker_ECcosmics::phidiff(double a,double b)
{
  double c=a-b;

  if(std::abs(c)>100)
    c/=int(c/(2*M_PI));
  while(c>=M_PI) c-=2*M_PI;
  while(c<-M_PI) c+=2*M_PI;
  return c;
}

#endif // TRT_TrackSegmentsMaker_ECcosmics_H

