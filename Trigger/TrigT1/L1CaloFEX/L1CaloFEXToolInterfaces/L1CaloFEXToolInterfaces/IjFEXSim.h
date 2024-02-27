/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFEXSim.h  -
//                              -------------------
//     begin                : 23 03 2019
//     email                :  jacob.julian.kempster@cern.ch
//  ***************************************************************************/

#ifndef IjFEXSim_H
#define IjFEXSim_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloEvent/CaloCellContainer.h"
#include "L1CaloFEXSim/jFEXOutputCollection.h"

#include "L1CaloFEXSim/jFEXTOB.h"


namespace LVL1 {
  
/*
Interface definition for jFEXSim
*/

  static const InterfaceID IID_IjFEXSim("LVL1::IjFEXSim", 1, 0);

  class IjFEXSim : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;

    virtual void init(int id) = 0;

    virtual void reset() = 0;

    virtual int ID() = 0;
    virtual void SetTowersAndCells_SG(int tmp[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width]) = 0;
    virtual void SetTowersAndCells_SG(int tmp[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width]) = 0;


    virtual std::vector<std::vector<std::vector<uint32_t>>> getFwdElTOBs() =0;
    
    virtual std::vector< std::vector<std::unique_ptr<jFEXTOB>> > getTauTOBs() =0;
    virtual std::vector< std::vector<std::unique_ptr<jFEXTOB>> > getSmallRJetTOBs() =0;
    virtual std::vector< std::vector<std::unique_ptr<jFEXTOB>> > getLargeRJetTOBs() =0;
    virtual std::vector<std::unique_ptr<jFEXTOB>> getSumEtTOBs() =0;
    virtual std::vector<std::unique_ptr<jFEXTOB>> getMetTOBs() =0;    
    
    virtual StatusCode ExecuteForwardASide(int tmp[2*FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width], jFEXOutputCollection* inputOutputCollection, const std::pair<unsigned int, const std::vector<int>&> & jetCalibrationParameters) = 0;
    virtual StatusCode ExecuteForwardCSide(int tmp[2*FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width], jFEXOutputCollection* inputOutputCollection, const std::pair<unsigned int, const std::vector<int>&> & jetCalibrationParameters) = 0;
    virtual StatusCode ExecuteBarrel(int tmp[2*FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width], jFEXOutputCollection* inputOutputCollection, const std::pair<unsigned int, const std::vector<int>&> & jetCalibrationParameters) = 0;

  private:

  };

  inline const InterfaceID& LVL1::IjFEXSim::interfaceID()
  {
    return IID_IjFEXSim;
  }

} // end of namespace

#endif
