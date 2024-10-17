/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCNV_RPC_IDDETDESCRCNV_H
#define MUONCNV_RPC_IDDETDESCRCNV_H

#include "T_Muon_IDDetDescrCnv.h"
#include "MuonIdHelpers/RpcIdHelper.h"


/**
 ** Converter for RpcIdHelper.
 **/
class RPC_IDDetDescrCnv: public T_Muon_IDDetDescrCnv<RpcIdHelper> {
public:
   RPC_IDDetDescrCnv(ISvcLocator* svcloc) :
     T_Muon_IDDetDescrCnv(svcloc, "RPC_IDDetDescrCnv") {}

};

#endif
