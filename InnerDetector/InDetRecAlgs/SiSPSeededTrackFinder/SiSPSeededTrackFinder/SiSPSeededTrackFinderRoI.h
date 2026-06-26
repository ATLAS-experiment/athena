// -*- C++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef SiSPSeededTrackFinderRoI_H
#define SiSPSeededTrackFinderRoI_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/DataHandle.h"

#include "InDetRecToolInterfaces/ISiSpacePointsSeedMaker.h"
#include "InDetRecToolInterfaces/ISiZvertexMaker.h" 
#include "InDetRecToolInterfaces/ISiTrackMaker.h"
#include "InDetRecToolInterfaces/IZWindowRoISeedTool.h"
#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "InDetRecToolInterfaces/IInDetEtaDependentCutsSvc.h"

#include "TrkSpacePoint/SpacePointContainer.h" 
#include "TrkTrack/TrackCollection.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "TrkExInterfaces/IPatternParametersPropagator.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkSurfaces/PerigeeSurface.h" 
#include "TrkEventUtils/PRDtoTrackMap.h"

#include "xAODTracking/VertexContainer.h"

#include <string>
#include <atomic>

namespace InDet {

  // Class-algorithm for track finding in Pixels and SCT
  // initiated by space points seeds filtering in a given
  // RoI within the z axis
  //
  class SiSPSeededTrackFinderRoI : public AthReentrantAlgorithm 
  {
    
    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
      
  public:
      
    ///////////////////////////////////////////////////////////////////
    // @name Standard Algotithm methods
    ///////////////////////////////////////////////////////////////////
    //@{
    SiSPSeededTrackFinderRoI(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~SiSPSeededTrackFinderRoI() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;    
    //@}


  protected:

    ///////////////////////////////////////////////////////////////////
    // @name Flags to configure SiSPSeededTrackFinderRoI
    ///////////////////////////////////////////////////////////////////     
    //@{

    FloatProperty m_RoIWidth{this, "RoIWidth", 1.0, "Fixed width of RoI along z-axis"}; 
    IntegerProperty m_maxNumberSeeds{this, "maxNumberSeeds", 3000000, "Max. number used seeds"};
    IntegerProperty m_maxPIXsp{this, "maxNumberPIXsp", 150000, "Max. number pixels space points"};
    IntegerProperty m_maxSCTsp{this, "maxNumberSCTsp", 500000, "Max. number sct    space points"};
    IntegerProperty m_nfreeCut{this, "FreeClustersCut", 1, "Min number free clusters"};
    BooleanProperty m_doRandomSpot{this, "doRandomSpot", false, "Low-pT tracking setting the RoI in a random position"};

    StringProperty m_fieldmode{this, "MagneticFieldMode", "MapSolenoid"};
    //@}

    ///////////////////////////////////////////////////////////////////
    // @name Input/Output Handles
    ///////////////////////////////////////////////////////////////////     
    //@{
    SG::WriteHandleKey<TrackCollection> m_outputTracksKey{this, "TracksLocation", "SiSPSeededTracksRoI", "Output track collection"};
    SG::WriteHandleKey<xAOD::VertexContainer> m_vxOutputKey{this, "VxOutputName", "LowPtRoIVertices", "Output Vertex collection with per-event RoI information"};

    SG::ReadHandleKey<SpacePointContainer> m_SpacePointsPixelKey{this, "SpacePointsPixelName", "PixelSpacePoints"};
    SG::ReadHandleKey<SpacePointContainer> m_SpacePointsSCTKey{this, "SpacePointsSCTName", "SCT_SpacePoints"};
    SG::ReadHandleKey<Trk::PRDtoTrackMap> m_prdToTrackMap{this,"PRDtoTrackMap",""};

    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};
    //@}

    ///////////////////////////////////////////////////////////////////
    // @name Algorithm Tools
    ///////////////////////////////////////////////////////////////////     
    //@{

    ToolHandle<ISiSpacePointsSeedMaker> m_seedsmaker{this, "SeedsTool", "InDet::SiSpacePointsSeedMaker_ATLxk/InDetSpSeedsMaker", "Space poins seed maker"};
    ToolHandle<ISiTrackMaker> m_trackmaker{this, "TrackTool", "InDet::SiTrackMaker_xk/InDetSiTrackMaker", "Track maker"};
    ToolHandle<Trk::IExtendedTrackSummaryTool> m_trackSummaryTool{this, "TrackSummaryTool", "InDetTrackSummaryToolNoHoleSearch"};

    ToolHandle<IZWindowRoISeedTool> m_ZWindowRoISeedTool{this, "ZWindowRoISeedTool", "InDet::ZWindowRoISeedTool", "Tool to determin per-event RoI"};
    ToolHandle<IZWindowRoISeedTool> m_RandomRoISeedTool{this, "RandomRoISeedTool", "InDet::RandomRoISeedTool", "Tool to run using additional RoIs randomly chosen away from the main one if doRandomSpot is set"};

    ServiceHandle<IInDetEtaDependentCutsSvc> m_etaDependentCutsSvc{this, "EtaDependentCutsSvc", "", "if enable, further require eta-dependent selections on track candidates"};
    //@}

    ///////////////////////////////////////////////////////////////////
    // Protected settings
    ///////////////////////////////////////////////////////////////////    
    Trk::MagneticFieldProperties  m_fieldprop;


    ///////////////////////////////////////////////////////////////////
    // Protected methods
    ///////////////////////////////////////////////////////////////////
    
    /** \brief assign a quality score to track candidates. 
    * 
    * The score is increased for each hit, depending on the 
    * technology (pix/sct) and the chi2 of the hit. 
    * A hit will never *reduce* the total score compared to having no hit at all. 
    * @param [in] track Track to evaluate the quality of 
    **/   
    double trackQuality(const Trk::Track*) const;

    /** \brief cleans up the collection of quality filtered tracks. 
    * 
    * Candidates which share most of their hits (steered by m_freeCut) 
    * with higher quality candidates are erased from the multimap 
    * @param [in,out] scoredTracks: Track candidates, sorted by by score, best scored first (implemented by assigning negative sign to scores)
    **/     
    void filterSharedTracks(std::multimap<double,Trk::Track*>&) const;

    void magneticFieldInit();

    /** \brief apply eta-dependent selections
     *
     * Candidate tracks can be further selected using eta-dependent selections.
     */
    bool passEtaDepCuts(const Trk::Track* track,
			int nClusters,
			int nFreeClusters,
			int nPixels) const;

    ///////////////////////////////////////////////////////////////////
    // @name Statistics and Debug Information
    ///////////////////////////////////////////////////////////////////
    //@{

    /// enums for Counter_t
    enum ECounter {kNSeeds, kNTracks, kNCounter};

    /// @class Counter_t
    /// For counters
    class Counter_t : public std::array<std::atomic_int, kNCounter> 
    {
    public:
      Counter_t& operator += (const Counter_t& counter) {
        for (unsigned int idx=0; idx <kNCounter; ++idx) {
          (*this)[idx] += counter[idx];
        }
        return *this;
      }
    };

    mutable Counter_t m_counterTotal ATLAS_THREAD_SAFE {};

    mutable std::atomic_int m_neventsTotal{0}; ///< Number events
    mutable std::atomic_int m_problemsTotal{0}; ///< Number events with number seeds > maxNumber

    MsgStream& dump(MSG::Level lvl, const SiSPSeededTrackFinderRoI::Counter_t*) const;
    MsgStream& dumptools(MsgStream& out) const;
    MsgStream& dumpevent(MsgStream& out, const SiSPSeededTrackFinderRoI::Counter_t& counter) const;

    //@}

  };

}

#endif // SiSPSeededTrackFinderRoI_H
