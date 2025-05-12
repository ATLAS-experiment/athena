/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFEXFPGA.h  -
//                              -------------------
//     begin                : 23 03 2019
//     email                :  jacob.julian.kempster@cern.ch
//  ***************************************************************************/

#ifndef IjFEXFPGA_H
#define IjFEXFPGA_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"
#include "TrigConfData/L1Menu.h"
#include "L1CaloFEXSim/jFEXOutputCollection.h"

#include "L1CaloFEXSim/jFEXTOB.h"

namespace LVL1 {
  
/*
Interface definition for jFEXFPGA
*/

  static const InterfaceID IID_IjFEXFPGA("LVL1::IjFEXFPGA", 1, 0);

  class IjFEXFPGA : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;

    virtual StatusCode init(int id, int efexid) = 0;

    virtual StatusCode execute(jFEXOutputCollection* inputOutputCollection, const std::pair<unsigned int, const std::vector<int>&> & jetCalibrationParameters) = 0;

    virtual void reset() = 0;

    virtual int ID() = 0;
    
    virtual std::vector <std::unique_ptr<jFEXTOB>> getSmallRJetTOBs() = 0;

    virtual std::vector <std::unique_ptr<jFEXTOB>> getLargeRJetTOBs() = 0;

    virtual std::vector <std::vector <uint32_t>> getFwdElTOBs() =0;

    virtual std::vector<std::unique_ptr<jFEXTOB>> getTauTOBs() = 0;

    virtual std::vector<std::unique_ptr<jFEXTOB>> getSumEtTOBs() =0;    

    virtual std::vector<std::unique_ptr<jFEXTOB>> getMetTOBs() =0;    
    
    

    virtual void SetTowersAndCells_SG(int [][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width]) = 0;
    virtual void SetTowersAndCells_SG(int [][FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width]) = 0;

    virtual int getTTowerET_EM     (unsigned int TTID ) =0; 
    virtual int getTTowerET_HAD    (unsigned int TTID ) =0; 
    virtual int getTTowerET        (unsigned int TTID ) =0; 
    virtual int getTTowerET_forMET (unsigned int TTID ) =0; 


  private:

  };

  inline const InterfaceID& LVL1::IjFEXFPGA::interfaceID()
  {
    return IID_IjFEXFPGA;
  }

} // end of namespace

#endif
