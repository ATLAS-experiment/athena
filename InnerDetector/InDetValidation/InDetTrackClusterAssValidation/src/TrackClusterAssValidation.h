/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef TrackClusterAssValidation_H
#define TrackClusterAssValidation_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "InDetPrepRawData/SiClusterContainer.h"
#include "InDetPrepRawData/TRT_DriftCircleContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "TrkSpacePoint/SpacePointContainer.h"
#include "TrkSpacePoint/SpacePointOverlapCollection.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "TrackClusterAssValidationUtils.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include <string>
#include <map>

namespace InDet {

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

      bool                               m_usePIX{}                 ;
      bool                               m_useSCT{}                 ;
      bool                               m_useTRT{}                 ;
      bool                               m_useOutliers{}            ;
      int                                m_pdg{}                    ;

      mutable std::mutex                           m_statMutex;
      mutable std::vector<TrackCollectionStat_t>   m_trackCollectionStat ATLAS_THREAD_SAFE; // Guarded by m_statMutex
      mutable EventStat_t                          m_eventStat ATLAS_THREAD_SAFE; // Guarded by m_statMutex

      unsigned int                       m_clcut{}                  ;
      unsigned int                       m_clcutTRT{}               ;
      unsigned int                       m_spcut{}                  ;
      double                             m_ptcut{}                  ;
      double                             m_ptcutmax{}               ;
      double                             m_rapcut{}                 ;
      double                             m_tcut{}                   ;
      double                             m_rmin{}                   ;
      double                             m_rmax{}                   ;
      SG::ReadHandleKeyArray<TrackCollection>        m_tracklocation;
      SG::ReadHandleKey<SpacePointContainer>         m_spacepointsSCTname;
      SG::ReadHandleKey<SpacePointContainer>         m_spacepointsPixelname;
      SG::ReadHandleKey<SpacePointOverlapCollection> m_spacepointsOverlapname;
      SG::ReadHandleKey<SiClusterContainer>          m_clustersSCTname;
      SG::ReadHandleKey<SiClusterContainer>          m_clustersPixelname;
      SG::ReadHandleKey<TRT_DriftCircleContainer>    m_clustersTRTname;
      SG::ReadHandleKey<PRD_MultiTruthCollection>    m_truth_locationPixel;
      SG::ReadHandleKey<PRD_MultiTruthCollection>    m_truth_locationSCT;
      SG::ReadHandleKey<PRD_MultiTruthCollection>    m_truth_locationTRT;
      // For P->T converter of PixelClusters
      SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetEleCollKey", "PixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};
      // For P->T converter of SCT_Clusters
      SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};

      struct EventData_t {
      public:
        
        EventData_t(unsigned int n_collections)
        : m_nspacepoints(0),
          m_nclusters(0),
          m_nqtracks(0),
          m_nclustersTRT(0),
          m_truthPIX{},
          m_truthSCT{},
          m_truthTRT{}
        {
          m_particles.resize(n_collections);
          m_difference.resize(n_collections);
          m_tracks.resize(n_collections);
          m_trackCollectionStat.resize(n_collections);
        }

        int                                m_nspacepoints{}           ;
        int                                m_nclusters{}              ;
        int                                m_nqtracks{}               ;
        int                                m_nclustersTRT{}           ;

        std::vector<std::unique_ptr<SG::VarHandleBase> >  m_clusterHandles;
        std::vector<SG::ReadHandle<TrackCollection> >     m_trackcontainer;
        std::vector<SG::ReadHandle<SpacePointContainer> > m_spacePointContainer;
        std::unique_ptr<SG::ReadHandle<SpacePointOverlapCollection> > m_spacepointsOverlap;
        const PRD_MultiTruthCollection           * m_truthPIX{}       ;
        const PRD_MultiTruthCollection           * m_truthSCT{}       ;
        const PRD_MultiTruthCollection           * m_truthTRT{}       ;
        std::multimap<int,const Trk::PrepRawData*> m_kinecluster    ;
        std::multimap<int,const Trk::PrepRawData*> m_kineclusterTRT ;
        std::multimap<int,const Trk::SpacePoint*>  m_kinespacepoint ;
        std::vector<std::list<PartPropCache> >           m_particles      ;
        std::vector<std::list<int> >               m_difference     ;
        std::vector<std::multimap<int,int> >       m_tracks         ;
        std::vector<TrackCollectionStat_t>         m_trackCollectionStat;
        EventStat_t                                m_eventStat{};
      };

      ///////////////////////////////////////////////////////////////////
      // Protected methods
      ///////////////////////////////////////////////////////////////////

      void newSpacePointsEvent     (const EventContext& ctx, InDet::TrackClusterAssValidation::EventData_t &event_data) const;
      void newClustersEvent        (const EventContext& ctx, InDet::TrackClusterAssValidation::EventData_t &event_data) const;
      void tracksComparison        (const EventContext& ctx, InDet::TrackClusterAssValidation::EventData_t &event_data) const;
      void efficiencyReconstruction(InDet::TrackClusterAssValidation::EventData_t &event_data) const;
      bool noReconstructedParticles(const InDet::TrackClusterAssValidation::EventData_t &event_data) const;

      int qualityTracksSelection(InDet::TrackClusterAssValidation::EventData_t &event_data) const;
      int kine(const InDet::TrackClusterAssValidation::EventData_t &event_data,const Trk::PrepRawData*,const Trk::PrepRawData*,int*,int) const;
      int kine (const InDet::TrackClusterAssValidation::EventData_t &event_data,const Trk::PrepRawData*,int*,int) const;
      static int kine0(const InDet::TrackClusterAssValidation::EventData_t &event_data,const Trk::PrepRawData*,int*,int) ;
      static bool isTruth(const InDet::TrackClusterAssValidation::EventData_t&  ,const Trk::PrepRawData*) ;
      static bool isTheSameDetElement(const InDet::TrackClusterAssValidation::EventData_t &event_data, int,const Trk::PrepRawData*) ;
      static bool isTheSameDetElement(const InDet::TrackClusterAssValidation::EventData_t &event_data,int,const Trk::SpacePoint* ) ;

      static PRD_MultiTruthCollection::const_iterator findTruth
      (const InDet::TrackClusterAssValidation::EventData_t &event_data,const Trk::PrepRawData*,PRD_MultiTruthCollection::const_iterator&) ;

      int charge(const InDet::TrackClusterAssValidation::EventData_t &event_data,std::pair<int,const Trk::PrepRawData*>,int&) const;

      MsgStream&    dumptools(MsgStream&    out, MSG::Level level) const;
      static MsgStream&    dumpevent(MsgStream&    out, const InDet::TrackClusterAssValidation::EventData_t &event_data) ;

    };

}
#endif // TrackClusterAssValidation_H
