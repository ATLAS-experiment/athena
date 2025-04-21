/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCalibStreamCnvSvc/MuonCalibRunLumiBlockCoolSvc.h"


StatusCode MuonCalibRunLumiBlockCoolSvc::GetRunEventNumber(unsigned int timestamp, int &run_number, int &lb_nr) {
    // comment out the implementation of get lb, set lb = 0, to be fixed later
    lb_nr = 0 ;
    ATH_MSG_DEBUG("timestamp : "<<timestamp<<" runNumber : "<<run_number<<" lb : "<<lb_nr);
    return StatusCode::SUCCESS;
}
