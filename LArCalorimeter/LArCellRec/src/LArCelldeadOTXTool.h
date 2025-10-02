/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCELLREC_LArCelldeadOTXTool_H
#define LARCELLREC_LArCelldeadOTXTool_H


#include "CaloInterface/ICaloCellMakerTool.h"
#include "LArRawEvent/LArRawSCContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArRecConditions/LArDeadOTXCorrFactors.h"

class CaloCellContainer;

class LArCelldeadOTXTool : public extends<AthAlgTool, ICaloCellMakerTool>  {
public:
  using base_class::base_class;

  ~LArCelldeadOTXTool() = default;
  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;
  //Implements the ICaloCellMaker interface
  virtual StatusCode process(CaloCellContainer* cellCollection, const EventContext& ctx) const override final;

 private: 
  SG::ReadHandleKey<LArRawSCContainer>  m_SCKey{this, "keySC", "SC_ET","Key for SuperCells container"};
  SG::ReadCondHandleKey<LArDeadOTXCorrFactors> m_factors{this,"SCFactors","LArDeadOTXCorrFactors"};
  Gaudi::Property<int> m_scCut{this,"SCEneCut",70,"Do not use super-cells with values below this cut"};
  Gaudi::Property<bool> m_testMode{this,"TestMode",false};
  
  mutable std::unordered_map<int,std::pair<float,int> > m_testMap ATLAS_THREAD_SAFE; //Only used in testMode + mtx-protected
  mutable std::mutex m_mtx;

  mutable std::atomic<int> m_nWarnings{0};

};

#endif     
