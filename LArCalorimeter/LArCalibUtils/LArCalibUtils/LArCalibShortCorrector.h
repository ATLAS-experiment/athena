/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-
#ifndef LARCALIBSHORTCORRECTOR_H
#define LARCALIBSHORTCORRECTOR_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"

#include "LArElecCalib/ILArPedestal.h"
#include "CxxUtils/checker_macros.h"

#include <vector>

class LArAccumulatedCalibDigit;
class CaloCell_ID;
class LArOnlineID;
class HWIdentifier;


class ATLAS_NOT_THREAD_SAFE LArCalibShortCorrector : public AthAlgorithm
{
 public:
  LArCalibShortCorrector(const std::string & name, ISvcLocator * pSvcLocator);
  ~LArCalibShortCorrector();

  //standard algorithm methods
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;


 private:

  StatusCode findShortedNeighbors();

  SG::ReadCondHandleKey<LArBadChannelCont> m_BCKey {this, "BadChanKey", "LArBadChannel", "SG key for LArBadChan object"};
  SG::ReadCondHandleKey<LArOnOffIdMapping>  m_cablingKey{this, "OnOffMap", "LArOnOffIdMap", "SG key for mapping object"};
    
  const LArOnlineID*  m_onlineId;
  const CaloCell_ID*  m_caloCellId;
  
  //Algorithm-Properties:
  std::vector<std::string> m_keylist;
  std::string m_pedKey;

  //private members
  std::vector<std::pair<HWIdentifier,HWIdentifier> > m_shortedNeighbors;
  typedef std::vector<std::pair<HWIdentifier,HWIdentifier> >::const_iterator SHORT_IT;
 
  bool m_shortsCached;
};

#endif
