# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from FTagAnalysisAlgorithms.FTagHelpers import getRecommendedBTagCalib


class KLFitterBlock(ConfigBlock):
    """ConfigBlock for KLFitter algorithms"""

    def __init__(self):
        super(KLFitterBlock, self).__init__()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption(
            "electrons",
            "",
            type=str,
            info="the input electron container, with a possible selection, in the format container or container.selection. The default is '' (empty string).",
        )
        self.addOption(
            "muons",
            "",
            type=str,
            info="the input muon container, with a possible selection, in the format container or container.selection. The default is '' (empty string).",
        )
        self.addOption(
            "jets",
            "",
            type=str,
            info="the input jet container, with a possible selection, in the format container or container.selection. The default is '' (empty string).",
        )
        self.addOption(
            "met",
            "",
            type=str,
            info="the input MET container. The default is '' (empty string).",
        )
        self.addOption(
            "likelihoodType",
            "",
            type=str,
            info="KLFitter likelihood, if only one is needed. See KLFitterEnums.h for possible values. The default is '' (empty string).",
        )
        self.addOption(
            "leptonType",
            "",
            type=str,
            info="type of lepton to use (only relevant to certain likelihood types), if only one is needed. See KLFitterEnums.h for possible values. The default is '' (empty string).",
        )
        self.addOption(
            "jetSelectionMode",
            "",
            type=str,
            info="jet selection mode to use, if only one is needed. See KLFitterEnums.h for possible values. The default is '' (empty string).",
        )
        self.addOption(
            "btaggingMethod",
            "kNoTag",
            type=str,
            info="strategy to handle b-jets, if only one is needed. See KLFitterEnums.h for possible values. The default is '' (empty string).",
        )
        self.addOption(
            "bTagCDIFile",
            None,
            type=str,
            info="CDI file to pass to the b-tagging efficiency tool",
        )
        self.addOption(
            "btagger",
            "GN2v01",
            type=str,
            info="b-tagging algorithm to use, if only one is needed. The default is 'GN2v01'.",
        )
        self.addOption(
            "btagWP",
            "",
            type=str,
            info="b-tagging efficiency WP to use, if only one is needed.",
        )
        self.addOption(
            "btagIgnoreOutOfValidityRange",
            False,
            type=bool,
            info="whether or not the b-tagger should ignore (and not fail) when a jet is outside the calibration range. The default is False.",
        )
        self.addOption(
            "selectionRegionsConfig",
            "",
            type=str,
            info="string of the form 'selectionName: sel1, optionA: opA, optionB: opB; selectionName: sel2, ...' where options can be likelihoodType, leptonType, jetSelectionMode, btaggingMethod, btagger or btagWP. The default is '' (empty string).",
        )
        self.addOption(
            "saveAllPermutations",
            False,
            type=bool,
            info="whether to save all permutations, or just the best one. The default is False (only save the best one).",
        )
        # list of dictionaries for the per-region config options
        self.perRegionConfiguration = list()

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def parseSelectionRegionsConfig(self):
        regions = self.selectionRegionsConfig.split(";")
        if len(regions) == 0:
            raise Exception(
                "KLFitterConfig: Could not determine any regions in your SelectionRegionsConfig"
            )
        for reg in regions:
            regstrip = reg.replace(" ", "")
            regionopts = dict(
                tuple(option.split(":")) for option in regstrip.split(",")
            )
            if "selectionName" not in regionopts:
                raise Exception(
                    "KLFitterConfig: Could not parse SelectionRegionsConfig selectionName for region ",
                    reg,
                )
            if "likelihoodType" in regionopts:
                raise Exception(
                    "KLFitterConfig: likelihoodType cannot be overriden per region. Create a separate instance of KLFitter block with different likelihoodType instead."
                )

            self.perRegionConfiguration.append(regionopts)

    def makeAlgs(self, config):
        self.parseSelectionRegionsConfig()
        for perRegionConfig in self.perRegionConfiguration:
            selectionName = perRegionConfig["selectionName"]
            alg = config.createAlgorithm(
                "EventReco::RunKLFitterAlg",
                f"RunKLFitterAlg_{selectionName}",
            )
            # input objects and their object selections
            alg.electrons, alg.electronSelection = config.readNameAndSelection(
                self.electrons
            )
            alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
            alg.jets, alg.jetSelection = config.readNameAndSelection(self.jets)
            alg.met = config.readName(self.met)
            alg.result = self.containerName + "_%SYS%"

            # global settings, in future expect to expose more options for configuration
            alg.SaveAllPermutations = self.saveAllPermutations

            # these settings can be defined per-region, but if not, we fallback to global setting
            alg.selectionDecorationName = selectionName + "_%SYS%,as_char"
            alg.LHType = self.likelihoodType
            alg.LeptonType = perRegionConfig.get("leptonType", self.leptonType)
            alg.JetSelectionMode = perRegionConfig.get(
                "jetSelectionMode", self.jetSelectionMode
            )
            btagAlgo = perRegionConfig.get("btagger", self.btagger)
            btagWP = perRegionConfig.get("btagWP", self.btagWP)
            alg.BTaggingDecoration = f"ftag_select_{btagAlgo}_{btagWP}"

            alg.BTaggingMethod = perRegionConfig.get(
                "btaggingMethod", self.btaggingMethod
            )
            if alg.BTaggingMethod == "kWorkingPoint":
                config.addPrivateTool("btagEffTool", "BTaggingEfficiencyTool")
                alg.btagEffTool.TaggerName = self.btagger
                alg.btagEffTool.OperatingPoint = self.btagWP
                jetCollection = config.originalName(self.jets.split(".")[0])
                alg.btagEffTool.JetAuthor = jetCollection
                alg.btagEffTool.ScaleFactorFileName = (
                    getRecommendedBTagCalib(config.geometry())
                    if self.bTagCDIFile is None
                    else self.bTagCDIFile
                )
                alg.btagEffTool.IgnoreOutOfValidityRange = (
                    self.btagIgnoreOutOfValidityRange
                )
                alg.btagEffTool.MinPt = (
                    20e3  # hardcoded to the recommendation for EMPFlow at the moment
                )
                # NOTE the efficiency tool is simply set to the default generator,
                # meaning the results are not correct for alternative showering generators!!

        finalizeAlg = config.createAlgorithm(
            "EventReco::KLFitterFinalizeOutputAlg",
            "KLFitterFinalizeOutputAlg",
        )
        finalizeAlg.resultContainerToCheck = self.containerName + "_%SYS%"
        finalizeAlg.resultContainerToWrite = self.containerName + "_%SYS%"

        config.setSourceName(self.containerName, self.containerName)
        config.addOutputContainer(self.containerName, self.containerName + "_%SYS%")

        config.addOutputVar(self.containerName, "eventProbability", "eventProbability")
        config.addOutputVar(self.containerName, "logLikelihood", "logLikelihood")
        if self.saveAllPermutations:
            config.addOutputVar(self.containerName, "selected", "selected")

        if self.likelihoodType != "ttbar_AllHad":
            config.addOutputVar(
                self.containerName, "model_bhad_jetIndex", "bhad_jetIndex"
            )
            config.addOutputVar(
                self.containerName, "model_blep_jetIndex", "blep_jetIndex"
            )
            config.addOutputVar(
                self.containerName, "model_lq1_jetIndex", "lq1_jetIndex"
            )
            if self.likelihoodType != "ttbar_BoostedLJets":
                config.addOutputVar(
                    self.containerName, "model_lq2_jetIndex", "lq2_jetIndex"
                )
            if self.likelihoodType == "ttH":
                config.addOutputVar(
                    self.containerName, "model_Higgs_b1_jetIndex", "Higgs_b1_jetIndex"
                )
                config.addOutputVar(
                    self.containerName, "model_Higgs_b2_jetIndex", "Higgs_b2_jetIndex"
                )

            config.addOutputVar(self.containerName, "model_nu_pt", "nu_pt")
            config.addOutputVar(self.containerName, "model_nu_eta", "nu_eta")
            config.addOutputVar(self.containerName, "model_nu_phi", "nu_phi")
            config.addOutputVar(self.containerName, "model_nu_E", "nu_E")

            if self.likelihoodType == "ttZTrilepton":
                config.addOutputVar(self.containerName, "model_lep_index", "lep_index")
                config.addOutputVar(
                    self.containerName, "model_lepZ1_index", "lepZ1_index"
                )
                config.addOutputVar(
                    self.containerName, "model_lepZ2_index", "lepZ2_index"
                )
        else:
            config.addOutputVar(
                self.containerName, "model_b_from_top1_jetIndex", "b_from_top1_jetIndex"
            )
            config.addOutputVar(
                self.containerName, "model_b_from_top2_jetIndex", "b_from_top2_jetIndex"
            )
            config.addOutputVar(
                self.containerName,
                "model_lj1_from_top1_jetIndex",
                "lj1_from_top1_jetIndex",
            )
            config.addOutputVar(
                self.containerName,
                "model_lj2_from_top1_jetIndex",
                "lj2_from_top1_jetIndex",
            )
            config.addOutputVar(
                self.containerName,
                "model_lj1_from_top2_jetIndex",
                "lj1_from_top2_jetIndex",
            )
            config.addOutputVar(
                self.containerName,
                "model_lj2_from_top2_jetIndex",
                "lj2_from_top2_jetIndex",
            )
