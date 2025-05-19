/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SGTOOLS_SGCOMMITAUDITOR_H
#define SGTOOLS_SGCOMMITAUDITOR_H

#include "Gaudi/Auditor.h"
#include "Gaudi/IAuditor.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/IHiveStoreMgr.h"

/////////////////////////////////////////////////////////////////////////
//
// SGCommitAuditor
//
// Auditor for use with GaudiHive, causes DataObjects recorded during
// the preceeding Algorithm's execute method to be made known to the
// WhiteBoard
//
// Author: C. Leggett
// Date: 2015-02-03
/////////////////////////////////////////////////////////////////////////

class SGCommitAuditor: public Gaudi::Auditor {

public:
  SGCommitAuditor(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~SGCommitAuditor();
  
  virtual StatusCode initialize() override;

  virtual void after(const std::string& event, const std::string& name,
                     const EventContext&, const StatusCode&) override;

private:

  ServiceHandle<IHiveStoreMgr> p_sg;

};

#endif
