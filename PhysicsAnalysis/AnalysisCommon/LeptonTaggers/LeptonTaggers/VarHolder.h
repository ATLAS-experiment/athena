// This is -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PROMPT_VARHOLDER_H
#define PROMPT_VARHOLDER_H

/**********************************************************************************
 * @Package: LeptonTaggers
 * @Class  : VarHolder
 * @Author : Rustem Ospanov
 * @Author : Rhys Roberts
 *
 * @Brief  :
 *
 *  VarHolder is a generic analysis object that holds variables as (int, double)
 *
 **********************************************************************************/

// C/C++
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

// Athena
#include <AsgMessaging/MessageCheck.h>

namespace Prompt
{
  //======================================================================================================
  namespace Def
  {
    enum Var
    {
      NONE = 0,
      TrackJetNTrack,
      DRlj,
      PtFrac,
      PtRel,

      // track VarHolder
      LepTrackDR,
      Pt,
      AbsEta,
      NumberOfPIXHits,
      NumberOfSCTHits,
      NumberOfSiHits,
      NumberOfSharedSiHits,
      NumberOfSiHoles,
      NumberOfPixelHoles,
      TrackJetDR,
      TrackPtOverTrackJetPt,
      Z0Sin,
      D0Sig,

      // PromptLeptonImproved
      Topoetcone30rel,
      Ptvarcone30rel,
      Ptvarcone30_TightTTVA_pt500rel,
      MVAXBin,
      RawPt,
      CaloClusterERel,
      CaloClusterSumEtRel,
      PromptLeptonRNN_prompt,
      CandVertex_normDistToPriVtxLongitudinalBest,
      CandVertex_normDistToPriVtxLongitudinalBest_ThetaCutVtx,
      CandVertex_NPassVtx
    };

    void StringTok(std::vector<std::string>& ls,
       const std::string& str,
       const std::string& tok);
  }

  //======================================================================================================
  class VarEntry
  {
  public:

    VarEntry();
    VarEntry(unsigned key, double value);
    ~VarEntry() {}

    unsigned getKey ()  const { return m_fKey;  }
    double   getVar ()  const { return m_fData; }
    double   getData()  const { return m_fData; }

  private:

    uint32_t m_fKey;  // variable key
    double   m_fData; // variable value
  };

  typedef std::vector<Prompt::VarEntry> VarEntryVec;
  typedef std::map<Def::Var, std::string> VarMap;

  //======================================================================================================
  class VarHolder
  {
  public:
    VarHolder();
    virtual ~VarHolder() {}

    static std::string getObjectType() { return "VarHolder"; }

    bool  replaceVar(unsigned key, double value);
    bool  addVar    (unsigned key, double value);
    bool  delVar    (unsigned key);

    double getVar   (const unsigned key) const;
    bool   getVar   (const unsigned key, double &value) const;
    bool   getVar   (const unsigned key, float  &value) const;

    bool  hasKey(unsigned key) const;
    bool  hasVar(unsigned key) const;

    int registerAllVars();
    bool registerVar(Prompt::Def::Var var, const std::string &name);
    Prompt::Def::Var registerDynamicVar(const std::string &name);

    std::string convert2Str(const Prompt::Def::Var var) const;
    Prompt::Def::Var convert2Var (const std::string &var);
    Prompt::Def::Var convert2Var(const uint32_t key);

    void getAllVarEnums();

    std::string asStr(const uint32_t key, const double val);
    std::string asStr(const Prompt::Def::Var var);

    std::vector<Prompt::Def::Var> readVars(const std::string &config);
    std::vector<Prompt::Def::Var> readVectorVars(
      const std::vector<std::string> &keys
    );

    virtual void clearVars();

  private:

    VarMap m_gPromptVars;
    VarEntryVec m_fVars;

    std::vector<Prompt::Def::Var> m_varEnums;
  };

  //======================================================================================================
  // VarEntry inline functions and comparison operators
  //
  inline VarEntry::VarEntry() :m_fKey(0), m_fData(0.)
  {
  }

  inline VarEntry::VarEntry(unsigned int key, double data)
    :m_fKey(key), m_fData(data)
  {
  }

  inline bool operator==(const VarEntry &lhs, const VarEntry &rhs)
  {
    return lhs.getKey() == rhs.getKey();
  }
  inline bool operator<(const VarEntry &lhs, const VarEntry &rhs)
  {
    return lhs.getKey() < rhs.getKey();
  }

  inline bool operator==(const VarEntry &var, unsigned key) { return var.getKey() == key; }
  inline bool operator==(unsigned key, const VarEntry &var) { return var.getKey() == key; }

  inline bool operator<(const VarEntry &var, unsigned key) { return var.getKey() < key; }
  inline bool operator<(unsigned key, const VarEntry &var) { return key < var.getKey(); }

  //======================================================================================================
  // VarHolder inline functions
  //
  inline bool VarHolder::replaceVar(const unsigned key, const double value)
  {
    if(!hasKey(key)) {
      m_fVars.push_back(VarEntry(key, value));
      return true;
    }
    else{
      delVar(key);
      addVar(key, value);
    }
    return false;
  }

  /**
   * @brief Adds a variable to the VarHolder.
   *
   * This function adds a variable to the VarHolder using the specified key and value.
   *
   * @param key The key of the variable.
   * @param value The value of the variable.
   * @return True if the variable was successfully added, false otherwise.
   */
  inline bool VarHolder::addVar(const unsigned key, const double value)
  {
    using namespace asg::msgUserCode;

    if(!hasKey(key)) {
      m_fVars.push_back(VarEntry(key, value));
      return true;
    }

    ANA_MSG_DEBUG(getObjectType() << "::addVar(" << key << ", " << value << ") - key already exists");
    return false;
  }

  inline bool VarHolder::delVar(const unsigned key)
  {
    VarEntryVec::iterator vit = m_fVars.begin();
    while(vit != m_fVars.end()) {
      if(vit->getKey() == key) {
        vit = m_fVars.erase(vit);
      }
      else {
        ++vit;
      }
    }

    return false;
  }

  inline bool VarHolder::hasKey(unsigned key) const
  {
    return std::find(m_fVars.begin(), m_fVars.end(), key) != m_fVars.end();
  }
  inline bool VarHolder::hasVar(unsigned key) const
  {
    return std::find(m_fVars.begin(), m_fVars.end(), key) != m_fVars.end();
  }

  inline bool VarHolder::getVar(const unsigned key, float &value) const
  {
    //
    // Read variable
    //
    const VarEntryVec::const_iterator ivar = std::find(m_fVars.begin(), m_fVars.end(), key);
    if(ivar != m_fVars.end()) {
      value = ivar->getData();
      return true;
    }

    return false;
  }

  inline bool VarHolder::getVar(const unsigned key, double &value) const
  {
    //
    // Read variable
    //
    const VarEntryVec::const_iterator ivar = std::find(m_fVars.begin(), m_fVars.end(), key);
    if(ivar != m_fVars.end()) {
      value = ivar->getData();
      return true;
    }

    return false;
  }

  inline double VarHolder::getVar(const unsigned key) const
  {
    //
    // Find and return, if exists, value stored at key
    //
    double val = -1.0e9;
    getVar(key, val);
    return val;
  }

  inline void VarHolder::clearVars()
  {
    m_fVars.clear();
  }
}

#endif // PROMPT_VARHOLDER_H
