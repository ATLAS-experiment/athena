/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4SIMTPCNV_TRACKRECORDCNV_P2_H
#define G4SIMTPCNV_TRACKRECORDCNV_P2_H

/*
Transient/Persistent converter for TrackRecord class
Author: Davide Costanzo
*/

class TrackRecord;
class TrackRecord_p2;

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

class MsgStream;


class TrackRecordCnv_p2  : public T_AthenaPoolTPCnvConstBase<TrackRecord, TrackRecord_p2>
{
public:
  using base_class::transToPers;
  using base_class::persToTrans;


  TrackRecordCnv_p2() {}

  virtual void          persToTrans(const TrackRecord_p2* persObj, TrackRecord* transObj, MsgStream &log) const override;
  virtual void          transToPers(const TrackRecord* transObj, TrackRecord_p2* persObj, MsgStream &log) const override;
};


#endif // G4SIMTPCNV_TRACKRECORDCNV_P2_H

