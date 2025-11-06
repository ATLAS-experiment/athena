/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#ifndef EVGENPRODTOOLS_FILLFILTERVALUES_H
#define EVGENPRODTOOLS_FILLFILTERVALUES_H

#include "GeneratorModules/GenBase.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

/**
 * @class FillFilterValues
 * @brief Copy MC gen values we filter on into the event info store
 *
 * @author E.M. Lobodzinska <ewelina.maria.lobodzinska@cern.ch>
 * @author Andy Buckley <andy.buckley@cern.ch>
 *
 **/

class FillFilterValues : public GenBase {
public:

  /// Constructor
  FillFilterValues(const std::string& name, ISvcLocator* svcLoc);

  /// Copy the filter values for each event
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

private:

SG::WriteDecorHandleKey<xAOD::EventInfo> m_mcFilterHTKey {this
      , "mcFilterHTKey"
      , "TMPEvtInfo.mcFilterHT"
      , "Decoration for MC Filter HT"};

SG::WriteDecorHandleKey<xAOD::EventInfo> m_mcFilterMETKey {this
      , "mcFilterMETKey"
      , "TMPEvtInfo.mcFilterMET" 
      , "Decoration for MC Filter MET"};


};


#endif

#endif
