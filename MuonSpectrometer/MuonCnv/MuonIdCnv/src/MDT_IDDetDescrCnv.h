/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCNV_MDT_IDDETDESCRCNV_H
#define MUONCNV_MDT_IDDETDESCRCNV_H

#include "T_Muon_IDDetDescrCnv.h"
#include "MuonIdHelpers/MdtIdHelper.h"


/**
 ** Converter for MdtIdHelper.
 **/
class MDT_IDDetDescrCnv: public T_Muon_IDDetDescrCnv<MdtIdHelper> {
public:
   MDT_IDDetDescrCnv(ISvcLocator* svcloc) :
     T_Muon_IDDetDescrCnv(svcloc, "MDT_IDDetDescrCnv") {}

};

#endif
