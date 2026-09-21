/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_ANALYSIS_ALGORITHMS__BJET_REGRESSION_ALG_H
#define JET_ANALYSIS_ALGORITHMS__BJET_REGRESSION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include "xAODJet/JetContainer.h"
#include "AthContainers/ConstAccessor.h"
#include "PATInterfaces/SystematicSet.h"

#include <memory>
#include <string>

namespace CP
{
  /// \brief Fold a b-jet energy regression into the jet four-momentum.
  ///
  /// The flavour-tagging inference writes the regression output at derivation
  /// time as an ABSOLUTE prediction in MeV, \c bJR4v01_pt for R=0.4 and
  /// \c bJR10v01_pt / \c bJR10v01_mass for R=1.0. Nothing is computed in the
  /// derivation beyond the inference itself, and no ratio is stored there: the
  /// derivation cannot know which calibration configuration an analysis will
  /// choose, so anything it stored relative to a calibration would be valid for
  /// one guess only.
  ///
  /// This algorithm runs at the END of the jet sequence, after
  /// JetUncertaintiesAlg. That placement is the whole point. The uncertainty
  /// tool therefore sees the standard calibrated jet and reads its histograms at
  /// the standard kinematics, with no code in it modified and no lookup
  /// redirection to configure. Because the uncertainty application is purely
  /// multiplicative (\c shift = 1 + unc, then \c shift*pt), applying the
  /// regression afterwards is exactly equivalent to applying it first:
  ///
  ///     nominal x ratio x shift  ==  nominal x shift x ratio
  ///
  /// \par The reference has to be the nominal jet
  ///
  /// The subtlety worth knowing: the regression output is a single absolute
  /// number with no systematic dependence. Computing \c regPt/pt against the
  /// jet of the CURRENT systematic and multiplying would give
  ///
  ///     (nominal x shift) x regPt/(nominal x shift) = regPt
  ///
  /// for every variation -- the shift cancels exactly and no systematic would
  /// propagate at all. The ratio must therefore be formed once against the
  /// NOMINAL jet and then applied to every variation, which gives
  /// \c regPt*(1+frac) as intended.
  ///
  /// That is what \c m_jetHandleNominal is for: every systematics handle takes an
  /// explicit CP::SystematicSet, and an empty one resolves to the nominal
  /// instance of the container. The ratios are cached once per event before the
  /// systematics loop, which also makes the algorithm immune to any aliasing
  /// between the nominal instance and the copy it writes.
  ///
  /// This works whether or not JetUncertaintiesAlg ran: with no systematics the
  /// container holds only the nominal instance, the ratio reference and the jet
  /// being scaled are the same object, and the result is the plain regressed jet.

  class BJetRegressionAlg final : public EL::AnaAlgorithm
  {
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute (const EventContext& ctx) override;

  private:

    SysListHandle m_systematicsList {this};

    /// \brief the jet collection to modify, one instance per systematic
    SysCopyHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "", "the jet collection to run on"};

    /// \brief the same collection, read at the nominal systematic, to form the
    /// ratio against. See the class documentation for why this is needed.
    SysReadHandle<xAOD::JetContainer> m_jetHandleNominal {
      this, "jetsNominal", "", "the jet collection to take the nominal kinematics from"};

    /// \brief which jets to correct at all; empty means every jet carrying the
    /// regression decoration.
    ///
    /// Evaluated ONCE, against the nominal jet, and folded into the same
    /// usability flag as the decoration checks -- not re-evaluated per
    /// systematic. A selection that depends on pT would otherwise admit a jet
    /// near its threshold in the up variation and reject it in the down one,
    /// which puts a step into an otherwise smooth systematic. This is the same
    /// reasoning that makes the ratio reference the nominal jet.
    SysReadSelectionHandle m_jetPreselection {
      this, "jetPreselection", "", "the preselection to apply to jets"};

    /// \brief regression output decorations, absolute and in MeV
    Gaudi::Property<std::string> m_regressedPtName {
      this, "regressedPtName", "", "decoration holding the absolute regressed pT in MeV"};
    Gaudi::Property<std::string> m_regressedMassName {
      this, "regressedMassName", "", "decoration holding the absolute regressed mass in MeV;"
                                     " empty for a pT-only model"};

    /// \brief the ratio, written out so an analysis can divide it out and recover
    /// the standard calibration for a cross-check.
    ///
    /// Carries a %SYS% suffix to follow the convention for algorithm output, but
    /// note that its VALUE is identical for every variation: it is formed once
    /// against the nominal jet, for the reason given above. The suffix also
    /// leaves room for a future systematic that varies the prediction itself
    /// rather than scaling it.
    SysWriteDecorHandle<float> m_ptRatio {
      this, "ptRatioDecorName", "bjrPtRatio_%SYS%",
      "name of the output pT ratio decoration"};
    SysWriteDecorHandle<float> m_massRatio {
      this, "massRatioDecorName", "bjrMassRatio_%SYS%",
      "name of the output mass ratio decoration"};

    /// \brief write the ratios but leave the jet four-momentum alone
    Gaudi::Property<bool> m_onlyDecorate {
      this, "onlyDecorate", false, "only decorate the ratios, do not modify the jet"};

    /// set in initialize() from whether regressedMassName is configured
    bool m_doMass{false};

    /// Accessors for the regression output. Built in initialize() and held per
    /// instance -- NOT function-local statics, which would be shared between the
    /// small-R and large-R instances of this algorithm and so would read the
    /// wrong decoration for one of them.
    std::unique_ptr<SG::ConstAccessor<float>> m_regPtAcc;
    std::unique_ptr<SG::ConstAccessor<float>> m_regMassAcc;
  };

}

#endif
