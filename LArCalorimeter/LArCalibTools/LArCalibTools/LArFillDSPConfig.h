//-*- C++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARCALIBTOOLS_LARFILLDSPCONFIG_H
#define LARCALIBTOOLS_LARFILLDSPCONFIG_H

#include <string>
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

class LArOnlineID;

class LArFillDSPConfig: public AthReentrantAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// Destructor: 
  virtual ~LArFillDSPConfig(); 

  // Athena algorithm's Hooks
  virtual StatusCode  execute(const EventContext&) const override {return StatusCode::SUCCESS;}
  virtual StatusCode  stop() override;

 private: 
  /// Default constructor: 
  LArFillDSPConfig() = delete;
  const LArOnlineID* m_onlineID = nullptr;

  StringProperty m_folderName { this, "Foldername", "/LAR/Configuraton/DSPConfiguration" };
  BooleanProperty m_dump  { this, "Dump", true };
  BooleanProperty m_lowmu { this, "isLowMu", false };
}; 

#endif 
