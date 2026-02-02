/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXJwoJAlgo - Jets without jets algorithm for gFEX
//                              -------------------
//     begin                : 10 08 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#ifndef gFEXJwoJAlgo_H
#define gFEXJwoJAlgo_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IgFEXJwoJAlgo.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/gFEXJwoJTOB.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"
#include "L1CaloFEXCond/gFEXDBCondData.h"



namespace LVL1 {

  class gFEXJwoJAlgo : public AthAlgTool, virtual public IgFEXJwoJAlgo {

  public:
    /** Constructors */
    gFEXJwoJAlgo(const std::string& type, const std::string& name, const IInterface* parent);

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;


    virtual void setAlgoConstant(int aFPGA_A, int bFPGA_A,
                                 int aFPGA_B, int bFPGA_B,
                                 int aFPGA_C, int bFPGA_C,
                                 int gXE_seedThrA, int gXE_seedThrB, int gXE_seedThrC) override;

    virtual std::vector<std::unique_ptr<gFEXJwoJTOB>> jwojAlgo(const gTowersType& Atwr,const gTowersType& Btwr, const gTowersType& Ctwr,
                                                                 std::array<uint32_t, 4> & outTOB) const override;



  private:
    SG::ReadCondHandleKey<gFEXDBCondData> m_DBToolKey{this, "DBToolKey", "gFEXDBParams", "Database tool key"};

    float m_aFPGA_A{};
    float m_bFPGA_A{};
    float m_aFPGA_B{};
    float m_bFPGA_B{};
    float m_aFPGA_C{};
    float m_bFPGA_C{};
    float m_gBlockthresholdA{};
    float m_gBlockthresholdB{};
    float m_gBlockthresholdC{};
    std::string m_fwVersion;
 

    void gBlockAB(const gTowersType & twrs, gTowersType & gBlkSum, gTowersType & hasSeed, int seedThreshold) const;

    void metFPGA(int FPGAnum,const gTowersType& twrs, 
                 const gTowersType & gBlkSum, int gBlockthreshold,
                 int aFPGA, int bFPGA,
                 int & MHT_x, int & MHT_y,
                 int & MST_x, int & MST_y,
                 int & MET_x, int & MET_y) const;

    void etFPGA(int FPGAnum,const gTowersType& twrs, gTowersType &gBlkSum,
                int gBlockthreshold, int A, int B, int &eth, int &ets, int &etw) const;

    void metTotal(int A_MET_x, int A_MET_y,
                  int B_MET_x, int B_MET_y,
                  int C_MET_x, int C_MET_y,
                  int & MET_x, int & MET_y) const;

    void etTotal(int A_ET, 
                 int B_ET, 
                 int C_ET, 
                 int & ET ) const;  


    float sinLUT(unsigned int phiIDX, unsigned int aw) const;

    float cosLUT(unsigned int phiIDX, unsigned int aw) const;


  };

} // end of namespace


#endif
