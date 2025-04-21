/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ISGAudSvc.h 
// Header file for class ISGAudSvc
// Author: Ilija Vukotic<ivukotic@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENAKERNEL_ISGAUDSVC_H
#define ATHENAKERNEL_ISGAUDSVC_H

// FrameWork includes
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ClassID.h"

class ISGAudSvc : virtual public IService
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 
  DeclareInterfaceID(ISGAudSvc, 1, 0);

  virtual ~ISGAudSvc();
 
  virtual void SGAudit(const std::string& key, const CLID& id,
                       const int& fnc, const int& store_id) = 0;

  /// For custom increased granularity
  virtual void setFakeCurrentAlg(const std::string&) = 0;
  virtual void clearFakeCurrentAlg() = 0;

}; 

#endif 
