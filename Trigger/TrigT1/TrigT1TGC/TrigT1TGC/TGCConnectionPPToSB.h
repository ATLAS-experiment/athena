/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//  Table of connection between Patch Panel and Slave Board.
#ifndef TrigT1TGC_ConnectionPPToSB_H_
#define TrigT1TGC_ConnectionPPToSB_H_

#include "TrigT1TGC/TGCBoardConnection.h"
#include "TrigT1TGC/TGCSector.h"

namespace LVL1TGCTrigger {

class TGCConnectionPPToSB : public TGCBoardConnection
{
 public:
  int getSBIdToPP(int type, int port, int index) const;
  void setSBIdToPP(int type, int port, int index, int idIn);

  TGCConnectionPPToSB();
  ~TGCConnectionPPToSB();
  TGCConnectionPPToSB(const TGCConnectionPPToSB& right);
  TGCConnectionPPToSB& operator=(const TGCConnectionPPToSB& right);


  inline int getNumberOfPort() const { return N_PP_PORTS; }

 private:
  static constexpr int N_PP_PORTS = 2;
  int* m_SBIdToPP[N_PP_PORTS][TGCSector::NumberOfPatchPanelType]{};
};

inline
int TGCConnectionPPToSB::getSBIdToPP(int type, int port, int index) const
{
  return m_SBIdToPP[port][type][index];
}

inline
void TGCConnectionPPToSB::setSBIdToPP(int type, int port, int index, int idIn)
{
  if(m_SBIdToPP[port][type]==0)
    m_SBIdToPP[port][type] = new int [m_id.at(type).size()];
  m_SBIdToPP[port][type][index] = idIn;
}


} //end of namespace bracket

#endif   // TGCConnectionPPToSB_hh
