/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LArTBH6TriggerTimeTool_H
#define LArTBH6TriggerTimeTool_H

#include "AthenaKernel/ITriggerTime.h"
#include "GaudiKernel/IIncidentListener.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "StoreGate/ReadHandle.h"
#include "LArSimEvent/LArHitContainer.h"

class LArTBH6TriggerTimeTool : public extends<AthAlgTool, ITriggerTime, IIncidentListener>
{

public:
  LArTBH6TriggerTimeTool(const std::string& type,
                         const std::string& name,
                         const IInterface* parent);
  
  
  virtual StatusCode initialize() override;

  virtual ~LArTBH6TriggerTimeTool() = default;
  
  /// returns the time offset of the current trigger
  virtual double time() override;

  virtual void handle(const Incident& incident) override;

  double larTime();
  double trackRecordTime();

private:
  Gaudi::Property<double> m_time{this, "FixedTime", 0.};
  Gaudi::Property<bool> m_fixed{this, "isFixed", true};
  bool m_newEvent{true};
  std::vector< SG::ReadHandle< LArHitContainer > > m_hitcoll;
};


#endif
