/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ======================================================================
// DecayGraphHelper.h
// James.Catmore@cern.ch
// Helper struct which implements two simple recursive functions;
// descends or ascends through a decay from a head particle provided by
// the user, and records all particles and vertices it encounters in the
// masks
//
// Also contains functions shared by TruthDressing tool and
// TruthIsolationTool.
// ======================================================================
#pragma once

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "MCTruthClassifier/MCTruthClassifier.h"
#include "TruthUtils/HepMCHelpers.h"
#include "TruthUtils/MagicNumbers.h"
#include <unordered_set>

namespace DerivationFramework {
    struct DecayGraphHelper {
        DecayGraphHelper() {
        }
        
        // Immediate relatives (parents, siblings, children)
        void immediateRelatives(const xAOD::TruthParticle* pHead,
                                std::vector<bool> &particleMask,
                                std::vector<bool> &vertexMask,
                                bool keepHadVtx) {
            
            // Save the particle position in the mask
            int headIndex = pHead->index();
            particleMask[headIndex] = true;
            
            // Get the production and decay vertex for the particle
            const xAOD::TruthVertex* decayVtx(0);
            const xAOD::TruthVertex* prodVtx(0);
            
            if (pHead->hasDecayVtx()) {
                decayVtx = pHead->decayVtx();
                int decayIndex = decayVtx->index();
                vertexMask[decayIndex] = true;
                unsigned int nParents = decayVtx->nIncomingParticles();
                unsigned int nChildren = decayVtx->nOutgoingParticles();
                
                // Hadronization vertex? (quarks,gluons -> hadrons)
                bool isHadVtx = true;
                for (unsigned int i=0; i<nParents; ++i) {
                    if (decayVtx->incomingParticle(i)==nullptr) continue; 
                    int idabs = std::abs(decayVtx->incomingParticle(i)->pdgId());
                    isHadVtx = isHadVtx && (idabs<6 || idabs==21);
                }
                for (unsigned int i=0; i<nChildren; ++i) {
                    if (decayVtx->outgoingParticle(i)==nullptr) continue;
                    int idabs = std::abs(decayVtx->outgoingParticle(i)->pdgId());
                    isHadVtx = isHadVtx && ((idabs>=80 && idabs<1000000) ||
                                            idabs>9000000);
                }
                
                if( !isHadVtx || keepHadVtx ){
                    // Get the particles leaving the decay vertex (CHILDREN)
                    for (unsigned int i=0; i<nChildren; ++i) {
                        if (decayVtx->outgoingParticle(i)==nullptr) continue; 
                        int childIndex = decayVtx->outgoingParticle(i)->index();
                        particleMask[childIndex] = true;
                    }
                }
            }
            
            if (pHead->hasProdVtx()) {
                prodVtx = pHead->prodVtx();
                int prodIndex = prodVtx->index();
                vertexMask[prodIndex] = true;
                unsigned int nParents = prodVtx->nIncomingParticles();
                unsigned int nSiblings = prodVtx->nOutgoingParticles();
                unsigned int nChildren = nSiblings;
                
                // Hadronization vertex? (quarks,gluons -> hadrons)
                bool isHadVtx = true;
                for (unsigned int i=0; i<nParents; ++i) {
                    if (prodVtx->incomingParticle(i)==nullptr) continue;
                    int idabs = std::abs(prodVtx->incomingParticle(i)->pdgId());
                    isHadVtx = isHadVtx && (idabs<6 || idabs==21);
                }
                for (unsigned int i=0; i<nChildren; ++i) {
                    if (prodVtx->outgoingParticle(i)==nullptr) continue;
                    int idabs = std::abs(prodVtx->outgoingParticle(i)->pdgId());
                    isHadVtx = isHadVtx && ((idabs>=80 && idabs<1000000) ||
                                            idabs>9000000);
                }
                
                if( !isHadVtx || keepHadVtx ){
                    // Get the particles entering the production vertex (PARENTS)
                    for (unsigned int i=0; i<nParents; ++i) {
                        if (prodVtx->incomingParticle(i)==nullptr) continue; 
                        int parentIndex = prodVtx->incomingParticle(i)->index();
                        particleMask[parentIndex] = true;
                    }
                    // Get the particles leaving the production vertex (SIBLINGS)
                    for (unsigned int i=0; i<nSiblings; ++i) {
                        if (prodVtx->outgoingParticle(i)==nullptr) continue;
                        int siblingIndex = prodVtx->outgoingParticle(i)->index();
                        particleMask[siblingIndex] = true;
                    }
                }
            }
            return;
        }
        
        // descendants: starting from particle (simple listing version)
        void descendants(const xAOD::TruthParticle* pHead,
                         std::vector<int> &particleList,
                         std::unordered_set<int> &encounteredUniqueIDs) {
            
            // Check that this unique ID hasn't been seen before (e.g. we are in a loop)
            std::unordered_set<int>::const_iterator found = encounteredUniqueIDs.find(HepMC::uniqueID(pHead));
            if (found!=encounteredUniqueIDs.end()) return;
            encounteredUniqueIDs.insert(HepMC::uniqueID(pHead));
            
            // Get the decay vertex
            const xAOD::TruthVertex* decayVtx(0);
            if (pHead->hasDecayVtx()) {decayVtx = pHead->decayVtx();}
            else {return;}
            
            // Save the PDG ID number of the particle
            particleList.push_back(pHead->pdgId());
            
            // Get children particles and self-call
            int nChildren = decayVtx->nOutgoingParticles();
            for (int i=0; i<nChildren; ++i) {
                if (decayVtx->outgoingParticle(i)==nullptr) continue;
                descendants(decayVtx->outgoingParticle(i),particleList,encounteredUniqueIDs);
            } 
            return;
        }
        
        // descendants: starting from particle
        void descendants(const xAOD::TruthParticle* pHead,
                         std::vector<bool> &particleMask,
                         std::vector<bool> &vertexMask,
                         std::unordered_set<int> &encounteredUniqueIDs,
                         bool includeGeant) {
            
            // Check that the particle exists
            if (pHead==nullptr) return;           
 
            // Check that this unique ID hasn't been seen before (e.g. we are in a loop)
            std::unordered_set<int>::const_iterator found = encounteredUniqueIDs.find(HepMC::uniqueID(pHead));
            if (found!=encounteredUniqueIDs.end()) return;
            encounteredUniqueIDs.insert(HepMC::uniqueID(pHead));
            
            // Save the particle position in the mask
            // If user doesn't want Geant, check and reject Geant particles
            if (!includeGeant && HepMC::is_simulation_particle(pHead) ) return;
            int headIndex = pHead->index();
            particleMask[headIndex] = true;
            
            // Get the decay vertex
            const xAOD::TruthVertex* decayVtx(0);
            if (pHead->hasDecayVtx()) {decayVtx = pHead->decayVtx();}
            else {return;}
            
            // Get children particles and self-call
            int  nChildren = decayVtx->nOutgoingParticles();
            bool saveVertex = false;
            for (int i=0; i<nChildren; ++i) {
              if (decayVtx->outgoingParticle(i)==nullptr) continue;
              descendants(decayVtx->outgoingParticle(i),particleMask,vertexMask,encounteredUniqueIDs,includeGeant);
              saveVertex = saveVertex || includeGeant || !(HepMC::is_simulation_particle(decayVtx->outgoingParticle(i)));
            }

            // Save the decay vertex
            if ( saveVertex ) {
              int vtxIndex = decayVtx->index();
              vertexMask[vtxIndex] = true;
            }
            return;
        }
        
        // ancestors: starting from particle
        void ancestors(const xAOD::TruthParticle* pHead,
                       std::vector<bool> &particleMask,
                       std::vector<bool> &vertexMask,
                       std::unordered_set<int> &encounteredUniqueIDs ) {
            
            // Check that the head particle exists
            if (pHead==nullptr) return;

            // Check that this unique ID hasn't been seen before (e.g. we are in a loop)
            std::unordered_set<int>::const_iterator found = encounteredUniqueIDs.find(HepMC::uniqueID(pHead));
            if (found!=encounteredUniqueIDs.end()) return;
            encounteredUniqueIDs.insert(HepMC::uniqueID(pHead));
            
            // Save particle position in the mask
            int headIndex = pHead->index();
            particleMask[headIndex] = true;
            
            // Get the production vertex
            const xAOD::TruthVertex* prodVtx(0);
            if (pHead->hasProdVtx()) {prodVtx = pHead->prodVtx();}
            else {return;}
            
            // Save the production vertex
            int vtxIndex = prodVtx->index();
            vertexMask[vtxIndex] = true;
            
            // Get children particles and self-call
            int nParents = prodVtx->nIncomingParticles();
            for (int i=0; i<nParents; ++i) ancestors(prodVtx->incomingParticle(i),particleMask,vertexMask,encounteredUniqueIDs);
            return;
        }
        
        bool constructListOfFinalParticles(const xAOD::TruthParticleContainer* allParticles,
                                           std::vector<const xAOD::TruthParticle*> &selectedlist,
                                           const std::vector<int> &pdgId, bool allowFromHadron = false,
                                           bool chargedOnly = false) const {
            //fill the vector selectedlist with only particles with abs(ID) in the list
            //pdgID that are 'good' for dressing: considered stable by the generator, produced during evgen (for the
            //photons), not from hadron decay,
            //skip pdgId check if pdgId is an empty vector,
            //ignore particles coming from hadrons if allowFromHadron=false,
            //only use charged particles if chargedOnly=true
            
            bool skipPdgCheck = (pdgId.size()==0);
            //bypass the hadron veto?
            static const SG::AuxElement::ConstAccessor<unsigned int> acc_class{"Classification"};
            for (xAOD::TruthParticleContainer::const_iterator pItr=allParticles->begin(); pItr!=allParticles->end(); ++pItr) {
                const xAOD::TruthParticle *particle = *pItr;

                if (!MC::isStable(particle) ) continue;                
                if (!skipPdgCheck && find(pdgId.begin(), pdgId.end(), abs(particle->pdgId())) == pdgId.end()) continue;
                
                //ensure particles are not from GEANT
                if ( HepMC::is_simulation_particle(particle)) continue;
                
                //check if we have a neutral particle (threeCharge returns int)
                if (chargedOnly && MC::threeCharge(particle->pdgId()) == 0) continue;
                
                //if we have a particle from hadron decay, and allowFromHadron=false, skip this particle
                if (!allowFromHadron) {
                  if (!acc_class.isAvailable(*particle))  return false;
                  unsigned int result = acc_class(*particle);
                  const bool isPrompt = MCTruthPartClassifier::isPrompt(result, true);
                  if (!isPrompt)  continue;
                }
                
                //good particle, add to list
                selectedlist.push_back(particle);
            }
            return true;
        }
        
    };
}


