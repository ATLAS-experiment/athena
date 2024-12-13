/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EFTRACKING_SMEARINGALG_H
#define EFTRACKING_SMEARINGALG_H 

#include "FakeTrackSmearer.h"
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "xAODTracking/TrackParticleContainer.h" 
#include "xAODTracking/TrackParticleAuxContainer.h" 
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"


class EFTrackingSmearingAlg: public ::AthHistogramAlgorithm { 
 public: 
  EFTrackingSmearingAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~EFTrackingSmearingAlg(); 

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute() override;

 private: 

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputTrackParticleKey { this, "InputTrackParticleContainer", "InDetTrackParticles_tosmear",
                                                                          "key for retrieval of input TrackParticles" };

  SG::WriteHandleKey<xAOD::TrackParticleContainer> m_outputTrackParticleKey { this, "OutputTrackParticleContainer", "InDetTrackParticles_smeared",
                                                                          "key for retrieval of output TrackParticles" };

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_inputTruthParticleKey{this,"InputTruthParticleContainer","TruthParticles_tosmear",
                                                                          "key for retrieval of input Truth particle"};
  
  SG::WriteHandleKey<xAOD::TruthParticleContainer> m_outputTruthParticleKey{this,"OutputTruthParticleContainer","TruthParticles_smeared",
                                                                          "key for retrieval of output Truth particle"};

  
  SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_d0DecoratorKey  {this, "d0", "TruthParticles.d0", "Particle d0 decoration, set at initialisation"};    
  SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_z0DecoratorKey  {this, "z0", "TruthParticles.z0", "Particle z0 decoration, set at initialisation"};    
  SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_ptDecoratorKey  {this, "pt", "TruthParticles.pt", "Particle pt decoration, set at initialisation"};    
  
  
  // to configure the smearer
  DoubleProperty   m_SigmaScaleFactor      {this, "SmearingScaleFactor", 1, "Set the smearing SF value for the sigmas"};
  BooleanProperty  m_UseResolutionPtCutOff {this, "UseResolutionPtCutOff", false, "Apply ptCutoff on smearing"};
  DoubleProperty   m_SetResolutionPtCutOff {this, "SetResolutionPtCutOff", 0., "Set ptCutoff off for semaring"};
  DoubleProperty   m_inputTracksPtCut      {this, "InputTracksPtCutGeV", 0.0, "Set input track pT cut"};
  DoubleProperty   m_outputTracksPtCut     {this, "OutputTracksPtCutGeV", 1.0, "Set the output track pT cut"};
  DoubleProperty   m_smearedTrackEfficiency{this, "SmearedTrackEfficiency", 1.0, "Set track efficiency for smearing"};
  BooleanProperty  m_parameterizedTrackEfficiency{this, "ParameterizedTrackEfficiency", false, "Enable parameterized efficiency for smearing"};  
  BooleanProperty  m_enableMonitoring      {this, "EnableMonitoring", false, "Enable debugging monitoring of the algorithm"};
  BooleanProperty  m_smearTruthParticle    {this, "SmearTruthParticle", false, "Enable smearing on truth particles, disabling the track smearing"};
  // this is for LRT
  BooleanProperty  m_parameterizedTrackEfficiency_LRT{this, "ParameterizedTrackEfficiency_LRT", false, "Enable parameterized efficiency for LRT smearing"};
  DoubleProperty   m_smearedTrackEfficiency_d0high_LRT{this, "SmearedTrackEfficiency_d0high_LRT", 1.0, "Set high d0 value for smeared track efficiency LRT"};
  DoubleProperty   m_smearedTrackEfficiency_d0low_LRT{this, "SmearedTrackEfficiency_d0low_LRT", 1.0, "Set low d0 value for smeared track efficiency LRT"};
  
  // these are for fake emulations
  BooleanProperty  m_EnableFakes      {this,"IncludeDuplicatesAndFakes",false,"Enable fake track production"};
  BooleanProperty  m_FakeKillerEnable {this,"FakeKillerEnable",false, "disable inclusion of broad fakes"};
  BooleanProperty  m_IncludeFakesInResolutionCalculation{this,"IncludeFakesInResolutionCalculation",false, 
                                      "modify calculation of covariance parameters to include weighted combination of core and broad RMS"};
  BooleanProperty  m_UseCoinToss      {this,"UseCoinToss",false, 
                                       "if True, fakes are generated with flat inefficiency, else use poissonian criteria"};
  
  LongLongProperty m_RandomSeed{this,"RandomSeed",0};
  
  std::unique_ptr<FakeTrackSmearer> m_mySmearer;
  StatusCode book_histograms();
  StatusCode smearTruthParticles(const EventContext& ctx);
}; 

#endif //> !EFTRACKING_SMEARINGALG_H
