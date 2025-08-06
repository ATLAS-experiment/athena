/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GLOBALSIM_EMB1CELLSFROMCALOCELLS_H
#define GLOBALSIM_EMB1CELLSFROMCALOCELLS_H


/* Obtain the cells in the EMB1 sampling layer of the LArEM (strips) */

#include "ICaloCellsProducer.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloEvent/CaloConstCellContainer.h"

namespace GlobalSim {
  class EMB1CellsFromCaloCells:
    public extends<AthAlgTool, ICaloCellsProducer> {

  public:
  
    EMB1CellsFromCaloCells(const std::string& type,
			   const std::string& name,
			   const IInterface* parent);

    virtual ~EMB1CellsFromCaloCells(){};

    virtual StatusCode initialize() override;
    virtual StatusCode cells(std::vector<const CaloCell*>&,
			     const EventContext&) const override;

  private:

    // CaloCells
    SG::ReadHandleKey<CaloCellContainer> m_caloCellsKey {
      this, "caloCells", "SeedLessFS", "key to read in a CaloCell container"};

    Gaudi::Property<bool> m_makeCaloCellContainerChecks {
      this,
      "makeCaloCellContainerChecks",
      true,
      "flag to run checks on read-in CaloCellContainer"};
  };
}
#endif

