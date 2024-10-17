/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCNV_CSC_IDDETDESCRCNV_H
#define MUONCNV_CSC_IDDETDESCRCNV_H

#include "T_Muon_IDDetDescrCnv.h"
#include "MuonIdHelpers/CscIdHelper.h"


/**
 ** Converter for CscIdHelper.
 **/
class CSC_IDDetDescrCnv: public T_Muon_IDDetDescrCnv<CscIdHelper> {
public:
   CSC_IDDetDescrCnv(ISvcLocator* svcloc) :
     T_Muon_IDDetDescrCnv(svcloc, "CSC_IDDetDescrCnv") {}

};

#endif
