// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#ifndef JETTOOLHELPERS_FUNCINPUT1D_H
#define JETTOOLHELPERS_FUNCINPUT1D_H

// FuncInput1D
//
// Analogue of HistoInput1D that evaluates a TF1 instead of reading a
// histogram bin. The TF1 is built from a formula string with fixed
// parameters and range. The x value is provided by a VarTool and is
// clamped to the function range before evaluation.

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgTools/ToolHandle.h"

#include "JetAnalysisInterfaces/IVarTool.h"
#include "JetToolHelpers/JetContext.h"

#include "xAODJet/Jet.h"

#include "TFormula.h"

#include <memory>
#include <string>
#include <vector>

namespace JetHelper {

  class FuncInput1D : public asg::AsgTool, virtual public IVarTool {
    ASG_TOOL_CLASS(FuncInput1D, IVarTool)

  public:
    FuncInput1D(const std::string& name);
    virtual ~FuncInput1D() = default;

    virtual StatusCode initialize() override;

    /// Evaluate the function at the value returned by the VarTool
    virtual float getValue(const xAOD::Jet& jet, const JetContext& ctx) const override;

  private:
    // Tool providing x
    ToolHandle<IVarTool> m_vartool {this, "varTool1", "VarTool", "InputVariable instance"};

    // Function definition
    Gaudi::Property<std::string> m_formula {this, "Formula", "", "TF1 formula in x and [0], [1], ..."};
    Gaudi::Property<std::vector<double>> m_params {this, "Parameters", {}, "Function parameters"};
    Gaudi::Property<double> m_xMin {this, "XMin", 0.0, "Lower edge of the function range"};
    Gaudi::Property<double> m_xMax {this, "XMax", 1.0, "Upper edge of the function range"};

    // Behaviour outside the range
    Gaudi::Property<bool> m_clamp {this, "ClampToRange", true, "Clamp x to [XMin, XMax] (otherwise return OutOfRangeValue)"};
    Gaudi::Property<float> m_outOfRangeValue {this, "OutOfRangeValue", 999.99, "Returned outside the range if ClampToRange is false"};

    std::unique_ptr<const TFormula> m_func;
  };

} // namespace JetHelper

#endif
