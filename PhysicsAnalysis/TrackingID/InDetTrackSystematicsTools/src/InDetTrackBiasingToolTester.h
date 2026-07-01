// -*- c++ -*-
/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKSYSTEMATICSTOOLS_INDETTRACKBIASINGTOOLTESTER_H
#define INDETTRACKSYSTEMATICSTOOLS_INDETTRACKBIASINGTOOLTESTER_H

// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "AsgTools/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
// EDM include(s):
#include "xAODTracking/TrackParticleContainer.h"
// Local include(s):
#include "InDetTrackSystematicsTools/IInDetTrackBiasingTool.h"
#include <TH1.h>


namespace InDet {

   /// Simple algorithm for testing the tools in Athena
   ///
   /// @author Michael (Felix) Clark <michael.ryan.clark@cern.ch>
   ///
   class InDetTrackBiasingToolTester : public AthHistogramAlgorithm {

   public:
      /// Regular Algorithm constructor
      InDetTrackBiasingToolTester( const std::string& name, ISvcLocator* svcLoc );
      /// Function initialising the algorithm
      virtual StatusCode initialize();
      /// Function executing the algorithm
      virtual StatusCode execute(const EventContext& ctx);

   private:
      /// StoreGate key for the track container
      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackKey{this, "TrackIP", "InDetTrackParticles", "Track particle container"};

      /// Connection to the biasing tool
      ToolHandle< IInDetTrackBiasingTool > m_biasTool;

   }; // class InDetTrackBiasingToolTester

} // namespace InDet

#endif
