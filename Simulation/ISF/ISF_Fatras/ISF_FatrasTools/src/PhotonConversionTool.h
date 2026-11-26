/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FATRASTOOLS_PHOTONCONVERSIONTOOL_H
#define ISF_FATRASTOOLS_PHOTONCONVERSIONTOOL_H

// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "CxxUtils/checker_macros.h"
// Trk
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkExUtils/MaterialUpdateMode.h"
#include "TrkDetDescrUtils/GeometrySignature.h"
// Fatras
#include "ISF_FatrasInterfaces/IPhotonConversionTool.h"
#include "ISF_FatrasInterfaces/IPhysicsValidationTool.h"
#include "ISF_Interfaces/IParticleBroker.h"
#include "ISF_Interfaces/ITruthSvc.h"

// CLHEP
#include "CLHEP/Units/SystemOfUnits.h"

class TTree;

class StoreGateSvc;

namespace Trk {
  class Layer;
  class CylinderVolumeBounds;
  class PdgToParticleHypothesis;
  class TrackingGeometry;
}

namespace iFatras {

  /** @class PhotonConversionTool
      
     The photon conversion tool, to be called by the MaterialUpdator.
  
     @author Sarka.Todorova@cern.ch
  */
   
  class ATLAS_NOT_THREAD_SAFE PhotonConversionTool : public extends<AthAlgTool, iFatras::IPhotonConversionTool> {  // deprecated: ATLASSIM-6020
  public:      
      /**AlgTool constructor for PhotonConversionTool*/
      using base_class::base_class;
      /**Destructor*/
      virtual ~PhotonConversionTool();
       
      /** AlgTool initailize method.*/
      StatusCode initialize();
      /** AlgTool finalize method */
      StatusCode finalize();

    
      /** interface for processing of the pair production */
      bool pairProduction(const Trk::MaterialProperties& mprop,
			  double pathCorrection,
			  double p) const;
      
      /** interface for processing of the presampled pair production */
      bool doConversion(double time, const Trk::NeutralParameters& parm,
			const Trk::ExtendedMaterialProperties* extMatProp=0) const;
      
      /** interface for processing of the presampled nuclear interactions on layer*/
      ISF::ISFParticleVector doConversionOnLayer(const ISF::ISFParticle* parent, 
						 double time, const Trk::NeutralParameters& parm,
						 const Trk::ExtendedMaterialProperties *ematprop=0) const;


    
   private:
      /** record childs - create interface already for mu-/mu+ pair production*/
      void recordChilds(double time, 
			const Amg::Vector3D& vertex,
			const Amg::Vector3D& photonDirection,
			double childEnergy, double photonEnergy,
			const Amg::Vector3D& childDirection,
			Trk::ParticleHypothesis childType=Trk::electron) const;
      
      /** get childs - for layer update */
      ISF::ISFParticleVector getChilds(const ISF::ISFParticle* parent,
				       double time,
				       const Amg::Vector3D& vertex,
				       const Amg::Vector3D& photonDirection,
				       double childEnergy, double photonEnergy,
				       const Amg::Vector3D& childDirection,
				       Trk::ParticleHypothesis childType) const;
	
      /** simulate the child energy */
      double childEnergyFraction(const Trk::MaterialProperties& mprop, double gammaMom) const;
      double childEnergyFraction(double gammaMom) const;
      
      /** simulate the one child direction - the second one is given clear then*/
      Amg::Vector3D childDirection(const Amg::Vector3D& gammaMom,
					  double childE) const;
      
      /** helper functions for the Phi1/phi2 */
      double phi1(double delta) const;
      
      /** helper functions for the Phi1/phi2 */
      double phi2(double delta) const;

      /** ISF services & Tools */
      ServiceHandle<ISF::IParticleBroker>          m_particleBroker{this, "ParticleBroker", "ISF_ParticleParticleBroker", "ISF ParticleBroker Svc"};
      ServiceHandle<ISF::ITruthSvc>                m_truthRecordSvc{this, "TruthRecordSvc", "ISF_TruthRecordSvc", "ISF Particle Truth Svc"};
      
      /** MCTruth process code for TruthIncidents created by this tool */
      IntegerProperty      m_processCode{this, "PhysicsProcessCode", 14, "MCTruth Physics Process Code"};
      
      /** Switch to use reference material */
      BooleanProperty                              m_referenceMaterial{this, "ReferenceMaterial", true};
      
      /** The cut from which on the child products are followed */
      DoubleProperty                               m_minChildEnergy{this, "MinimumChildEnergy", 50.*CLHEP::MeV};
      // NB: ChildEnergyScaling=1 corresponds to energy sharing formulas of GEANT4
      DoubleProperty                               m_childEnergyScaleFactor{this, "ChildEnergyScaling", 1.};
      DoubleProperty                               m_conversionProbScaleFactor{this, "ConversionProbScaling", 0.98};
      
      
      /** Random Generator service  */
      ServiceHandle<IAtRndmGenSvc>                 m_rndGenSvc{this, "RandomNumberService", "AtDSFMTGenSvc", "Random number generator"};
      CLHEP::HepRandomEngine*                      m_randomEngine{nullptr};
      StringProperty                               m_randomEngineName{this, "RandomStreamName", "FatrasRnd", "Name of the random number stream"}; //!< Name of the random number stream
      
      /** debug output - count the conversions */
      mutable int                                  m_recordedConversions{0};
      mutable int                                  m_droppedConversions{0};
      
      // ------------------------ Validation section ------------------------------------
      BooleanProperty                              m_validationMode{this, "ValidationMode", false};
      std::string                                  m_validationTreeName{"FatrasPhotonConversions"};        //!< validation tree name - to be acessed by this from root
      std::string                                  m_validationTreeDescription{"Validation output from the PhotonConversionTool"}; //!< validation tree description - second argument in TTree
      std::string                                  m_validationTreeFolder{"/val/FatrasPhotonConversions"};      //!< stream/folder to for the TTree to be written out
      
      TTree*                                       m_validationTree{nullptr};            //!< Root Validation Tree
      PublicToolHandle<IPhysicsValidationTool>     m_validationTool{this, "PhysicsValidationTool", ""};
      
      // ------ ntuple branches
      mutable float                                m_conversionPointX{};
      mutable float                                m_conversionPointY{};
      mutable float                                m_conversionPointR{};
      mutable float                                m_conversionPointZ{};
      mutable float                                m_conversionPhotonEnergy{};
      mutable float                                m_conversionChildEnergy{};
      mutable float                                m_conversionChildAngle{};
      // --------------------------------------------------------------------------------
      
      static const Trk::PdgToParticleHypothesis          s_pdgToHypo;
   };
  

inline double PhotonConversionTool::phi1(double delta) const {
  if (delta <= 1.)
     return 20.867 - 3.242 * delta  + 0.625*delta*delta;
  else
    return 21.12 - 4.184*log(delta+0.952);
}

inline double PhotonConversionTool::phi2(double delta) const {
  if (delta <= 1.)
     return 20.209 - 1.930 * delta  + 0.086*delta*delta;
   return 21.12 - 4.184*log(delta+0.952);
}

}

#endif
