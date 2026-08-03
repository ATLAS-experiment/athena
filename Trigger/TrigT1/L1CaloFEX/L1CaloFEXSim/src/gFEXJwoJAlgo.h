/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "L1CaloFEXToolInterfaces/IgFEXJwoJAlgo.h" //also has gTowersType typedef
#include "L1CaloFEXCond/gFEXDBCondData.h"
#include "TrigConfData/L1Menu.h"

#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <array>

class gFEXJwoJTOB;

namespace LVL1 {

  class gFEXJwoJAlgo : public AthAlgTool, virtual public IgFEXJwoJAlgo {

  public:
    /** Constructors */
    gFEXJwoJAlgo(const std::string& type, const std::string& name, const IInterface* parent);

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;

    virtual std::vector<std::unique_ptr<gFEXJwoJTOB>> jwojAlgo(const EventContext& ctx,
							       const gTowersType& Atwr, int pucA_JWJ,
                                                               const gTowersType& Btwr, int pucB_JWJ,
                                                               const gTowersType& Ctwr, int pucC_JWJ,
                                                               std::array<int32_t, 4> & outTOB) const override;



  private:

    SG::ReadCondHandleKey<gFEXDBCondData> m_DBToolKey{this, "DBToolKey", "gFEXDBParams", "Database tool key"};

    SG::ReadHandleKey<TrigConf::L1Menu> m_l1MenuKey{this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu","Name of the L1Menu object to read configuration from"};


    void gBlockAB(const gTowersType & twrs, gTowersType & gBlkSum, gTowersType & hasSeed, int seedThreshold) const;

    void metFPGA_rho(int FPGAnum, const gTowersType& twrs, int puc_jwj, 
                 const gTowersType & gBlkSum, int gBlockthreshold,
                 int aFPGA, int bFPGA,
                 int & MHT_x, int & MHT_y,
                 int & MST_x, int & MST_y,
                 int & MET_x, int & MET_y) const;

    void metFPGA(int FPGAnum,const gTowersType& twrs, 
                 const gTowersType & gBlkSum, int gBlockthreshold,
                 int aFPGA, int bFPGA,
                 int & MHT_x, int & MHT_y,
                 int & MST_x, int & MST_y,
                 int & MET_x, int & MET_y) const;

    void etFPGA(int FPGAnum,const gTowersType& twrs, gTowersType &gBlkSum,
                int gBlockthreshold, int A, int B, int &eth, int &ets, int &etw) const;

    void etFastFPGA(int FPGAnum,const gTowersType& twrs, gTowersType &gBlkSum,
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
