/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigInDetEventTPCnv
 * @class  : TrigInDetTrack_p2
 *
 * @brief persistent partner for TrigInDetTrack
 *
 * @author Andrew Hamilton  <Andrew.Hamilton@cern.ch>  - U. Geneva
 * @author Francesca Bucci  <f.bucci@cern.ch>          - U. Geneva
 *
 **********************************************************************************/

#ifndef TrigInDetEventTPCnv_TrigInDetTrack_p2_H
#define TrigInDetEventTPCnv_TrigInDetTrack_p2_H

#include "AthenaPoolUtilities/TPObjRef.h"


class TrigInDetTrack_p2 
{

 public:

     TrigInDetTrack_p2() {}
     friend class TrigInDetTrackCnv_p2;

     //private:

     int      m_algId{};
     TPObjRef m_param;    //actual type is TrigInDetTrackFitPar*
     TPObjRef m_endParam; //actual type is TrigInDetTrackFitPar*
     double  m_chi2{};
     int     m_NStrawHits{};
     int     m_NStraw{};
     int     m_NStrawTime{};
     int     m_NTRHits{};
     int     m_NPixelSpacePoints{};
     int     m_NSCT_SpacePoints{};
     long    m_HitPattern{};

};

#endif

