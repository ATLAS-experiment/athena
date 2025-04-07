/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-
#ifndef CALOREC_CALOCELLCONTAINERCHECKERTOOL_H
#define CALOREC_CALOCELLCONTAINERCHECKERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloInterface/ICaloCellMakerTool.h"
#include "CaloInterface/ICaloConstCellMakerTool.h"


class CaloCell_ID;

class CaloCellContainerCheckerTool
  : public extends<AthAlgTool, ICaloCellMakerTool, ICaloConstCellMakerTool>
{
 
public:    
  using base_class::base_class;

  virtual StatusCode initialize() override; 

  virtual StatusCode process (CaloCellContainer* theCellContainer,
                              const EventContext& ctx) const override;
  virtual StatusCode process (CaloConstCellContainer* theCellContainer,
                              const EventContext& ctx) const override;

 private:
  StatusCode doProcess (const CaloCellContainer* theCellContainer,
                        const EventContext& ctx) const;

  Gaudi::Property<unsigned> m_eventsToCheck{this,"EventsToCheck",5};
  const CaloCell_ID* m_theCaloCCIDM  = nullptr;
  
};

#endif

