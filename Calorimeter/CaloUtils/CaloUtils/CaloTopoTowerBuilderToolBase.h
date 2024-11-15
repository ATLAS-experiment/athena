/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOUTILS_CALOTOPOTOWERBUILDERTOOLBASE_H
#define CALOUTILS_CALOTOPOTOWERBUILDERTOOLBASE_H

/**
 * @brief  CaloTopoTowerBuilderToolBase is abstract base class for tower builders
 *
 * @author Peter Loch <loch@physics.arizona.edu>
 * @date   April 30, 2004 - first implementation
 */

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloInterface/ICaloTopoTowerBuilderToolBase.h"
#include "CaloEvent/CaloTowerSeg.h"

class CaloTopoTowerBuilderToolBase : public extends<AthAlgTool, ICaloTopoTowerBuilderToolBase>
{
 public:
  
  /// AlgTool constructor
  CaloTopoTowerBuilderToolBase(const std::string& name
			       , const std::string& type
			       , const IInterface* parent);
  virtual ~CaloTopoTowerBuilderToolBase();

  /// common initialization
  virtual StatusCode initialize() override;

  virtual void setTowerSeg(const CaloTowerSeg& theTowerSeg) override;

 protected:
  CaloTowerSeg m_theTowerSeg;
};

#endif
