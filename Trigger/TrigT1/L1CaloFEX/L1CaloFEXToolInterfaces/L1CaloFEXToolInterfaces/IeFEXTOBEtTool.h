/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           IeFEXTOBEtTool.h  -
//                              -------------------
//     begin                : 13 12 2022
//     email                : Alan.Watson@CERN.CH
//  ***************************************************************************/

#ifndef IeFEXTOBEtTool_H
#define IeFEXTOBEtTool_H


#include "GaudiKernel/IAlgTool.h"

namespace LVL1 {
  
/*
Interface definition for eFEXTOBEtTool
*/

  static const InterfaceID IID_IeFEXTOBEtTool("LVL1::IeFEXTOBEtTool", 1, 0);

  class IeFEXTOBEtTool : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;

    /** Tool to calculate eEM discriminant sums */
    virtual StatusCode getegSums(float etaTOB, float phiTOB, int seed, int UnD, 
                                  std::vector<unsigned int> &ClusterCellETs,
                                  std::vector<unsigned int> &RetaSums,
                                  std::vector<unsigned int> &RhadSums, 
                                  std::vector<unsigned int> &WstotSums) = 0;
								  
    virtual StatusCode getTOBCellEnergies(float etaTOB, float phiTOB, std::vector<unsigned int> &ClusterCellETs) const = 0;


    /** Tool to calculate eTaudiscriminant sums */
    virtual StatusCode gettauSums(float eta, float phi, int seed, int UnD, 
                                   std::vector<unsigned int> &RcoreSums,
                                   std::vector<unsigned int> &RemSums) = 0;

    /** Tool to find eTower identifier from an eta, phi coordinate pair */
    virtual unsigned int eTowerID(float eta, float phi) const = 0;

    /** Tool to find eFEX and FPGA numbers and eta index of a TOB within the FPGA */
    virtual void location(float etaTOB, float phiTOB, int& eFEX, int& FPGA, int& fpgaEta) const = 0;



  private:

  };

  inline const InterfaceID& LVL1::IeFEXTOBEtTool::interfaceID()
  {
    return IID_IeFEXTOBEtTool;
  }

} // end of namespace

#endif

