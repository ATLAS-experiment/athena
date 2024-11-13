/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//              jFEXTOB - TOBs info in jFEX
//                              -------------------
//     begin                : 18 02 2021
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************

#include "L1CaloFEXSim/jFEXTOB.h"

namespace LVL1 {

jFEXTOB::jFEXTOB():
  m_fpga{0},
  m_jfex{0},
  m_word{0},
  m_res{0},
  m_ttid{0},
  m_seedEt{0}
{}


void jFEXTOB::initialize(uint8_t fpga, uint8_t jfex, uint32_t word, uint res, uint ttid, int seedEt )
{
    setFpga   (fpga);
    setjFex   (jfex);
    setWord   (word);
    setRes    (res);
    setTTID   (ttid);
    setSeedEt (seedEt);

}
    
}


