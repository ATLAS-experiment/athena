/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCELLREC_FCALMUCORRTOOL_H
#define LARCELLREC_FCALMUCORRTOOL_H

#include "CaloInterface/ICaloCellMakerTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArRecConditions/LArFcalDegFactors.h"

class CaloCellContainer;
class CaloCell_ID;

class FCalMuCorrTool : public extends<AthAlgTool, ICaloCellMakerTool> {
public:
  using base_class::base_class;

  virtual StatusCode initialize() override final;
  virtual StatusCode process(CaloCellContainer* cellCollection, const EventContext& ctx) const override final;

private:
  Gaudi::Property<double> m_muOverride{this, "MuOverride", -1.,
      "If > 0, use this mu instead of averageInteractionsPerCrossing from EventInfo"};
  SG::ReadCondHandleKey<LArFcalDegFactors> m_degKey{this, "FcalDegKey", "LArFcalDegMap",
      "FCal1 degradation table D(channel, mu) from the conditions database"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey", "EventInfo",
      "EventInfo with the average mu of the lumi block"};

  const CaloCell_ID* m_caloID{nullptr};
};

#endif // LARCELLREC_FCALMUCORRTOOL_H
