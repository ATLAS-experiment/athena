//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef CHECKLARFEBHEADERS_H
#define CHECKLARFEBHEADERS_H
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h"
#include "LArIdentifier/LArOnlineID.h"

// Small algorithm to test TTC information between FEB and ROD
// All this is stored in FebHeader objects
// Created July 17th 2004 by R. Lafaye

class CheckLArFebHeader : public AthAlgorithm
{
 public:
  using AthAlgorithm::AthAlgorithm;

  ~CheckLArFebHeader();

  //standart algorithm methods
  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();

 private:
  int m_count{0};
  const LArOnlineID* m_onlineHelper{nullptr};
};


#endif
