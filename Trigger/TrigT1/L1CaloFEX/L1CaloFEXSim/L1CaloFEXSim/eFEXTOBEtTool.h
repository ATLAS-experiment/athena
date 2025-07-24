/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXTOBEtTool.h  -  
//                              -------------------
//     begin                : 12 12 2022
//     email                : Alan.Watson@CERN.CH
//  ***************************************************************************/


#ifndef eFEXTOBEtTool_H
#define eFEXTOBEtTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/eFEXTOBEtTool.h"
#include "L1CaloFEXSim/eFEXtauAlgoBase.h"
#include "L1CaloFEXSim/eFEXegAlgo.h"
#include "L1CaloFEXSim/eTowerContainer.h"

#include <vector>

namespace LVL1 {
  //Doxygen class description below:
  /** The eFEXTOBEtTool class is a utility for recalculating the jet discriminant ("isolation")
      quantities that are not read out as part of the (x)TOB data
      Its purpose is:
      - To use the simulation tools to recalculate the various sums for a specified RoI coordinate
      It requires:
      - An eTower container has been filled before use (for the simulation tools to work)
      - That a TOB coordinate, seed cell and UpNotDown flag are provided
  */
  static const InterfaceID IID_IeFEXTOBEtTool("LVL1::eFEXTOBEtTool", 1, 0);

    class eFEXTOBEtTool : public AthAlgTool {
    
  public:
        static const InterfaceID& interfaceID() { return IID_IeFEXTOBEtTool; };
    /** Constructors */
    eFEXTOBEtTool(const std::string& type,const std::string& name,const IInterface* parent);

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize();
    /** Destructor */
    virtual ~eFEXTOBEtTool();

    /** Tool to calculate eEM discriminant sums */
    virtual
    StatusCode getegSums(float etaTOB, float phiTOB, int seed, int UnD, 
                                  std::vector<unsigned int> &ClusterCellETs,
                                  std::vector<unsigned int> &RetaSums,
                                  std::vector<unsigned int> &RhadSums, 
                                  std::vector<unsigned int> &WstotSums);
								  
    virtual
    StatusCode getTOBCellEnergies(float etaTOB, float phiTOB, std::vector<unsigned int> &ClusterCellETs) const;


    /** Tool to calculate eTaudiscriminant sums */
    virtual
    StatusCode gettauSums(float etaTOB, float phiTOB, int seed, int UnD, 
                                   std::vector<unsigned int> &RcoreSums,
                                   std::vector<unsigned int> &RemSums);

    /** Tool to find eTower identifier from an eta, phi coordinate pair */
    virtual
    unsigned int eTowerID(float eta, float phi) const;

    /** Tool to find eFEX and FPGA numbers and eta index of a TOB within the FPGA */
    virtual
    void location(float etaTOB, float phiTOB, int& eFEX, int& FPGA, int& fpgaEta) const;

    /** Internal data */
  private:

    const float m_dphiTower = M_PI/32;
    const float m_detaTower = 0.1;

    ToolHandle<eFEXtauAlgoBase> m_eFEXtauAlgoTool {
      this, "eFEXtauAlgoTool", "LVL1::eFEXtauAlgo", 
	"Tool that runs the eFEX tau algorithm"};
    ToolHandle<eFEXegAlgo> m_eFEXegAlgoTool {
      this, "eFEXegAlgoTool", "LVL1::eFEXegAlgo", 
	"Tool that runs the eFEX e/gamma algorithm"};
	
	// Key for input towers
    SG::ReadHandleKey<LVL1::eTowerContainer> m_eTowerContainerKey {this, "MyETowers", "eTowerContainer", "Input container for eTowers"};
    
  };
  
} // end of namespace

//CLASS_DEF( LVL1::eFEXTOBEtTool , 32201201 , 1 )


#endif
