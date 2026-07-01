/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATOR_PHOTOSPP_H
#define GENERATOR_PHOTOSPP_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaKernel/IAthRNGSvc.h"
#include "CLHEP/Random/RandomEngine.h"

extern "C" double phoranc_(int *idum);

class Photospp_i : public AthAlgorithm{

public:

  /// Standard Athena algorithm constructor
  Photospp_i(const std::string &name, ISvcLocator *pSvcLocator);

  /// Initialise the Photospp_i algorithm and required services
  StatusCode initialize();

  /// Run Photos on one event
  /// Will require a pre-existing HepMC event in Storegate
  StatusCode execute(const EventContext& ctx);

  /// Set up the Photos class
  /// This may be called in the initialize method or directly before the
  /// generation of the first event
  void setupPhotos();

  /// This external fortran function is the PHOTOS++ random number generator
  /// We make it a friend so it can access our AthRNGSvc
  friend double ::phoranc_(int *idum);

  static CLHEP::HepRandomEngine* p_rndmEngine;

private:

  /// @name Features for derived classes to use internally
  //@{
  void reseedRandomEngine(const std::string& streamName, const EventContext& ctx);
  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName, unsigned long int randomSeedOffset, const EventContext& ctx) const;
  CLHEP::HepRandomEngine* getRandomEngineDuringInitialize(const std::string& streamName, unsigned long int randomSeedOffset, unsigned int conditionsRun=1, unsigned int lbn=1) const;
  //@}

  // Random number service
  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};

  //Gen_tf run args.
  IntegerProperty m_dsid{this, "Dsid", 999999};

  /// Seed for random number engine
  IntegerProperty m_randomSeed{this, "RandomSeed", 1234567, "Random seed for the built-in random engine"}; // FIXME make this into an unsigned long int?

  /// The GenEvent StoreGate key - FIXME should be using Read/WriteHandles here
  StringProperty m_genEventKey{this, "MCEventKey", "GEN_EVENT"};

  /// Whether to use exponentiation mode (default = yes)
  BooleanProperty m_exponentiation{this, "ExponentiationMode", true};

  /// Whether to create history entries (default = yes)
  BooleanProperty m_createHistory{this, "CreateHistory", false};

  /// Whether to stop on critical error (default = no)
  BooleanProperty m_stopCritical{this, "StopCriticalErrors", false};

  /// Delay initialisation until just before first event execution (default = no)
  BooleanProperty m_delayInitialisation{this, "DelayInitialisation", false};

  /// Whether to apply ME correction to Z decays (default = no, until validated)
  BooleanProperty m_ZMECorrection{this, "ZMECorrection", false};

  /// Whether to apply ME correction to W decays (default = no, until validated)
  BooleanProperty m_WMECorrection{this, "WMECorrection", false};

  /// Whether to include photon splitting
  BooleanProperty m_photonSplitting{this, "PhotonSplitting", false};

  ///
  DoubleProperty m_infraRedCutOff{this, "InfraRedCutOff", -1./*, 1.e-07, 0.01/91.187*/};
  ///
  DoubleProperty m_maxWtInterference{this, "WtInterference", 3.};

  /// Value of alpha_QED
  DoubleProperty m_alphaQED{this, "AlphaQED", 0.00729735039};

};
#endif
