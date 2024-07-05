/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigInDetEventTPCnv
 * @class  : TrigSpacePointCounts_p2
 *
 * @brief persistent partner for TrigSpacePointCounts
 *
 * @author Regina Kwee at cern dot ch                  - CERN/HUB
 * @author Andrew Hamilton  <Andrew.Hamilton@cern.ch>  - U. Geneva
 * @author Francesca Bucci  <F.Bucci@cern.ch>          - U. Geneva
 *
 **********************************************************************************/
#ifndef TRIGINDETEVENTTPCNV_TRIGSPACEPOINTCOUNTS_P4_H
#define TRIGINDETEVENTTPCNV_TRIGSPACEPOINTCOUNTS_P4_H

#include "TrigInDetEventTPCnv/TrigHisto2D_p1.h"

class TrigSpacePointCounts_p4 
{
  friend class TrigSpacePointCountsCnv_p4;

 public:
  
  TrigSpacePointCounts_p4() = default;
  virtual ~TrigSpacePointCounts_p4() = default;
  
  TrigHisto2D_p1 m_allTheTrigHisto2D[3];
  std::vector<unsigned int> m_droppedPixelModules;
  unsigned int m_sctSpEndcapC{};
  unsigned int m_sctSpBarrel{};
  unsigned int m_sctSpEndcapA{};
  std::vector<unsigned int> m_droppedSctModules;

};

#endif
