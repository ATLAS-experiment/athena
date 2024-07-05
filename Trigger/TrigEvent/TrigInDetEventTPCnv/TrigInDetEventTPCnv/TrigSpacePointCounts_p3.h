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
#ifndef TRIGINDETEVENTTPCNV_TRIGSPACEPOINTCOUNTS_P3_H
#define TRIGINDETEVENTTPCNV_TRIGSPACEPOINTCOUNTS_P3_H

#include "TrigInDetEventTPCnv/TrigHisto2D_p1.h"

class TrigSpacePointCounts_p3 
{
  friend class TrigSpacePointCountsCnv_p3;

 public:
  
  TrigSpacePointCounts_p3() = default;
  virtual ~TrigSpacePointCounts_p3() = default;
  
  TrigHisto2D_p1 m_pixelClusEndcapC;
  TrigHisto2D_p1 m_pixelClusBarrel;
  TrigHisto2D_p1 m_pixelClusEndcapA;
  std::vector<unsigned int> m_droppedPixelModules;
  unsigned int m_sctSpEndcapC{};
  unsigned int m_sctSpBarrel{};
  unsigned int m_sctSpEndcapA{};
  std::vector<unsigned int> m_droppedSctModules;

};

#endif
