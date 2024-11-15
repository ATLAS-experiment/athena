/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TrigT1TGC_BIS78CoincidenceMap_H_
#define TrigT1TGC_BIS78CoincidenceMap_H_

#include <vector>
#include <map>
#include <string>

#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrigT1TGC/TGCNumbering.h"

namespace LVL1TGCTrigger {
class TGCArguments;
}

namespace LVL1TGC {

class BIS78TrigOut;

class TGCBIS78CoincidenceMap : public AthMessaging {
 public:
  TGCBIS78CoincidenceMap(LVL1TGCTrigger::TGCArguments* tgcargs, const std::string& version);
  virtual ~TGCBIS78CoincidenceMap() = default;

  TGCBIS78CoincidenceMap(const TGCBIS78CoincidenceMap& right);

 public:
  int TGCBIS78_pt(const BIS78TrigOut *bis78Out, int /*roi*/) const;
  int getFlagROI(const unsigned int roi, const unsigned int ssc, const unsigned int sec, const unsigned int side) const;
  const std::string&  getVersion() const { return m_verName; }

  bool readMap();
  LVL1TGCTrigger::TGCArguments* tgcArgs() { return m_tgcArgs;}
  const LVL1TGCTrigger::TGCArguments* tgcArgs() const { return m_tgcArgs;}

 private:
  TGCBIS78CoincidenceMap() = delete;// hide default constructor

 private:
  static constexpr unsigned int N_DETA = 64;
  static constexpr unsigned int N_DPHI = 16;

 private:
  std::vector<short int> m_CW[N_DETA][N_DPHI];
  
  int m_flagROI[kNRoiInSSC][kNMaxSSC][kNEndcapTrigSector];
  // 1 use; 0: not use; -1: not used for Trigger

  std::string m_verName;

  LVL1TGCTrigger::TGCArguments* m_tgcArgs;
};

}  // end of namespace

#endif  // TrigT1TGC_BIS78CoincidenceMap_H_

