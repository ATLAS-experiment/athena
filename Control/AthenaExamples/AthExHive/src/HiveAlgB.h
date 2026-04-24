/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  HiveAlgB.h
 * @brief Simple Algorithm that writes a single HiveDataObj to the store.
 * It will also accumulate some data on a per-slot basis using a 
 * ContextSpecificData<int>
 */


#ifndef ATHEXHIVE_ALGB_H
#define ATHEXHIVE_ALGB_H 1

#include "HiveAlgBase.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthExHive/HiveDataObj.h"
#include "GaudiKernel/ContextSpecificPtr.h"

#include <string>

class HiveAlgB : public HiveAlgBase {
  
public:
  
  // Standard Algorithm Constructor:
  
  HiveAlgB (const std::string& name, ISvcLocator* pSvcLocator);
  ~HiveAlgB();
  
  // Define the initialize, execute and finalize methods:
  
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;
  
private:
  
  void dump();

  SG::WriteHandleKey<HiveDataObj> m_wrh1 {this, "Key_W1", "b1", "WHK 1"};

  // context specific data: which hides an array structure behind a simple
  // interface, keeping separate versions of the data for each concurrent
  // event (slot)
  mutable Gaudi::Hive::ContextSpecificData<int> m_di ATLAS_THREAD_SAFE;

};
#endif
