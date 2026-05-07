/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IPRD_TruthTrajectoryBuilder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRK_TRKTRUTHTRACKS_IPRD_TRUTHTRAJECTORYBUILDER_H
#define TRK_TRKTRUTHTRACKS_IPRD_TRUTHTRAJECTORYBUILDER_H 1

// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include "TrkTruthTrackInterfaces/PRD_TruthTrajectory.h"

#include "AtlasHepMC/GenParticle_fwd.h"

namespace Trk {

  /**
   @class IPRD_TruthTrajectoryBuilder

   @brief The interface for the truth PRD trajectory finder
       
   @author Andreas.Salzburger -at- cern.ch, Thijs.Cornelissen -at- cern.ch, Roland.Wolfgang.Jansky -at- cern.ch 
   */
     
  class IPRD_TruthTrajectoryBuilder : virtual public IAlgTool {

     public:
       /** Interface ID */
       DeclareInterfaceID(IPRD_TruthTrajectoryBuilder, 1, 0);

       /** Virtual destructor */
       virtual ~IPRD_TruthTrajectoryBuilder(){}

       /** return a vector of PrepRawData trajectories */
       virtual std::map< HepMC::ConstGenParticlePtr, PRD_TruthTrajectory > truthTrajectories(const EventContext& ctx) const = 0;
       
  };

} // end of namespace

#endif // TRK_TRKTRUTHTRACKS_IPRD_TRUTHTRAJECTORYBUILDER_H
