/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARDIGITIZATION_IFCALDIGITMODIFIERTOOL_H
#define LARDIGITIZATION_IFCALDIGITMODIFIERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/StatusCode.h"
#include "Identifier/HWIdentifier.h"

#include <span>

/**
 * Interface for a tool modelling the FCal signal degradation
 */
class IFCalDigitModifierTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IFCalDigitModifierTool, 1, 0);
  virtual ~IFCalDigitModifierTool() = default;

  virtual StatusCode modifySamples(const EventContext& ctx,
                                   const HWIdentifier& chId,
                                   std::span<double> samples) const = 0;
};

#endif // LARDIGITIZATION_IFCALDIGITMODIFIERTOOL_H
