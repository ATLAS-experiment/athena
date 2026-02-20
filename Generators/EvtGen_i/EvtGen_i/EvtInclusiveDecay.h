/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//*****************************************************************************
//
// Generators/EvtGen_i/EvtInclusiveDecay.h
//
// EvtInclusiveDecay is a TopAlg that takes HepMC events from StoreGate and
// generates particle decays using EvtGen. Depending on job options either all or
// only a subset of the particles which have decays defined in the EvtGen
// decay files will be handed to EvtGen. Both undecayed particles and particles
// with an existing decay tree can be handled (in the latter case,
// EvtInclusiveDecay will remove the existing decay tree).
//
// Written in February 2006 by Juerg Beringer, based in part on the existing
// EvtDecay module.
//
//*****************************************************************************

#ifndef GENERATORMODULESEVTDECAYINCLUSIVE_H
#define GENERATORMODULESEVTDECAYINCLUSIVE_H

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"

#include "HepPDT/ParticleDataTable.hh"

#include "EvtGenBase/EvtParticle.hh"
#include "EvtGen/EvtGen.hh"
#include "EvtGenBase/EvtRandomEngine.hh"

#include "GeneratorModules/GenBase.h"
#include "GaudiKernel/ISvcLocator.h"

#include "AthenaKernel/IAthRNGSvc.h"
#include "GeneratorObjects/McEventCollection.h"

namespace CLHEP {
  class HepRandomEngine;
};

class EvtInclusiveAtRndmGen : public EvtRandomEngine {
public:
  EvtInclusiveAtRndmGen(CLHEP::HepRandomEngine* engine);
  virtual ~EvtInclusiveAtRndmGen() = default;
  double random();
  CLHEP::HepRandomEngine* getEngine() { return m_engine; }
private:
  CLHEP::HepRandomEngine* m_engine{};
};



class EvtInclusiveDecay:public GenBase {

public:
  EvtInclusiveDecay(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~EvtInclusiveDecay();

  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
  std::string xmlpath(void);
  struct ParticleIdCompare {bool operator()(const HepMC::GenParticlePtr& a,const HepMC::GenParticlePtr& b) const {return (a)&&(b)&&(a->momentum().e() < b->momentum().e());}};
private:

  /// @name Features for derived classes to use internally
  //@{
  void reseedRandomEngine(const std::string& streamName, const EventContext& ctx);
  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName, unsigned long int randomSeedOffset, const EventContext& ctx) const;
  CLHEP::HepRandomEngine* getRandomEngineDuringInitialize(const std::string& streamName, unsigned long int randomSeedOffset, unsigned int conditionsRun=1, unsigned int lbn=1) const;
  //@}

  StatusCode traverseDecayTree(HepMC::GenParticlePtr p,
                               bool isToBeRemoved,
                               std::set<HepMC::GenVertexPtr>& visited,
                               std::set<HepMC::GenParticlePtr,ParticleIdCompare>& toBeDecayed);
  void removeDecayTree(HepMC::GenEvent* hepMC, HepMC::GenParticlePtr p);
  void decayParticle(HepMC::GenEvent* hepMC, HepMC::GenParticlePtr p);
  void addEvtGenDecayTree(HepMC::GenEvent* hepMC, HepMC::GenParticlePtr part, EvtParticle* evtPart, EvtVector4R treeStart, double momentumScaleFactor = 1.0);

  bool isToBeDecayed(HepMC::ConstGenParticlePtr p, bool doCrossChecks);
  bool isDefaultB(const int pId) const;

  bool passesUserSelection(HepMC::GenEvent* hepMC);
  double invMass(HepMC::ConstGenParticlePtr p1, HepMC::ConstGenParticlePtr p2);

  // Utility functions to print HepMC record for debugging with optional
  // coloring by status code and highlighting of particles in a specific list
  void printHepMC(HepMC::GenEvent* hepMC, std::set<HepMC::GenParticlePtr,ParticleIdCompare>* particleSet = nullptr);
  unsigned int printTree(HepMC::GenParticlePtr p, std::set<HepMC::GenVertexPtr>& visited, int level, std::set<HepMC::GenParticlePtr,ParticleIdCompare>* particleSet = nullptr);
  std::string pdgName(HepMC::ConstGenParticlePtr p, bool statusHighlighting = false, std::set<HepMC::GenParticlePtr,ParticleIdCompare>* particleSet = nullptr);

  // Random number service
  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};

  //Gen_tf run args.
  IntegerProperty m_dsid{this, "Dsid", 999999};

  /// Seed for random number engine
  IntegerProperty m_randomSeed{this, "RandomSeed", 1234567, "Random seed for the built-in random engine"}; // FIXME make this into an unsigned long int?

  McEventCollection* m_mcEvtColl{};

  // EvtGen interface
  EvtInclusiveAtRndmGen*  m_evtAtRndmGen{};
  EvtGen*                 m_myEvtGen{};

  // jobOption params
  std::string m_pdtFile;
  std::string m_decayFile;
  std::string m_userDecayFile;
  std::string m_randomStreamName;
  std::string m_inputKeyName;
  std::string m_outputKeyName;

  bool m_readExisting;
  bool m_prohibitFinalStateDecay;
  bool m_prohibitReDecay;
  bool m_prohibitUnDecay;
  bool m_prohibitRemoveSelfDecay;
  std::vector<int> m_blackList;
  std::set<int> m_blackListSet;   // filed from m_blackList for speed optimization

  bool m_allowAllKnownDecays;
  bool m_allowDefaultBDecays;
  std::vector<int> m_whiteList;
  std::set<int> m_whiteListSet;   // filed from m_whilteList for speed optimization

  bool m_printHepMCBeforeEvtGen;
  bool m_printHepMCAfterEvtGen;
  bool m_printHepMCHighlighted;
  bool m_printHepMCHighLightTopLevelDecays;

  bool m_checkDecayTree;
  bool m_checkDecayChannels;
  std::map<int,long> m_noDecayChannels;

  int m_nRepeatedDecays;

  int m_maxNRepeatedDecays;

  bool m_applyUserSelection;
  bool m_userSelRequireOppositeSignedMu;
  double m_userSelMu1MinPt;
  double m_userSelMu2MinPt;
  double m_userSelMu1MaxEta;
  double m_userSelMu2MaxEta;
  double m_userSelMinDimuMass;
  double m_userSelMaxDimuMass;
  bool m_isfHerwig;
  bool m_setVMtransversePol;
};

#endif
