/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file GeneratorFilters/TTbarWithJpsimumuFilter.h
 * @author Fr??d??ric Derue
 * @date Jan. 2018
 * @brief filter to select ttbar with Jpsi->mumu events
 */


#ifndef GeneratorFilters_XAODTTbarWithJpsimumuFilter_H
#define GeneratorFilters_XAODTTbarWithJpsimumuFilter_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

class xAODTTbarWithJpsimumuFilter: public GenFilter {
 public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterFinalize() override final;
  virtual StatusCode filterEvent() override final;

 private:

  /// properties
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};
  Gaudi::Property<bool> m_selectJpsi{this, "SelectJpsi", true};
  Gaudi::Property<double> m_JpsiPtMinCut{this, "JpsipTMinCut", 0.}; /// MeV
  Gaudi::Property<double> m_JpsiEtaMaxCut{this, "JpsietaMaxCut", 5.};

  // method to check if Jpsi decays into pair of leptons
  bool isLeptonDecay(const xAOD::TruthParticle* part, int type) const;

  // method to check if Jpsi pass some selection criteria
  bool passJpsiSelection(const xAOD::TruthParticle* part) const;

};

#endif /// GeneratorFilters_xAODTTbarWithJpsimumuFilter_H
