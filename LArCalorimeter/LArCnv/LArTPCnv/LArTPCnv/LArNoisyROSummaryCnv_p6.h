/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARTPCNV_LARNOISYROSUMMARYCNV_p6_H
#define LARTPCNV_LARNOISYROSUMMARYCNV_p6_H

#include "LArRecEvent/LArNoisyROSummary.h"
#include "LArTPCnv/LArNoisyROSummary_p6.h"
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"


class MsgStream;

class LArNoisyROSummaryCnv_p6: public T_AthenaPoolTPCnvConstBase<LArNoisyROSummary,LArNoisyROSummary_p6>
{
 public:
  LArNoisyROSummaryCnv_p6() {};
  using base_class::persToTrans;
  using base_class::transToPers;

  virtual void   persToTrans(const LArNoisyROSummary_p6* pers, LArNoisyROSummary* trans, MsgStream &log) const override;
  virtual void   transToPers(const LArNoisyROSummary* trans, LArNoisyROSummary_p6* pers, MsgStream &log) const override;
  
};


#endif
