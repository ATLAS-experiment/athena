#ifndef JETTOOLHELPERS_IVARTOOL_H
#define JETTOOLHELPERS_IVARTOOL_H

#include <cmath>
#include <memory>
#include <functional>
#include <string>

#include "JetToolHelpers/JetContext.h"
#include "xAODJet/Jet.h"
#include "AthContainers/AuxElement.h"
#include "AsgTools/IAsgTool.h"
#include "JetAnalysisInterfaces/IInputVariable.h"

/// A tool interface class for tools implementing the @ref IInputVariable interface
///
/// This is needed because often the input variables are configured from python
/// and set via a `ToolHandle`.  This is separate from `IInputVariable` because
/// not all implementations of that interface are tools.

namespace JetHelper {
class IVarTool : virtual public asg::IAsgTool, virtual public IInputVariable
{
    ASG_TOOL_INTERFACE(IVarTool)

    public:
        //[[nodiscard]] virtual float getValue(const xAOD::Jet& jet, const JetContext& jc) const = 0;

};
} // namespace JetHelper
#endif

