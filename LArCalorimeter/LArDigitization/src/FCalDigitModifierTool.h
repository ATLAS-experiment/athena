/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARDIGITIZATION_FCALDIGITMODIFIERTOOL_H
#define LARDIGITIZATION_FCALDIGITMODIFIERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "LArRecConditions/LArFcalDegFactors.h"
#include "IFCalDigitModifierTool.h"

class CaloCell_ID;

/**
 * FCal1 signal degradation from ion build-up at high luminosity: the energy samples of each
 * FCal1 cell are multiplied by the amplitude factor D(channel, mu), see FCalAmplitudeDegradation.
 */
class FCalDigitModifierTool : public extends<AthAlgTool, IFCalDigitModifierTool> {
public:
  using base_class::base_class;

  virtual StatusCode initialize() override;

  virtual StatusCode prepareEvent(const EventContext& ctx, EventData& data) const override;

  virtual void modifySamples(const EventData& data,
                             const Identifier& cellId,
                             std::span<double> samples) const override;

private:
  Gaudi::Property<double> m_muOverride{this, "MuOverride", -1.,
      "If > 0, use this mu instead of averageInteractionsPerCrossing from EventInfo"};
  SG::ReadCondHandleKey<LArFcalDegFactors> m_degKey{this, "FcalDegKey", "LArFcalDegMap",
      "FCal1 degradation table D(channel, mu) from the conditions database"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey", "EventInfo",
      "EventInfo with the average mu of the lumi block"};

  const CaloCell_ID* m_caloID{nullptr};
};

#endif // LARDIGITIZATION_FCALDIGITMODIFIERTOOL_H
