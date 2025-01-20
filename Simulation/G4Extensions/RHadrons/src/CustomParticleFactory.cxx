/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <iterator>
#include <map>
#include <set>

#include "PhysicsConfigurationHelper.h"
#include "CustomParticle.h"
#include "CustomParticleFactory.h"

#include "TruthUtils/HepMCHelpers.h"

#include "G4DecayTable.hh"
#include "G4ParticleTable.hh"
#include "G4PhaseSpaceDecayChannel.hh"

bool CustomParticleFactory::isCustomParticle(G4ParticleDefinition *particle)
{
  static const std::set<G4ParticleDefinition *> particles = load();
  return (particle!=nullptr && particles.find(particle)!=particles.end());
}

void CustomParticleFactory::loadCustomParticles()
{
  // tickle the loading of the particles if it wasn't done yet
  isCustomParticle(nullptr);
}

std::set<G4ParticleDefinition *> CustomParticleFactory::load()
{
  // Reading decays from file
  std::map< std::string, std::vector<std::vector<std::string>* > > decayMap;
  std::ifstream decayFile("decays.txt");
  std::string line;
  while (getline(decayFile,line)) {
    std::istringstream is(line);
    std::vector<std::string>* txtvec = new std::vector<std::string>(std::istream_iterator<std::string>(is),std::istream_iterator<std::string>());
    const std::string name = (*txtvec)[0];
    try {
      decayMap.at(name).push_back(txtvec);
    }
    catch (const::std::out_of_range& e) {
      std::vector<std::vector<std::string>* > decays;
      decays.push_back(txtvec);
      decayMap[name] = decays;
    }
  }
  decayFile.close();

  // the existing particle table
  G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();

  std::set<G4ParticleDefinition *> particles;

  std::ifstream configFile("particles.txt");
  G4String pType="custom";
  G4String pSubType="";
  G4double spectatormass = 0;
  G4ParticleDefinition* spectator = 0;
  bool first = true;
  double mass;
  int pdgCode;
  bool stable{true};
  G4double lifetime{-1.0};
  std::string name;
  const PhysicsConfigurationHelper *parameters = PhysicsConfigurationHelper::Instance();

  // This should be compatible IMO to SLHA
  while (getline(configFile,line)) {
    G4cout << "-------------------------------------------------" << G4endl;
    std::string::size_type beg_idx,end_idx;

    beg_idx = line.find_first_not_of("\t #");
    if (beg_idx > 0 && line[beg_idx-1] == '#') continue;
    end_idx = line.find_first_of( "\t ", beg_idx);
    if (end_idx == std::string::npos) continue;
    char *endptr;
    const std::string pdgCodeString = line.substr( beg_idx, end_idx - beg_idx );
    pdgCode  = strtol(pdgCodeString.c_str(), &endptr, 0);
    if (endptr[0] != '\0') {
      throw std::invalid_argument("Could not convert string to int: " + line.substr( beg_idx, end_idx - beg_idx ));
    }

    beg_idx = line.find_first_not_of("\t ",end_idx);
    end_idx = line.find_first_of( "\t #", beg_idx);
    if (end_idx == std::string::npos) continue;
    mass  = atof(line.substr( beg_idx, end_idx - beg_idx ).c_str());

    beg_idx = line.find_first_not_of("\t# ",end_idx);
    end_idx = line.length();
    if (beg_idx==std::string::npos) beg_idx = end_idx;
    name = line.substr( beg_idx, end_idx - beg_idx );
    while (name.c_str()[0] == ' ') { name.erase(0,1); }
    while (name[name.size()-1] == ' ') { name.erase(name.size()-1,1); }

    if (abs(pdgCode) / 1000000 == 0){
      G4cout << "Pdg code too low " << pdgCode << " "<<abs(pdgCode) / 1000000  << std::endl;
      continue;
    }

    stable = !(parameters->DoDecays() == 1 || (decayMap.contains(name) && name.find("cloud")>name.size() ));
    lifetime = stable ? -1.0 : parameters->Lifetime();

    pType="custom";
    if (MC::isRHadron(pdgCode)) pType = "rhadron";
    if (MC::isSlepton(pdgCode)) pType = "sLepton";

    G4cout<<"pType: "<<pType<<G4endl;
    G4cout<<"Charge of "<<name<<" is "<<MC::charge(pdgCode)<<G4endl;

    G4ParticleDefinition* previousDefinition = theParticleTable->FindParticle(pdgCode);
    // if the particle has somehow already been added to the G4ParticleTable remove it
    if (previousDefinition) {
      G4cout << "Found an existing G4ParticleDefinition for " << name << "(" << pdgCode << ") in G4ParticleTable. Assuming that we want the new definition, so removing previous definition! May cause issues elsewhere though..." << G4endl;
      previousDefinition->DumpTable();
      theParticleTable->Remove(previousDefinition);
      delete previousDefinition;
    }
    //    Arguments for constructor are as follows
    //               name             mass          width         charge
    //             2*spin           parity  C-conjugation
    //          2*Isospin       2*Isospin3       G-parity
    //               type    lepton number  baryon number   PDG encoding
    //             stable         lifetime    decay table
    //             shortlived      subType    anti_encoding
    CustomParticle *particle  = new CustomParticle(
                                                   name,           mass * CLHEP::GeV ,        0.0*CLHEP::MeV,       CLHEP::eplus * MC::charge(pdgCode),
                                                   MC::spin2(pdgCode),              +1,             0,
                                                   0,              0,             0,
                                                   pType,               0,            +1, pdgCode,
                                                   stable,            lifetime,          nullptr );
    if (pType=="custom") {
      spectatormass = mass;
      spectator = particle;
    }
    if (first) {
      first = false;
      spectatormass = mass;
      spectator = particle;
    } else {
      G4String cloudname = name+"cloud";
      G4String cloudtype = pType+"cloud";
      G4double cloudmass = mass-spectatormass;
      const bool cloudstable = !(parameters->DoDecays() == 1 || decayMap.contains(cloudname));
      const G4double cloudlifetime = cloudstable ? -1.0 : parameters->Lifetime();

      std::unique_ptr<G4ParticleDefinition> tmpParticle  = std::unique_ptr<CustomParticle>( new CustomParticle(
                                                                                            cloudname,           cloudmass * CLHEP::GeV ,        0.0*CLHEP::MeV,  0 ,
                                                                                            0,              +1,             0,
                                                                                            0,              0,             0,
                                                                                            cloudtype,               0,            +1, 0,
                                                                                            cloudstable,         cloudlifetime,          nullptr ) );
      particle->SetCloud(tmpParticle);
      particle->SetSpectator(spectator);

      G4cout << name << " being assigned cloud " << particle->GetCloud()->GetParticleName() << " and spectator " << particle->GetSpectator()->GetParticleName() << G4endl;
      G4cout << "Masses: "
             << particle->GetPDGMass()/CLHEP::GeV << " Gev, "
             << particle->GetCloud()->GetPDGMass()/CLHEP::GeV << " GeV and "
             << particle->GetSpectator()->GetPDGMass()/CLHEP::GeV << " GeV."
             << G4endl;
    }

    particles.insert(particle);
  }
  configFile.close();
  G4cout << "-------------------------------------------------" << G4endl;

  // Looping over custom particles to add decays
  for (G4ParticleDefinition *part : particles) {
    name=part->GetParticleName();
    if ( decayMap.contains(name) ) {
      std::vector<std::vector<std::string>* >& mydecays = decayMap.at(name);
      // Did I get any decays?
      if (mydecays.empty()) { continue; } // Should not happen.
      int ndec=mydecays.size();
      G4DecayTable* table = new G4DecayTable();
      G4VDecayChannel** mode = new G4VDecayChannel*[ndec]{};
      for (int i=0;i!=ndec;i++) {
        std::vector<std::string> thisdec=*(mydecays[i]);//deliberate copy
        if (thisdec.empty()) continue;
        const double branch = std::stod(thisdec.back()); // Reading branching ratio
        thisdec.pop_back(); // Removing the number from the vector
        for (const auto & thisString:thisdec) G4cout<<thisString<<G4endl;
        if (thisdec.size()==3) {
          mode[i] = new G4PhaseSpaceDecayChannel(thisdec[0],branch,2,thisdec[1],thisdec[2]);
        } else if (thisdec.size()==4) {
          mode[i] = new G4PhaseSpaceDecayChannel(thisdec[0],branch,3,thisdec[1],thisdec[2],thisdec[3]);
        } else {
          G4cout<<"Decay chain invalid!!!"<<std::endl;
        }
      }
      for (G4int index=0; index <ndec; index++ ) { table->Insert(mode[index]); }
      delete [] mode;
      part->SetDecayTable(table);
    }
    else { continue; }
  }
  return particles;
}
