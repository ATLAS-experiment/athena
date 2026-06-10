/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PartonJetAugmentationTool.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/PseudoJet.hh"

#include "AthenaKernel/errorcheck.h"
#include "TLorentzVector.h"
#include <cmath> //std::abs

namespace DerivationFramework { 
    // ============================
    // Initialize
    // ============================
    StatusCode PartonJetAugmentationTool::initialize() {
    ATH_MSG_INFO("Initializing PartonJetAugmentationTool");

    ATH_CHECK(m_truthKey.initialize());
    ATH_CHECK(m_partonJetsKey.initialize());

    return StatusCode::SUCCESS;
    }


    // ============================
    // EVENT FUNCTION (runs per event)
    // ============================
    StatusCode PartonJetAugmentationTool::addBranches(const EventContext& ctx) const 
    {

        // Get truth particles
        //const xAOD::TruthParticleContainer* truth = nullptr;
        //ATH_CHECK(evtStore()->retrieve(truth, "TruthParticles"));
        SG::ReadHandle<xAOD::TruthParticleContainer> truth(m_truthKey, ctx);
        if (!truth.isValid()) {
            ATH_MSG_ERROR("Failed to retrieve TruthParticles");
            return StatusCode::FAILURE;
        }


        // Create output jet container
        auto jets = std::make_unique<xAOD::JetContainer>();
        auto jetsAux = std::make_unique<xAOD::JetAuxContainer>();
        jets->setStore(jetsAux.get());

        TLorentzVector lj, slj, tlj;    

        double jet_radius = 0.4;
        double min_pT = 20;

        std::vector<TLorentzVector> ttbarDecayProducts;
        std::vector<int> decayProduct_pdgID;

        for(const auto* p : *truth){

            int pdg = p->pdgId();
            
            // Only consider W bosons and b quarks
            if(std::abs(pdg) != 5 && std::abs(pdg) != 24)
                continue;

            bool fromTop = false;

            // check parents
            for(unsigned int ip = 0; ip < p->nParents(); ++ip){
                const xAOD::TruthParticle* parent = p->parent(ip);

                if(parent && parent->absPdgId() == 6){
                    fromTop = true;
                    break;
                }
            }

            if(!fromTop) continue;
            TLorentzVector vec;
            vec.SetPtEtaPhiM(p->pt(), p->eta(), p->phi(), p->m());

            ttbarDecayProducts.push_back(vec);
            decayProduct_pdgID.push_back(pdg);
        }

        bool ok = extrajet(truth.cptr(), lj, slj, tlj, ttbarDecayProducts, decayProduct_pdgID, jet_radius, min_pT);


        auto saveJet = [&](const TLorentzVector& v) {
            if(v.Pt() <= 0) return; // skip empty jets
            xAOD::Jet* jet = new xAOD::Jet();
            jets->push_back(jet);
            jet->setJetP4(xAOD::JetFourMom_t(v.Pt(), v.Eta(), v.Phi(), v.M()));
        };

        if(ok){
            saveJet(lj);
            saveJet(slj);
            saveJet(tlj);
        }

        // Store in output DAOD
       //ATH_CHECK(evtStore()->record(std::move(jets), "PartonJets"));
       // ATH_CHECK(evtStore()->record(std::move(jetsAux), "PartonJetsAux."));
       SG::WriteHandle<xAOD::JetContainer> jetsHandle(m_partonJetsKey, ctx);
       ATH_CHECK(jetsHandle.record(std::move(jets), std::move(jetsAux)));
    
       

        return StatusCode::SUCCESS;
    }



    bool PartonJetAugmentationTool::isLastBeforeHadronization(const xAOD::TruthParticle* particle) const {

        const xAOD::TruthParticle* parent = particle;  
        bool isLastBeforeHadronization = false;

        if (particle->nChildren() > 0)
        {
            if (particle->child(0)->absPdgId() > 37)
            {
                isLastBeforeHadronization = true;
            }//if
            if (particle->nChildren() > 1) //safety measure because some particles have only themselves as child
            {
                if (particle->child(1)->absPdgId() > 37)
                {
                    isLastBeforeHadronization = true;
                }//if
            }//if
        }//if       

        // check if not after hadronization
        while(parent->nParents()>0 && isLastBeforeHadronization == true)
            {

                parent = parent->parent(0);

                if (parent->absPdgId() > 37 && parent->absPdgId() != 2212) //proton
                {
                    isLastBeforeHadronization=false;
                }
            
            }//while    

        return isLastBeforeHadronization;
    }


    bool PartonJetAugmentationTool::extrajet(
    const xAOD::TruthParticleContainer* truthParticles,
    TLorentzVector& lj,
    TLorentzVector& slj,
    TLorentzVector& /*tlj unused*/,
    const std::vector<TLorentzVector>& ttbarDecayProducts,
    const std::vector<int>& decayProduct_pdgID,
    double Rparam,
    double pt_min) const {

        // if true, decay products of the top quark and their children are removed from clustering
        const bool exclude_topdecay = true; 
        std::vector<fastjet::PseudoJet> vec_status62; //partons just before top decay
    
        for (const xAOD::TruthParticle* particle : *truthParticles) {
            // particles with status 81-100 are for internal MC only and are not to be used in clustering
            // particles with status > 100 are hadrons or other unstable particle
            // They're not relevant for parton jets
            if  (particle->absPdgId() > 80 || (particle->absPdgId() > 22 && particle->absPdgId() < 38)) continue; //exclude bosons which are not gluons or photons 

            // Find last parton before hadronization (corresponding to status 62 in pythia)
            //satus 11 is Pythia, status 71 is Herwig - we want to run for both
            if((particle->status()==11 || particle->status()==71) && particle->absPdgId()!=6 ){

                    const xAOD::TruthParticle* topParent = particle;
                    bool exclude=false;

                    if (!isLastBeforeHadronization(particle))
                    {
                        continue;
                    }

                     //Remove all partons that come from the top quark decay
                    if (exclude_topdecay)
                    {
                        while(topParent->nParents()>0 )
                        {
                        topParent = topParent->parent(0);
                        if (topParent->nChildren() > 0 )
                        {
                            if(   ( topParent->absPdgId()==24 && topParent->nParents()>0 && topParent->parent(0)->absPdgId()==6 ) ||
                                    ( topParent->absPdgId()==5  && topParent->nParents()>0 && topParent->parent(0)->absPdgId()==6 )   ) exclude=true;
                        }
                        if (exclude) break;
                        }//while
                    }// if (exclude_topdecay)
            

                    if(!exclude){
                        fastjet::PseudoJet tmp(particle->px(), particle->py(), particle->pz(), particle->e());
                        vec_status62.push_back(tmp);
                    } //if(!exclude)      
                    
            }
        }

        // Fastjet analysis - select algorithm and parameters
        fastjet::JetDefinition  jetDef(fastjet::antikt_algorithm, Rparam, fastjet::E_scheme, fastjet::Best);
    
        std::vector <fastjet::PseudoJet> inclusivePertJet62, sortedPertJet62;

        fastjet::ClusterSequence clustSeqPertJet62(vec_status62, jetDef);
        // Extract inclusive jets sorted by pT, and ask for Pt>pt_min 
        inclusivePertJet62 = clustSeqPertJet62.inclusive_jets(pt_min * 1000.); //GeV
        sortedPertJet62    = fastjet::sorted_by_pt(inclusivePertJet62);  
        //    std::cout<<"containers size: "<<inclusivePertJet22.size()<<"  "<<inclusivePertJet22.size()<<"\n";
        
        if(inclusivePertJet62.size()==0){
        //      std::cout<<"Extrajet containers are empty! No partonic extrajet!\n";
        return false;
        } 

        if (inclusivePertJet62.size()!=0)
        {
            // remove jets that are in deltaR <= 0.4 to one of the top decay products
            for (long unsigned int  i = 0; i < ttbarDecayProducts.size(); ++i)
            {
                for (long unsigned int  j = 0; j < sortedPertJet62.size(); ++j)
                {
                    if (sortedPertJet62.at(j).delta_R(ttbarDecayProducts[i]) <= 0.4
                        && std::abs(decayProduct_pdgID[i]) != 11 // electron 
                        && std::abs(decayProduct_pdgID[i]) != 12 // electron neutrino
                        && std::abs(decayProduct_pdgID[i]) != 13 // muon
                        && std::abs(decayProduct_pdgID[i]) != 14 // muon neutrino
                        // do not veto tau leptons since they also decay hadronically
                        && std::abs(decayProduct_pdgID[i]) != 16 // tau neutrino
                        )
                    {
                        
                        sortedPertJet62.erase(sortedPertJet62.begin() + j);
                        
                        --j; 
                    }
                }//for
                
            }//for

            bool leading_jet_found = false;
            
            
            /* The following variables are now unused, left for review
              bool third_jet_found = false;
              bool subleading_jet_found = false;
            */

            for (std::vector<fastjet::PseudoJet>::iterator jet = sortedPertJet62.begin(); jet != sortedPertJet62.end(); ++jet)
            {
                    if(!leading_jet_found){
                        lj.SetPxPyPzE(jet->px(),jet->py(),jet->pz(),jet->e());  //only saves leading jet
                        leading_jet_found = true;
                    }
                    else { //leading_jet_found is 'true' and subleading_jet_found is 'false'
                        slj.SetPxPyPzE(jet->px(),jet->py(),jet->pz(),jet->e());  //only saves leading jet
                        /* the following is set but then never used
                          subleading_jet_found = true; 
                        */
                        break;//exits loop if leading_jet_found
                    }
                    /** The following code is unreachable in the loop but left commented for further review
                    else if(leading_jet_found && subleading_jet_found && !third_jet_found){
                        tlj.SetPxPyPzE(jet->px(),jet->py(),jet->pz(),jet->e());  //only saves leading jet
                        third_jet_found = true;
                        break;
                    }
                    **/
            } //for 
        }
    
        
        if (lj.Pt() >= pt_min*1000.) return true;
        
        return false;

    }

}