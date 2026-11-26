/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FATRASTOOLSG4_G4HADINTPROCESSOR_H
#define ISF_FATRASTOOLSG4_G4HADINTPROCESSOR_H

// GaudiKernel & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

// Fatras
#include "ISF_FatrasInterfaces/IHadronicInteractionProcessor.h"
#include "ISF_Geant4Tools/IG4RunManagerHelper.h"

// Geant4
#include "G4ThreeVector.hh"
#include "G4MaterialCutsCouple.hh"

// Trk
#include "TrkEventPrimitives/ParticleHypothesis.h"

// ISF
#include "ISF_Event/ISFParticleContainer.h"
#include "ISF_Interfaces/IParticleBroker.h"
#include "ISF_Interfaces/ITruthSvc.h"

// Forward Declarations
class G4DynamicParticle;
class G4VProcess;
class G4RunManager;
class G4VUserPhysicsList;
class G4LayerDetectorConstruction;
class G4LayerPrimaryGeneratorAction;
class G4LayerTrackingAction;
class G4Step;
class G4StepPoint;
class G4AtlasRunManager;
class G4Material;

namespace Trk {
  class Material;
  class MaterialProperties;       // TODO: get rid of MatProp dependence
}

namespace iFatras {

  /** @class G4HadIntProcessor

    Wrapper class for multiple scattering, energyloss, hadronic interaction
    tool from Geant4.

    @author Andreas.Salzburger@cern.ch
    */

  class ATLAS_NOT_THREAD_SAFE G4HadIntProcessor : public extends<AthAlgTool, iFatras::IHadronicInteractionProcessor> {  // deprecated: ATLASSIM-6020
    public:      
      /** AlgTool constructor for G4HadIntProcessor*/
      using base_class::base_class;
      /**Destructor*/
      virtual ~G4HadIntProcessor();

      /** AlgTool initailize method.*/
      StatusCode initialize();
      /** AlgTool finalize method */
      StatusCode finalize();

      /** interface for processing of the nuclear interactions */
      bool hadronicInteraction(const Amg::Vector3D& position, const Amg::Vector3D& momentum, 
			       double p, double E, double charge, 
                               const Trk::MaterialProperties& mprop, double pathCorrection,
                               Trk::ParticleHypothesis particle=Trk::pion) const;

      bool doHadronicInteraction(double time, const Amg::Vector3D& position, const Amg::Vector3D& momentum,
                                 const Trk::Material *ematprop,
				 Trk::ParticleHypothesis particle=Trk::pion,
                                 bool  processSecondaries=true) const;

      /** interface for processing of the presampled nuclear interactions on layer*/
      ISF::ISFParticleVector doHadIntOnLayer(const ISF::ISFParticle* parent, double time, 
					     const Amg::Vector3D& position, const Amg::Vector3D& momentum,
					     const Trk::Material *ematprop,
					     Trk::ParticleHypothesis particle=Trk::pion) const ;

    private:
      /** initialize G4RunManager on first call if not done by then */
      StatusCode initG4RunManager ATLAS_NOT_THREAD_SAFE ();

      /** collect secondaries for layer material update */                           
      ISF::ISFParticleVector getHadState(const ISF::ISFParticle* parent,
					 double time, const Amg::Vector3D& position, const Amg::Vector3D& momentum, 
					 const Trk::Material *ematprop) const;

      //!< Initialize inleastic hadronic Geant4 processes 
      std::map<int,G4VProcess*>::const_iterator  initProcessPDG(int pdg);

      //!< choose for list of predefined (pure) materials
      unsigned int retrieveG4MaterialIndex(const Trk::Material* ematprop) const;

      //!< random number service
      ServiceHandle<IAtRndmGenSvc>         m_rndGenSvc{this, "RandomNumberService", "AtDSFMTGenSvc", "Random number generator"};

      PublicToolHandle<ISF::IG4RunManagerHelper> m_g4RunManagerHelper{this, "G4RunManagerHelper", "iGeant4::G4RunManagerHelper/G4RunManagerHelper"};

      //!< steering: enable elastic interactions?
      BooleanProperty                      m_doElastic{this, "DoElasticInteractions", false};

      /* scale factors for hadronic/electromagnetic interactions */
      DoubleProperty                       m_hadIntProbScale{this, "HadronicInteractionScaleFactor", 1.0, "Scale probability of HadrInteractions"};

      // internal steering : clone type
      DoubleProperty                       m_minMomentum{this, "MomentumCut", 50.0};

      //!< Geant4 processes <PDGcode, process>  TODO : fission, capture
      std::map<int, G4VProcess*>   m_g4HadrInelasticProcesses;
      std::map<int, G4VProcess*>   m_g4HadrElasticProcesses;

      std::vector<std::pair<float,std::pair< G4Material*, G4MaterialCutsCouple> > > m_g4Material;

      /** ISF services & Tools */
      ServiceHandle<ISF::IParticleBroker>  m_particleBroker{this, "ParticleBroker", "ISF_ParticleBrokerSvc", "ISF Particle Broker"};
      ServiceHandle<ISF::ITruthSvc>        m_truthRecordSvc{this, "TruthRecordSvc", "ISF_ValidationTruthService", "ISF Particle Truth Svc"};

      /** Random engine  */
      CLHEP::HepRandomEngine*              m_randomEngine{nullptr};
      StringProperty                       m_randomEngineName{this, "RandomStreamName", "FatrasRnd", "Name of the random number stream"};       //!< Name of the random number stream

  };

}

#endif
