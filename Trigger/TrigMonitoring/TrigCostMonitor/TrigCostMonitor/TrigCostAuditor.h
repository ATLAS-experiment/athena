/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGCOSTMONITOR_TRIGCOSTAUDITOR_H
#define TRIGCOSTMONITOR_TRIGCOSTAUDITOR_H 1

#include "Gaudi/Auditor.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaBaseComps/AthMessaging.h"

#include "ITrigCostSvc.h"

/**
 * @class TrigCostAuditor
 * @brief Gaudi Auditor implementation to hook algorithm executions and notify the Trigger Cost Service
 *
 * Only monitors the Execute event type.
 */
class TrigCostAuditor : public Gaudi::Auditor, public AthMessaging {

  using AthMessaging::msg;

  public:

  /**
   * @brief Standard Gaudi Auditor constructor
   * @param[in] name The algorithm object's name
   * @param[in] svcloc A pointer to a service location service
   */
  TrigCostAuditor( const std::string& name, ISvcLocator* svcloc );

  /**
   * @brief Initialise auditor. Return handle to Trigger Cost Service
   * @return Success if service handle obtained
   */
  virtual StatusCode initialize() override;

  /**
   * @brief Does nothing
   * @return Success
   */
  virtual StatusCode finalize() override;


  /**
   * @brief Audit before an algorithm standard event type is called
   * @param[in] evt The event type. Only Execute is monitored
   * @param[in] caller The name of the calling algorithm
   * @param[in] ctx Event context
   */
  virtual void before(const std::string& event, const std::string& caller,
                      const EventContext& ctx) override;

  /**
   * @brief Audit after an algorithm standard event type is called
   * @param[in] evt The event type. Only Execute is monitored
   * @param[in] caller The name of the calling algorithm
   * @param[in] ctx Event context
   * @param[in] sc StatusCode of algorithm execution
   */
  virtual void after(const std::string& event, const std::string& caller,
                     const EventContext& ctx, const StatusCode& sc) override;

private:

  /**
   * @brief Performs internal call to the trigger cost service
   * @param[in] caller Name of algorithm being audited
   * @param[in] type AuditType::Before or AuditType::After depending on if the start or stop of execution
   * @param[in] ctx Event context
   */
  void callService(const std::string& caller, ITrigCostSvc::AuditType type, const EventContext& ctx);

  ServiceHandle<ITrigCostSvc> m_trigCostSvcHandle { this, "TrigCostSvc", "TrigCostSvc", 
    "The trigger cost service to pass audit information to" };

};

#endif // TRIGCOSTMONITOR_TRIGCOSTAUDITOR_H
