/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOREC_CALOCELLCONTAINERCORRECTORTOOL_H
#define CALOREC_CALOCELLCONTAINERCORRECTORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloInterface/ICaloCellMakerTool.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloUtils/CaloCellCorrection.h"

#include "GaudiKernel/ToolHandle.h"

class CaloCellContainerCorrectorTool
  : public extends<AthAlgTool, ICaloCellMakerTool>
{
public: 
  using base_class::base_class;

  virtual StatusCode initialize() override;
  // update theCellContainer
  virtual StatusCode process ( CaloCellContainer* theCellContainer,
                               const EventContext& ctx ) const override;


 private:
// properties

  Gaudi::Property<std::vector<int> >m_caloNums{this,"CaloNums",{1,static_cast<int>(CaloCell_ID::NSUBCALO)} } ; // which calo to correct
  //reminder  enum SUBCALO { LAREM = 0, LARHEC = 1, LARFCAL = 2, TILE = 3, NSUBCALO = 4, NOT_VALID=999999 };

  ToolHandleArray<CaloCellCorrection> m_cellCorrectionTools{this,"CellCorrectionToolNames",{}};

  bool m_caloSelection=false;

  StatusCode processOnCellIterators(const CaloCellContainer::iterator  &  itrCellBeg,
                                    const CaloCellContainer::iterator & itrCellEnd,
                                    const EventContext& ctx) const;
  

};

#endif

