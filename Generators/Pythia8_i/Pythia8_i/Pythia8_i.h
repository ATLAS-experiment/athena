/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  Author: James Monk
*/

#ifndef GENERATOR_PYTHIA8_H
#define GENERATOR_PYTHIA8_H

#include "GeneratorModules/GenModule.h"

// calls to fortran routines
#include "CLHEP/Random/RandFlat.h"
#include "GaudiKernel/ToolHandle.h"
#include "Pythia8_i/UserHooksFactory.h"
#include "Pythia8_i/IPythia8Custom.h"

//#include "Pythia8/../Pythia8Plugins/HepMC2.h"
#ifdef HEPMC3
#include "Pythia8Plugins/HepMC3.h"
namespace HepMC {
  typedef HepMC3::Pythia8ToHepMC3 Pythia8ToHepMC;
}
#else
#include "Pythia8Plugins/HepMC2.h"
#endif

#include <stdexcept>


/**
 *  Author: James Monk (jmonk@cern.ch)
 */

namespace Pythia8{
  class Sigma2Process;
}


class customRndm : public Pythia8::RndmEngine {
public:

  // Constructor.
  customRndm() {}

  // Routine for generating a random number.
  inline double flat(){
    m_RNC++;
    return CLHEP::RandFlat::shoot(m_engine);
  };

  // Initialisation Routine
  inline void init(CLHEP::HepRandomEngine* engine) {m_engine=engine; m_RNC=0;}
  inline CLHEP::HepRandomEngine* getEngine() { return m_engine; }
  inline unsigned long long int getRNCalls() {return m_RNC;}
private:
  unsigned long long int m_RNC;
  CLHEP::HepRandomEngine* m_engine{};
};


//namespace Generator{
class Pythia8_i: public GenModule{

public:
  Pythia8_i(const std::string &name, ISvcLocator *pSvcLocator);

  ~Pythia8_i();

  class CommandException : public std::runtime_error{
  public:

    CommandException(const std::string &cmd): std::runtime_error("Cannot interpret command: " + cmd){
    }
  };

  virtual StatusCode genInitialize();
  virtual StatusCode callGenerator();
  virtual StatusCode fillEvt(HepMC::GenEvent *evt);
  virtual StatusCode fillWeights(HepMC::GenEvent *evt);
  virtual StatusCode genFinalize();

  double pythiaVersion() const;

  static const std::string& pythia_stream();
  static std::string xmlpath();
  
protected:

  bool useRndmGenSvc() const { return m_useRndmGenSvc; }
  bool useReseed() const {return m_useReseed; }

  std::unique_ptr<Pythia8::Pythia> m_pythia{};
  HepMC::Pythia8ToHepMC m_pythiaToHepMC;
  UnsignedIntegerProperty m_maxFailures{this, "MaxFailures", 10};

  BooleanProperty m_useRndmGenSvc{this, "useRndmGenSvc", true, "the max number of consecutive failures"};
  std::shared_ptr<customRndm> m_atlasRndmEngine{};

  BooleanProperty m_useReseed{this,"useReseed", false};

  IntegerProperty m_dsid{this, "Dsid", 999999, "Dataset ID number"};
  StringArrayProperty m_userHooks{this, "UserHooks", {} };
//for Py8B
  DoubleProperty m_pt0timesMPI{this,"pT0timesMPI", 1.0};
  DoubleProperty m_numberAlphaS{this,"numberAlphaS", 3.0};
  BooleanProperty m_sameAlphaSAsMPI{this,"useSameAlphaSasMPI", false};

private:

  static std::string findValue(const std::string &command, const std::string &key);

  int m_internal_event_number{0};

  double m_version{-1.};

  StringArrayProperty m_commands{this, "Commands", {} };
  std::vector<std::string> m_userParams;
  std::vector<std::string> m_userModes;

  enum PDGID {PROTON=2212, ANTIPROTON=-2212, LEAD=1000822080, NEUTRON=2112, ANTINEUTRON=-2112, MUON=13, ANTIMUON=-13, ELECTRON=11, POSITRON=-11, INVALID=0};

  DoubleProperty m_collisionEnergy{this, "CollisionEnergy", 14000.0};


  StringProperty m_beam1{this, "Beam1", "PROTON"};
  StringProperty m_beam2{this, "Beam2", "PROTON"};
  bool m_override_transform_beamenergy{false};

  StringProperty m_lheFile{this, "LHEFile", ""};

  BooleanProperty m_doCKKWLAcceptance{this, "CKKWLAcceptance", false};
  BooleanProperty m_doFxFxXS{this, "FxFxXS", false};
  BooleanProperty m_computeEfficiency{this, "computeEfficiency", false};
  double m_nAccepted{0.};
  double m_nMerged{0.};
  double m_sigmaTotal{0.};
  double m_conversion{1.};

  unsigned int m_failureCount{0};

  std::map<std::string, PDGID> m_particleIDs;

  std::vector<long int> m_seeds{};

  StringProperty m_userProcess{this, "UserProcess", ""};

  // ptr to possible user process
  std::shared_ptr<Pythia8::Sigma2Process> m_procPtr{};

  std::vector<UserHooksPtrType> m_userHooksPtrs{};

  StringProperty m_userResonances{this, "UserResonances", ""};

  std::vector<std::shared_ptr<Pythia8::ResonanceWidths> > m_userResonancePtrs;

  BooleanProperty m_useLHAPDF{this, "UseLHAPDF", true};

  StringProperty m_particleDataFile{this, "ParticleData", ""};
  StringProperty m_outputParticleDataFile{this, "OutputParticleData", "ParticleData.local.xml"};

  double m_mergingWeight{1.0};
  double m_enhanceWeight{1.0};
  std::vector<std::string> m_weightIDs{};
  std::vector<std::string> m_weightNames{};
  bool m_doLHE3Weights{false};
  std::vector<std::string> m_weightCommands{};
  std::vector<std::string> m_showerWeightNames{"nominal"};
  StringArrayProperty m_showerWeightNamesProp{this, "ShowerWeightNames", {} };

  PublicToolHandle<IPythia8Custom> m_athenaTool{this, "CustomInterface", ""};

  BooleanProperty m_saveLHE{this, "SaveLHERecord", false};

  static int s_allowedTunes(double version);

  Pythia8::SuppressSmallPT *m_SuppressSmallPT{};

};

#endif
