/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


// C/C++
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <mutex>

// Local
#include "LeptonTaggers/VarHolder.h"

Prompt::VarHolder::VarHolder() {
    registerAllVars();
}

//======================================================================================================
bool Prompt::VarHolder::registerVar(Prompt::Def::Var var, const std::string &name)
{
  for(const VarMap::value_type &v: m_gPromptVars) {
    if(name == v.second) {
      asg::msgUserCode::ANA_MSG_WARNING("Def::registerVar - WARNING - variable with this name already exists: var=" << var << ", name=" << name);
      return false;
    }
  }

  std::pair<VarMap::iterator, bool> vit = m_gPromptVars.insert(VarMap::value_type(var, name));

  if(!vit.second) {
    asg::msgUserCode::ANA_MSG_WARNING("Def::registerVar - WARNING - variable with this key already exists: var=" << var << ", name=" << name);
    return false;
  }

  return true;
}

//======================================================================================================
Prompt::Def::Var Prompt::VarHolder::registerDynamicVar(const std::string &name)
{
  unsigned last_key = Prompt::Def::NONE;

  for(const Prompt::VarMap::value_type &v: m_gPromptVars) {
    if(name == v.second) {
      asg::msgUserCode::ANA_MSG_WARNING("Def::registerDynamicVar - variable with this name already exists: " << name);

      return v.first;
    }

    last_key = std::max<unsigned>(last_key, v.first);
  }

  const Def::Var new_key = static_cast<Def::Var>(last_key+1);


  if(!registerVar(new_key, name)) {
    asg::msgUserCode::ANA_MSG_WARNING("Def::registerDynamicVar - WARNING - failed to register variable name=" << name);
    return Prompt::Def::NONE;
  }

  return new_key;
}

//======================================================================================================
void Prompt::VarHolder::getAllVarEnums()
{
  if(m_varEnums.empty()) {
    for(const Prompt::VarMap::value_type &v: m_gPromptVars) {
      m_varEnums.push_back(v.first);
    }
  }
}

//======================================================================================================
std::string Prompt::VarHolder::convert2Str(const Prompt::Def::Var var) const
{
  const VarMap::const_iterator vit = m_gPromptVars.find(var);

  if(vit == m_gPromptVars.end()) {
    asg::msgUserCode::ANA_MSG_WARNING("Def::convert2Str - WARNING - unknown variable: " << var);
    return "UNKNOWN";
  }

  return vit->second;
}

//======================================================================================================
Prompt::Def::Var Prompt::VarHolder::convert2Var (const std::string &var)
{
  for (const VarMap::value_type &v: m_gPromptVars) {
    if(var == v.second) {
      return v.first;
    }
  }

  asg::msgUserCode::ANA_MSG_WARNING("Def::convert2Var - WARNING - unknown variable: " << var);

  return Prompt::Def::NONE;
}

//======================================================================================================
Prompt::Def::Var Prompt::VarHolder::convert2Var(const uint32_t key)
{
  getAllVarEnums();

  //
  // Find matching enum by value
  //
  for(Prompt::Def::Var var: m_varEnums) {
    if(static_cast<uint32_t>(var) == key) {
      return var;
    }
  }

  asg::msgUserCode::ANA_MSG_WARNING("Def::convert2Var - WARNING - unknown key: " << key);

  return Prompt::Def::NONE;
}

//======================================================================================================
std::vector<Prompt::Def::Var> Prompt::VarHolder::readVars(
  const std::string &config
)
{
  //
  // Read vector of variable names and convert to Var enums
  //
  std::vector<std::string> keys;
  Def::StringTok(keys, config, ", ");

  std::vector<Prompt::Def::Var> vars;

  for(const std::string &key: keys) {
    const Prompt::Def::Var var = convert2Var(key);
    if(var != Prompt::Def::NONE) {
      vars.push_back(var);
    }
    else {
      asg::msgUserCode::ANA_MSG_WARNING("Prompt::Def::readVars - unknown variable name: " << key);
    }
  }

  return vars;
}

//======================================================================================================
std::vector<Prompt::Def::Var> Prompt::VarHolder::readVectorVars(
  const std::vector<std::string> &keys
)
{
  //
  // Read vector of variable names and convert to Var enums
  //
  std::vector<Prompt::Def::Var> vars;

  for(const std::string &key: keys) {
    const Prompt::Def::Var var = convert2Var(key);
    if(var != Prompt::Def::NONE) {
      vars.push_back(var);
    }
    else {
      asg::msgUserCode::ANA_MSG_WARNING("Prompt::Def::readVars - unknown variable name: " << key);
    }
  }

  return vars;
}

//======================================================================================================
std::string Prompt::VarHolder::asStr(const Prompt::Def::Var var)
{
  return convert2Str(var);
}

//======================================================================================================
std::string Prompt::VarHolder::asStr(const uint32_t key, const double val)
{
  std::stringstream s;

  const Prompt::Def::Var var = convert2Var(key);

  if(var != Prompt::Def::NONE) {
    s << asStr(var) << ": " << val;
  }
  else {
    s << var << ": " << val;
  }

  return s.str();
}

//======================================================================================================
void Prompt::Def::StringTok(std::vector<std::string>& ls,
          const std::string& str,
          const std::string& tok)
{
  //======================================================================
  // Split a long string into a set of shorter strings spliting along
  // divisions makers by the characters listed in the token string
  //======================================================================
  const std::string::size_type S = str.size();
  std::string::size_type  i = 0;

  while (i < S) {
    // eat leading whitespace
    while (i < S && tok.find(str[i]) != std::string::npos) {
      ++i;
    }
    if (i == S) break;  // nothing left but WS

    // find end of word
    std::string::size_type  j = i+1;
    while (j < S && tok.find(str[j]) == std::string::npos) {
      ++j;
    }

    // add word
    ls.push_back(str.substr(i,j-i));

    // set up for next loop
    i = j+1;
  }
}

//======================================================================================================
int Prompt::VarHolder::registerAllVars()
{
  using namespace Prompt::Def;

  int result = 0;

  result += registerVar( NONE,            "NONE");
  result += registerVar( TrackJetNTrack,  "TrackJetNTrack");
  result += registerVar( DRlj,            "DRlj");
  result += registerVar( PtFrac,          "PtFrac");
  result += registerVar( PtRel,           "PtRel");

  // track VarHolder
  result += registerVar( LepTrackDR,            "LepTrackDR");
  result += registerVar( Pt,                    "Pt");
  result += registerVar( AbsEta,                "AbsEta");
  result += registerVar( NumberOfPIXHits,       "NumberOfPIXHits");
  result += registerVar( NumberOfSCTHits,       "NumberOfSCTHits");
  result += registerVar( NumberOfSiHits,        "NumberOfSiHits");
  result += registerVar( NumberOfSharedSiHits,  "NumberOfSharedSiHits");
  result += registerVar( NumberOfSiHoles,       "NumberOfSiHoles");
  result += registerVar( NumberOfPixelHoles,    "NumberOfPixelHoles");
  result += registerVar( TrackJetDR,            "TrackJetDR");
  result += registerVar( TrackPtOverTrackJetPt, "TrackPtOverTrackJetPt");
  result += registerVar( Z0Sin,                 "Z0Sin");
  result += registerVar( D0Sig,                 "D0Sig");

  // PromptLeptonImproved
  result += registerVar( MVAXBin,                                                 "MVAXBin");
  result += registerVar( RawPt,                                                   "RawPt");
  result += registerVar( PromptLeptonRNN_prompt,                                  "PromptLeptonRNN_prompt");
  result += registerVar( CaloClusterERel,                                         "CaloClusterERel");
  result += registerVar( Topoetcone30rel,                                         "topoetcone30rel");
  result += registerVar( Ptvarcone30rel,                                          "ptvarcone30rel");
  result += registerVar( Ptvarcone30_TightTTVA_pt500rel,                          "ptvarcone30_TightTTVA_pt500rel");
  result += registerVar( CaloClusterSumEtRel,                                     "CaloClusterSumEtRel");
  result += registerVar( CandVertex_normDistToPriVtxLongitudinalBest,             "CandVertex_normDistToPriVtxLongitudinalBest");
  result += registerVar( CandVertex_normDistToPriVtxLongitudinalBest_ThetaCutVtx, "CandVertex_normDistToPriVtxLongitudinalBest_ThetaCutVtx");
  result += registerVar( CandVertex_NPassVtx,                                     "CandVertex_NPassVtx");

  return result;
}

