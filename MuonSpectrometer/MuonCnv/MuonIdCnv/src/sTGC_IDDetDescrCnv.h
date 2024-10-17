/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCNV_STGC_IDDETDESCRCNV_H
#define MUONCNV_STGC_IDDETDESCRCNV_H

#include "T_Muon_IDDetDescrCnv.h"
#include "MuonIdHelpers/sTgcIdHelper.h"


/**
 ** Converter for sTgcIdHelper.
 **/
class sTGC_IDDetDescrCnv: public T_Muon_IDDetDescrCnv<sTgcIdHelper> {
public:
   sTGC_IDDetDescrCnv(ISvcLocator* svcloc) :
     T_Muon_IDDetDescrCnv(svcloc, "sTGC_IDDetDescrCnv") {}

};

#endif
