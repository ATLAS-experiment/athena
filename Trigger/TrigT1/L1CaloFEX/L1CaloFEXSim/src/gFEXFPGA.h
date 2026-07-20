/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXFPGA - Defines FPGA tools
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#ifndef gFEXFPGA_H
#define gFEXFPGA_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IgFEXFPGA.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "L1CaloFEXSim/gFEXOutputCollection.h"
#include "L1CaloFEXCond/gFEXDBCondData.h"
#include <vector>

namespace LVL1
{

  class gFEXFPGA : public AthAlgTool, virtual public IgFEXFPGA
  {

  public:
    /** Constructors */
    gFEXFPGA(const std::string &type, const std::string &name, const IInterface *parent);

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;
    /** Destructor */
    virtual ~gFEXFPGA();

    // virtual void SetTowersAndCells_SG(gTowersCentral) override ;
    // virtual void SetTowersAndCells_SG(gTowersForward) override ;

    // virtual void GetEnergyMatrix(gTowersCentral &) const override ;
    // virtual void GetEnergyMatrix(gTowersForward &) const override ;

    virtual void FillgTowerEDMCentral(const EventContext& , SG::WriteHandle<xAOD::gFexTowerContainer> &, int, const gTowersCentral &, gTowersType &, gTowersType &, gTowersType &) const override;
    virtual void FillgTowerEDMForward(const EventContext& , SG::WriteHandle<xAOD::gFexTowerContainer> &, int, const gTowersForward &, const gTowersForward &, gTowersType &, gTowersType &, gTowersType &) const override;

    /** Internal data */
  private:
    SG::ReadCondHandleKey<gFEXDBCondData> m_DBToolKey{this, "DBToolKey", "gFEXDBParams", "Database tool key"};

    void gtCalib(gTowersType &twrs, const gTowersType &offsets, const gTowersType &noiseCuts, const gTowersType &slopes) const;

    void calLookup(int *tower, const int offset, const int noiseCut, const int slope) const;

    void calExpand(gTowersType &offsets, gTowersType &noiseCuts, gTowersType &slopes, const int offset, const std::array<int, 12>& columnNoiseCuts, const std::array<int, 12>& columnSlopes) const;

    SG::ReadHandleKey<LVL1::gTowerContainer> m_gFEXFPGA_gTowerContainerKey{this, "MyGTowers", "gTowerContainer", "Input container for gTowers"};
    SG::ReadHandleKey<LVL1::gTowerContainer> m_gFEXFPGA_gTower50ContainerKey{this, "MyGTowers50", "gTower50Container", "Input container for gTowers"};
  };

} // end of namespace

#endif
