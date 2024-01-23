/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ParticleJetTools/JetPartonTruthLabel.h"
#include "xAODTruth/TruthParticle.h"
#include "TruthUtils/HepMCHelpers.h"

using namespace std;
using namespace Analysis;

int JetPartonTruthLabel::modifyJet(xAOD::Jet &jet) const {
  ATH_MSG_VERBOSE("In " << name() << "::modifyJet()");

	int label = -1;
	double e_max = 0;
	double pt_max = 0;
	double dr_max = 0;

	vector<const xAOD::TruthParticle*> partons;
	bool success = jet.getAssociatedObjects("GhostPartons", partons);
	if(success) {
		for(auto it = partons.begin(); it != partons.end(); it++) {
                  if(MC::isTop(*it)) continue; // Skip top

			if((*it)->e() > e_max) {
				label = (*it)->absPdgId();
				e_max = (*it)->e();
				pt_max = (*it)->pt();
				dr_max = (*it)->p4().DeltaR(jet.p4());
			}
		}
	}

  jet.setAttribute("PartonTruthLabelID", label);
  jet.setAttribute("PartonTruthLabelPt", pt_max);
  jet.setAttribute("PartonTruthLabelEnergy", e_max);
  jet.setAttribute("PartonTruthLabelDR", dr_max);

  return 0;
}
