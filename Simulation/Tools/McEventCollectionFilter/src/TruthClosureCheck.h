/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTHCLOSURECHECK_H
#define TRUTHCLOSURECHECK_H

// Base class include
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Athena includes
#include "StoreGate/ReadHandleKey.h"
#include "GeneratorObjects/McEventCollection.h"

class TruthClosureCheck : public AthReentrantAlgorithm {

public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

private:
  StatusCode sanityCheck(const HepMC::GenEvent& event) const;
#ifdef HEPMC3
  StatusCode compareGenVertex(const HepMC::ConstGenVertexPtr& origVertex,
                              const HepMC::ConstGenVertexPtr& resetVertex) const;
  StatusCode compareGenParticle(const HepMC::ConstGenParticlePtr& origParticle,
                                const HepMC::ConstGenParticlePtr& resetParticle) const;
  void printGenVertex(const HepMC::ConstGenVertexPtr& origVertex,
                      const HepMC::ConstGenVertexPtr& resetVertex) const;
#else
  StatusCode compareGenVertex(const HepMC::GenVertex& origVertex,
                              const HepMC::GenVertex& resetVertex) const;
  StatusCode compareGenParticle(const HepMC::GenParticle& origParticle,
                                const HepMC::GenParticle& resetParticle) const;
  void printGenVertex(const HepMC::GenVertex& origVertex,
                      const HepMC::GenVertex& resetVertex) const;
#endif
  StatusCode compareMomenta(const HepMC::FourVector& origMomenta,
                            const HepMC::FourVector& resetMomenta) const;

  SG::ReadHandleKey<McEventCollection> m_originalMcEventCollection{this, "OriginalMcEventCollection" , "BeamTruthEvent"};
  SG::ReadHandleKey<McEventCollection> m_resetMcEventCollection{this, "ResetMcEventCollection" , "NewTruthEvent"};
  Gaudi::Property<bool> m_compareMomenta{this, "CompareMomenta", true};
  double m_momentaLimit{0.000000000001};
  Gaudi::Property<bool> m_postSimulation{this, "PostSimulation", false};
};
#endif
