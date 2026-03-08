/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef IgFEXJwoJAlgo_H
#define IgFEXJwoJAlgo_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/gFEXJwoJTOB.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"

namespace LVL1 {

/*
Interface definition for gFEXJwoJAlgo
*/

  static const InterfaceID IID_IgFEXJwoJAlgo("LVL1::IgFEXJwoJAlgo", 1, 0);
  typedef  std::array<std::array<int, 12>, 32> gTowersType;

  class IgFEXJwoJAlgo : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;


    virtual void setAlgoConstant(int aFPGA_A, int bFPGA_A,
                                 int aFPGA_B, int bFPGA_B,
                                 int aFPGA_C, int bFPGA_C,
                                 int gXE_seedThrA, int gXE_seedThrB, int gXE_seedThrC) = 0;

    virtual std::vector<std::unique_ptr<gFEXJwoJTOB>> jwojAlgo(const gTowersType& Atwr, int pucA_JWJ,
                                                               const gTowersType& Btwr, int pucB_JWJ,
                                                               const gTowersType& Ctwr, int pucC_JWJ,
                                                               std::array<int32_t, 4> & outTOB) const = 0;


  };

  inline const InterfaceID& LVL1::IgFEXJwoJAlgo::interfaceID()
  {
    return IID_IgFEXJwoJAlgo;
  }

} // end of namespace

#endif
