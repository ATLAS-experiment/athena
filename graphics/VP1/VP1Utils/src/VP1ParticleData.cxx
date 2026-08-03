/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1ParticleData                   //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: March 2008                               //
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1Utils/VP1ParticleData.h"
#include "VP1Base/VP1Msg.h"

#include "GeneratorModules/GenData.h"
#include "TruthUtils/HepMCHelpers.h"

#include <cstdlib>

//____________________________________________________________________
class VP1ParticleData::Imp {
public:
  static GenData m_genData;
  static std::map<int,double> m_particleAbsPDGCodeToMass;
  static std::map<int,double> m_particlePDGCodeToCharge;
  static std::map<int,QString> m_particleAbsPDGCodeToName;
  static const double m_badValue;
  static const QString m_badName;
};

GenData VP1ParticleData::Imp::m_genData;
std::map<int,double> VP1ParticleData::Imp::m_particleAbsPDGCodeToMass;
std::map<int,double> VP1ParticleData::Imp::m_particlePDGCodeToCharge;
std::map<int,QString> VP1ParticleData::Imp::m_particleAbsPDGCodeToName;
const double VP1ParticleData::Imp::m_badValue = -1.0e99;
const QString VP1ParticleData::Imp::m_badName = "_Bad_Name_";

//____________________________________________________________________
double VP1ParticleData::particleMass( const int& pdgcode, bool& ok )
{
  const int absPdgCode = std::abs(pdgcode);
  std::map<int,double>::const_iterator it = Imp::m_particleAbsPDGCodeToMass.find(absPdgCode);
  if (it!=Imp::m_particleAbsPDGCodeToMass.end()) {
    ok = it->second != Imp::m_badValue;
    return ok ? it->second : 0;
  }
  const auto mOpt = Imp::m_genData.particleMass(absPdgCode);
  const double m = mOpt.value_or(Imp::m_badValue);

  Imp::m_particleAbsPDGCodeToMass[absPdgCode] = m;
  ok = m != Imp::m_badValue;
  return m;
}

//____________________________________________________________________
double VP1ParticleData::particleCharge( const int& pdgcode, bool& ok )
{
  if (pdgcode==22) {
    ok = true;
    return 0.0;
  }
  std::map<int,double>::const_iterator it = Imp::m_particlePDGCodeToCharge.find(pdgcode);
  if (it!=Imp::m_particlePDGCodeToCharge.end()) {
    ok = it->second != Imp::m_badValue;
    return ok ? it->second : 0;
  }
  const double c = MC::isValid(pdgcode) ? MC::charge(pdgcode) : Imp::m_badValue;
  if (c == Imp::m_badValue && VP1Msg::verbose()) {
    VP1Msg::messageVerbose("VP1ParticleData WARNING: Invalid PDG code for charge lookup pdgcode="+QString::number(pdgcode));
  }

  Imp::m_particlePDGCodeToCharge[pdgcode] = c;
  ok = c != Imp::m_badValue;
  return c;
}

//____________________________________________________________________
QString VP1ParticleData::particleName( const int& pdgcode, bool& ok )
{
  std::map<int,QString>::const_iterator it = Imp::m_particleAbsPDGCodeToName.find(pdgcode);
  if (it!=Imp::m_particleAbsPDGCodeToName.end()) {
    ok = (it->second != Imp::m_badName);
    return ok ? it->second : "";
  }

  QString name;
  switch (pdgcode) {
  case   21: name = "gluon"; break;
  case   22: name = "photon"; break;
  case  -11: name = "e+"; break;
  case   11: name = "e-"; break;
  case  -13: name = "mu+"; break;
  case   13: name = "mu-"; break;
  case  -15: name = "tau+"; break;
  case   15: name = "tau-"; break;
  case -211: name = "pi-"; break;
  case  211: name = "pi+"; break;
  case    1: name = "d"  ; break;
  case    2: name = "u"  ; break;
  case    3: name = "s"  ; break;
  case    4: name = "c"  ; break;
  case    5: name = "b"  ; break;
  case    6: name = "t"  ; break;
  case   -1: name = "dbar"  ; break;
  case   -2: name = "ubar"  ; break;
  case   -3: name = "sbar"  ; break;
  case   -4: name = "cbar"  ; break;
  case   -5: name = "bbar"  ; break;
  case   -6: name = "tbar"  ; break;
  case   92: name = "frag string" ; break;
  default:
    break;
  }

  if (name.isEmpty()) {
    const auto nameOpt = Imp::m_genData.particleName(std::abs(pdgcode));
    if (nameOpt)
      name = (pdgcode<0?"anti-":"")+QString::fromStdString(*nameOpt);//fixme: anything [[:alpha:]](+|-) we
                                                                  //    change + and -
    else
      name = Imp::m_badName;
  }

  Imp::m_particleAbsPDGCodeToName[pdgcode] = name;
  ok = (name != Imp::m_badName);
  return name;
}
