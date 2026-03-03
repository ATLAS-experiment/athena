/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LArNoisyROSummaryCnv_h
#define LArNoisyROSummaryCnv_h

#include "LArRecEvent/LArNoisyROSummary.h"
#include "LArTPCnv/LArNoisyROSummary_p6.h"
#include "LArTPCnv/LArNoisyROSummaryCnv_p6.h"
#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"


typedef LArNoisyROSummary_p6 LArNoisyROSummary_PERSISTENT;
typedef T_AthenaPoolCustomCnv<LArNoisyROSummary,LArNoisyROSummary_PERSISTENT> LArNoisyROSummaryCnvBase;
 
class LArNoisyROSummaryCnv : public LArNoisyROSummaryCnvBase 
{
public:
  LArNoisyROSummaryCnv(ISvcLocator*);
 protected:
  virtual LArNoisyROSummary* createTransient(const Token* token);
  virtual LArNoisyROSummary_PERSISTENT* createPersistent(LArNoisyROSummary*);
  private:
 
  LArNoisyROSummaryCnv_p6 m_converter;
 
};
 

#endif
