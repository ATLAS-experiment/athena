/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include <TF2.h>
#include <TGraph2D.h>

#include <algorithm>
#include <cmath>

#include "CxxUtils/checker_macros.h"
#include "IsolationSelection/IsolationConditionGraph.h"

namespace CP {
IsolationConditionGraph::IsolationConditionGraph(
    const std::string& name, const std::vector<std::string>& isoTypes,
    std::unique_ptr<TF1> isoFunction,
    std::vector<TGraph*>&
        cutGraphs,  // std::vector<std::unique_ptr<TGraph>> cutGraphs,
    std::unique_ptr<TH1F> binning, const std::string& isoDecSuffix,
    bool invertCut)
    : IsolationCondition(name, isoTypes, isoDecSuffix),
      m_cutGraphs(cutGraphs),
      m_isoFunction(std::move(isoFunction)),
      m_binning(std::move(binning)),
      m_invertCut(invertCut) {}

bool IsolationConditionGraph::accept(const xAOD::IParticle& x) const {

  const float cutValue = getCutValue(x.pt(), x.eta());
  std::vector<double> isoVars(num_types(), 0);

  for (unsigned int iacc = 0; iacc < num_types(); ++iacc) {
    const FloatAccessor& acc = accessor(iacc);

    if (!acc.isAvailable(x)) {
      // Temporary fix for missing closeByCorr variables if no primary vertex
      // exists for the event If closeByCorr variable does not exist, fallback
      // to the standard isolation variable 2025/02
      const FloatAccessor& acc_noCloseBy = accessor_noCloseBy(iacc);
      if (acc_noCloseBy.isAvailable(x)) {
        isoVars[iacc] = acc_noCloseBy(x);
      } else {
        ATH_MSG_WARNING(__FILE__
                        << ":" << __LINE__ << "Accessor "
                        << SG::AuxTypeRegistry::instance().getName(acc.auxid())
                        << " is not available. Expected when using primary "
                           "AODs, post-p3793 derivations (only for *FixedRad "
                           "or FixedCutPflow* for electrons), "
                        << " pre-p3517 derivations (only for FC*), or "
                           "pre-p3830 derivations (for other electron WPs)");
        if (!m_isoDecSuffix.empty())
          throw std::runtime_error(
              "IsolationConditionCombined: IsolationSelectionTool property "
              "'IsoDecSuffix' is set to " +
              m_isoDecSuffix +
              ". Must run on derivation made with IsolationCloseByCorrection "
              "to create the isolation variables with this suffix, or remove "
              "'IsoDecSuffix'. ");
        isoVars[iacc] = FLT_MAX;
      }
    } else
      isoVars[iacc] = acc(x);
  }

  // In general TF1::EvalPar is non-const/not thread-safe. But for the special
  // case of using it as TFormula it can be considered const/safe. It would be
  // safer to change the interface to not allow a generic TF1.
  TF1* f ATLAS_THREAD_SAFE = m_isoFunction.get();
  const float isoValue = f->EvalPar(isoVars.data());

  ATH_MSG_DEBUG("PLIV Isolation values of electron %.3f " << isoValue);

  if (!m_invertCut)
    return isoValue <= cutValue;
  return isoValue > cutValue;
}

bool IsolationConditionGraph::accept(const strObj& x) const {
  const float cutValue = getCutValue(x.pt, x.eta);
  std::vector<double> isoVars;
  for (unsigned int itype = 0; itype < num_types(); ++itype)
    isoVars.push_back(x.isolationValues[type(itype)]);
  TF1* f ATLAS_THREAD_SAFE = m_isoFunction.get();
  const float isoValue = f->EvalPar(isoVars.data());

  ATH_MSG_DEBUG("PLIV Isolation values of electron %.3f " << isoValue);

  if (!m_invertCut)
    return isoValue <= cutValue;
  return isoValue > cutValue;
}

float IsolationConditionGraph::getCutValue(const float pt,
                                           const float eta) const {
  //
  int bin = m_binning->GetXaxis()->FindBin(fabs(eta));
  ATH_MSG_DEBUG(Form(" Electron (eta=%.3f) falls into bin %d", eta, bin - 1));

  if (bin <= 0 || (unsigned int)bin > m_cutGraphs.size()) {
    ATH_MSG_DEBUG(
        "Stored binning in file does not comply with binning of graphs for "
        "working point "
        << this->name() << "! Please contact the IFF group");
    return -999;
  }
  ATH_MSG_DEBUG(
      Form(" Electron cut value %.3f for pt %.3f as calculated from graph %s",
           (m_cutGraphs[bin - 1])->Eval(pt / 1000.), pt / 1000.,
           (m_cutGraphs[bin - 1])->GetName()));

  return (m_cutGraphs[bin - 1])->Eval(pt / 1000.);
}

}  // namespace CP
