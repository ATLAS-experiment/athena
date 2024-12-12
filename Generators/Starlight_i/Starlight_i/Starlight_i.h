/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// --------------------------------------------------
//
// File:  Generators/Starlight_i.h
// Description:
//    This code is used to get a Starlight Monte Carlo event.
//    genInitialize() is used to read parameters
//    callGenerator() makes the event
//    genFinalize() writes log files etc
//    fillEvt(GeneratorEvent* evt) passes the event to HepMC
//
//    The output will be stored in the transient event store so it can be
//    passed to the simulation.
//
// AuthorList:
//         Andrzej Olszewski, March 2015

#ifndef GENERATORMODULESSTARLIGHT_H
#define GENERATORMODULESSTARLIGHT_H

#include "GeneratorModules/GenModule.h"

#include "starlight.h"
#include "upcevent.h"
#include "inputParameters.h"

class Starlight_i:public GenModule {
public:
    Starlight_i(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~Starlight_i();

    virtual StatusCode genInitialize();
    virtual StatusCode callGenerator();
    virtual StatusCode genFinalize();
    virtual StatusCode fillEvt(HepMC::GenEvent* evt);

protected:
  IntegerProperty m_dsid{this, "Dsid", 999999, "Dataset ID number"};
  StringProperty  m_configFileName{this, "ConfigFileName", ""};
  BooleanProperty  m_lheOutput{this, "lheOutput", false};
  UnsignedIntegerProperty m_maxevents{this, "maxevents", 5500};
  BooleanProperty m_doTauolappLheFormat{this, "doTauolappLheFormat", false};
  BooleanProperty m_suppressVMdecay{this, "suppressVMdecay", false};
  // Commands to setup starlight
  StringArrayProperty m_InitializeVector{this, "Initialize", {} };

  int              m_events{0}; // event counter
  starlight*       m_starlight{};         // pointer to starlight instance // TODO convert to unique_ptr
  std::shared_ptr<randomGenerator> m_randomGenerator{};
  inputParameters  m_inputParameters;   // parameter instance
  double           m_axionMass{1.};
  upcEvent        *m_event{}; // TODO convert to unique_ptr

  unsigned int m_beam1Z{0};
  unsigned int m_beam1A{0};
  unsigned int m_beam2Z{0};
  unsigned int m_beam2A{0};
  double       m_beam1Gamma{0.};
  double       m_beam2Gamma{0.};
  double       m_maxW{0.};
  double       m_minW{0.};
  unsigned int m_nmbWBins{0};
  double       m_maxRapidity{0.};
  unsigned int m_nmbRapidityBins{0};
  bool         m_accCutPt{false};
  double       m_minPt{0.};
  double       m_maxPt{0.};
  bool         m_accCutEta{false};
  double       m_minEta{0.};
  double       m_maxEta{0.};
  int          m_productionMode{0};
  unsigned int m_nmbEventsTot{0};
  int          m_prodParticleId{0};
  int          m_outputFormat{0};
  int          m_beamBreakupMode{0};
  bool         m_interferenceEnabled{false};
  double       m_interferenceStrength{0.};
  bool         m_coherentProduction{false};
  double       m_incoherentFactor{0.};
  double       m_bford{0.};
  double       m_maxPtInterference{0.};
  int          m_nmbPtBinsInterference{0};
  double       m_ptBinWidthInterference{0.};
  bool         m_xsecMethod{false};
  int          m_nThreads{1};
  bool         m_pythFullRec{false};

  bool starlight2lhef();

  bool set_user_params();
  bool prepare_params_file();
};

#endif
