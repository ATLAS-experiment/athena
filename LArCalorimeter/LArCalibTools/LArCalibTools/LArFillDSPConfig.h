//-*- C++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARCALIBTOOLS_LARFILLDSPCONFIG_H
#define LARCALIBTOOLS_LARFILLDSPCONFIG_H

#include <string>
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

class LArOnlineID;

class LArFillDSPConfig: public AthAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 
  /// Constructor with parameters: 
  LArFillDSPConfig( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~LArFillDSPConfig(); 

  // Athena algorithm's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext&) override {return StatusCode::SUCCESS;}
  virtual StatusCode  stop() override;

 private: 
  /// Default constructor: 
  LArFillDSPConfig();
  const LArOnlineID* m_onlineID;

  std::string m_folderName;
  bool m_dump;
  bool m_lowmu;
}; 

#endif 
