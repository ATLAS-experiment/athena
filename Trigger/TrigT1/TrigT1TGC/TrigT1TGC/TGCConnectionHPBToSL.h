/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//  table of connection between High-Pt Board and Sector Logic
#ifndef TrigT1TGC_ConnectionHPBToSL_H_
#define TrigT1TGC_ConnectionHPBToSL_H_

#include "TrigT1TGC/TGCBoardConnection.h"
#include "TrigT1TGC/TGCSector.h"
#include <array>

namespace LVL1TGCTrigger {

class TGCConnectionHPBToSL : public TGCBoardConnection
{
 public:
  TGCConnectionHPBToSL();
  TGCConnectionHPBToSL(const TGCConnectionHPBToSL& right);
  virtual ~TGCConnectionHPBToSL();

  int getSLPortToHPB(int type, int index) const;
  void setSLPortToHPB(int type, int index, int port);

  TGCConnectionHPBToSL& operator=(const TGCConnectionHPBToSL& right);

 private:
  std::array<int*,  TGCSector::NumberOfHighPtBoardType>  m_SLPortToHPB{};
};

inline
int TGCConnectionHPBToSL::getSLPortToHPB(int type, int index) const {
  return m_SLPortToHPB[type][index];
}

inline
void TGCConnectionHPBToSL::setSLPortToHPB(int type, int index, int port) {
  if (m_SLPortToHPB[type] == 0) m_SLPortToHPB[type] = new int [m_id.at(type).size()];
  m_SLPortToHPB[type][index] = port;
}

}  // end of namespace

#endif  // TrigT1TGC_ConnectionHPBToSL_H_
