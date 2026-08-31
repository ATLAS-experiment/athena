/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef ITkTrackClusterAssValidation_H
#define ITkTrackClusterAssValidation_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "InDetPrepRawData/SiClusterContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "TrkSpacePoint/SpacePointContainer.h"
#include "TrkSpacePoint/SpacePointOverlapCollection.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "TrackClusterAssValidationUtils.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include <string>
#include <cmath>
#include <map>

namespace ITk {

  // Class-algorithm for track cluster association validation
  //
  class TrackClusterAssValidation : public AthReentrantAlgorithm
    {

      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////

    public:

      ///////////////////////////////////////////////////////////////////
      // Standard Algotithm methods
      ///////////////////////////////////////////////////////////////////

      TrackClusterAssValidation(const std::string &name, ISvcLocator *pSvcLocator);
      virtual ~TrackClusterAssValidation();
      StatusCode initialize();
      StatusCode execute(const EventContext& ctx) const;
      StatusCode finalize();

    protected:

      ///////////////////////////////////////////////////////////////////
      // Protected data
      ///////////////////////////////////////////////////////////////////

      BooleanProperty m_usePix{this, "usePixel", true};
      BooleanProperty m_useStrip{this, "useStrip", true};
      BooleanProperty m_useOutliers{this, "useOutliers", false};
      IntegerProperty m_pdg{this, "pdgParticle", 0};

      mutable std::mutex                           m_statMutex;
      mutable std::vector<InDet::TrackCollectionStat_t>   m_trackCollectionStat ATLAS_THREAD_SAFE; // Guarded by m_statMutex
      mutable InDet::EventStat_t                          m_eventStat ATLAS_THREAD_SAFE; // Guarded by m_statMutex

      UnsignedIntegerProperty m_spcut{this, "MinNumberSpacePoints", 3};
      FloatProperty m_ptcutmax{this, "MomentumMaxCut", 1.e20};
      FloatProperty m_rapcut{this, "RapidityCut", 4.0};
      FloatProperty m_ptcut{this, "MomentumCut", {}};
      FloatProperty m_rmin{this, "RadiusMin", 0.};
      FloatProperty m_rmax{this, "RadiusMax", 20.};

      FloatArrayProperty m_etabins{this, "EtaBins", {}};
      FloatArrayProperty m_ptcuts{this, "PtCuts", {}};
      UnsignedIntegerArrayProperty m_clcuts{this, "MinNumberClustersCuts", {}};

      float m_tcut = 0;

      SG::ReadHandleKeyArray<TrackCollection>        m_tracklocation
	{this, "TracksLocation", {"CombinedITkTracks"}};
      SG::ReadHandleKey<SpacePointContainer>         m_spacepointsStripname
	{this, "SpacePointsStripName", "ITkStripSpacePoints"};
      SG::ReadHandleKey<SpacePointContainer>         m_spacepointsPixelname
	{this, "SpacePointsPixelName", "ITkPixelSpacePoints"};
      SG::ReadHandleKey<SpacePointOverlapCollection> m_spacepointsOverlapname
	{this, "SpacePointsOverlapName", "ITkOverlapSpacePoints"};
      SG::ReadHandleKey<InDet::SiClusterContainer>          m_clustersStripname
	{this, "StripClusterContainer", "ITkStripClusters"};
      SG::ReadHandleKey<InDet::SiClusterContainer>          m_clustersPixelname
	{this, "PixelClusterContainer", "ITkPixelClusters"};
      SG::ReadHandleKey<PRD_MultiTruthCollection>    m_truth_locationPixel
	{this, "TruthLocationPixel", "PRD_MultiTruthITkPixel"};      
      SG::ReadHandleKey<PRD_MultiTruthCollection>    m_truth_locationStrip
	{this, "TruthLocationStrip", "PRD_MultiTruthITkStrip"};

      // For P->T converter of PixelClusters
      SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetEleCollKey", "ITkPixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};
      // For P->T converter of StripClusters
      SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_StripDetEleCollKey{this, "StripDetEleCollKey", "ITkStripDetectorElementCollection", "Key of SiDetectorElementCollection for Strip"};

      struct EventData_t {
      public:
        
        EventData_t(unsigned int n_collections)
        : m_nspacepoints(0),
          m_nclusters(0),
          m_nqtracks(0),
          m_truthPix{},
          m_truthStrip{}
        {
          m_particles.resize(n_collections);
          m_difference.resize(n_collections);
          m_tracks.resize(n_collections);
          m_trackCollectionStat.resize(n_collections);
        }

        int                                m_nspacepoints{}           ;
        int                                m_nclusters{}              ;
        int                                m_nqtracks{}               ;

        std::vector<std::unique_ptr<SG::VarHandleBase> >  m_clusterHandles;
        std::vector<SG::ReadHandle<TrackCollection> >     m_trackcontainer;
        std::vector<SG::ReadHandle<SpacePointContainer> > m_spacePointContainer;
        std::unique_ptr<SG::ReadHandle<SpacePointOverlapCollection> > m_spacepointsOverlap;
        const PRD_MultiTruthCollection           * m_truthPix{}     ;
        const PRD_MultiTruthCollection           * m_truthStrip{}   ;
        std::multimap<int,const Trk::PrepRawData*> m_kinecluster    ;
        std::multimap<int,const Trk::SpacePoint*>  m_kinespacepoint ;
        std::vector<std::list<InDet::PartPropCache> >           m_particles      ;
        std::vector<std::list<int> >               m_difference     ;
        std::vector<std::multimap<int,int> >       m_tracks         ;
        std::vector<InDet::TrackCollectionStat_t>  m_trackCollectionStat;
	InDet::EventStat_t                         m_eventStat{};
      };

      ///////////////////////////////////////////////////////////////////
      // Protected methods
      ///////////////////////////////////////////////////////////////////

      void newSpacePointsEvent
	(const EventContext& ctx,
	 ITk::TrackClusterAssValidation::EventData_t &event_data) const;
      void newClustersEvent
	(const EventContext& ctx,
	 ITk::TrackClusterAssValidation::EventData_t &event_data) const;
      void tracksComparison
	(const EventContext& ctx,
	 ITk::TrackClusterAssValidation::EventData_t &event_data) const;
      void efficiencyReconstruction
	(ITk::TrackClusterAssValidation::EventData_t &event_data) const;
      bool noReconstructedParticles
	(const ITk::TrackClusterAssValidation::EventData_t &event_data) const;

      int qualityTracksSelection
	(ITk::TrackClusterAssValidation::EventData_t &event_data) const;
      int kine
	(const ITk::TrackClusterAssValidation::EventData_t &event_data,
	 const Trk::PrepRawData*, const Trk::PrepRawData*,
	 int*, int) const;
      int kine
	(const ITk::TrackClusterAssValidation::EventData_t &event_data,
	 const Trk::PrepRawData*,
	 int*, int) const;
      static int kine0
	(const ITk::TrackClusterAssValidation::EventData_t &event_data,
	 const Trk::PrepRawData*,
	 int*, int);
      static bool isTruth
	(const ITk::TrackClusterAssValidation::EventData_t&,
	 const Trk::PrepRawData*);
      static bool isTheSameDetElement
	(const ITk::TrackClusterAssValidation::EventData_t &event_data,
	 int, const Trk::PrepRawData*);
      static bool isTheSameDetElement
	(const ITk::TrackClusterAssValidation::EventData_t &event_data,
	 int, const Trk::SpacePoint*);

      static PRD_MultiTruthCollection::const_iterator findTruth
      (const ITk::TrackClusterAssValidation::EventData_t &event_data,
       const Trk::PrepRawData*, PRD_MultiTruthCollection::const_iterator&) ;

      int charge
	(const ITk::TrackClusterAssValidation::EventData_t &event_data,
	 std::pair<int,const Trk::PrepRawData*>, int&, double&) const;
      int charge
	(const ITk::TrackClusterAssValidation::EventData_t &event_data,
	 std::pair<int,const Trk::PrepRawData*> pa, int& rap) const
      { double eta; return charge(event_data, pa, rap, eta); };

      float minpT(float eta) const;
      unsigned int minclusters(float eta) const;

      MsgStream& dumptools(MsgStream& out, MSG::Level level) const;
      static MsgStream& dumpevent
	(MsgStream& out,
	 const ITk::TrackClusterAssValidation::EventData_t &event_data) ;

    };

  inline float ITk::TrackClusterAssValidation::minpT(float eta) const {
    float aeta = std::abs(eta);
    for(int n = int(m_ptcuts.size()-1); n>0; --n) {
      if(aeta > m_etabins.value().at(n)) return m_ptcuts.value().at(n);
    }
    return m_ptcuts.value().at(0);
  }

  inline unsigned int ITk::TrackClusterAssValidation::minclusters(float eta) const {
    float aeta = std::abs(eta);
    for(int n = int(m_clcuts.size()-1); n>0; --n) {
      if(aeta > m_etabins.value().at(n)) return m_clcuts.value().at(n);
    }
    return m_clcuts.value().at(0);
  }


}
#endif // TrackClusterAssValidation_H
