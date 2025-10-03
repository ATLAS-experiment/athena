/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef LArBadChannel2Ascii_H
#define LArBadChannel2Ascii_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include <string>
#include <vector>
#include <iosfwd> //ofstream

class LArBadChannel2Ascii : public AthAlgorithm 
{
public:

  //Delegate constructor
  using AthAlgorithm::AthAlgorithm;
  
  ~LArBadChannel2Ascii() =default;

  virtual StatusCode initialize() final;
  virtual StatusCode execute() final;

private:

  SG::ReadCondHandleKey<LArBadChannelCont> m_BCKey{this,"BCKey","LArBadChannel"};
  SG::ReadCondHandleKey<LArBadFebCont> m_BFKey{this,"BFKey","LArBadFeb"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"LArOnOffIdMapKey","LArOnOffIdMap"};
  
  Gaudi::Property<std::string>  m_fileName{this,"FileName",""};
  Gaudi::Property<std::string>  m_executiveSummaryFile{this,"ExecutiveSummaryFile",""};
  Gaudi::Property<bool>         m_wMissing{this,"WithMissing",false};
  Gaudi::Property<bool>         m_skipDisconnected{this,"SkipDisconnected",true};
  Gaudi::Property<bool>         m_isSC{this,"SuperCell",false};

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

  void writeSum(std::ofstream& exeFile, const std::vector<unsigned>& probs, const std::vector<unsigned>& nChans) const ;
};

#endif
