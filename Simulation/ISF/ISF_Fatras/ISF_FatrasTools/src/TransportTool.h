/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TransportTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_FATRASTOOLS_TRANSPORTTOOL_H
#define ISF_FATRASTOOLS_TRANSPORTTOOL_H

// Athena Base
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "CxxUtils/checker_macros.h"
#include "TrkExInterfaces/ITimedExtrapolator.h"
#include "ISF_FatrasInterfaces/IParticleDecayHelper.h"
#include "ISF_FatrasInterfaces/IPhysicsValidationTool.h"
#include "ISF_FatrasInterfaces/IProcessSamplingTool.h"
#include "ISF_FatrasInterfaces/ISimHitCreator.h"
#include "ISF_Interfaces/IParticleFilter.h"
#include "ISF_Interfaces/IParticleHelper.h"

// IFS
#include "ISF_Interfaces/IParticleProcessor.h"

// Tracking
#include "TrkEventPrimitives/PdgToParticleHypothesis.h"
#include "TrkParameters/TrackParameters.h"
// ROOT forward declarations
class TTree;

//class AtlasDetectorID;

namespace ISF {
    class ISFParticle;
}

namespace iFatras 
{
 
    
  /** @class TransportTool 
      
      Fatras AlgTool to create a ISFParticle at a volume entry/exit
      - universal transport tool
      
      @author Sharka Todorova Sarka.Todorova -at- cern.ch
      @author Andreas.Salzburger -at- cern.ch
  */  
  
  class ATLAS_NOT_THREAD_SAFE TransportTool : public extends<AthAlgTool, ISF::IParticleProcessor>  // deprecated: ATLASSIM-6020
  {
  public:
    /** Constructor */
    using base_class::base_class;
    
    /** Destructor */
    virtual ~TransportTool ();
    
    /** AlgTool initialize method */
    virtual StatusCode initialize() override;
    
    /** AlgTool finalize method */

    virtual StatusCode finalize() override;
     
    /** Creates a new ParticleState from a given ParticleState, universal transport tool */
    virtual
    ISF::ISFParticle* process(const ISF::ISFParticle& isp, CLHEP::HepRandomEngine*) const override;
    
  private:
     /** templated Tool retrieval - gives unique handling & look and feel */
     template <class T> StatusCode retrieveTool(ToolHandle<T>& thandle){
       if (!thandle.empty() && thandle.retrieve().isFailure()){
         ATH_MSG_FATAL( "[ fatras setup ] Cannot retrieve " << thandle << ". Abort.");
         return StatusCode::FAILURE;
       }
       else {
         ATH_MSG_DEBUG("[ fatras setup ] Successfully retrieved " << thandle);
       }
       return StatusCode::SUCCESS;
     }
      
    /*---------------------------------------------------------------------
     *  Private members
     *---------------------------------------------------------------------*/

     /** Random Generator service  */
     ServiceHandle<IAtRndmGenSvc>                 m_rndGenSvc{this, "RandomNumberService", "AtDSFMTGenSvc"};
     CLHEP::HepRandomEngine*                      m_randomEngine{nullptr};
     std::string                                  m_randomEngineName{"FatrasRnd"};         //!< Name of the random number stream


    /** Validation output with histogram service */
    BooleanProperty                                              m_validationOutput{this, "ValidationOutput", false}; //!< turn validation mode on/off
    PublicToolHandle<IPhysicsValidationTool>                     m_validationTool{this, "PhysicsValidationTool", ""};   //!< the ntuple

    /** Particle Decay */
    PublicToolHandle<IParticleDecayHelper>    m_particleDecayHelper{this, "ParticleDecayHelper", ""};

    /** The SimHit Creation */
    PublicToolHandle<ISimHitCreator>          m_simHitCreatorID{this, "SimHitCreatorID", ""};
    PublicToolHandle<ISimHitCreator>          m_simHitCreatorMS{this, "SimHitCreatorMS", ""};

    /** The Extrapolator setup */
    PublicToolHandle<Trk::ITimedExtrapolator>      m_extrapolator{this, "Extrapolator", ""};          
    
    /** Filtering setup & other ISF stuff*/
    PublicToolHandle<ISF::IParticleFilter>    m_trackFilter{this, "TrackFilter", ""};
    PublicToolHandle<ISF::IParticleFilter>    m_neutralHadronFilter{this, "NeutralFilter", ""};
    PublicToolHandle<ISF::IParticleFilter>    m_photonFilter{this, "PhotonFilter", ""};
    PublicToolHandle<ISF::IParticleHelper>    m_iparticleHelper{this, "ParticleHelper", ""};
    PublicToolHandle<IProcessSamplingTool>    m_samplingTool{this, "ProcessSamplingTool", ""};

    //const AtlasDetectorID*              m_idHelper;     //*> hit type identification   
    Trk::PdgToParticleHypothesis        m_pdgToParticleHypothesis;

    BooleanProperty                     m_errorPropagation{this, "DoErrorPropagation", false};    // error propagation for eloss validation
    BooleanProperty                     m_hitsOff{this, "TurnOffHitCreation", false};             // steering of sim hit production

  }; 
}

#endif // FATRASTOOLS_TRANSPORTTOOL_H
