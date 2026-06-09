/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#ifndef TAUOLAPP_H
#define TAUOLAPP_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"

#include "CLHEP/Random/RandomEngine.h"


/// @brief This Algorithm provides an easy interface to Tauola C++ interface
/// @author Nadia Davidson, Marcin Wolter
/// @todo Convert to use standard GenModule base class
class ATLAS_NOT_THREAD_SAFE TauolaPP : public AthAlgorithm {
public:

  /// Constructor
  TauolaPP (const std::string& name, ISvcLocator* pSvcLocator);

  /// Initialization of Tauola++ and setting of JO configurables
  virtual StatusCode initialize() override;

  /// Pass each event in the McEventCollection to Tauola to (re)decay the taus
  virtual StatusCode execute(const EventContext& ctx) override;

  static CLHEP::HepRandomEngine* p_rndmEngine;

private:

  /// @name Features for derived classes to use internally
  //@{
  void reseedRandomEngine(const std::string& streamName, const EventContext& ctx);
  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName, unsigned long int randomSeedOffset, const EventContext& ctx) const;
  CLHEP::HepRandomEngine* getRandomEngineDuringInitialize(const std::string& streamName, unsigned long int randomSeedOffset, unsigned int conditionsRun=1, unsigned int lbn=1) const;
  //@}

  /// Event record container key - FIXME should be using Read/WriteHandles here
  StringProperty m_key{this, "McEventKey", "GEN_EVENT"};

  // Random number service
  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};

  //Gen_tf run args.
  IntegerProperty m_dsid{this, "Dsid", 999999};

  /// Seed for random number engine
  IntegerProperty m_randomSeed{this, "RandomSeed", 1234567, "Random seed for the built-in random engine"}; // FIXME make this into an unsigned long int?

  /// @name Variables used to configure Tauola
  //@{

  /// PDG ID of particle to study
  IntegerProperty m_decay_particle{this, "decay_particle", 15};

  /// TAUOLA decay mode of particles with same charge as "decay_particle"
  IntegerProperty m_decay_mode_same{this, "decay_mode_same", 1};

  /// TAUOLA decay mode of particles with opposite charge as "decay_particle"
  IntegerProperty m_decay_mode_opp{this, "decay_mode_opposite", 2};

  /// tau mass to be taken by TAUOLA
  DoubleProperty m_tau_mass{this, "tau_mass", 1.77684};

  /// TAUOLA switch for spin effects
  BooleanProperty m_spin_correlation{this, "spin_correlation", true};

  /// TAUOLA switch for radiative corrections for leptonic tau decays
  BooleanProperty m_setRadiation{this, "setRadiation", true};

  /// TAUOLA cut-off for radiative corrections
  DoubleProperty m_setRadiationCutOff{this, "setRadiationCutOff", 0.01};

};

#endif
