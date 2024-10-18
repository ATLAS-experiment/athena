/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCNV_MM_IDDETDESCRCNV_H
#define MUONCNV_MM_IDDETDESCRCNV_H

#include "T_Muon_IDDetDescrCnv.h"
#include "MuonIdHelpers/MmIdHelper.h"


/**
 ** Converter for MmIdHelper.
 **/
class MM_IDDetDescrCnv: public T_Muon_IDDetDescrCnv<MmIdHelper> {
public:
   MM_IDDetDescrCnv(ISvcLocator* svcloc) :
     T_Muon_IDDetDescrCnv(svcloc, "MM_IDDetDescrCnv") {}

};

#endif
