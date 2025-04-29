/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigInDetEventTPCnv
 * @class  : TrigInDetTrackCnv_p4
 *
 * @brief transient-persistent converter for TrigInDetTrack
 *
 * @author Andrew Hamilton  <Andrew.Hamilton@cern.ch>  - U. Geneva
 * @author Francesca Bucci  <f.bucci@cern.ch>          - U. Geneva
 **********************************************************************************/
#ifndef TRIGINDETEVENTTPCNV_TRIGINDETTRACKCNV_P4_H
#define TRIGINDETEVENTTPCNV_TRIGINDETTRACKCNV_P4_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

#include "TrigInDetEvent/TrigInDetTrack.h"
#include "TrigInDetEvent/TrigInDetTrackFitPar.h"

#include "TrigInDetEventTPCnv/TrigInDetTrack_p4.h"

#include "InDetIdentifier/PixelID.h"

class MsgStream;

class TrigInDetTrackCnv_p4: public T_AthenaPoolTPCnvBase<TrigInDetTrack, TrigInDetTrack_p4>
{
public:

  TrigInDetTrackCnv_p4() = default;

  virtual void persToTrans( const TrigInDetTrack_p4 *, TrigInDetTrack *, MsgStream& );
  virtual void transToPers( const TrigInDetTrack *, TrigInDetTrack_p4 *, MsgStream& );

  void setPixelID (const PixelID* pixId);
  
protected:
  
  ITPConverterFor<TrigInDetTrackFitPar>        	*m_fpCnv{};
  const PixelID *m_pixId{};
  bool m_isInitialized{};
  StatusCode initialize(MsgStream &log);

};

#endif 
