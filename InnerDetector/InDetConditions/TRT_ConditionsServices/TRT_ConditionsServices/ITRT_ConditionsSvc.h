/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITRT_CONDITIONSSVC_H
#define ITRT_CONDITIONSSVC_H

#include "GaudiKernel/IService.h"
#include "GaudiKernel/StatusCode.h"

#include "InDetConditionsSummaryService/InDetHierarchy.h"

#include <string>

#ifndef TRT_CONDFLAGS
#define TRT_CONDFLAGS
namespace InDet {
  /** status flag to be derived from the inheriting tool's conditions info*/
  enum TRT_CondFlag {
    TRT_COND_NOINFO,
    TRT_COND_GOOD,
    TRT_COND_CAUTION,
    TRT_COND_CAUTION_TRACKING,
    TRT_COND_CAUTION_PID,
    TRT_COND_BAD,
    TRT_COND_SKIPME
  };
}
#endif // TRT_CONDFLAGS

class Identifier;
class IdentifierHash;

class ITRT_ConditionsSvc : virtual public IService {
 public:
  DeclareInterfaceID(ITRT_ConditionsSvc,1,0);

  virtual ~ITRT_ConditionsSvc() {};

  virtual InDet::TRT_CondFlag condSummaryStatus( const Identifier& ) = 0;

};

#endif // ITRT_CONDITIONSSVC_H
