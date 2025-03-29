/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRT_CONDITIONSSERVICES_TRT_CONDITIONSSUMMARYSVC_H
#define TRT_CONDITIONSSERVICES_TRT_CONDITIONSSUMMARYSVC_H

/**
 * @file TRT_ConditionsSummarySvc.h
 * @author Christian.Schmitt@cern.ch, Denver.Whittington@cern.ch
**/

//STL includes
#include <vector>
#include <string>
//Gaudi Includes
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ServiceHandle.h"

//interface includes
#include "InDetConditionsSummaryService/IInDetConditionsSvc.h"
#include "TRT_ConditionsServices/ITRT_ConditionsSvc.h"

//forward declarations
class ISvcLocator;
class Identifier;
class IdentifierHash;
class StatusCode;
class TRT_ID;
namespace InDetDD {
  class TRT_DetectorManager;
}

/**
 * @class TRT_ConditionsSummarySvc
 * Service providing summary of status of an TRT detector element
 * Interface is IInDetConditionsSvc class
**/
class TRT_ConditionsSummarySvc :
  public extends<AthService, IInDetConditionsSvc>
{
 public:
  TRT_ConditionsSummarySvc( const std::string& name, ISvcLocator* svc );//!< Service constructor
  virtual ~TRT_ConditionsSummarySvc();
  //@name Gaudi Service Implementation
  //@{
  virtual StatusCode initialize() override;          //!< Service init
  //@}
  
  //@name reimplemented from IInDetConditionsSvc
  //@{
  virtual bool isActive(const Identifier & elementId, const InDetConditions::Hierarchy h=InDetConditions::DEFAULT) override;
  virtual bool isActive(const IdentifierHash & elementHash) override;
  virtual bool isActive(const IdentifierHash & elementHash, const Identifier & elementId) override;
  virtual double activeFraction(const IdentifierHash & elementHash, const Identifier & idStart, const Identifier & idEnd) override;
  virtual bool isGood(const Identifier & elementId, const InDetConditions::Hierarchy h=InDetConditions::DEFAULT) override;
  virtual bool isGood(const IdentifierHash & elementHash) override;
  virtual bool isGood(const IdentifierHash & elementHash, const Identifier & elementId) override;
  virtual double goodFraction(const IdentifierHash & elementHash, const Identifier & idStart, const Identifier & idEnd) override;
  //@}

private:
  ServiceHandleArray<ITRT_ConditionsSvc> m_svcCollection;
  InDet::TRT_CondFlag condSummaryStatus( const Identifier & ident);

  const InDetDD::TRT_DetectorManager* m_manager{nullptr};
  const TRT_ID* m_trtid{nullptr};

};

#endif // TRT_ConditionsSummarySvc_h
