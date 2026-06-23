/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GeneratorFilters/TTbarPlusHeavyFlavorFilter.h
 * @author Georges Aad
 * @date Sep. 2014
 * @brief filter to select ttbar+jets/HF events
*/


#ifndef GeneratorFilters_TTbarPlusHeavyFlavorFilter_H
#define GeneratorFilters_TTbarPlusHeavyFlavorFilter_H

#include "GeneratorModules/GenFilter.h"

#include "AtlasHepMC/GenParticle_fwd.h"

class TTbarPlusHeavyFlavorFilter: public GenFilter {
public:
  TTbarPlusHeavyFlavorFilter(const std::string& fname, ISvcLocator* pSvcLocator);
  virtual ~TTbarPlusHeavyFlavorFilter();
  virtual StatusCode filterInitialize();
  virtual StatusCode filterFinalize();
  virtual StatusCode filterEvent(const EventContext& ctx);

private:


  /// properties
  bool m_useFinalStateHadrons;
  bool m_selectB;
  bool m_selectC;
  bool m_selectL;


  double m_bPtMinCut;
  double m_bEtaMaxCut;
  double m_cPtMinCut;
  double m_cEtaMaxCut;

  int m_bMultiCut;
  int m_cMultiCut;

  bool m_excludeBFromTop;
  bool m_excludeCFromTop;


  bool passBSelection(const HepMC::ConstGenParticlePtr& part) const;
  bool passCSelection(const HepMC::ConstGenParticlePtr& part) const;

  int hadronType(int pdgid) const;
  bool isBHadron(const HepMC::ConstGenParticlePtr& part) const;
  bool isCHadron(const HepMC::ConstGenParticlePtr& part) const;

  bool isInitialHadron(const HepMC::ConstGenParticlePtr& part) const;
  bool isFinalHadron(const HepMC::ConstGenParticlePtr& part) const;

  bool isQuarkFromHadron(const HepMC::ConstGenParticlePtr& part) const;
  bool isCHadronFromB(const HepMC::ConstGenParticlePtr& part) const;

  HepMC::ConstGenParticlePtr   findInitial(const HepMC::ConstGenParticlePtr& part) const;

  bool isFromTop(const HepMC::ConstGenParticlePtr& part) const;
  bool isDirectlyFromTop(const HepMC::ConstGenParticlePtr& part) const;
  bool isDirectlyFromWTop(const HepMC::ConstGenParticlePtr& part) const;



  

  

};

#endif /// GeneratorFilters_TTbarPlusHeavyFlavorFilter_H
