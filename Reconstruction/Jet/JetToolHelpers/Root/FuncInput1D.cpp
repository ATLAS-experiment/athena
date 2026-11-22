// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#include "JetToolHelpers/FuncInput1D.h"

#include <algorithm>

namespace JetHelper {

  FuncInput1D::FuncInput1D(const std::string& name)
    : asg::AsgTool(name)
  {}

  StatusCode FuncInput1D::initialize() {
    ATH_CHECK(m_vartool.retrieve());

    if (m_formula.value().empty()) {
      ATH_MSG_ERROR("Formula is empty");
      return StatusCode::FAILURE;
    }
    if (m_xMax.value() <= m_xMin.value()) {
      ATH_MSG_ERROR("XMax must be larger than XMin");
      return StatusCode::FAILURE;
    }

    // Keep the formula private to this tool: don't add it to ROOT's global function list
    auto func = std::make_unique<TFormula>((name() + "_func").c_str(),
                                           m_formula.value().c_str(), false);
    if (!func->IsValid()) {
      ATH_MSG_ERROR("Invalid formula: " << m_formula.value());
      return StatusCode::FAILURE;
    }

    if (static_cast<int>(m_params.value().size()) != func->GetNpar()) {
      ATH_MSG_ERROR("Formula expects " << func->GetNpar() << " parameters, got "
                    << m_params.value().size());
      return StatusCode::FAILURE;
    }

    // Evaluate once so any lazy JIT compilation happens before the event loop
    double xTest = m_xMin.value();
    func->EvalPar(&xTest, m_params.value().data());

    m_func = std::move(func);

    ATH_MSG_DEBUG("Using " << m_formula.value() << " in [" << m_xMin.value() << ", "
		  << m_xMax.value() << "] with x from " << m_vartool.name());
    return StatusCode::SUCCESS;
  }

  float FuncInput1D::getValue(const xAOD::Jet& jet, const JetContext& ctx) const {
    double x = m_vartool->getValue(jet, ctx);

    if (x < m_xMin.value() || x > m_xMax.value()) {
      if (!m_clamp) return m_outOfRangeValue;
      x = std::clamp(x, m_xMin.value(), m_xMax.value());
    }

    // TFormula::EvalPar is const, so this is safe from a const method
    return m_func->EvalPar(&x, m_params.value().data());
  }

} // namespace JetHelper
