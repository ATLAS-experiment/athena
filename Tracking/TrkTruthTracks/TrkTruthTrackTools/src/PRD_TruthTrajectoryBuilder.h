/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PRD_TruthTrajectoryBuilder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRK_TRUTHTRACKTOOLS_PRD_TRUTHTRAJECTORYBUILDER_H
#define TRK_TRUTHTRACKTOOLS_PRD_TRUTHTRAJECTORYBUILDER_H 1

// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
// Trk includes
#include "TrkTruthTrackInterfaces/IPRD_TruthTrajectoryBuilder.h"
#include "TrkTruthTrackInterfaces/PRD_TruthTrajectory.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "TrkTruthTrackInterfaces/IPRD_TruthTrajectoryManipulator.h"
#include "TrkToolInterfaces/IPRD_Provider.h"

class AtlasDetectorID;

#include "AtlasHepMC/GenParticle_fwd.h"
  
namespace Trk {
    
  class PrepRawData;
    
  /**
   @class IPRD_TruthTrajectoryBuilder

   @brief The truth PRD trajectory builder, it distinguishes between ID and MS and calls the sub-helpers.

   The truth PRD trajectory builder distinguishes between ID and MS and calls the sub-helpers. This is done
   because it should also run for upgrade layouts and thus not know about dedicated detector technologies.

   A dedicated PRD_TruthTrajectoryManipulator can be used to shape the TruthTrajectories after beeing created.
       
   @author Andreas.Salzburger -at- cern.ch, Thijs.Cornelissen -at- cern.ch
   */
     
  class PRD_TruthTrajectoryBuilder : public AthAlgTool, virtual public IPRD_TruthTrajectoryBuilder {

     public:     
        //** Constructor with parameters */
       PRD_TruthTrajectoryBuilder( const std::string& t, const std::string& n, const IInterface* p );
 
       // Athena algtool's Hooks
       virtual StatusCode  initialize() override;

       /** return a vector of PrepRawData trajectories - uses internal cache**/
       virtual std::map< HepMC::ConstGenParticlePtr, PRD_TruthTrajectory > truthTrajectories(const EventContext& ctx) const override;

     private:
       const AtlasDetectorID*                               m_idHelper;                         //! Helper to detect type of sub-detector from PRD->identify().
                                                                                                
        ToolHandle<IPRD_Provider>                           m_idPrdProvider{this, "InDetPRD_Provider", ""};                    //!< Identifier to PRD relation in the Inner Detector
        ToolHandle<IPRD_Provider>                           m_msPrdProvider{this, "MuonPRD_Provider", ""};                    //!< Identifier to PRD relation in the Muons System
        
        ToolHandleArray<IPRD_TruthTrajectoryManipulator>    m_prdTruthTrajectoryManipulators{this, "PRD_TruthTrajectoryManipulators", {}};   //!< PRD truth tracjectory manipulators
        
      	SG::ReadHandleKeyArray<PRD_MultiTruthCollection>    m_prdMultiTruthCollectionNames{this,"PRD_MultiTruthCollections",{"PixelClusterTruth","SCT_ClusterTruth","TRT_DriftCircleTruth"}, "PRD multi truth collection names this builder is working on"};

      	Gaudi::Property<double>                             m_minPt{this,"MinimumPt",400.,"minimum pT to be even considered"};
      	Gaudi::Property<bool>                               m_geantinos{this,"Geantinos",false,"Track geantinos or not"};
        
  };

} // end of namespace

#endif // TRK_TRUTHTRACKTOOLS_PRD_TRUTHTRAJECTORYBUILDER_H
