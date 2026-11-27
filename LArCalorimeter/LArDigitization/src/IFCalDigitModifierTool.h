/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARDIGITIZATION_IFCALDIGITMODIFIERTOOL_H
#define LARDIGITIZATION_IFCALDIGITMODIFIERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/StatusCode.h"
#include "Identifier/Identifier.h"
#include "LArRecConditions/LArFcalDegFactors.h"

#include <span>

/**
 * Interface for a tool modelling the FCal signal degradation in digitization.
 * prepareEvent() reads the per-event input once per event; modifySamples() is then called
 * for each FCal cell with this input.
 */
class IFCalDigitModifierTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IFCalDigitModifierTool, 2, 0);
  virtual ~IFCalDigitModifierTool() = default;

  /// Per-event input of the tool, filled once per event by prepareEvent()
  struct EventData {
    const LArFcalDegFactors* degradation{nullptr};
    double mu{0.};
  };

  virtual StatusCode prepareEvent(const EventContext& ctx, EventData& data) const = 0;

  /// Energy samples of one FCal cell, before noise and pedestal are added
  virtual void modifySamples(const EventData& data,
                             const Identifier& cellId,
                             std::span<double> samples) const = 0;
};

#endif // LARDIGITIZATION_IFCALDIGITMODIFIERTOOL_H
