/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PhysicsConfigurationHelper.h"
#include "CLHEP/Units/PhysicalConstants.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "CxxUtils/checker_macros.h"

PhysicsConfigurationHelper::PhysicsConfigurationHelper()
{
  G4cout << "PhysicsConfigurationHelper constructor: start" << G4endl;

  std::map<G4String,G4double> parameters;
  ReadInPhysicsParameters(parameters);
  if (parameters["Resonant"]!=0.) m_resonant=true;
  m_ek_0 = parameters["ResonanceEnergy"]*CLHEP::GeV;
  m_gamma = parameters["Gamma"]*CLHEP::GeV;
  m_amplitude = parameters["Amplitude"]*CLHEP::millibarn;
  m_xsecmultiplier = parameters["XsecMultiplier"];
  m_suppressionfactor = parameters["ReggeSuppression"];
  m_hadronlifetime = parameters["HadronLifeTime"];
  m_mixing = parameters["Mixing"];
  if (parameters["ReggeModel"]!=0.) m_reggemodel=true;
  m_doDecays=parameters["DoDecays"];

  G4cout<<"Read in physics parameters:"<<G4endl;
  G4cout<<"Resonant = "<< m_resonant <<G4endl;
  G4cout<<"ResonanceEnergy = "<<m_ek_0/CLHEP::GeV<<" GeV"<<G4endl;
  G4cout<<"XsecMultiplier = "<<m_xsecmultiplier<<G4endl;
  G4cout<<"Gamma = "<<m_gamma/CLHEP::GeV<<" GeV"<<G4endl;
  G4cout<<"Amplitude = "<<m_amplitude/CLHEP::millibarn<<" millibarn"<<G4endl;
  G4cout<<"ReggeSuppression = "<<100*m_suppressionfactor<<" %"<<G4endl;
  G4cout<<"HadronLifeTime = "<<m_hadronlifetime;
  if (m_doDecays) G4cout<<" ns"<<G4endl;
  else G4cout<<" s"<<G4endl;
  G4cout<<"ReggeModel = "<< m_reggemodel <<G4endl;
  G4cout<<"Mixing = "<< m_mixing*100 <<" %"<<G4endl;
  G4cout<<"DoDecays = "<< m_doDecays << G4endl;

  if ((!m_doDecays && m_hadronlifetime>0.) ||
      (m_doDecays && m_hadronlifetime<=0.) ){
    G4cout << "WARNING: Inconsistent treatment of R-Hadron properties! Lifetime of " << m_hadronlifetime
           << " and doDecays= " << m_doDecays << G4endl;
  }

  return;
}


const PhysicsConfigurationHelper* PhysicsConfigurationHelper::Instance()
{
  static const PhysicsConfigurationHelper instance;
  return &instance;
}


void PhysicsConfigurationHelper::ReadInPhysicsParameters(std::map<G4String,G4double>&  parameters) const
  {
    parameters["Resonant"]=0.;
    parameters["ResonanceEnergy"]=0.;
    parameters["XsecMultiplier"]=1.;
    parameters["Gamma"]=0.;
    parameters["Amplitude"]=0.;
    parameters["ReggeSuppression"]=0.;
    parameters["HadronLifeTime"]=0.;
    parameters["ReggeModel"]=0.;
    parameters["Mixing"]=0.;
    parameters["DoDecays"]=0;

    std::ifstream physics_stream ("PhysicsConfiguration.txt");
    G4String line;
    char** endptr=0;
    while (getline(physics_stream,line)) {
      std::vector<G4String> tokens;
      //Getting a line
      ReadAndParse(line,tokens,"=");
      G4String key = tokens[0];
      G4double val = strtod(tokens[1],endptr);
      parameters[key]=val;
    }
    physics_stream.close();
  }


void PhysicsConfigurationHelper::ReadAndParse(const G4String& str,
                                   std::vector<G4String>& tokens,
                                   const G4String& delimiters) const
{
  // Skip delimiters at beginning.
  G4String::size_type lastPos = str.find_first_not_of(delimiters, 0);
  if (lastPos==G4String::npos) return;

  // Find first "non-delimiter".
  G4String::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (G4String::npos != pos || G4String::npos != lastPos)
    {
      //Skipping leading / trailing whitespaces
      G4String temp = str.substr(lastPos, pos - lastPos);
      while(temp.c_str()[0] == ' ') temp.erase(0,1);
      while(temp[temp.size()-1] == ' ') temp.erase(temp.size()-1,1);

      // Found a token, add it to the vector.
      tokens.push_back(temp);

      // Skip delimiters.  Note the "not_of"
      lastPos = str.find_first_not_of(delimiters, pos);
      if (lastPos==G4String::npos) continue;

      // Find next "non-delimiter"
      pos = str.find_first_of(delimiters, lastPos);
    }
}
