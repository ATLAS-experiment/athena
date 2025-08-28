/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HWMAP_H
#define HWMAP_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include <vector>
#include <string>
#include <memory>
namespace TRTCond{
  class HWMap{
  public:
    HWMap() {
    // Initialize HV-line/pad maps
    constexpr int nBarrelPadsTotal = 32*(42+65+100);
    m_Barrel_HV_CoolChanNames.reset(new std::vector<std::string>(nBarrelPadsTotal,""));
    m_Barrel_HV_CoolChanNums.reset(new std::vector<int>(nBarrelPadsTotal,-1));
    constexpr int nEndcapCellsTotal = 32*3*(6*4+8*2);
    m_EndcapA_HV_CoolChanNames.reset(new std::vector<std::string>(nEndcapCellsTotal,""));
    m_EndcapA_HV_CoolChanNums.reset(new std::vector<int>(nEndcapCellsTotal,-1));
    m_EndcapC_HV_CoolChanNames.reset(new std::vector<std::string>(nEndcapCellsTotal,""));
    m_EndcapC_HV_CoolChanNums.reset(new std::vector<int>(nEndcapCellsTotal,-1));
  }

  HWMap (const HWMap&) = delete;
  HWMap& operator= (const HWMap&) = delete;

  virtual ~HWMap() = default;

  const std::vector<std::string>* get_Barrel_HV_Names() const {return m_Barrel_HV_CoolChanNames.get(); } 
  const std::vector<std::string>* get_EndcapA_HV_Names() const {return m_EndcapA_HV_CoolChanNames.get(); } 
  const std::vector<std::string>* get_EndcapC_HV_Names() const {return m_EndcapC_HV_CoolChanNames.get(); } 
  const std::vector<int>* get_Barrel_HV_Nums() const {return m_Barrel_HV_CoolChanNums.get(); }
  const std::vector<int>* get_EndcapA_HV_Nums() const {return m_EndcapA_HV_CoolChanNums.get(); }
  const std::vector<int>* get_EndcapC_HV_Nums() const {return m_EndcapC_HV_CoolChanNums.get(); }
 
  void setBarrelName(int i, const std::string & name) {
    m_Barrel_HV_CoolChanNames->at(i)=name;
  }
  void setEndcapAName(int i, const std::string & name) {
    m_EndcapA_HV_CoolChanNames->at(i)=name;
  }
  void setEndcapCName(int i, const std::string & name) {
    m_EndcapC_HV_CoolChanNames->at(i)=name;
  }
 
 void setBarrelNum(int i, const int & channel_number) {
    m_Barrel_HV_CoolChanNums->at(i)=channel_number;
  }

 void setEndcapANum(int i, const int & channel_number) {
    m_EndcapA_HV_CoolChanNums->at(i)=channel_number;
  }

 void setEndcapCNum(int i, const int & channel_number) {
    m_EndcapC_HV_CoolChanNums->at(i)=channel_number;
  }
     
  private:
  using StringVecPtr = std::unique_ptr<std::vector<std::string>>;
  using IntVecPtr = std::unique_ptr<std::vector<int>>;
  StringVecPtr m_Barrel_HV_CoolChanNames;
  StringVecPtr m_EndcapA_HV_CoolChanNames;
  StringVecPtr m_EndcapC_HV_CoolChanNames;
  IntVecPtr m_Barrel_HV_CoolChanNums;
  IntVecPtr m_EndcapA_HV_CoolChanNums;
  IntVecPtr m_EndcapC_HV_CoolChanNums;

  };
}
CLASS_DEF(TRTCond::HWMap,132179951,1)
CONDCONT_DEF(TRTCond::HWMap,183899721);
#endif
