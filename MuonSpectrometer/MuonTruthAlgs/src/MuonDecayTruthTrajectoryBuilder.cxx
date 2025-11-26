/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Niels van Eldik 2010

#include "MuonDecayTruthTrajectoryBuilder.h"



#include "AthContainers/DataVector.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "GeneratorObjects/HepMcParticleLink.h"
#include "TrkTruthData/TruthTrajectory.h"
#include "TruthUtils/HepMCHelpers.h"
#include <stack>
namespace Muon {

    //================================================================
    MuonDecayTruthTrajectoryBuilder::MuonDecayTruthTrajectoryBuilder(const std::string& type, const std::string& name,
                                                                     const IInterface* parent) :
        AthAlgTool(type, name, parent)
    // ,m_isDecayIntoTwoMuons(false)
    {
        declareInterface<Trk::ITruthTrajectoryBuilder>(this);
    }
    //================================================================
    void MuonDecayTruthTrajectoryBuilder::buildTruthTrajectory(TruthTrajectory* result, const HepMC::ConstGenParticlePtr& input) const {
        result->clear();
        if (input) {
            HepMC::ConstGenParticlePtr next{nullptr};
            auto current = input;
            ATH_MSG_DEBUG(" New TruthTrajectory: input: " << input << " PDG id " << input->pdg_id());

            // Extend trajectory outwards.  The last particle should go at [0]
            // in the TruthTrajectory, so we need to use a tmp storage while
            // traversing the structure.
            std::stack<HepMC::ConstGenParticlePtr> tmp;
            while ((next = getDaughter(current))) { tmp.push(current = next); }

            // copy the outer half to result
            while (!tmp.empty()) {
                ATH_MSG_DEBUG(" Adding daughter: " << current); // FIXME "current" is not changed in this loop - it might make sense to print tmp.top() instead here?
                result->emplace_back(tmp.top(),tmp.top()->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM);
                tmp.pop();
            }

            // The input particle itself
            result->emplace_back(input,input->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM);

            // Now continue towards the interaction point
            while ((next = getMother(current))) {
                ATH_MSG_DEBUG(" Adding mother:  " << current); // FIXME "current" here refers to the child particle rather than the mother? Consider moving this after "current = next"
                current = next;
                result->emplace_back(current,current->parent_event()->event_number(),HepMcParticleLink::IS_EVENTNUM);
            }

            ATH_MSG_DEBUG(" Final TruthTrajectory: ");
            std::vector<HepMcParticleLink>::const_iterator pit = result->begin();
            std::vector<HepMcParticleLink>::const_iterator pit_end = result->end();
            for (; pit != pit_end; ++pit) {
                const HepMC::GenParticle& par = *pit->cptr();
                if (msgLvl(MSG::DEBUG)) {
                    msg(MSG::DEBUG) << " PDG ID " << par.pdg_id() 
                                    << " pt: " << par.momentum().perp();
                    if (par.production_vertex())
                        msg(MSG::DEBUG) << " vertices prod: r " << par.production_vertex()->position().perp() << " z "
                                        << par.production_vertex()->position().z();
                    if (par.end_vertex())
                        msg(MSG::DEBUG) << " end: r " << par.end_vertex()->position().perp() << " z " << par.end_vertex()->position().z();
                    msg(MSG::DEBUG) << endmsg;
                }
            }
        }
    }

    //================================================================
    MuonDecayTruthTrajectoryBuilder::MotherDaughter MuonDecayTruthTrajectoryBuilder::truthTrajectoryCuts(
        const HepMC::ConstGenVertexPtr& vtx) const {
        HepMC::ConstGenParticlePtr mother{nullptr};
        HepMC::ConstGenParticlePtr daughter{nullptr};
        int particles_in_size = vtx ? vtx->particles_in_size() : 0;
        int particles_out_size = vtx ? vtx->particles_out_size() : 0;
        if (vtx && msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << " new vertex: nparticles in " << particles_in_size << endmsg;
        // only truth vertices with 1 incoming particle
        if (vtx && (particles_in_size == 1)) {
#ifdef HEPMC3
            mother = vtx->particles_in().front();
#else
            mother = *vtx->particles_in_const_begin();
#endif

            if (mother && msgLvl(MSG::DEBUG))
                msg(MSG::DEBUG) << " new mother: " << mother->pdg_id() << " status " << mother->status() << " particles out "
                                << particles_out_size << endmsg;
            // Allow status code 1 and 2.  E.g. a pion that produced a long track can decay  outside of InDet and have status==2.
            if (mother && MC::isPhysical(mother)) {
                unsigned int nDecayMuons = 0;
                HepMC::ConstGenParticlePtr passed_cuts{nullptr};
                for (const auto& candidate : *vtx) {
                    if (msgLvl(MSG::DEBUG)) {
                        msg(MSG::DEBUG) << " PDG ID " << candidate->pdg_id() << " particle: " << candidate
                                        << " pt: " << candidate->momentum().perp();
                        if (candidate->production_vertex())
                            msg(MSG::DEBUG) << " vertices prod: r " << candidate->production_vertex()->position().perp() << " z "
                                            << candidate->production_vertex()->position().z();
                        if (candidate->end_vertex())
                            msg(MSG::DEBUG) << " end: r " << candidate->end_vertex()->position().perp() << " z "
                                            << candidate->end_vertex()->position().z();
                        msg(MSG::DEBUG) << endmsg;
                    }

                    if (candidate->pdg_id() == mother->pdg_id()) {
                        if (passed_cuts && MC::isElectron(mother)) {  // second negative electron is a special case
                            if (candidate->momentum().e() > passed_cuts->momentum().e()) { passed_cuts = candidate; }
                        } else {
                            passed_cuts = candidate;
                        }
                    } else if (MC::isMuon(candidate)) {
                        ATH_MSG_DEBUG(" selecting Decay into muon ");
                        ++nDecayMuons;
                        passed_cuts = candidate;
                    } else {  // temp addition for debugging
                        msg(MSG::DEBUG) << " Neither muon nor identical pdgId " << endmsg;
                        passed_cuts = candidate;
                    }
                }

                if (nDecayMuons > 0) {
                    daughter = std::move(passed_cuts);
                    if (nDecayMuons == 2) {
                        ATH_MSG_DEBUG(" decay into two muons ");
                    }
                }
            }  
        }

        return std::make_pair(mother, daughter);
    }

    //================================================================
    HepMC::ConstGenParticlePtr MuonDecayTruthTrajectoryBuilder::getDaughter(const HepMC::ConstGenParticlePtr& mother) const {
      if(mother) {
        MotherDaughter res = truthTrajectoryCuts(mother->end_vertex());
        if(res.first == mother) return res.second;
      }
      return {nullptr};
    }

    //================================================================
    HepMC::ConstGenParticlePtr MuonDecayTruthTrajectoryBuilder::getMother(const HepMC::ConstGenParticlePtr& daughter) const {
        return daughter ? truthTrajectoryCuts(daughter->production_vertex()).first : HepMC::ConstGenParticlePtr{nullptr};
    }

    //================================================================

}  // namespace Muon
