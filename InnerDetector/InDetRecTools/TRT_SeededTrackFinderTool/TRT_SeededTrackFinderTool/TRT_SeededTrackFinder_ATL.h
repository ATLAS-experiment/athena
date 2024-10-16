/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
   Header file for class TRT_SeededTrackFinder_ATL
  (c) ATLAS Detector software
  Class for Trk::Track production in SCT and Pixels
  Version 1.0: 04/12/2006
  Authors    : Thomas Koffas
  email      : Thomas.Koffas@cern.ch
**********************************************************************************/

#ifndef TRT_SeededTrackFinder_ATL_H
#define TRT_SeededTrackFinder_ATL_H

#include "AthenaBaseComps/AthAlgTool.h"

//Tool Handler
//
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ServiceHandle.h"

//Tool Interface
//
#include "InDetRecToolInterfaces/ITRT_SeededTrackFinder.h"
#include "InDetRecToolInterfaces/ITRT_SeededSpacePointFinder.h"
#include "TrkEventUtils/EventDataBase.h"

//Magnetic field
//
#include "TrkGeometry/MagneticFieldProperties.h"

// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"

//Si Tools
//
#include "TRT_SeededTrackFinderTool/SiNoise_bt.h"
#include "SiSPSeededTrackFinderData/SiDetElementRoadMakerData_xk.h"

//Combinatorial Track Finder Tool
//
#include "InDetRecToolInterfaces/ISiCombinatorialTrackFinder.h"

//Tool for getting the SiDetElements from geometry
#include "InDetRecToolInterfaces/ISiDetElementsRoadMaker.h"

//Updator tool
#include "TrkToolInterfaces/IUpdator.h"

//Propagator tool
#include "TrkExInterfaces/IPropagator.h"

//ReadHandle key
//
#include "StoreGate/ReadHandleKey.h"

#include <list>
#include <vector>
#include <map>
#include <iosfwd>

class MsgStream;
class TRT_ID   ;

namespace InDet{
  class ISiDetElementsRoadMaker;
  class SiCombinatorialTrackFinderData_xk;

  /**
  @class TRT_SeededTrackFinder_ATL

  InDet::TRT_SeededTrackFinderATL is an algorithm which produces tracks
  along the road of InDetDD::SiDetectorElement* sorted in propagation order.
  @author Thomas.Koffas@cern.ch
  */

  class TRT_SeededTrackFinder_ATL :virtual public ITRT_SeededTrackFinder, public AthAlgTool{
     
    public:

      ///////////////////////////////////////////////////////////////////
      /** Standard tool methods                                        */
      ///////////////////////////////////////////////////////////////////

      TRT_SeededTrackFinder_ATL(const std::string&,const std::string&,const IInterface*);
      virtual ~TRT_SeededTrackFinder_ATL();
      virtual StatusCode initialize() override;
      virtual StatusCode finalize  () override;

      ///////////////////////////////////////////////////////////////////
      /** Main methods for local track finding                         */
      ///////////////////////////////////////////////////////////////////

      /** Main method. Calls private methods and returns a list of Si tracks */
      virtual std::list<Trk::Track*> 
        getTrack (const EventContext& ctx, InDet::ITRT_SeededTrackFinder::IEventData &event_data,
                const Trk::TrackSegment&) const override;
      /** New event initialization */
      virtual std::unique_ptr<InDet::ITRT_SeededTrackFinder::IEventData>
        newEvent(const EventContext& ctx, SiCombinatorialTrackFinderData_xk& combinatorialData) const override;
      /** New region intialization */
      virtual std::unique_ptr<InDet::ITRT_SeededTrackFinder::IEventData>
         newRegion(const EventContext& ctx, SiCombinatorialTrackFinderData_xk& combinatorialData,
                   const std::vector<IdentifierHash>&,const std::vector<IdentifierHash>&) const override;
      /** End of event tasks       */
      virtual void 
        endEvent(InDet::ITRT_SeededTrackFinder::IEventData &event_data) const override;

      ///////////////////////////////////////////////////////////////////
      /** Print internal tool parameters and status                    */
      ///////////////////////////////////////////////////////////////////

      MsgStream&    dump(MsgStream&    out) const override;
      std::ostream& dump(std::ostream& out) const override;

    protected:

       class EventData;
       class EventData : public Trk::EventDataBase<EventData,InDet::ITRT_SeededTrackFinder::IEventData> {
       public:
          friend class TRT_SeededTrackFinder_ATL;
          EventData(SiCombinatorialTrackFinderData_xk& combinatorialData,
                    std::unique_ptr<InDet::ITRT_SeededSpacePointFinder::IEventData> &&spacePointFinderEventData)
             : m_combinatorialData(&combinatorialData),
               m_spacePointFinderEventData(std::move(spacePointFinderEventData) )
          {}
          virtual InDet::SiCombinatorialTrackFinderData_xk &combinatorialData() override             { return *m_combinatorialData; }
          virtual const InDet::SiCombinatorialTrackFinderData_xk &combinatorialData() const override { return *m_combinatorialData; }

          InDet::ITRT_SeededSpacePointFinder::IEventData &spacePointFinderEventData(){ return *m_spacePointFinderEventData; }
          std::multimap<const Trk::PrepRawData*,const Trk::Track*> &clusterTrack() { return m_clusterTrack; }
          void setCaloClusterROIEM(const ROIPhiRZContainer &rois)   { m_caloClusterROIEM  = &rois; }
          const ROIPhiRZContainer *caloClusterROIEM() const         { return m_caloClusterROIEM; }
          std::vector<double>&                                      caloF()        { return m_caloF; }
          std::vector<double>&                                      caloE()        { return m_caloE; }
          const std::vector<double>&                                caloF()  const { return m_caloF; }
          const std::vector<double>&                                caloE()  const { return m_caloE; }
          InDet::SiNoise_bt&                                        noise()        { return m_noise; }
          const InDet::SiNoise_bt&                                  noise()  const { return m_noise; }
          InDet::SiDetElementRoadMakerData_xk& roadMakerData() {return m_roadMakerData; }
       protected:
          SiCombinatorialTrackFinderData_xk                              *m_combinatorialData;
          std::unique_ptr<InDet::ITRT_SeededSpacePointFinder::IEventData> m_spacePointFinderEventData;
          std::multimap<const Trk::PrepRawData*,const Trk::Track*> m_clusterTrack  ; /** Multimap of tracks and associated PRDs  */
          const ROIPhiRZContainer        *m_caloClusterROIEM {};
          std::vector<double>              m_caloF         ;
          std::vector<double>              m_caloE         ;

          /** Needed for adding material related noise   */
          InDet::SiNoise_bt                    m_noise        ;
          InDet::SiDetElementRoadMakerData_xk  m_roadMakerData; 
       };

      ///////////////////////////////////////////////////////////////////
      /** Protected Data                                               */
      ///////////////////////////////////////////////////////////////////

      StringProperty m_fieldmode{this, "MagneticFieldMode", "MapSolenoid",
	"Magnetic field mode"};

      Trk::MagneticFieldProperties        m_fieldprop     ;  /** Magnetic field properties */

      /** Tools used  */

      ToolHandle<InDet::ISiDetElementsRoadMaker> m_roadmaker
	{this, "RoadTool", "InDet::SiDetElementsRoadMaker_xk"};
      ToolHandle<InDet::ITRT_SeededSpacePointFinder> m_seedmaker
	{this, "SeedTool", ""};
      ToolHandle<Trk::IPropagator> m_proptool
	{this, "PropagatorTool", "Trk::RungeKuttaPropagator/InDetPropagator"};
      ToolHandle<Trk::IUpdator> m_updatorTool
	{this, "UpdatorTool", "Trk::KalmanUpdator_xk/InDetPatternUpdator"};
      ToolHandle<InDet::ISiCombinatorialTrackFinder> m_tracksfinder
	{this, "CombinatorialTrackFinder", "InDet::SiCombinatorialTrackFinder_xk"};

      SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCondObjInputKey {this, "AtlasFieldCacheCondObj",
        "fieldCondObj", "Name of the Magnetic Field conditions object key"};

      /**ID TRT helper*/
      const TRT_ID* m_trtId = nullptr;

      /** Track quality cuts to be passed to the combinatorial track finder */
      DoubleProperty m_xi2max{this, "Xi2max", 15., "max Xi2 for updators"};
      DoubleProperty m_xi2maxNoAdd{this, "Xi2maxNoAdd", 50.,
	"max Xi2 for outliers"};
      DoubleProperty m_xi2maxlink{this, "Xi2maxlink", 100.,
	"max Xi2 for clusters"};
      DoubleProperty m_pTmin{this, "pTmin", 500., "min pT"};
      IntegerProperty m_nholesmax{this, "nHolesMax", 1, "Max number holes"};
      IntegerProperty m_dholesmax{this, "nHolesGapMax", 1,
	"Max gap between holes"};
      IntegerProperty m_nclusmin{this, "nClustersMin", 4, "Min number clusters"};
      IntegerProperty m_nwclusmin{this, "nWClustersMin", 4,
	"Min number weighted clusters"};
      BooleanProperty m_bremCorrect{this, "BremCorrection", false,
	"Optional Brem correction"};
      BooleanProperty m_propR{this, "ConsistentSeeds", false,
	"Check seed-TRT segment consistency at large eta"};
      BooleanProperty m_useassoTool{this, "UseAssociationTool", false,
	"Use prd-track association tool"};
      InDet::TrackQualityCuts      m_trackquality  ;
      DoubleArrayProperty m_errorScale
	{this, "ErrorScaling", {1., 1., 1., 1., 1.},
	 "Optional error scaling of track parameters"};
      DoubleProperty m_outlierCut{this, "OutlierCut", 25.,
	"Outlier chi2 cut when propagating through the seed"};
      BooleanProperty m_searchInCaloROI{this, "SearchInCaloROI", false,
	"Outlier chi2 cut when propagating through the seed"};
      SG::ReadHandleKey<ROIPhiRZContainer> m_caloClusterROIKey{this, "EMROIPhiRZContainer", ""};


      ///////////////////////////////////////////////////////////////////
      /** Private Methods                                              */
      ///////////////////////////////////////////////////////////////////

      /** Get Magnetic field properties  */
      void magneticFieldInit();

      /** Update track parameters through space point propagation  */
      std::unique_ptr<const Trk::TrackParameters> 
      getTP(MagField::AtlasFieldCache& fieldCache, const Trk::SpacePoint*,
             const Trk::TrackParameters&, bool&, 
             InDet::TRT_SeededTrackFinder_ATL::EventData &event_data) const;

      /** Find the corresponding list of Si tracks  */
      std::list<Trk::Track*>
      findTrack(const EventContext& ctx, MagField::AtlasFieldCache& fieldCache,
                InDet::TRT_SeededTrackFinder_ATL::EventData &event_data,
                const Trk::TrackParameters&,const Trk::TrackSegment&) const;

      /** Add material effects   */
      std::unique_ptr<const Trk::TrackParameters> 
      addNoise(const SiNoise_bt &,const Trk::TrackParameters&,int) const;


      /** Check consistency of seed and TRT track segment */
      bool
      checkSeed(std::vector<const Trk::SpacePoint*>&,const Trk::TrackSegment&,
                const Trk::TrackParameters&) const;

      /** Modify track parameters if brem correction  */
      static std::unique_ptr<const Trk::TrackParameters>
      modifyTrackParameters(const Trk::TrackParameters&,int) ;

      /** Set the track quality cuts for combinatorial track finding   */
      void
      setTrackQualityCuts();

      /** Map PRDs-tracks */
      static void
      clusterTrackMap(Trk::Track*,InDet::TRT_SeededTrackFinder_ATL::EventData &event_data);

      /** Seed used by another track?  */
      static bool
      newClusters(const std::vector<const Trk::SpacePoint*>&,
                  InDet::TRT_SeededTrackFinder_ATL::EventData &event_data) ;

      /** Seed SPs used by other high quality tracks? */
      static bool
      newSeed(const std::vector<const Trk::SpacePoint*>&,
              InDet::TRT_SeededTrackFinder_ATL::EventData &event_data) ;

      /** Clean-up duplicate tracks  */
      static bool
      isNewTrack(Trk::Track*,InDet::TRT_SeededTrackFinder_ATL::EventData &event_data) ;

      /** Eliminate spurious Pixel clusters in track  */
      std::list<Trk::Track*> 
      cleanTrack(std::list<Trk::Track*>) const;

      /** Only propagate to the Si if the TRT segment is compatible with a calo measurement */
      bool isCaloCompatible(const Trk::TrackParameters&, const InDet::TRT_SeededTrackFinder_ATL::EventData &event_data) const;
      DoubleProperty m_phiWidth{this, "phiWidth", 0.3};

      MsgStream&    dumpconditions(MsgStream&    out) const;

    };

}  // end of namespace

#endif // TRT_SeededTrackFinder_ATL
