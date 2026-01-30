/// emacs: this is c++
/**
 **   @file    TrackCollections.h        
 **                   
 **   @author  sutt
 **   @date    Mon 19 Jan 2026 19:10:52 GMT
 **
 **   $Id: TrackCollections.h, v0.0   Mon 19 Jan 2026 19:10:52 GMT sutt $
 **
 **   Copyright (C) 2026 sutt (sutt@cern.ch)    
 **
 **/


#ifndef INDETTRACKPERFMON_TRACKCOLLECTIONS_H
#define INDETTRACKPERFMON_TRACKCOLLECTIONS_H


/// Athena includes
//#include "AthenaBaseComps/AthMsgStreamMacros.h"
//#include "AthenaBaseComps/AthCheckMacros.h"
//#include "AthenaBaseComps/AthMessaging.h"
//#include "GaudiKernel/ISvcLocator.h"
//#include "GaudiKernel/Service.h"
//#include "StoreGate/ReadHandleKey.h"
//#include "StoreGate/ReadHandle.h"

/// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
// #include "xAODTruth/TruthEventContainer.h"
// #include "xAODTruth/TruthPileupEventContainer.h"
#include "xAODTracking/VertexContainer.h"

/// local includes
// #include "InDetTrackPerfMon/ITrackAnalysisDefinitionSvc.h"
// #include "InDetTrackPerfMon/ITrackMatchingLookup.h"

/// STD includes
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <utility>

template<typename T, typename S=T>
class TrackCollections : public AthMessaging { /// AthMessaging ?????

public:

  /// Constructor 
  TrackCollections( const std::vector< const T* >& test, const std::vector< const T* >&  ref )
    : m_test(test), m_ref(ref) { } 
  
  /// Destructor
  ~TrackCollections() = default;
  
  /// initialize
  StatusCode initialize();
  
  
  std::vector< const T* >& test() const { return m_test; }
  std::vector< const S* >&  ref() const { return m_ref; }
  
  
protected:
  
  /// TrackAnalysis properties
  
  /// --- Collections class variables ---
  /// EventInfo, TruthEvent, && TruthPUEvent
  const xAOD::EventInfo* m_eventInfo{nullptr};
  
  /// vectors of track/truth particles at different stages of the selection/workflow
  std::vector< const T* > m_test;
  std::vector< const S* > m_ref;
  
  /// vectors of reco/truth vertices at different stages of the selection/workflow
  std::vector< const xAOD::Vertex* >   m_testvtx;
  std::vector< const xAOD::Vertex* >   m_refvtx;
  

  /// Lookup table for test-reference matching
  // std::unique_ptr< ITrackMatchingLookup > m_matches;
  
}; // class TrackCollections


#endif // > !INDETTRACKPERFMON_TRACKCOLLECTIONS_H
