/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
  testBJetRegression

  Checks the b-jet-regression additions to JetUncertaintiesTool:

    1. the two flat Constant components are exactly 1% (pT) and 2% (mass), and each
       moves only the quantity it is declared to scale;
    2. they are only built when BJetRegressionApplied is set.

  Note what is NOT tested here any more, because it no longer exists. Earlier
  versions redirected the tool's histogram lookups to a stored pre-regression jet
  scale, and most of this test existed to police that. The regression is now
  applied by CP::BJetRegressionAlg AFTER this tool has run, so the tool always sees
  the standard calibrated jet and needs no redirection: there is nothing left to
  get wrong. Whether the shifts compose correctly is an integration question,
  checked end to end in bjr_validation/v2.

  Usage:
    testBJetRegression [<configFile>]

  The config file is an ORDINARY, UNMODIFIED recommendation config -- the one from
  CVMFS. That it needs no special config is the point: the regression is configured
  entirely through tool properties (BJetRegressionApplied and the two flat
  uncertainty values), so an analysis never needs a private calibration area to
  switch it on.
*/

#include "JetUncertainties/JetUncertaintiesTool.h"

#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODJet/JetAccessors.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventInfoContainer.h"
#include "xAODEventInfo/EventInfoAuxContainer.h"

#include "PATInterfaces/SystematicSet.h"
#include "PATInterfaces/SystematicVariation.h"

#include <cmath>
#include <cstdio>
#include <memory>
#include <string>

namespace {

const std::string JETDEF = "AntiKt10UFOCSSKSoftDropBeta100Zcut10";
const std::string MCTYPE = "MC20";

// Nominal (pre-regression) kinematics, in GeV since the tool is set to GeV below.
constexpr double NOM_PT = 500.;
constexpr double NOM_ETA = 1.0;
constexpr double NOM_MASS = 100.;

// A deliberately different "regressed" four-vector.
constexpr double REG_PT = 550.;
constexpr double REG_MASS = 102.;

int g_failures = 0;

void check (bool ok, const std::string& what)
{
    printf("  [%s] %s\n", ok ? "PASS" : "FAIL", what.c_str());
    if (!ok) ++g_failures;
}

void checkClose (double got, double expected, double tol, const std::string& what)
{
    const bool ok = std::fabs(got - expected) < tol;
    printf("  [%s] %s: got %.6f, expected %.6f\n",
           ok ? "PASS" : "FAIL", what.c_str(), got, expected);
    if (!ok) ++g_failures;
}

// The flat uncertainties under test, as fractions.
constexpr double FLAT_PT = 0.01;
constexpr double FLAT_MASS = 0.02;

std::unique_ptr<JetUncertaintiesTool> makeTool (const std::string& name,
                                                const std::string& configFile,
                                                bool regressionApplied)
{
    auto tool = std::make_unique<JetUncertaintiesTool>(name);
    if (tool->setProperty("JetDefinition", JETDEF).isFailure()) return nullptr;
    if (tool->setProperty("MCType", MCTYPE).isFailure()) return nullptr;
    if (tool->setProperty("ConfigFile", configFile).isFailure()) return nullptr;
    if (tool->setProperty("BJetRegressionApplied", regressionApplied).isFailure()) return nullptr;
    if (regressionApplied)
    {
        // Only the flat uncertainties are set. Nothing is read from the
        // configuration file, which is why an unmodified config works here.
        if (tool->setProperty("BJetRegressionPtUncertainty", FLAT_PT).isFailure()) return nullptr;
        if (tool->setProperty("BJetRegressionMassUncertainty", FLAT_MASS).isFailure()) return nullptr;
    }
    if (tool->setScaleToGeV().isFailure()) return nullptr;
    if (tool->initialize().isFailure()) return nullptr;
    return tool;
}

/// Put the jet back to the regressed four-vector, with the nominal scale momenta
/// stored alongside it -- the state JetCalibTools leaves a regressed jet in.
void resetJet (xAOD::Jet& jet, double regPt, double regMass)
{
    const xAOD::JetFourMom_t nominal(NOM_PT, NOM_ETA, 0., NOM_MASS);

    // Mass-definition scales some large-R components consult.
    static const xAOD::JetAttributeAccessor::AccessorWrapper<xAOD::JetFourMom_t>
        jms("JetJMSScaleMomentum");
    static const xAOD::JetAttributeAccessor::AccessorWrapper<xAOD::JetFourMom_t>
        jmsCalo("JetJMSScaleMomentumCalo");
    jms.setAttribute(jet, nominal);
    jmsCalo.setAttribute(jet, nominal);

    jet.setJetP4(xAOD::JetFourMom_t(regPt, NOM_ETA, 0., regMass));
}

bool applyOne (JetUncertaintiesTool& tool, xAOD::Jet& jet, const xAOD::EventInfo& eInfo,
               const std::string& np, double sigma)
{
    CP::SystematicSet syst;
    syst.insert(CP::SystematicVariation(np, sigma));
    if (tool.applySystematicVariation(syst) != StatusCode::SUCCESS)
    {
        printf("  [FAIL] could not select %s\n", np.c_str());
        ++g_failures;
        return false;
    }
    if (tool.applyCorrection(jet, eInfo) != CP::CorrectionCode::Ok)
    {
        printf("  [FAIL] applyCorrection failed for %s\n", np.c_str());
        ++g_failures;
        return false;
    }
    return true;
}

} // anonymous namespace


int main (int argc, char* argv[])
{
    // An ordinary recommendation config. No modified copy is needed, because the
    // regression is configured entirely through properties.
    const std::string config = (argc > 1)
        ? argv[1]
        : "rel22/Summer2025_PreRec/R10_CategoryJES_FullJER_FullJMS.config";
    printf("config: %s\n", config.c_str());

    StatusCode::enableFailure();

    // Containers to hold the objects we manipulate
    xAOD::JetContainer jets;
    jets.setStore(new xAOD::JetAuxContainer());
    jets.push_back(new xAOD::Jet());
    xAOD::Jet* jet = jets.at(0);

    xAOD::EventInfoContainer eInfos;
    eInfos.setStore(new xAOD::EventInfoAuxContainer());
    eInfos.push_back(new xAOD::EventInfo());
    const xAOD::EventInfo* eInfo = eInfos.at(0);

    // ------------------------------------------------------------------
    printf("\n=== 1. the gate: flat NPs absent unless the regression is applied ===\n");
    {
        auto off = makeTool("bjrOff", config, false);
        if (!off) { printf("  [FAIL] tool failed to initialize with the gate closed\n"); return 2; }
        check(!off->isAffectedBySystematic(CP::SystematicVariation("JET_BJR_R10_PtScale")),
              "JET_BJR_R10_PtScale absent when BJetRegressionApplied is false");
        check(!off->isAffectedBySystematic(CP::SystematicVariation("JET_BJR_R10_MassScale")),
              "JET_BJR_R10_MassScale absent when BJetRegressionApplied is false");
    }

    auto tool = makeTool("bjrOn", config, true);
    if (!tool) { printf("  [FAIL] tool failed to initialize with the gate open\n"); return 2; }
    check(tool->isAffectedBySystematic(CP::SystematicVariation("JET_BJR_R10_PtScale")),
          "JET_BJR_R10_PtScale present when BJetRegressionApplied is true");
    check(tool->isAffectedBySystematic(CP::SystematicVariation("JET_BJR_R10_MassScale")),
          "JET_BJR_R10_MassScale present when BJetRegressionApplied is true");

    // ------------------------------------------------------------------
    printf("\n=== 2. the flat pT NP is exactly 1%%, and moves only the pT ===\n");
    for (const double sigma : {+1.0, -1.0})
    {
        resetJet(*jet, REG_PT, REG_MASS);
        if (applyOne(*tool, *jet, *eInfo, "JET_BJR_R10_PtScale", sigma))
        {
            checkClose(jet->pt(), REG_PT * (1. + 0.01 * sigma), 1.e-4,
                       std::string("pT at ") + (sigma > 0 ? "+" : "-") + "1 sigma");
            checkClose(jet->m(), REG_MASS, 1.e-4,
                       std::string("mass untouched at ") + (sigma > 0 ? "+" : "-") + "1 sigma");
        }
    }

    printf("\n=== 3. the flat mass NP is exactly 2%%, and moves only the mass ===\n");
    for (const double sigma : {+1.0, -1.0})
    {
        resetJet(*jet, REG_PT, REG_MASS);
        if (applyOne(*tool, *jet, *eInfo, "JET_BJR_R10_MassScale", sigma))
        {
            checkClose(jet->m(), REG_MASS * (1. + 0.02 * sigma), 1.e-4,
                       std::string("mass at ") + (sigma > 0 ? "+" : "-") + "1 sigma");
            checkClose(jet->pt(), REG_PT, 1.e-4,
                       std::string("pT untouched at ") + (sigma > 0 ? "+" : "-") + "1 sigma");
        }
    }

    // ------------------------------------------------------------------
    printf("\n%s: %d check(s) failed\n", g_failures ? "FAILURE" : "SUCCESS", g_failures);
    return g_failures == 0 ? 0 : 1;
}
