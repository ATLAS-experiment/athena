/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXtauTOB.h  -  
//                              -------------------
//     begin                : 01 03 2020
//     email                : nicholas.andrew.luongo@cern.ch
//  **************************************************************************


#pragma once
#include "AthenaKernel/CLASS_DEF.h"

namespace LVL1 {
  class eFEXtauTOB
  {
    //eFEXtauAlgo class description below:
    /** The eFEXtauAlgo.h class store the energy, the location and the isolation variables of eFEX TOBs
     */
  private:
    unsigned int m_eta;
    unsigned int m_phi;
    unsigned int m_et;
    unsigned int m_bitwise_et;
    float m_iso;
    unsigned int m_rcore_core;
    unsigned int m_rcore_env;
    unsigned int m_rhad_core;
    unsigned int m_rhad_env;
    unsigned int m_fpga_id;
    unsigned int m_efex_id;
    bool m_seed_und;
    unsigned int m_seed;
    unsigned int m_bdt_score;
    unsigned int m_is_bdt_algo;
    uint32_t m_tobword;
    uint32_t m_xtobword0;
    uint32_t m_xtobword1;
    
  public:
    eFEXtauTOB();
    ~eFEXtauTOB() {};
    
    inline unsigned int getEta() const {return m_eta;}
    inline unsigned int getPhi() const {return m_phi;}
    inline unsigned int getEt() const {return m_et;}
    inline unsigned int getBitwiseEt() const {return m_bitwise_et;}
    inline float getIso() const {return m_iso;}
    inline unsigned int getFPGAID() const {return m_fpga_id;}
    inline unsigned int geteFEXID() const {return m_efex_id;}
    // if seed is above (higher phi) central supercell of the seed
    inline bool getSeedUnD() const {return m_seed_und;}
    // seed index in eta
    inline unsigned int getSeed() const {return m_seed;}
    inline unsigned int getRcoreCore() const {return m_rcore_core;}
    inline unsigned int getRcoreEnv() const {return m_rcore_env;}
    inline unsigned int getRhadCore() const {return m_rhad_core;}
    inline unsigned int getRhadEnv() const {return m_rhad_env;}
    inline unsigned int getBDTScore() const {return m_bdt_score;}
    inline unsigned int getIsBDTAlgo() const {return m_is_bdt_algo;}
    inline uint32_t getTobword() const {return m_tobword;}
    inline uint32_t getxTobword0() const {return m_xtobword0;}
    inline uint32_t getxTobword1() const {return m_xtobword1;}
    
    void setEta(unsigned int);
    void setPhi(unsigned int);
    void setEt(unsigned int);
    void setBitwiseEt(unsigned int);
    void setIso(unsigned int);
    void setFPGAID(unsigned int);
    void seteFEXID(unsigned int);
    void setSeedUnD(bool);
    void setSeed(unsigned int);
    void setRcoreCore(unsigned int);
    void setRcoreEnv(unsigned int);
    void setRhadCore(unsigned int);
    void setRhadEnv(unsigned int);
    void setBDTScore(unsigned int);
    void setIsBDTAlgo(unsigned int);
    void setTobword(uint32_t);
    void setxTobword0(uint32_t);
    void setxTobword1(uint32_t);
  };
  
} // end of namespace

CLASS_DEF( LVL1::eFEXtauTOB, 32202275 , 1 )
