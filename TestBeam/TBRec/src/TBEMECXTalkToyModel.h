/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBEMECXTALKTOYMODEL_H
#define TBREC_TBEMECXTALKTOYMODEL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloInterface/ICaloCellMakerTool.h"
#include "CaloEvent/CaloCellContainer.h"

#include "GaudiKernel/ToolHandle.h"
#include "CaloIdentifier/CaloCell_ID.h"

class TBEMECXTalkToyModel
  : public extends<AthAlgTool, ICaloCellMakerTool>
{
public:    
  TBEMECXTalkToyModel(const std::string& type, 
                      const std::string& name, 
                      const IInterface* parent) ;

  virtual StatusCode initialize() override;
  // update theCellContainer
  virtual StatusCode process (CaloCellContainer* theCellContainer,
                              const EventContext& ctx) const override;


 private:
// properties

  Gaudi::Property<std::vector<int>> m_caloNums{this, "CaloNums", {static_cast<int>(CaloCell_ID::NSUBCALO)}, "which calo to correct (default: all)"};
  //reminder  enum SUBCALO { LAREM = 0, LARHEC = 1, LARFCAL = 2, TILE = 3, NSUBCALO = 4, NOT_VALID=999999 };

  bool m_caloSelection{false};

  const CaloCell_ID * m_calo_id{nullptr};


  Gaudi::Property<float> m_xtalkScaleLong{this, "XTalkScaleLong", 0, "Longitudinal cross-talk scale"};
  Gaudi::Property<float> m_xtalkScaleEta{this, "XTalkScaleEta", 0, "Eta cross-talk scale"};
  Gaudi::Property<float> m_xtalkScaleEMEC2Eta{this, "XTalkScaleEMEC2Eta", 0, "EMEC2 eta cross-talk scale"};

  StatusCode processOnCellIterators(const CaloCellContainer::iterator  &  itrCellBeg, const CaloCellContainer::iterator & itrCellEnd ) const;
  

};

#endif

