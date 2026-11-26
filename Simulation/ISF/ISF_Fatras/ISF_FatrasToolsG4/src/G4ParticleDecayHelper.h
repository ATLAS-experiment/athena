/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FATRASTOOLSG4_G4PARTICLEDECAYHELPER_H
#define ISF_FATRASTOOLSG4_G4PARTICLEDECAYHELPER_H

// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "CxxUtils/checker_macros.h"
// ISF
#include "ISF_Event/ISFParticleContainer.h"
// iFatras
#include "ISF_FatrasInterfaces/IParticleDecayHelper.h"
#include "ISF_Interfaces/IParticleBroker.h"
#include "ISF_Interfaces/ITruthSvc.h"
#include "ISF_FatrasInterfaces/IPhysicsValidationTool.h"
#include "PDGToG4Particle.h"
#include "ISF_Geant4Tools/IG4RunManagerHelper.h"
// STD
#include <vector>
#include <math.h>

class StoreGateSvc;

namespace ISF {
    class ISFParticle;
}

namespace iFatras {

   class ITrackCreator;

 /** @class G4ParticleDecayHelper

     This Tool is a helper tool wrapping the IParticleDecayCreator,
     and organised the flight distance to 
  
      @author Andreas.Salzburger@cern.ch, Joerg.Mechnich@cern.ch
   */

  class ATLAS_NOT_THREAD_SAFE G4ParticleDecayHelper : public extends<AthAlgTool, IParticleDecayHelper> {  // deprecated: ATLASSIM-6020
    public:
      /**AlgTool constructor for ParticleDecayHelper*/
      using base_class::base_class;
      /**Destructor*/
      ~G4ParticleDecayHelper();
       
      /** AlgTool initailize method.*/
      StatusCode initialize();
      /** AlgTool finalize method */
      StatusCode finalize();
      
      /** free path estimator (-1 for stable particle) */
      double freePath(const ISF::ISFParticle& isp) const;

      /** decay handling secondaries */
      void decay(const ISF::ISFParticle& isp,
		 const Amg::Vector3D& vertex,
		 const Amg::Vector3D& mom,
		 double timeStamp = 0) const;

      /** decay */
      std::vector<ISF::ISFParticle*> decayParticle(const ISF::ISFParticle& parent,
                                                 const Amg::Vector3D& vertex,
                                                 const Amg::Vector3D& mom,
                                                 double timeStamp = 0) const;


   private:
      /** initialize G4RunManager on first call if not done by then */
      bool initG4RunManager() const;
                                  
      /** fill decay products: into broker svc, truth svc */
      void handleDecayParticles(const ISF::ISFParticle& isp, 
                                const ISF::ISFParticleVector& children) const;                             

      /*------------------------------------------------------------
       *  ServiceHandles
       *------------------------------------------------------------*/
      ServiceHandle<ISF::IParticleBroker> m_particleBroker{this, "ParticleBroker", "ISF_ParticleBroker", "ISF Particle Broker Svc"};        //!< Broker Svc for ISF particles
      ServiceHandle<ISF::ITruthSvc>       m_truthRecordSvc{this, "ParticleTruthSvc", "ISF_TruthRecordSvc", "ISF Particle Truth Svc"};        //!< Truth Svc for truth tree


      ServiceHandle<IAtRndmGenSvc>        m_rndmSvc{this, "RandomNumberService", "AtDSFMTGenSvc", "Random number generator"};               //!< Random Svc
      CLHEP::HepRandomEngine*             m_randomEngine{nullptr};          //!< Random engine (updated to streams)
      StringProperty                      m_randomEngineName{this, "RandomStreamName", "FatrasRnd", "Name of the random number stream"};      //!< Name of the random number stream
      StringProperty                      m_G4RandomEngineName{this, "G4RandomStreamName", "FatrasG4", "Name of the random number stream for G4 tools"};    //!< Name of the random number stream for G4 tools

      /*------------------------------------------------------------
       *  ToolHandles
       *------------------------------------------------------------*/
      PublicToolHandle<ISF::IG4RunManagerHelper> m_g4RunManagerHelper{this, "G4RunManagerHelper", "iGeant4::G4RunManagerHelper/G4RunManagerHelper"};   //!< G4RunManager needs to be initialized before G4 tables are accessed
      PublicToolHandle<PDGToG4Particle>   m_pdgToG4Conv{this, "PDGToG4ParticleConverter", "iFatras::PDGToG4Particle/PDGToG4ParticleConverter"};           //!< Handle for the  PDGToG4Particle converter tool

      /** Validation output with histogram service */
      BooleanProperty                                              m_validationMode{this, "ValidationMode", false}; //!< turn validation mode on/off
      PublicToolHandle<IPhysicsValidationTool>                     m_validationTool{this, "PhysicsValidationTool", ""};   //!< the ntuple
      
   };
                    
}                   

#endif
