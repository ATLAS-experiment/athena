//Dear emacs, this is -*- C++ -*- 

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARRECUTILS_LARCALIBLINEMAPPINGALG_H
#define LARRECUTILS_LARCALIBLINEMAPPINGALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "LArRecConditions/LArCalibLineMapping.h"
#include "PersistentDataModel/AthenaAttributeList.h"

class LArCalibLineMappingAlg: public AthCondAlgorithm {

public:
  //Delegate constructor
  using AthCondAlgorithm::AthCondAlgorithm;

  virtual ~LArCalibLineMappingAlg() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadCondHandleKey<AthenaAttributeList>   m_readKey{this,"ReadKey","/LAR/Identifier/CalibIdMap"};
  SG::WriteCondHandleKey<LArCalibLineMapping>  m_writeKey{this,"WriteKey","LArCalibLineMap"};
  Gaudi::Property<bool> m_isSuperCell{this,"isSuperCell",false};
  Gaudi::Property<unsigned> m_maxCalibLines{this,"MaxCL",4,"in case of SuperCell should be set to higher value then default 4"};

};



#endif
