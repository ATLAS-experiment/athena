/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IOVSVC_CONDSVC_H
#define IOVSVC_CONDSVC_H

#include "GaudiKernel/ICondSvc.h"
#include "GaudiKernel/Service.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaBaseComps/AthService.h"
#include "ICondSvcSetupDone.h"

#include <unordered_map>
#include <set>
#include <vector>
#include <mutex>

class ConditionSlotFuture;
class ICondtionIOSvc;

class CondSvc final: public extends<AthService, ICondSvc, ICondSvcSetupDone> {
public:

  CondSvc(const std::string& name, ISvcLocator* svc);

  virtual StatusCode initialize() override;
  virtual StatusCode start() override;
  virtual StatusCode stop() override;

  virtual StatusCode regHandle(IAlgorithm* alg, const Gaudi::DataHandle& id) override;

  virtual bool isValidID(const EventContext&, const DataObjID&) const override;

  virtual const std::set<IAlgorithm*>& condAlgs() const override { return m_condAlgs; }
  virtual bool isRegistered(const DataObjID& id) const override { return m_condIDs.contains(id); }
  virtual bool isRegistered(IAlgorithm* ialg) const override { return m_condAlgs.contains(ialg); }
  virtual const DataObjIDColl& conditionIDs() const override { return m_condIDs; }

  virtual StatusCode validRanges( std::vector<EventIDRange>& ranges,
                                  const DataObjID& id ) const override;

  virtual void dump(std::ostream&) const override;

  /// To be called after changes to the set of conditions containers
  /// in the conditions store.
  /// May not be called concurrently with any other methods of this class.
  virtual StatusCode setupDone() override;


  ///@{ unimplemented interfaces

  /// Asynchronously setup conditions
  virtual ConditionSlotFuture* startConditionSetup(const EventContext&) override {
    return nullptr;
  }

  /// register an IConditionIOSvc (alternative to Algorithm processing of 
  /// Conditions)
  virtual StatusCode registerConditionIOSvc(IConditionIOSvc*)  override {
    return StatusCode::FAILURE;
  }
  ///@}


private:

  StatusCode regHandle_i(IAlgorithm* alg, const Gaudi::DataHandle& id);

  ServiceHandle<StoreGateSvc> m_sgs;

  std::set<IAlgorithm*> m_condAlgs;
  DataObjIDColl m_condIDs;

  /// Map from DataObjID to Algorithm to avoid duplicates
  std::unordered_map<DataObjID, IAlgorithm*, DataObjID_Hasher> m_idMap;

  /// Map from DataObjID to CondContBase (populated in setupDone)
  std::unordered_map<DataObjID, const CondContBase*, DataObjID_Hasher> m_condConts;

  mutable std::mutex m_lock;

};

#endif
