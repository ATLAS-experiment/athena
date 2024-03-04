/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MCTruthClassifier/IMCTruthClassifier.h"
#include "GaudiKernel/ServiceHandle.h"
#include "xAODTruth/TruthEvent.h"
#include "TruthUtils/HepMCHelpers.h"
#include "GaudiKernel/ToolHandle.h"
namespace Common {
  bool prompt( const xAOD::TruthParticle* part,ToolHandle<IMCTruthClassifier>& m_classif );
  bool fromTau(const HepMC::ConstGenParticlePtr& part );
  bool fromWZ(const HepMC::ConstGenParticlePtr& part );
void classify(ToolHandle<IMCTruthClassifier>& m_classif,
const xAOD::TruthParticle * theParticle,
        unsigned int& particleOutCome,
        unsigned int& result,
        int& hadron_pdg,
        unsigned int& particleType,
        unsigned int& particleOrigin );
}
