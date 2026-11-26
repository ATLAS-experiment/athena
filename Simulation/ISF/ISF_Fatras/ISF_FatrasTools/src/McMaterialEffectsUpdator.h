/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FATRASTOOLS_TRACKONLAYERUPDATOR_H
#define ISF_FATRASTOOLS_TRACKONLAYERUPDATOR_H

// GaudiKernel & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "CxxUtils/checker_macros.h"

// Trk
#include "TrkDetDescrInterfaces/ITrackingGeometrySvc.h"
#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkDetDescrUtils/LayerIndexSampleMap.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkEventPrimitives/PdgToParticleHypothesis.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkExInterfaces/ITimedMatEffUpdator.h"
#include "TrkExUtils/MaterialUpdateMode.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkParameters/TrackParameters.h"

// ISF
#include "ISF_Event/ISFParticleContainer.h"
#include "ISF_Interfaces/IParticleBroker.h"
#include "ISF_Interfaces/ITruthSvc.h"
#include "ISF_FatrasInterfaces/IHadronicInteractionProcessor.h"
#include "ISF_FatrasInterfaces/IParticleDecayHelper.h"
#include "ISF_FatrasInterfaces/IPhotonConversionTool.h"
#include "ISF_FatrasInterfaces/IPhysicsValidationTool.h"
#include "ISF_FatrasInterfaces/IProcessSamplingTool.h"

// Trk
#include "TrkExInterfaces/IEnergyLossUpdator.h"
#include "TrkExInterfaces/IMultipleScatteringUpdator.h"

// CLHEP
#include "CLHEP/Units/SystemOfUnits.h"

#include <cmath>

#ifndef MAXHADINTCHILDREN
#define MAXHADINTCHILDREN 20
#endif

class StoreGateSvc;

class TTree;

namespace Trk {
class EnergyLoss;
class CylinderVolumeBounds;
class TrackingGeometry;
}

namespace ISF {
class ISFParticle;
}

namespace iFatras {

/** Exception to be thrown when TrackingGeometry not found */
class McMaterialEffectsUpdatorException : public std::exception
{
  const char* what() const throw()
  {
    return "Problem with TrackingGeometry loading";
  }
};

/** @class McMaterialEffectsUpdator

    Updator for a track on a Trk::Layer, it extends the IMaterialEffecsUpdator
   to be used inside the Extrapolator with an update based on a Random number.

    The McMaterialEffectsUpdator uses both an extended EnergyLossUpdator
    and the standard ATLAS MultipleScatteringUpdator configured for the
   Gaussian-Mixture-Model.

    @author Andreas.Salzburger@cern.ch, Carsten.Magass@cern.ch
 */

class ATLAS_NOT_THREAD_SAFE McMaterialEffectsUpdator  // deprecated: ATLASSIM-6020
  : public extends<AthAlgTool, Trk::ITimedMatEffUpdator>
{
public:
  /**AlgTool constructor for McMaterialEffectsUpdator*/
  using base_class::base_class;
  /**Destructor*/
  virtual ~McMaterialEffectsUpdator();

  /** AlgTool initailize method.*/
  StatusCode initialize();
  /** AlgTool finalize method */
  StatusCode finalize();

  /** Updator interface (full update for a layer):
     A unique_ptr to Trk::TrackParameters is returned
     it could be a clone of the input if now update
     has happened. The imput is handled by the client.
    */
  std::unique_ptr<Trk::TrackParameters> update(
    const Trk::TrackParameters* parm,
    const Trk::Layer& sf,
    Trk::TimeLimit& time,
    Trk::PathLimit& path,
    Trk::GeometrySignature geoID,
    Trk::PropDirection dir = Trk::alongMomentum,
    Trk::ParticleHypothesis particle = Trk::pion) const;

  std::unique_ptr<Trk::TrackParameters> update(
    double time,
    const Trk::TrackParameters* parm,
    const Trk::MaterialEffectsOnTrack& meff,
    Trk::ParticleHypothesis particle = Trk::pion,
    Trk::MaterialUpdateMode matupmode = Trk::addNoise) const;

  /** Updator interface:
    The parmeters are given as a reference,
    MaterialProperties based material update
    */
  std::unique_ptr<Trk::TrackParameters> update(
    double time,
    const Trk::TrackParameters& parm,
    const Trk::MaterialProperties& mprop,
    double pathcorrection,
    Trk::PropDirection dir = Trk::alongMomentum,
    Trk::ParticleHypothesis particle = Trk::pion,
    Trk::MaterialUpdateMode matupmode = Trk::addNoise) const;

  std::unique_ptr<Trk::TrackParameters> interact(
    double time,
    const Amg::Vector3D& position,
    const Amg::Vector3D& momentum,
    Trk::ParticleHypothesis particle,
    int process,
    const Trk::Material* extMatProp = 0) const;

  /** the helper function for a brem photon record */
  void recordBremPhoton(double time,
                        double pElectron,
                        double gammaE,
                        const Amg::Vector3D& vertex,
                        Amg::Vector3D& particleDir,
                        Trk::ParticleHypothesis particle) const;

private:
  std::unique_ptr<Trk::TrackParameters> updateInLay(
    const ISF::ISFParticle* isp,
    const Trk::TrackParameters* parm,
    double& matFraction,
    Trk::TimeLimit& time,
    Trk::PathLimit& path,
    Trk::PropDirection dir = Trk::alongMomentum,
    Trk::ParticleHypothesis particle = Trk::pion) const;

  ISF::ISFParticleVector interactLay(
    const ISF::ISFParticle* isp,
    double time,
    const Trk::TrackParameters& parm,
    Trk::ParticleHypothesis particle,
    int process,
    const Trk::MaterialProperties* extMatProp = 0) const;

  void radiate(const ISF::ISFParticle* parent,
               AmgVector(5) & updatedParameters,
               const Amg::Vector3D& pos,
               Amg::Vector3D& dir,
               Trk::TimeLimit time,
               double dX0,
               double& matFraction,
               double matTot,
               Trk::PropDirection pdir,
               Trk::ParticleHypothesis particle) const;

  void ionize(const Trk::TrackParameters& parm,
              AmgVector(5) & updatedPar,
              double dInX0,
              Trk::PropDirection pdir,
              Trk::ParticleHypothesis particle) const;

  /** handle the Energy loss */
  // bool handleEnergyLoss(double time,
  //			  const Trk::TrackParameters& parm,
  //                       Amg::Vector3D*       updatedParameter,
  //                       Trk::EnergyLoss* sampldEnergyLoss,
  //                       const Trk::Layer& lay,
  //                       double p, double E, double m,
  //                       Trk::ParticleHypothesis particle=Trk::pion) const;

  /** the private multiple scattering sigma calculation*/
  double msSigma(double dInX0,
                 double p,
                 Trk::ParticleHypothesis particle) const;

  /** the private multiple Scattering update method, thetaMs is the projected
   * random number*/
  void multipleScatteringUpdate(const Trk::TrackParameters& parm,
                                AmgVector(5) & parameters,
                                double sigmaMSproj) const;

  /** the helper function for a brem photon record */
  void recordBremPhotonLay(const ISF::ISFParticle* parent,
                           Trk::TimeLimit time,
                           double pElectron,
                           double gammaE,
                           const Amg::Vector3D& vertex,
                           Amg::Vector3D& particleDir,
                           double matFraction,
                           Trk::PropDirection dir,
                           Trk::ParticleHypothesis particle) const;

  //!< retrieve or provide the layerIndexSampleMap */
  const Trk::LayerIndexSampleMap* layerIndexSampleMap() const;

  //!< return the TrackingGeometry used
  virtual const Trk::TrackingGeometry* trackingGeometry(
    const EventContext& ctx) const;

  /** IEnergyLossUpdator */
  BooleanProperty m_eLoss{this, "EnergyLoss", true};
  PublicToolHandle<Trk::IEnergyLossUpdator> m_eLossUpdator{this, "EnergyLossUpdator", "iFatras::McEnergyLossUpdator/FatrasEnergyLossUpdator"};

  /** IMultipleScatteringUpdator */
  BooleanProperty m_ms{this, "MultipleScattering", true};
  PublicToolHandle<Trk::IMultipleScatteringUpdator> m_msUpdator{this, "MultipleScatteringUpdator", "Trk::MultipleScatteringUpdator/AtlasMultipleScatteringUpdator"};

  /** IPhotonConversionTool */
  PublicToolHandle<iFatras::IPhotonConversionTool> m_conversionTool{this, "PhotonConversionTool", "iFatras::McConversionCreator/FatrasConversionCreator"};

  /** MCTruth process code for TruthIncidents created by this tool */
  IntegerProperty m_processCode{this, "BremProcessCode", 3, "MCTruth Physics Process Code"};

  /** MCTruth process sampling */
  PublicToolHandle<iFatras::IProcessSamplingTool> m_samplingTool{this, "ProcessSamplingTool", ""};

  /** hadronic interaction setting */
  BooleanProperty m_hadInt{this, "HadronicInteraction", true};
  PublicToolHandle<IHadronicInteractionProcessor> m_hadIntProcessor{this, "HadronicInteractionProcessor", "iFatras::G4HadIntProcessor/FatrasG4HadIntProcessor"};

  /** Particle Decay */
  PublicToolHandle<IParticleDecayHelper> m_particleDecayer{this, "ParticleDecayHelper", ""};

  /** Minimum momentum cut */
  DoubleProperty m_minimumMomentum{this, "MomentumCut", 50.*CLHEP::MeV};

  /** Minimum momentum for brem photons */
  DoubleProperty m_minimumBremPhotonMomentum{this, "MinimumBremPhotonMomentum", 50.*CLHEP::MeV};

  /** describe deflection parametric/do real deflection */
  BooleanProperty m_parametricScattering{this, "ParametericScattering", false};

  /** switch between MSUpdator and local parametrization */
  BooleanProperty m_use_msUpdator{this, "UseMultipleScatteringUpdator", false};

  /** use the relativistic hertz dipole for brem photon radiation */
  bool m_uniformHertzDipoleAngle{false};

  /** Switch to use reference material */
  BooleanProperty m_referenceMaterial{this, "ReferenceMaterial", true};

  /** Switch to use bending correction */
  BooleanProperty m_bendingCorrection{this, "BendingCorrection", false};

  /** Random Generator service  */
  ServiceHandle<IAtRndmGenSvc> m_rndGenSvc{this, "RandomNumberService", "AtDSFMTGenSvc", "Random number generator"};
  /** Random engine  */
  CLHEP::HepRandomEngine* m_randomEngine{nullptr};
  StringProperty m_randomEngineName{this, "RandomStreamName", "FatrasRnd", "Name of the random number stream"}; //!< Name of the random number stream

  /** for statistics output */
  mutable std::atomic<unsigned int> m_recordedBremPhotons{0};

  /** for the calo energy recording */
  int m_currentSample{-1};                   //!< the currentSample
  BooleanProperty m_recordEnergyDeposition{this, "RecordEnergyDeposition", false}; //!< for deposition methods
  StringProperty m_layerIndexCaloSampleMapName{this, "LayerIndexCaloSampleMap", "LayerIndexCaloSampleMap"}; //!< name to record it
  mutable const Trk::LayerIndexSampleMap*
    m_layerIndexCaloSampleMap{nullptr}; //!< the map for the calo-layer index map

  // ---------------------------- Tracking Geometry ---------------------------
  bool m_useConditions{};
  SG::ReadCondHandleKey<Trk::TrackingGeometry> m_trackingGeometryReadKey{
    this,
    "TrackingGeometryReadKey",
    "ISF_FatrasTrackingGeometry",
    "Key of input TrackingGeometry"
  };

  /// ToolHandle to the TrackingGeometrySvc
  ServiceHandle<Trk::ITrackingGeometrySvc> m_trackingGeometrySvc{
    this,
    "TrackingGeometrySvc",
    "",
    ""
  };
  /// Name of the TrackingGeometry as given in Detector Store
  std::string m_trackingGeometryName{ "ISF_FatrasTrackingGeometry" };
  // --------------------------------------------------------------------------

  /** projection factor for the non-parametric scattering */
  double m_projectionFactor{std::sqrt(2.)};


  // ------------------------ Validation section
  // ------------------------------------
  BooleanProperty m_validationMode{this, "ValidationMode", false};
  PublicToolHandle<IPhysicsValidationTool> m_validationTool{this, "PhysicsValidationTool", ""};
  std::string m_validationTreeName{"FatrasMaterialEffects"}; //!< validation tree name - to be acessed by
                                    //!< this from root
  std::string m_validationTreeDescription{"Validation output from the McMaterialEffectsUpdator"}; //!< validation tree description -
                                           //!< second argument in TTree
  std::string m_validationTreeFolder{"/val/FatrasSimulationMaterial"}; //!< stream/folder to for the TTree to be
                                      //!< written out

  TTree* m_validationTree{nullptr}; //!< Root Validation Tree

  // ----------- ntuple branches
  mutable int
    m_layerIndex{}; //!< ntuple variable : layer index of material effects update
  mutable float m_tInX0{};        //!< nutple variable : t/X0
  mutable float m_thetaMSproj{};  //!< ntuple variable : projected ms
  mutable float m_thetaMSphi{};   //!< ntuple variable : ms in phi
  mutable float m_thetaMStheta{}; //!< ntuple variable : ms in theta
  mutable float m_deltaP{};       //!< nutple variable : energy loss
  mutable float m_deltaPsigma{};  //!< ntuple variable : stragling on energy loss

  BooleanProperty m_bremValidation{this, "BremPhotonValidation", false};
  std::string m_bremValidationTreeName{"FatrasBremPhotons"}; //!< validation tree name - to be
                                               //!< acessed by this from root
  std::string m_bremValidationTreeDescription{"Validation output from the McMaterialEffectsUpdator"}; //!< validation tree description
                                               //!< - second argument in TTree
  std::string m_bremValidationTreeFolder{"/val/FatrasBremPhotons"}; //!< stream/folder to for the TTree to
                                          //!< be written out

  TTree* m_bremValidationTree{nullptr}; //!< Root Validation Tree

  // ------ ntuple branches
  mutable float m_bremPointX{}; //!< ntuple variable : brem point x coordinate
  mutable float m_bremPointY{}; //!< ntuple variable : brem point y coordinate
  mutable float m_bremPointR{}; //!< ntuple variable : brem point r distance
  mutable float m_bremPointZ{}; //!< ntuple variable : brem point z coordinate
  mutable float m_bremMotherEnergy{}; //!< ntuple variable : brem mother energy
  mutable float m_bremPhotonEnergy{}; //!< ntuple variable : brem photon energy
  mutable float m_bremPhotonAngle{};  //!< ntuple variable : brem photon angle

  // --------------------------------------------------------------------------------

  BooleanProperty m_edValidation{this, "EnergyDepositValidation", false};
  std::string m_edValidationTreeName{"FatrasEnergyInCaloDeposit"}; //!< validation tree name - to be acessed
                                      //!< by this from root
  std::string m_edValidationTreeDescription{"Validation output from the McMaterialEffectUpdator"}; //!< validation tree description -
                                             //!< second argument in TTree
  std::string m_edValidationTreeFolder{"/val/FatrasEnergyInCaloDeposit"}; //!< stream/folder to for the TTree to
                                        //!< be written out

  TTree* m_edValidationTree{nullptr}; //!< Root Validation Tree

  // ------ ntuple branches
  mutable float
    m_edLayerIntersectX{}; //!< ntuple variable : energy deposit x coordinate
  mutable float
    m_edLayerIntersectY{}; //!< ntuple variable : energy deposit y coordinate
  mutable float
    m_edLayerIntersectZ{}; //!< ntuple variable : energy deposit z coordinate
  mutable float
    m_edLayerIntersectR{}; //!< ntuple variable : energy deposit r coordinate
  mutable float
    m_edLayerEnergyDeposit{};      //!< ntuple variable : energy despoit - value
  mutable float m_edLayerSample{}; //!< ntuple variable : layer sample

  /** useful for the angle calculation of the brem photon */
  double m_oneOverThree{1./3.};

  ServiceHandle<ISF::IParticleBroker> m_particleBroker{this, "ParticleBroker", "ISF_ParticleParticleBroker", "ISF Particle Broker Svc"};
  ServiceHandle<ISF::ITruthSvc> m_truthRecordSvc{this, "TruthRecordSvc", "ISF_TruthRecordSvc", "ISF Particle Truth Svc"};

  /** cache incoming particle */
  mutable const ISF::ISFParticle* m_isp{nullptr};

  /** cache layer properties */
  mutable const Trk::Layer* m_layer{nullptr};
  mutable const Trk::MaterialProperties* m_matProp{nullptr};
  Trk::PdgToParticleHypothesis m_pdgToParticleHypothesis;
};

}

#endif // ISF_FATRASTOOLS_TRACKONLAYERUPDATOR_H
