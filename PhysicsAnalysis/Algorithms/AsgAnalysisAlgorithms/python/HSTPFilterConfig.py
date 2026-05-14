# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class HSTPFilterBlock(ConfigBlock):
    """Config Block for the HSTPFilter algorithm"""

    def __init__(self):
        super(HSTPFilterBlock, self).__init__()
        self.addOption(
            "truthHSCollection",
            "AntiKt4TruthDressedWZJets",
            type=str,
            info="the truth HS jet collection to be used. Default is `AntiKt4TruthDressedWZJets` since the nominal recommendation from JETM (`AntiKt4TruthJets`) is not available in DAOD_PHYS. The differences are expected to be small.",
        )
        self.addOption(
            "truthPUCollection",
            "InTimeAntiKt4TruthJets",
            type=str,
            info="the truth PU jet collection to be used.",
        )

        selected_dsids = []
        selected_dsids += [
            str(dsid) for dsid in range(364700, 364713)
        ]  # Run 2 Pythia8 Baseline
        # Recommended for all run3 dijet samples (https://atlas-jetetmiss.docs.cern.ch/users/QCD-samples/):
        selected_dsids += [
            str(dsid) for dsid in range(801165, 801175)
        ]  # Pythia8 Baseline
        selected_dsids += [
            str(dsid) for dsid in range(830152, 830161)
        ]  # Herwig72 dipole Alternative
        selected_dsids += [
            str(dsid) for dsid in range(830187, 830196)
        ]  # Herwig72 lund Alternative
        selected_dsids += [
            str(dsid) for dsid in range(602693, 602700)
        ]  # PowhegHerwig72 Alternative
        selected_dsids += [
            str(dsid) for dsid in range(700825, 700834)
        ]  # Sherpa2214_Dire Alternative
        selected_dsids += [
            str(dsid) for dsid in range(700798, 700807)
        ]  # Sherpa2214_Lund Alternative
        selected_dsids += [
            str(dsid) for dsid in range(830143, 830152)
        ]  # Herwig72 Systematic
        selected_dsids += [
            str(dsid) for dsid in range(601700, 601708)
        ]  # PowhegPytha8 Systematic
        selected_dsids += [
            str(dsid) for dsid in range(700816, 700825)
        ]  # Sherpa2214 Systematic

        self.setOptionValue("skipOnData", True)
        self.setOptionValue("onlyForDSIDs", selected_dsids)

    def instanceName(self):
        """Return the instance name for this block"""
        return "HSTPFilter"

    def makeAlgs(self, config):

        alg = config.createAlgorithm("CP::HSTPFilterAlg", "HSTPFilterAlg", reentrant=True)

        alg.truthHSCollection = self.truthHSCollection
        alg.truthPUCollection = self.truthPUCollection
