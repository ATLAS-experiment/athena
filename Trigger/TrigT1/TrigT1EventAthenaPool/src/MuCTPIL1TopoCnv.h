// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1EVENTATHENAPOOL_MuCTPIL1TopoCNV_H
#define TRIGT1EVENTATHENAPOOL_MuCTPIL1TopoCNV_H

// Gaudi/Athena include(s):
#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"

// TrigT1 include(s):
#include "TrigT1Interfaces/MuCTPIL1Topo.h"
#include "TrigT1EventTPCnv/MuCTPIL1Topo_p1.h"
#include "TrigT1EventTPCnv/MuCTPIL1TopoCnv_p1.h"


// Define the latest persistent representation of MuCTPIL1Topo:
typedef MuCTPIL1Topo_p1 MuCTPIL1Topo_PERS;
typedef T_AthenaPoolCustomCnv< LVL1::MuCTPIL1Topo, MuCTPIL1Topo_PERS > MuCTPIL1TopoCnvBase;

/**
 *   @short POOL converter for MuCTPIL1Topo
 *
 *          Custom POOL converter for the MuCTPIL1Topo object that implements the
 *          T/P separation for the LVL1 object.
 *
 *  @author Anil Sonay
 */
class MuCTPIL1TopoCnv : public MuCTPIL1TopoCnvBase {

  friend class CnvFactory< MuCTPIL1TopoCnv >;

public:
  using MuCTPIL1TopoCnvBase::MuCTPIL1TopoCnvBase;
  
protected:
  virtual MuCTPIL1Topo_PERS* createPersistent( LVL1::MuCTPIL1Topo* transObj ) override;
  virtual LVL1::MuCTPIL1Topo* createTransient(const Token* token) override;
  
private:
  MuCTPIL1TopoCnv_p1 m_converter;


}; // class MuCTPIL1TopoCnv

#endif // TRIGT1EVENTATHENAPOOL_MuCTPIL1TopoCNV_H
