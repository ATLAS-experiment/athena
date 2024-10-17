/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCNV_TGC_IDDETDESCRCNV_H
#define MUONCNV_TGC_IDDETDESCRCNV_H

#include "T_Muon_IDDetDescrCnv.h"
#include "MuonIdHelpers/TgcIdHelper.h"


/**
 ** Converter for TgcIdHelper.
 **/
class TGC_IDDetDescrCnv: public T_Muon_IDDetDescrCnv<TgcIdHelper> {
public:
   TGC_IDDetDescrCnv(ISvcLocator* svcloc) :
     T_Muon_IDDetDescrCnv(svcloc, "TGC_IDDetDescrCnv") {}

};

#endif
