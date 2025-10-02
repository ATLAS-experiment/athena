/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARALIGNMENTALGS_LARALIGNDBALG_H
#define LARALIGNMENTALGS_LARALIGNDBALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "RegistrationServices/IIOVRegistrationSvc.h"

class IIOVRegistrationSvc;

/**
 ** Algorithm for writing LAr alignment constants to Cond DB and reading them back.
 **/

class LArAlignDbAlg: public AthAlgorithm 
{
 public:
  LArAlignDbAlg(const std::string& name, ISvcLocator* pSvcLocator);
  ~LArAlignDbAlg();
  
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;
  
 private:

  StatusCode createCondObjects();
  StatusCode printCondObjects();
  StatusCode streamOutCondObjects();
  StatusCode registerCondObjects();
  
  StatusCode registerIOV(const CLID& clid);
  
  BooleanProperty           m_writeCondObjs{this, "WriteCondObjs", false};
  BooleanProperty           m_regIOV{this, "RegisterIOV", false};
  StringProperty            m_streamName{this, "StreamName", "CondStream1"};
  StringProperty            m_inpFile{this, "InpFile", "LArAlign.inp"};
  StringProperty            m_outpFile{this, "OutpFile", "LArAlign-TEST.pool.root"};
  StringProperty            m_outpTag{this, "TagName", "LARAlign-TEST"};

  ServiceHandle<IIOVRegistrationSvc>   m_regSvc;
  ToolHandle<IAthenaOutputStreamTool>  m_streamer;
};

#endif // LARALIGNDBALG_LARALIGNDBALG_H
