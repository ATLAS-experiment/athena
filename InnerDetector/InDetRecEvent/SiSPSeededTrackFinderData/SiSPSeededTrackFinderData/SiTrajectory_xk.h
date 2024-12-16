/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class SiTrajectory_xk
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for trajectory in Pixels and SCT
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 3/10/2004 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiTrajector_xk_H
#define SiTrajector_xk_H

#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "TrkTrack/Track.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "SiSPSeededTrackFinderData/SiTools_xk.h"
#include "SiSPSeededTrackFinderData/SiTrajectoryElement_xk.h"
#include "SiSPSeededTrackFinderData/SiDetElementBoundaryLink_xk.h"
#include <iosfwd>
#include <map>
#include <memory>

namespace InDet{

  
  /// Helper struct for hole search results from the pattern recognition
  struct PatternHoleSearchOutcome{ 
    int nPixelHoles{0}; 
    int nSCTHoles{0}; 
    int nSCTDoubleHoles{0}; 
    int nPixelDeads{0}; 
    int nSCTDeads{0}; 
    bool passPatternHoleCut{true};
  }; 


  class SiTrajectory_xk final
    {
      friend class  SiCombinatorialTrackFinder_xk;

      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////
      
    public:
      
      SiTrajectory_xk();
      SiTrajectory_xk(const SiTrajectory_xk&);
      ~SiTrajectory_xk() = default;
      SiTrajectory_xk& operator  = (const SiTrajectory_xk&);

      ///////////////////////////////////////////////////////////////////
      // Main methods
      ///////////////////////////////////////////////////////////////////

      const int&  nholes        ()       const {return m_nholes        ;}
      const int&  dholes        ()       const {return m_dholes        ;}
      const int&  nHolesBefore  ()       const {return m_nHolesBefore  ;}
      const int&  nHolesAfter   ()       const {return m_nHolesAfter   ;}
      const int&  nclusters     ()       const {return m_nclusters     ;}
      const int&  ndf           ()       const {return m_ndf           ;}
      const int&  nclustersNoAdd()       const {return m_nclustersNoAdd;}
      const int&  nElements     ()       const {return m_nElements     ;}
      const int&  naElements    ()       const {return m_nActiveElements;}
      const int&  difference    ()       const {return m_difference    ;}
      const int&  elementsMap(int& i) const {return m_elementsMap[i];}
      const PatternHoleSearchOutcome&  getHoleSearchResult() const {return m_patternHoleOutcome;}

      void setTools(const InDet::SiTools_xk*); 
      void setParameters(); 

      bool initialize
        (bool,bool,
         const PixelClusterContainer*                         ,
         const SCT_ClusterContainer*                          ,
         const Trk::TrackParameters                          &,
         std::vector<const InDet::SiCluster*>                  &,
         std::vector<const InDet::SiDetElementBoundaryLink_xk*>&,
         bool                                                &,
         const EventContext&);

      double pTseed(const Trk::TrackParameters&,
                    std::vector<const InDet::SiCluster*>&,
                    std::vector<const InDet::SiDetElementBoundaryLink_xk*>&,
                    const EventContext&);

      bool trackParametersToClusters
        (const PixelClusterContainer*                             ,
         const SCT_ClusterContainer*                              ,
         const Trk::TrackParameters                              &,
         std::vector<const InDet::SiDetElementBoundaryLink_xk*>  &,
         std::multimap<const Trk::PrepRawData*,const Trk::Track*>&,
         std::vector<const InDet::SiCluster*>                    &,
         const EventContext& ctx);
      
      bool globalPositionsToClusters
        (const PixelClusterContainer*                             ,
         const SCT_ClusterContainer*                              ,
         const std::vector<Amg::Vector3D>                         &,
         std::vector<const InDet::SiDetElementBoundaryLink_xk*>  &,
         std::multimap<const Trk::PrepRawData*,const Trk::Track*>&,
         std::vector<const InDet::SiCluster*>                      &);

      bool backwardExtension(int, const EventContext&);
      bool forwardExtension (bool,int, const EventContext&);
      bool forwardFilter    (const EventContext&);
      bool filterWithPreciseClustersError(const EventContext&);
      bool backwardSmoother (bool, const EventContext&);
      bool isLastPixel      () const;

      /** @brief Return the pattern track parameters of the first element of this trajectory matching its status
       * @return nullptr or a pointer to the element owned pattern track parameters matching the current status
       *   of the trajectory element.
       */
      const Trk::PatternTrackParameters *firstParameters() const;
      std::unique_ptr<Trk::TrackParameters> firstTrackParameters();
      void getClusters(std::vector<const InDet::SiCluster*>&) const;

      Trk::TrackStates
      convertToTrackStateOnSurface();

      Trk::TrackStates
      convertToTrackStateOnSurface(int);

      Trk::TrackStates
      convertToTrackStateOnSurfaceWithNewDirection();

      Trk::TrackStates
      convertToNextTrackStateOnSurface();

      Trk::TrackStates
      convertToSimpleTrackStateOnSurface(const EventContext& ctx);

      Trk::TrackStates
      convertToSimpleTrackStateOnSurface(int, const EventContext& ctx);

      Trk::TrackStates
      convertToSimpleTrackStateOnSurfaceWithNewDirection();

      Trk::TrackStates
      convertToSimpleTrackStateOnSurfaceForDisTrackTrigger(const EventContext& ctx);

      Trk::TrackStates
      convertToSimpleTrackStateOnSurfaceForDisTrackTrigger(int, const EventContext& ctx);

      std::unique_ptr<Trk::FitQuality> convertToFitQuality() const;

      void updateHoleSearchResult(); 
      
      void sortStep          ();
      bool goodOrder         ();
      bool jumpThroughPerigee();
      double quality         () const;
      double qualityOptimization();
      double pTfirst         () const;
      std::ostream& dump(std::ostream& out) const;

    protected:
      
      ///////////////////////////////////////////////////////////////////
      // Protected Data
      ///////////////////////////////////////////////////////////////////

      int                               m_firstElement{}    ; /// index of the first element where we have 
                                                            /// a cluster 
      int                               m_lastElement{}     ; /// index of the last element where we have 
                                                            /// a cluster
      int                               m_nclusters{}       ; /// Number of clusters on trajectory
      int                               m_nclustersNoAdd{}  ; // (NCL)
      int                               m_difference{}      ; // forward-bacward diff 
      int                               m_nHolesBefore{}         ; // holes before
      int                               m_nHolesAfter{}         ; // holes after
      int                               m_nholes{}          ; // holes
      int                               m_dholes{}          ; // dholes
      int                               m_nActiveElements{}      ; /// count active elements 
      int                               m_nElements{}       ; // index 
      int                               m_elementsMap[300]{}; // index
      int                               m_ndfcut{}          ; //
      int                               m_ndf{}             ; //
      int                               m_ntos{}            ; //
      int                               m_atos[100]{}       ; //
      int                               m_itos[100]{}       ; //
      SiTrajectoryElement_xk            m_elements   [300]{}; /// Trajectory elements on this trajectory. 
                                                            /// Each one corresponds to one detector element on
                                                            /// the search road 
      const InDet::SiTools_xk*          m_tools{}           ; //
      std::unique_ptr<const Trk::Surface> m_surfacedead   ;
      PatternHoleSearchOutcome    m_patternHoleOutcome{}; 

      ///////////////////////////////////////////////////////////////////
      // Methods
      ///////////////////////////////////////////////////////////////////

      void erase(int);
      bool isNewTrack(std::multimap<const Trk::PrepRawData*,const Trk::Track*>&) const;
    };
  
  std::ostream& operator << (std::ostream&,const SiTrajectory_xk&);

} // end of name space

#include "SiSPSeededTrackFinderData/SiTrajectory_xk.icc"

#endif // SiTrajectory_xk
