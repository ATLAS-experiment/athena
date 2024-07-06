/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXJetTOB - Forms the Jet TOBs for gFEX
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#pragma once
#include "AthenaKernel/CLASS_DEF.h"

namespace LVL1 {
  /** The gFEXJetTOB.h class stores the energy, the eta, phi coordinate,
   * the status and the type (gRho, gBlock, gJet) of the gFEX Jet TOBs
   */
  class gFEXJetTOB
  {

  private:
    unsigned int m_eta{99999};
    unsigned int m_phi{99999};
    unsigned int m_ET{99999};
    unsigned int m_st{99999};
    unsigned int m_tobID{99999};
    uint32_t     m_word{0};

  public:
    gFEXJetTOB() = default;
    ~gFEXJetTOB() {};

    inline unsigned int getEta() const {return m_eta;}
    inline unsigned int getPhi() const {return m_phi;}
    inline unsigned int getET() const {return m_ET;}
    inline unsigned int getStatus() const {return m_st;}
    inline unsigned int getTobID() const {return m_tobID;}
    inline uint32_t      getWord() const {return m_word;}

    void setEta(unsigned int);
    void setPhi(unsigned int);
    void setET(unsigned int);
    void setStatus(unsigned int);
    void setTobID(unsigned int);
    void setWord(uint32_t);
  };

} // end of namespace

CLASS_DEF( LVL1::gFEXJetTOB, 1562118 , 1 )
