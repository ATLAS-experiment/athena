/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigInDetEventTPCnv
 * @class  : TrigInDetTrack_p1
 *
 * @brief persistent partner for TrigInDetTrack
 *
 * @author Andrew Hamilton  <Andrew.Hamilton@cern.ch>  - U. Geneva
 * @author Francesca Bucci  <f.bucci@cern.ch>          - U. Geneva
 *
 **********************************************************************************/

#ifndef TrigInDetEventTPCnv_TrigInDetTrack_p1_H
#define TrigInDetEventTPCnv_TrigInDetTrack_p1_H

#include "AthenaPoolUtilities/TPObjRef.h"

class TrigInDetTrack_p1 
{

 public:

     TrigInDetTrack_p1() {}
     friend class TrigInDetTrackCnv_p1;

     int      m_algId{};    //holds TrigInDetTrack::AlgoId
     TPObjRef m_param;    //actual type is TrigInDetTrackFitPar*
     TPObjRef m_endParam; //actual type is TrigInDetTrackFitPar*
     double  m_chi2{};
     int     m_NStrawHits{};
     int     m_NStraw{};
     int     m_NStrawTime{};
     int     m_NTRHits{};


};

#endif

