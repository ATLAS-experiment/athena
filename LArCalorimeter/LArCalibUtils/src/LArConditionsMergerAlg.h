//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCALIBUTILS_LARCONDITIONSMERGERALG
#define LARCALIBUTILS_LARCONDITIONSMERGERALG

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "LArRawConditions/LArConditionsContainer.h"
#include "LArRawConditions/LArPhysWaveContainer.h"

template<class T, class T1=LArPhysWaveContainer>
class LArConditionsMergerAlg: public AthAlgorithm {
 public:
  using AthAlgorithm::AthAlgorithm;
  ~LArConditionsMergerAlg() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  
  SG::ReadCondHandleKeyArray<T> m_readKeys{this, "ReadKeys",{},"Input keys" };
  Gaudi::Property<std::string> m_writeKey{this,"WriteKey","output","Output key" };

  Gaudi::Property<std::string> m_groupingType{this,"GroupingType","","Cool-channel grouping"};

  //SG::ReadHandleKeyArray<T1> m_detStoreKeys{this, "DetStoreReadKeys",{},"Input keys to read from DetStore"};
  Gaudi::Property<std::vector<std::string> > m_detStoreKeys{this, "DetStoreReadKeys",{},"Input keys to read from DetStore"};
};

#include "LArConditionsMergerAlg.icc"

#include "LArRawConditions/LArPedestalComplete.h"
typedef LArConditionsMergerAlg<LArPedestalComplete>  LArPedestalMerger;

#include "LArRawConditions/LArOFCComplete.h"
typedef LArConditionsMergerAlg<LArOFCComplete>  LArOFCMerger;

#include "LArRawConditions/LArShapeComplete.h"
typedef LArConditionsMergerAlg<LArShapeComplete>  LArShapeMerger;

#include "LArRawConditions/LArRampComplete.h"
typedef LArConditionsMergerAlg<LArRampComplete>  LArRampMerger;

#include "LArRawConditions/LArMphysOverMcalComplete.h"
typedef LArConditionsMergerAlg<LArMphysOverMcalComplete>  LArMphysOverMcalMerger;

#include "LArRawConditions/LArAutoCorrComplete.h"
typedef LArConditionsMergerAlg<LArAutoCorrComplete>  LArAutoCorrMerger;

// misusing one complete conditions object for merger reading from DetStore
#include "LArRawConditions/LArDAC2uAComplete.h"

typedef LArConditionsMergerAlg<LArDAC2uAComplete, LArPhysWaveContainer>  LArPhysWaveMerger;

#include "LArRawConditions/LArCaliWaveContainer.h"
typedef LArConditionsMergerAlg<LArDAC2uAComplete, LArCaliWaveContainer>  LArCaliWaveMerger;

#include "LArRawConditions/LArCaliPulseParamsComplete.h"
typedef LArConditionsMergerAlg<LArDAC2uAComplete, LArCaliPulseParamsComplete>  LArCaliPulseParamsMerger;

#include "LArRawConditions/LArDetCellParamsComplete.h"
typedef LArConditionsMergerAlg<LArDAC2uAComplete, LArDetCellParamsComplete>  LArDetCellParamsMerger;


#endif
