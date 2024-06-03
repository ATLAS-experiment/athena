/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMACALOTOOLS_EGAMMACELLRECOVERYTOOL_H
#define EGAMMACALOTOOLS_EGAMMACELLRECOVERYTOOL_H

/// @class egammaCellRecoveryTool
/// @brief tool to recover cells lost because of the topocluster timing cut

#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODCaloEvent/CaloClusterFwd.h"
#include "egammaInterfaces/IegammaCellRecoveryTool.h"

class egammaCellRecoveryTool : public AthAlgTool, virtual public IegammaCellRecoveryTool {

 public:

  /** @brief Default constructor*/
  egammaCellRecoveryTool(const std::string& type,
	    const std::string& name,
	    const IInterface* parent);

  /** @brief Destructor*/
  ~egammaCellRecoveryTool() = default; 

  /** @brief initialize method*/
  StatusCode initialize() override;
  /** @brief finalize method*/
  StatusCode finalize() override { return StatusCode::SUCCESS; }
  /** @brief  Method to just calculate hadronic leakage*/
  virtual StatusCode execute(const xAOD::CaloCluster& cluster,
			     Info& info) const override final;

 private:

  // time cut to recover, is this safe enough given the rounding of time ?
  static constexpr double m_timeCut = 12.;

  // phi size of layer 2 and 3 cells
  static constexpr double m_phiSize = 2*M_PI/256;
  
  // Sizes of the window to search for missing cells
  static const int m_nL2 = 5*7;
  static const int m_nL3 = 10;

  struct existingCells {
    bool existL2[m_nL2];
    bool existL3[m_nL3];
  };

  existingCells buildCellArrays(const xAOD::CaloCluster *clus,
				double etamax, double phimax) const;
  
};

#endif
