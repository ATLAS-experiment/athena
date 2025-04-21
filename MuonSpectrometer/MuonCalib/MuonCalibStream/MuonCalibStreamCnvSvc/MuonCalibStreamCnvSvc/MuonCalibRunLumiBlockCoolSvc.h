/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonCalibRunLumiBlockCoolSvc_h
#define MuonCalibRunLumiBlockCoolSvc_h

#include "AthenaBaseComps/AthService.h"


class MuonCalibRunLumiBlockCoolSvc : public AthService {
public:
    using AthService::AthService;

    // retrieve lumiblock for timestamp. If run_number<0 get run number too.
    StatusCode GetRunEventNumber(unsigned int timestamp, int &run_number, int &lb_nr);
};

#endif
