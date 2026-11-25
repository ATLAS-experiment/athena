/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// HoleSearchValidation.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRK_HOLESEARCHVALIDATION_H
#define TRK_HOLESEARCHVALIDATION_H

// Gaudi includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TrkToolInterfaces/ITrackHoleSearchTool.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "TrkTrack/TrackCollection.h"
#include <array>
#include <mutex>

// forward declarations
class AtlasDetectorID;
class PixelID;
class SCT_ID;
class TRT_ID; 
class SiliconID;

namespace CLHEP {
  class HepRandomEngine; 
}

namespace Trk {
    class TrackStateOnSurface;
    class Track;
}

/** @class HoleSearchValidation

   Test algorithm that can run after the ID reco.
   It takes a track collection, runs the hole search, dumps the holes found, 
   then removes some hits from the track by making an incomplete copy of the track
   (no refit !!!), dumps into the logfile which hits are removed, 
   then runs the hole search again and dumps the result again.

   @author Till.Eifert@cern.ch
*/

namespace InDet {

  class HoleSearchValidation : public AthReentrantAlgorithm {
  public:
    /** Standard Athena-Algorithm Constructor */
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    /** Default Destructor */
    ~HoleSearchValidation();

    /** standard Athena-Algorithm method */
    StatusCode          initialize();
    /** standard Athena-Algorithm method */
    StatusCode          execute(const EventContext& ctx) const;
    /** standard Athena-Algorithm method */
    StatusCode          finalize();

  private:

    void printInfoTSoS( const Trk::TrackStateOnSurface* tsos) const;
    unsigned int doHoleSearch( const Trk::Track* track) const;

    const AtlasDetectorID*                m_idHelper{};
    const PixelID*                        m_pixelID{};
    const SCT_ID*                         m_sctID{}; 
    const TRT_ID*                         m_trtID{}; 
    const SiliconID*                      m_siliconID{}; 

    PublicToolHandle<Trk::ITrackHoleSearchTool>    m_holeSearchTool{this, "HoleSearchTool", "InDet::InDetTrackHoleSearchTool"};
    SG::ReadHandleKey<TrackCollection> m_trackCollectionKey{this, "TrackCollection", "Tracks", "Name of the reference track collection"};     //!< jobOption: name of the TrackCollection
    SG::WriteHandleKey<TrackCollection> m_trackCollectionOutputKey;     //!< jobOption: name of the TrackCollection
    //unsigned int   m_numberHitsToRemove;  //!< jobOption: number of hits to remove from tracks
    //unsigned int   m_numberHitsToKeep;    //!< jobOption: number of hits to remove 
    BooleanProperty m_saveNewTracksInSG{this, "SaveNewTracksInSG", false, "Boolean: save tracks in SG"};   //!< jobOption: save new tracks to SG ?
   

    struct Parts {
      enum EParts {
        kPix0,
        kPix1,
        kPix2,

        kSct0,
        kSct1,
        kSct2,
        kSct3,
        kSct4,
        kSct5,
        kSct6,
        kSct7,
        kSct8,
        kSctSide0,
        kSctSide1,
        kNParts
      };
    };

    // the order must follow Parts::EParts
    std::array<BooleanProperty, Parts::kNParts> m_removeParts{{
      {this, "RemovePIX0", false, "Remove hit(s) in PIXEL layer 0"},
      {this, "RemovePIX1", false, "Remove hit(s) in PIXEL layer 1"},
      {this, "RemovePIX2", false, "Remove hit(s) in PIXEL layer 2"},
      {this, "RemoveSCT0", false, "Remove hit(s) in SCT layer 0"},
      {this, "RemoveSCT1", false, "Remove hit(s) in SCT layer 1"},
      {this, "RemoveSCT2", false, "Remove hit(s) in SCT layer 2"},
      {this, "RemoveSCT3", false, "Remove hit(s) in SCT layer 3"},
      {this, "RemoveSCT4", false, "Remove hit(s) in SCT endcap layer 4"},
      {this, "RemoveSCT5", false, "Remove hit(s) in SCT endcap layer 5"},
      {this, "RemoveSCT6", false, "Remove hit(s) in SCT endcap layer 6"},
      {this, "RemoveSCT7", false, "Remove hit(s) in SCT endcap layer 7"},
      {this, "RemoveSCT8", false, "Remove hit(s) in SCT endcap layer 8"},
      {this, "RemoveSCTSide0", false, "Remove SCT hits on side 0"},
      {this, "RemoveSCTSide1", false, "Remove SCT hits on side 1"}
    }};
    BooleanProperty m_removeOverlapHitsOnly{this, "RemoveOverlapHitsOnly", false, "Only remove overlap track hits"};
    BooleanProperty m_ignoreTrackEnds{this, "IgnoreTrackEnds", true, "Ignore hits at the end of the track"};

    BooleanProperty m_randomRemovalMode{this, "RandomRemovalMode", false, "Randomly remove track hits (overwrites the other flags!)"};
    UnsignedIntegerProperty m_maxNumberOfHoles{this, "MaximalHoles", 3, "Number of maximal holes to be created"};

    ServiceHandle<IAthRNGSvc>    m_rndmGenSvc{"AthRNGSvc", name()};       //!< Random number generator
    std::string                  m_randomEngineName{"HoleSearchRnd"}; //!< Name of the random number stream

    // some information for statistics -> printout in finalize
    mutable std::mutex                                  m_trackStatsMutex;
    mutable std::vector< std::vector< unsigned int > >  m_trackStats ATLAS_THREAD_SAFE; //!< # tracks as function of [#generated holes, #found holes]. Guarded by m_trackStatsMutex

  }; // end of class

} // close of namespace

#endif // TRK_HOLESEARCHVALIDATION_H

