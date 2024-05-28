/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTOOLHELPERS_IINPUTVARIABLE_H
#define JETTOOLHELPERS_IINPUTVARIABLE_H

#include <cmath>
#include <memory>
#include <functional>
#include <string>
#include <optional>

#include "JetToolHelpers/JetContext.h"
#include "xAODJet/Jet.h"
#include "AthContainers/AuxElement.h"

namespace JetHelper {

class IInputVariable
{

    public:
        [[nodiscard]] virtual float getValue(const xAOD::Jet& jet, const JetContext& jc) const = 0;
};

} // namespace JetHelper

#endif

