/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RHADRONS_G4PROCESSHELPER_HH
#define RHADRONS_G4PROCESSHELPER_HH 1

#include "CxxUtils/checker_macros.h"
#include "PhysicsConfigurationHelper.h"

#include"globals.hh"
#include"G4ParticleDefinition.hh"
#include"G4DynamicParticle.hh"
#include"G4Element.hh"
#include"G4Track.hh"
#include<vector>
#include<map>

//Typedefs just made to make life easier :-)
typedef std::vector<G4int> ReactionProduct;
typedef std::vector<ReactionProduct > ReactionProductList;
typedef std::map<G4int , ReactionProductList> ReactionMap;

class G4ParticleTable;

class G4ProcessHelper {

public:
  static const G4ProcessHelper* Instance();

  G4bool ApplicabilityTester(const G4ParticleDefinition& aPart) const;

  G4double GetInclusiveCrossSection(const G4DynamicParticle *aParticle,
                                    const G4Element *anElement) const;

  //Make sure the element is known (for n/p-decision)
  ReactionProduct GetFinalState(const G4Track& aTrack,G4ParticleDefinition*& aTarget) const;

  void SetCustomParticleLifeTime(G4ParticleDefinition* aPart, bool decaysIdentified) const;

protected:
  G4ProcessHelper();
  G4ProcessHelper(const G4ProcessHelper&);
  G4ProcessHelper& operator= (const G4ProcessHelper&);

private:

  void ReadAndParse(const G4String& str,
                    std::vector<G4String>& tokens,
                    const G4String& delimiters = " ") const;

  // Version where we know if we baryonize already
  ReactionProduct GetFinalStateInternal(const G4Track& aTrack,G4ParticleDefinition*& aTarget, const bool baryonize_failed) const;

  G4double Regge(const double boost) const;
  G4double Pom(const double boost) const;

  G4double PhaseSpace(const ReactionProduct& aReaction,const G4ParticleDefinition& aTarget,const G4DynamicParticle* aDynamicParticle) const;

  G4double ReactionProductMass(const ReactionProduct& aReaction,const G4ParticleDefinition& aTarget,const G4DynamicParticle* aDynamicParticle) const;

  G4bool ReactionIsPossible(const ReactionProduct& aReaction,const G4ParticleDefinition& aTarget,const G4DynamicParticle* aDynamicParticle) const;
  G4bool ReactionGivesBaryon(const ReactionProduct& aReaction) const;

  G4ParticleDefinition* theProton{};
  G4ParticleDefinition* theNeutron{};
  G4ParticleDefinition* theRmesoncloud{};
  G4ParticleDefinition* theRbaryoncloud{};

  //Map of applicable particles
  std::map<const G4ParticleDefinition*,G4bool> m_knownParticles;

  const PhysicsConfigurationHelper *m_parameters{};

  //Proton-scattering processes
  ReactionMap pReactionMap;

  //Neutron-scattering processes
  ReactionMap nReactionMap;

  G4ParticleTable* particleTable{};

};
#endif // RHADRONS_G4PROCESSHELPER_HH
