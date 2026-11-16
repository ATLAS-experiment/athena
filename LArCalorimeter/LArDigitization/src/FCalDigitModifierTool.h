/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARDIGITIZATION_FCALDIGITMODIFIERTOOL_H
#define LARDIGITIZATION_FCALDIGITMODIFIERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "IFCalDigitModifierTool.h"

/**
 * First implementation of the FCal signal degradation model.
 */
class FCalDigitModifierTool : public extends<AthAlgTool, IFCalDigitModifierTool> {
public:
  using base_class::base_class;

  virtual StatusCode initialize() override;

  virtual StatusCode modifySamples(const EventContext& ctx,
                                   const HWIdentifier& chId,
                                   std::span<double> samples) const override;

private:
  Gaudi::Property<double> m_scaleFactor{this, "ScaleFactor", 1.05,
      "Multiplicative factor applied to FCal energy samples"};
};

#endif // LARDIGITIZATION_FCALDIGITMODIFIERTOOL_H
