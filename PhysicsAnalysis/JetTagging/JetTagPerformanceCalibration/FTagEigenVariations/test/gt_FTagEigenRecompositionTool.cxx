/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "FTagEigenVariations/CDIInput.h"
#include "FTagEigenVariations/EigenDecomposition.h"
#include "FTagEigenVariations/EigenRecomposition.h"
#include "FTagEigenVariations/EigenReduction.h"
#include "FTagEigenVariations/IFTagEigenRecompositionTool.h"
#include "FTagEigenVariations/SystematicNames.h"

#include "AsgTools/StandaloneToolHandle.h"
#include "CalibrationDataInterface/CalibrationDataContainer.h"
#include "FTagAnalysisInterfaces/IBTaggingEigenVectorRecompositionTool.h"
#include "PathResolver/PathResolver.h"
#ifndef XAOD_STANDALONE
#include "POOLRootAccess/TEvent.h"
#endif

#include "TFile.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <format>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using ftag::ev::IFTagEigenRecompositionTool;
using ftag::ev::RecompositionTable;
namespace cdi = ftag::ev::cdi;

namespace {

  const std::string cdiFile = "xAODBTaggingEfficiency/13TeV/MC20_2025-06-17_GN2v01_v4.root";
  const std::string tagger = "GN2v01";
  const std::string jetCollection = "AntiKt4EMPFlowJets";
  const std::string operatingPoint = "Continuous";

  // The excluded high-pT extrapolation for continuous working points, as in the BTaggingEfficiencyTool
  const std::map<std::string, std::string> highPtExclusion = {{"B", "extrapolation_pt_b_Eigen*"},
                                                              {"C", "extrapolation_pt_c_Eigen*"},
                                                              {"T", "extrapolation_pt_c_Eigen*"},
                                                              {"Light", "extrapolation_pt_l_Eigen*"}};

  // Unique across interfaces, since the Gaudi ToolSvc returns the existing tool of a name
  std::string uniqueToolName() {
    static int count = 0;
    return std::format("ftag::ev::FTagEigenRecompositionTool/RecompositionTool{}", count++);
  }

  template <typename Interface = IFTagEigenRecompositionTool>
  std::unique_ptr<asg::StandaloneToolHandle<Interface>> makeTool(const std::map<std::string, std::string>& properties,
                                                                 bool splitTauCharm = false) {
    auto tool = std::make_unique<asg::StandaloneToolHandle<Interface>>(uniqueToolName());
    EXPECT_TRUE(tool->setProperty("ScaleFactorFileName", cdiFile).isSuccess());
    EXPECT_TRUE(tool->setProperty("TaggerName", tagger).isSuccess());
    EXPECT_TRUE(tool->setProperty("JetAuthor", jetCollection).isSuccess());
    EXPECT_TRUE(tool->setProperty("OperatingPoint", operatingPoint).isSuccess());
    EXPECT_TRUE(tool->setProperty("splitTauCharmEff", splitTauCharm).isSuccess());
    for (const auto& [name, value] : properties) EXPECT_TRUE(tool->setProperty(name, value).isSuccess());
    return tool;
  }

  cdi::Container container(const std::string& label) {
    std::unique_ptr<TFile> file(TFile::Open(PathResolverFindCalibFile(cdiFile).c_str(), "READ"));
    Analysis::CalibrationDataHistogramContainer* raw = nullptr;
    file->GetObject(std::format("{}/{}/{}/{}/default_SF", tagger, jetCollection, operatingPoint, label).c_str(), raw);
    std::unique_ptr<Analysis::CalibrationDataHistogramContainer> cnt(raw);
    if (!cnt) throw std::runtime_error("no container for " + label);
    return cdi::readContainer(*cnt, label);
  }

  // The recomposition computed directly with the core, for one flavour
  ftag::ev::Recomposition expected(const std::string& label, cdi::Reduction reduction) {
    const cdi::Container cnt = container(label);
    cdi::Settings settings;
    settings.exclusions = {{label, {highPtExclusion.at(label)}}};
    settings.reductions = {{label, reduction}};
    const ftag::ev::Decomposition decomposition = ftag::ev::decompose(cdi::perFlavourInput(cnt, settings));
    const std::optional<Eigen::Index> nKeep = cdi::eigenvariationsToKeep(cnt, settings);
    const Eigen::MatrixXd variations = nKeep ? ftag::ev::reduce(decomposition, *nKeep) : decomposition.variations;
    return ftag::ev::recompose(decomposition, variations);
  }

  // The recomposition of the global eigenvariations computed directly with the core
  ftag::ev::Recomposition expectedGlobal(const std::vector<std::string>& labels) {
    std::vector<cdi::Container> containers;
    cdi::Settings settings;
    for (const std::string& label : labels) {
      containers.push_back(container(label));
      settings.exclusions[label] = {highPtExclusion.at(label)};
    }
    const ftag::ev::Decomposition decomposition = ftag::ev::decompose(cdi::globalInput(containers, settings));
    return ftag::ev::recompose(decomposition, decomposition.variations);
  }

  void expectSameAsCore(const RecompositionTable& table, const std::string& label,
                        const ftag::ev::Recomposition& core) {
    const Eigen::Index nVariations = core.coefficients.cols();
    ASSERT_EQ(static_cast<Eigen::Index>(table.nuisanceParameters.size()), nVariations);
    ASSERT_EQ(static_cast<Eigen::Index>(table.sources.size()), core.coefficients.rows());
    for (Eigen::Index k = 0; k < nVariations; ++k) {
      EXPECT_EQ(table.nuisanceParameters.at(k), ftag::ev::eigenName(label, k));
      EXPECT_DOUBLE_EQ(table.describedFraction.at(k), core.describedFraction(k));
      for (Eigen::Index s = 0; s < core.coefficients.rows(); ++s) {
        EXPECT_DOUBLE_EQ(table.coefficients.at(k).at(s), core.coefficients(s, k));
      }
    }
  }

}

TEST(FTagEigenRecompositionTool, LooseMatchesCore) {
  auto tool = makeTool({});
  ASSERT_TRUE(tool->initialize().isSuccess());
  for (const std::string label : {"B", "C", "Light"}) {
    SCOPED_TRACE(label);
    expectSameAsCore((*tool)->recomposition(label), label, expected(label, cdi::Reduction::Loose));
  }
  const std::vector<std::string>& sources = (*tool)->recomposition("B").sources;
  for (const std::string& source : sources) EXPECT_TRUE(source.starts_with("FT_EFF_"));
  EXPECT_TRUE(std::ranges::none_of(sources, [](const std::string& s) { return s.contains("extrapolation_pt_b"); }));
}

TEST(FTagEigenRecompositionTool, ReductionMatchesCore) {
  auto tool = makeTool({{"EigenvectorReductionB", "Medium"}, {"EigenvectorReductionC", "Tight"}});
  ASSERT_TRUE(tool->initialize().isSuccess());
  expectSameAsCore((*tool)->recomposition("B"), "B", expected("B", cdi::Reduction::Medium));
  expectSameAsCore((*tool)->recomposition("C"), "C", expected("C", cdi::Reduction::Tight));
  expectSameAsCore((*tool)->recomposition("Light"), "Light", expected("Light", cdi::Reduction::Loose));
}

TEST(FTagEigenRecompositionTool, Suffixes) {
  auto tool = makeTool({{"UncertaintyBSuffix", "run2"}, {"UncertaintyCSuffix", " _run2 "}});
  ASSERT_TRUE(tool->initialize().isSuccess());
  EXPECT_EQ((*tool)->recomposition("B").nuisanceParameters.at(0), "FT_EFF_Eigen_B_0_run2");
  EXPECT_EQ((*tool)->recomposition("C").nuisanceParameters.at(0), "FT_EFF_Eigen_C_0_run2");
  EXPECT_EQ((*tool)->recomposition("Light").nuisanceParameters.at(0), "FT_EFF_Eigen_Light_0");
}

TEST(FTagEigenRecompositionTool, TauSharesCharmUnlessSplit) {
  auto shared = makeTool({{"UncertaintyTSuffix", "ignored"}});
  ASSERT_TRUE(shared->initialize().isSuccess());
  EXPECT_EQ(&(*shared)->recomposition("T"), &(*shared)->recomposition("C"));
  EXPECT_EQ((*shared)->recomposition("T").nuisanceParameters.at(0), "FT_EFF_Eigen_C_0");

  auto split = makeTool({{"UncertaintyTSuffix", "tau"}}, true);
  ASSERT_TRUE(split->initialize().isSuccess());
  EXPECT_EQ((*split)->recomposition("T").nuisanceParameters.at(0), "FT_EFF_Eigen_T_0_tau");
}

TEST(FTagEigenRecompositionTool, TauTakesCharmSettings) {
  auto reference = makeTool<IBTaggingEigenVectorRecompositionTool>({});
  ASSERT_TRUE(reference->initialize().isSuccess());
  const std::vector<std::string> charm = (*reference)->getListOfOriginalNuisanceParameters("C");
  ASSERT_FALSE(charm.empty());
  const std::string& excluded = charm.front();
  auto tool = makeTool({{"EigenvectorReductionC", "Tight"}, {"ExcludeFromEigenVectorCTreatment", excluded}}, true);
  ASSERT_TRUE(tool->initialize().isSuccess());
  const RecompositionTable& c = (*tool)->recomposition("C");
  const RecompositionTable& t = (*tool)->recomposition("T");
  EXPECT_EQ(t.nuisanceParameters.size(), c.nuisanceParameters.size());
  EXPECT_EQ(t.sources, c.sources);
  EXPECT_EQ(t.sources.size(), charm.size() - 1);
  EXPECT_TRUE(std::ranges::none_of(t.sources, [](const std::string& s) { return s.contains("extrapolation_pt_c"); }));
}

TEST(FTagEigenRecompositionTool, GlobalStrategy) {
  // One table for all flavours, named FT_EFF_Eigen_Global_<k> without suffix; tau jets take the charm part
  auto tool = makeTool({{"SystematicsStrategy", "SFGlobalEigen"}, {"UncertaintyLightSuffix", "run2"}});
  ASSERT_TRUE(tool->initialize().isSuccess());
  const RecompositionTable& global = (*tool)->recomposition("B");
  expectSameAsCore(global, "Global", expectedGlobal({"B", "C", "Light"}));
  for (const std::string label : {"C", "T", "Light"}) EXPECT_EQ(&(*tool)->recomposition(label), &global);
  EXPECT_THROW((*tool)->recomposition("Global"), std::invalid_argument);

  auto split = makeTool({{"SystematicsStrategy", "SFGlobalEigen"}}, true);
  ASSERT_TRUE(split->initialize().isSuccess());
  expectSameAsCore((*split)->recomposition("T"), "Global", expectedGlobal({"B", "C", "T", "Light"}));

  auto current = makeTool<IBTaggingEigenVectorRecompositionTool>({{"SystematicsStrategy", "SFGlobalEigen"}});
  ASSERT_TRUE(current->initialize().isSuccess());
  const std::map<std::string, std::map<std::string, float>> map = (*current)->getCoefficientMap("Light");
  ASSERT_EQ(map.size(), global.nuisanceParameters.size());
  EXPECT_TRUE(map.contains("Eigen_Global_0"));

  EXPECT_TRUE(makeTool({{"SystematicsStrategy", "SFGlobalEigen"}, {"EigenvectorReductionB", "Medium"}})
                ->initialize()
                .isFailure());
}

TEST(FTagEigenRecompositionTool, ExclusionsApply) {
  auto all = makeTool({});
  auto excluded = makeTool({{"ExcludeFromEigenVectorTreatment", "FT_EFF_Generator; "}});
  ASSERT_TRUE(all->initialize().isSuccess());
  ASSERT_TRUE(excluded->initialize().isSuccess());
  const std::vector<std::string>& sources = (*all)->recomposition("Light").sources;
  ASSERT_NE(std::ranges::find(sources, "FT_EFF_Generator"), sources.end());
  EXPECT_EQ((*excluded)->recomposition("Light").sources.size(), sources.size() - 1);
}

TEST(FTagEigenRecompositionTool, BTaggingEigenVectorRecompositionInterface) {
  auto current = makeTool({});
  auto tool = makeTool<IBTaggingEigenVectorRecompositionTool>({});
  ASSERT_TRUE(current->initialize().isSuccess());
  ASSERT_TRUE(tool->initialize().isSuccess());
  for (const std::string label : {"B", "T"}) {
    SCOPED_TRACE(label);
    const RecompositionTable& table = (*current)->recomposition(label);
    const std::vector<std::string> sources = (*tool)->getListOfOriginalNuisanceParameters(label);
    ASSERT_EQ(sources.size(), table.sources.size());
    for (std::size_t s = 0; s < sources.size(); ++s) EXPECT_EQ(ftag::ev::sourceName(sources[s]), table.sources[s]);
    ASSERT_EQ((*tool)->getNumEigenVectors(label), static_cast<int>(table.nuisanceParameters.size()));

    const std::map<std::string, std::map<std::string, float>> map = (*tool)->getCoefficientMap(label);
    ASSERT_EQ(map.size(), table.nuisanceParameters.size());
    for (unsigned int k = 0; k < table.nuisanceParameters.size(); ++k) {
      const std::vector<float> coefficients = (*tool)->getCoefficients(label, k);
      const std::map<std::string, float>& row = map.at(std::format("Eigen_{}_{}", label, k));
      ASSERT_EQ(coefficients.size(), sources.size());
      for (std::size_t s = 0; s < sources.size(); ++s) {
        EXPECT_EQ(coefficients[s], static_cast<float>(table.coefficients[k][s]));
        EXPECT_EQ(row.at(sources[s]), coefficients[s]);
      }
    }
  }
  EXPECT_EQ((*tool)->getCoefficientMap("B", {1, 3}).size(), 2u);
  EXPECT_THROW((*tool)->getCoefficients("B", (*tool)->getNumEigenVectors("B")), std::invalid_argument);
  EXPECT_THROW((*tool)->getNumEigenVectors("b"), std::invalid_argument);
  EXPECT_TRUE((*tool)->affectingSystematics().empty());
  EXPECT_TRUE((*tool)->applySystematicVariation({}).isSuccess());
}

TEST(FTagEigenRecompositionTool, ConfigurationErrors) {
  EXPECT_TRUE(makeTool({{"EigenvectorReductionB", "VeryLoose"}})->initialize().isFailure());
  EXPECT_TRUE(makeTool({{"SystematicsStrategy", "Envelope"}})->initialize().isFailure());
  EXPECT_TRUE(makeTool({{"ExcludeFromEigenVectorLightTreatment", "no such source"}})->initialize().isFailure());
  EXPECT_TRUE(makeTool({{"ExcludeFromEigenVectorTreatment", "no such source"}})->initialize().isFailure());
  EXPECT_TRUE(makeTool({{"ExcludeFromEigenVectorTreatment", "no such source*"}})->initialize().isSuccess());
  EXPECT_TRUE(makeTool({{"OperatingPoint", "NoSuchPoint"}})->initialize().isFailure());
  EXPECT_TRUE(makeTool({{"TaggerName", ""}})->initialize().isFailure());

  auto tool = makeTool({});
  ASSERT_TRUE(tool->initialize().isSuccess());
  EXPECT_THROW((*tool)->recomposition("Global"), std::invalid_argument);
  EXPECT_THROW((*tool)->recomposition("b"), std::invalid_argument);
}

int main(int argc, char** argv) {
#ifndef XAOD_STANDALONE
  POOL::Init();
#endif
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
