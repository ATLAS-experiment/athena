/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HEPMCTRUTHREADER_H
#define HEPMCTRUTHREADER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "AtlasHepMC/GenEvent_fwd.h"
#include "AtlasHepMC/GenVertex_fwd.h"
#include "AtlasHepMC/GenParticle_fwd.h"
#include "StoreGate/ReadHandleKey.h"

#include "GeneratorObjects/McEventCollection.h"


/// @short Algorithm demonstrating reading of HepMC truth, and printing to screen
/// @author James Catmore <James.Catmore@cern.ch>
class HepMCTruthReader : public AthReentrantAlgorithm {
public:

  /// Regular algorithm constructor
  HepMCTruthReader(const std::string& name, ISvcLocator* svcLoc);

  /// Function initialising the algorithm
  virtual StatusCode initialize();

  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const;


private:

  /// The key of the input HepMC truth container
  SG::ReadHandleKey<McEventCollection> m_hepMCContainerKey{
      this, "HepMCContainerKey", "GEN_EVENT", "The input McEvenCollection"};

  /// Flag to printout in pt,eta,phi instead of px,py,pz
  Gaudi::Property<bool> m_do4momPtEtaPhi{this, "Do4momPtEtaPhi", false};

  static void printEvent(const HepMC::GenEvent* evt,                bool do4momPtEtaPhi);
  static void printVertex(const HepMC::ConstGenVertexPtr& vtx,      bool do4momPtEtaPhi );
  static void printParticle(const HepMC::ConstGenParticlePtr& part, bool do4momPtEtaPhi);

}; // class HepMCTruthReader



#endif // HEPMCTRUTHREADER_H
