# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaConfiguration.Enums import LHCPeriod

from ReconstructionUtils.ReconstructionAlgorithmsUtils import _resolve_reco_partons_prefix


class HyPERBlock(ConfigBlock):
    """ConfigBlock for HyPER algorithms"""

    def __init__(self):
        super(HyPERBlock, self).__init__()
        self.addOption(
            "btagger",
            "GN2v01_Continuous",
            type=str,
            info="the b-tagging algorithm used to determine the PCBT quantile. Irrelevant for the `TtbarLJetsNoBTag` topology. The available models have been trained using GN2.",
        )
        self.addOption(
            "electrons",
            "",
            type=str,
            info="the input electron container, with a possible selection, in the format `container` or `container.selection`.",
        )
        self.addOption(
            "muons",
            "",
            type=str,
            info="the input muon container, with a possible selection, in the format `container` or `container.selection`.",
        )
        self.addOption(
            "jets",
            "",
            type=str,
            info="the input jet container, with a possible selection, in the format `container` or `container.selection`.",
        )
        self.addOption("met", "", type=str, info="the input MET container.")
        self.addOption(
            "eventSelection",
            "",
            type=str,
            info="an optional event filter to run on. Make sure your selection makes sense in order to build the HyPER graph. For instance, a pre-selection of events containing three jets will not allow you to reconstruct two tops.",
        )
        self.addOption(
            "topology",
            "",
            type=str,
            info="the HyPER model to run. Currently `TtbarAllHadronic`, `TtbarLJetsNoBTag`, `TtbarLJets` and `TtbarDiLepton` are supported.",
        )
        self.addOption(
            "writeRecoPartonsKinematics",
            False,
            type=bool,
            info="whether to publish partial reconstructed parton four-vectors following the `{topology}_{producer}` naming contract.",
        )
        self.addOption(
            "outputName",
            "HyPER",
            type=str,
            info=(
                "name of the algorithm instance. This is used as a label for the output branches which allows multiple HyPER instances to be run."
                "The final prefix is `<topology>_<producer>`, using the configured topology. "
                "For example if `topology=TtbarDiLepton` and `outputName=HyPER`, the output branches will be named `TtbarDiLepton_HyPER_<branch>`. "
            ),
        )
        self.addOption(
            "OutputLevel",
            3,
            type=int,
            info="the verbosity of the algorithm. This is for debugging purposes. Options: 3 (INFO), 2 (DEBUG), 1 (VERBOSE).",
        )
        self.addOption("fullLogEventNumber", 0, type=int, info="TODO")

    def instanceName(self):
        """Return the instance name for this block"""
        return self.outputName

    def makeAlgs(self, config):
        decorator_prefix = _resolve_reco_partons_prefix(self.topology, self.outputName)
        alg = config.createAlgorithm(
            "EventReco::RunHyPERAlg", f"RunHyPERAlg_{decorator_prefix}"
        )

        alg.btagger = self.btagger
        alg.electrons, alg.electronSelection = config.readNameAndSelection(
            self.electrons
        )
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        alg.jets, alg.jetSelection = config.readNameAndSelection(self.jets)
        alg.met = config.readName(self.met)
        alg.eventSelection = self.eventSelection
        alg.topology = self.topology
        # Check that config is ok if a particular event was passed for full log
        if self.fullLogEventNumber > 0:
            if self.OutputLevel != 3:
                raise ValueError(
                    "OutputLevel must be 3 (INFO) when tracing only one particular event!"
                )

        alg.OutputLevel = self.OutputLevel
        alg.fullLogEventNumber = self.fullLogEventNumber

        def form_even_odd_path(topology, run):
            base_path = "TopReconstruction/HyPERModels/" + topology
            even_path = base_path + "_" + run + "_trained_on_even.onnx"
            odd_path = base_path + "_" + run + "_trained_on_odd.onnx"
            return even_path, odd_path

        # Confifigure model onnx paths
        if config.geometry() == LHCPeriod.Run2:
            if self.topology == "TtbarAllHadronic":
                even_path, odd_path = form_even_odd_path(
                    "TtbarAllHadronic", "run2"
                )
            elif self.topology == "TtbarLJets":
                even_path, odd_path = form_even_odd_path(
                    "TtbarLJets", "run2"
                )
            elif self.topology == "TtbarLJetsNoBTag":
                even_path, odd_path = form_even_odd_path(
                    "TtbarLJetsNoBTag", "run2"
                )
            elif self.topology == "TtbarDiLepton":
                even_path, odd_path = form_even_odd_path(
                    "TtbarDiLepton",
                    "run3",  # For the time being use Run3 models for DiLepton in Run2
                )
            else:
                print("Not being able to set the model paths for the given topology.")
                raise ValueError("Unknown topology: " + self.topology)
        elif config.geometry() == LHCPeriod.Run3:
            if self.topology == "TtbarAllHadronic":
                even_path, odd_path = form_even_odd_path(
                    "TtbarAllHadronic", "run3"
                )
            elif self.topology == "TtbarLJets":
                even_path, odd_path = form_even_odd_path(
                    "TtbarLJets", "run3"
                )
            elif self.topology == "TtbarLJetsNoBTag":
                even_path, odd_path = form_even_odd_path(
                    "TtbarLJetsNoBTag", "run3"
                )
            elif self.topology == "TtbarDiLepton":
                even_path, odd_path = form_even_odd_path(
                    "TtbarDiLepton", "run3"
                )
            else:
                print("Not being able to set the model paths for the given topology.")
                raise ValueError("Unknown topology: " + self.topology)
        else:
            print("Not being able to set the model paths for the given run period.")
            raise ValueError(
                "HyPER models not available for run period: " + config.geometry()
            )

        # One Athena ONNX inference tool per cross-validation fold. The model
        # file is a property of each tool's session tool, and is resolved with
        # the PathResolver by the session tool itself.
        for handle, model_path in (
            ("onnxToolTrainedOnEven", even_path),
            ("onnxToolTrainedOnOdd", odd_path),
        ):
            config.addPrivateTool(handle, "AthOnnx::OnnxRuntimeInferenceTool")
            config.addPrivateTool(
                handle + ".ORTSessionTool", "AthOnnx::OnnxRuntimeSessionToolCPU"
            )
            getattr(alg, handle).ORTSessionTool.ModelFileName = model_path

        if self.writeRecoPartonsKinematics:
            if self.topology == "TtbarDiLepton":
                alg.top_b_p4 = decorator_prefix + "_Top_b_p4_%SYS%"
                alg.topbar_bbar_p4 = decorator_prefix + "_Topbar_bbar_p4_%SYS%"
                alg.top_lep_p4 = decorator_prefix + "_Top_lep_p4_%SYS%"
                alg.topbar_lepbar_p4 = decorator_prefix + "_Topbar_lepbar_p4_%SYS%"
            if self.topology == "TtbarAllHadronic":
                alg.top_b_p4 = decorator_prefix + "_Top_b_p4_%SYS%"
                alg.topbar_bbar_p4 = decorator_prefix + "_Topbar_bbar_p4_%SYS%"
                alg.top_Wplus_decay0_p4 = (
                    decorator_prefix + "_Top_Wplus_decay0_p4_%SYS%"
                )
                alg.top_Wplus_decay1_p4 = (
                    decorator_prefix + "_Top_Wplus_decay1_p4_%SYS%"
                )
                alg.topbar_Wminus_decay0_p4 = (
                    decorator_prefix + "_Topbar_Wminus_decay0_p4_%SYS%"
                )
                alg.topbar_Wminus_decay1_p4 = (
                    decorator_prefix + "_Topbar_Wminus_decay1_p4_%SYS%"
                )
            if self.topology in ["TtbarLJets", "TtbarLJetsNoBTag"]:
                alg.toplep_b_p4 = decorator_prefix + "_TopLep_b_p4_%SYS%"
                alg.toplep_lep_p4 = decorator_prefix + "_TopLep_lep_p4_%SYS%"
                alg.tophad_b_p4 = decorator_prefix + "_TopHad_b_p4_%SYS%"
                alg.tophad_w_decay0_p4 = decorator_prefix + "_TopHad_W_decay0_p4_%SYS%"
                alg.tophad_w_decay1_p4 = decorator_prefix + "_TopHad_W_decay1_p4_%SYS%"

        # give appropriate names for the handles to decorate
        if self.topology == "TtbarAllHadronic":
            config.addOutputVar(
                "EventInfo",
                "TtbarAllHadronic_HyPER_Top1_Indices_%SYS%",  # This is the name in the Cpp code.
                f"{decorator_prefix}_Top1_Indices",  # This is the name going to the output tree.
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarAllHadronic_HyPER_Top1_Score_%SYS%",
                f"{decorator_prefix}_Top1_Score",
                auxType="float",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarAllHadronic_HyPER_Top2_Indices_%SYS%",
                f"{decorator_prefix}_Top2_Indices",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarAllHadronic_HyPER_Top2_Score_%SYS%",
                f"{decorator_prefix}_Top2_Score",
                auxType="float",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarAllHadronic_HyPER_W1_Indices_%SYS%",
                f"{decorator_prefix}_W1_Indices",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarAllHadronic_HyPER_W1_Score_%SYS%",
                f"{decorator_prefix}_W1_Score",
                auxType="float",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarAllHadronic_HyPER_W2_Indices_%SYS%",
                f"{decorator_prefix}_W2_Indices",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarAllHadronic_HyPER_W2_Score_%SYS%",
                f"{decorator_prefix}_W2_Score",
                auxType="float",
            )
            if self.writeRecoPartonsKinematics:
                for output_var in [
                    ("top_b_p4", decorator_prefix + "_Top_b_p4"),
                    ("topbar_bbar_p4", decorator_prefix + "_Topbar_bbar_p4"),
                    (
                        "top_Wplus_decay0_p4",
                        decorator_prefix + "_Top_Wplus_decay0_p4",
                    ),
                    (
                        "top_Wplus_decay1_p4",
                        decorator_prefix + "_Top_Wplus_decay1_p4",
                    ),
                    (
                        "topbar_Wminus_decay0_p4",
                        decorator_prefix + "_Topbar_Wminus_decay0_p4",
                    ),
                    (
                        "topbar_Wminus_decay1_p4",
                        decorator_prefix + "_Topbar_Wminus_decay1_p4",
                    ),
                ]:
                    config.addOutputVar(
                        "EventInfo",
                        getattr(alg, output_var[0]),
                        output_var[1],
                        auxType="PtEtaPhiMVector",
                        noSys=False,
                    )
        elif self.topology == "TtbarLJets" or self.topology == "TtbarLJetsNoBTag":
            config.addOutputVar(
                "EventInfo",
                "TtbarLJets_HyPER_Classification_Score_%SYS%",
                f"{decorator_prefix}_Classification_Score",
                auxType="float",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarLJets_HyPER_TopHad_Indices_%SYS%",
                f"{decorator_prefix}_TopHad_Indices",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarLJets_HyPER_TopHad_Score_%SYS%",
                f"{decorator_prefix}_TopHad_Score",
                auxType="float",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarLJets_HyPER_TopHad_IDs_%SYS%",
                f"{decorator_prefix}_TopHad_IDs",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarLJets_HyPER_TopLep_Indices_%SYS%",
                f"{decorator_prefix}_TopLep_Indices",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarLJets_HyPER_TopLep_Score_%SYS%",
                f"{decorator_prefix}_TopLep_Score",
                auxType="float",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarLJets_HyPER_TopLep_IDs_%SYS%",
                f"{decorator_prefix}_TopLep_IDs",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarLJets_HyPER_WHad_Indices_%SYS%",
                f"{decorator_prefix}_WHad_Indices",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarLJets_HyPER_WHad_Score_%SYS%",
                f"{decorator_prefix}_WHad_Score",
                auxType="float",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarLJets_HyPER_WLep_Indices_%SYS%",
                f"{decorator_prefix}_WLep_Indices",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarLJets_HyPER_WLep_Score_%SYS%",
                f"{decorator_prefix}_WLep_Score",
                auxType="float",
            )
            if self.writeRecoPartonsKinematics:
                for output_var in [
                    ("toplep_b_p4", decorator_prefix + "_TopLep_b_p4"),
                    ("toplep_lep_p4", decorator_prefix + "_TopLep_lep_p4"),
                    ("tophad_b_p4", decorator_prefix + "_TopHad_b_p4"),
                    (
                        "tophad_w_decay0_p4",
                        decorator_prefix + "_TopHad_W_decay0_p4",
                    ),
                    (
                        "tophad_w_decay1_p4",
                        decorator_prefix + "_TopHad_W_decay1_p4",
                    ),
                ]:
                    config.addOutputVar(
                        "EventInfo",
                        getattr(alg, output_var[0]),
                        output_var[1],
                        auxType="PtEtaPhiMVector",
                        noSys=False,
                    )
        elif self.topology == "TtbarDiLepton":
            config.addOutputVar(
                "EventInfo",
                "TtbarDiLepton_HyPER_Classification_Score_%SYS%",
                f"{decorator_prefix}_Classification_Score",
                auxType="float",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarDiLepton_HyPER_Top1_Indices_%SYS%",
                f"{decorator_prefix}_Top1_Indices",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarDiLepton_HyPER_Top1_IDs_%SYS%",
                f"{decorator_prefix}_Top1_IDs",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarDiLepton_HyPER_Top1_Score_%SYS%",
                f"{decorator_prefix}_Top1_Score",
                auxType="float",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarDiLepton_HyPER_Top2_Indices_%SYS%",
                f"{decorator_prefix}_Top2_Indices",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarDiLepton_HyPER_Top2_Score_%SYS%",
                f"{decorator_prefix}_Top2_Score",
                auxType="float",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarDiLepton_HyPER_Top2_IDs_%SYS%",
                f"{decorator_prefix}_Top2_IDs",
                auxType="vector_int",
            )
            config.addOutputVar(
                "EventInfo",
                "TtbarDiLepton_HyPER_HE_Score_%SYS%",
                f"{decorator_prefix}_HE_Score",
                auxType="float",
            )

            if self.writeRecoPartonsKinematics:
                for output_var in [
                    ("top_b_p4", decorator_prefix + "_Top_b_p4"),
                    ("topbar_bbar_p4", decorator_prefix + "_Topbar_bbar_p4"),
                    (
                        "top_lep_p4",
                        decorator_prefix + "_Top_lep_p4",
                    ),
                    (
                        "topbar_lepbar_p4",
                        decorator_prefix + "_Topbar_lepbar_p4",
                    ),
                ]:
                    config.addOutputVar(
                        "EventInfo",
                        getattr(alg, output_var[0]),
                        output_var[1],
                        auxType="PtEtaPhiMVector",
                        noSys=False,
                    )
        else:
            raise ValueError("Unknown topology: " + self.topology)
