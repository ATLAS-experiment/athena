/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef LArBadChannel2Ascii_H
#define LArBadChannel2Ascii_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include <string>

class LArBadChannel2Ascii : public AthAlgorithm 
{
public:
  LArBadChannel2Ascii(const std::string& name, ISvcLocator* pSvcLocator);
  ~LArBadChannel2Ascii();

  virtual StatusCode initialize() final;
  virtual StatusCode execute() final;

private:

  SG::ReadCondHandleKey<LArBadChannelCont> m_BCKey;
  SG::ReadCondHandleKey<LArBadFebCont> m_BFKey;
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey;
  
  std::string                  m_dbFolder;
  std::string                  m_fileName;
  std::string                  m_executiveSummaryFile;
  bool                         m_wMissing;
  bool                         m_skipDisconnected;
  bool                         m_isSC;

  enum DetPart {
    EMB=0,
    EMEC,
    HEC,
    FCAL,
    nParts
  };

  enum CoarseProblemType {
    DeadReadout=0,
    DeadPhys,
    DeadCalib,
    DeadFEB,
    Noisy,
    Sporadic,
    Distorted,
    PeakReco,
    Fibre,
    GrandTotalDead,
    nProblemTypes 
  };

  static void writeSum(std::ofstream& exeFile, const std::vector<unsigned>& probs, const std::vector<unsigned> nChans) ;


};

#endif
