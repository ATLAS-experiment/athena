/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXaltMetAlgo - Noise cut and Rho+RMS algorithm for gFEX MET
//                              -------------------
//     begin                : 31 03 2022
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#ifndef gFEXaltMetAlgo_H
#define gFEXaltMetAlgo_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IgFEXaltMetAlgo.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"
#include "TrigConfData/L1Menu.h"


namespace LVL1 {

  class gFEXaltMetAlgo : public extends<AthAlgTool, IgFEXaltMetAlgo> {

  public:
    /** Constructor */
    gFEXaltMetAlgo(const std::string& type, const std::string& name, const IInterface* parent);

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;

                                 
    virtual void altMetAlgo(const EventContext& ctx,
			    const gTowersCentral &Atwr,
			    const gTowersCentral &Btwr,
			    const gTowersCentral &Ctwr,
			    std::array<uint32_t, 4> & outTOB) const override;

  private:

    SG::ReadHandleKey<TrigConf::L1Menu> m_l1MenuKey{this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu","Name of the L1Menu object to read configuration from"};

    // make this configurable if needed
    const int m_rhoPlusThr = 10000/200;
    
    void metFPGA(const gTowersCentral &twrs, int & MET_x, int & MET_y, const unsigned short FPGA_NO, const std::array<std::vector<int>, 3>& etaThr) const;

    void metTotal(const int A_MET_x, const int A_MET_y,
              const int B_MET_x, const int B_MET_y,
              const int C_MET_x, const int C_MET_y,
              int & MET_x, int & MET_y, int & MET) const;

    int get_rho(const gTowersCentral &twrs) const;

    int get_sigma(const gTowersCentral &twrs) const;

    void rho_MET(const gTowersCentral &twrs, int & MET_x, int & MET_y, const int rho, const int sigma) const;

    int sumEtFPGAnc(const gTowersCentral &twrs, const unsigned short FPGA_NO, const std::array<std::vector<int>, 3>& etaThr) const;

    int sumEtFPGArms(const gTowersCentral &twrs, const int sigma) const;

    int sumEt(const int A_sumEt, const int B_sumEt, const int C_sumEt) const;

    float sinLUT(const unsigned int phiIDX, const unsigned int aw) const;

    float cosLUT(const unsigned int phiIDX, const unsigned int aw) const;

  };

} // end of namespace


#endif
