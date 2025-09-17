# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
import AnaAlgorithm.DualUseConfig as DualUseConfig


class TruthCollectionsFixerBlock(ConfigBlock):
    """This Config Block is meant to help fix older DAOD samples that have the old HepMC barcode instead of uid.
       See also https://gitlab.cern.ch/atlas/athena/-/merge_requests/82613
       and https://gitlab.cern.ch/atlas/athena/-/merge_requests/82746
    """

    def __init__(self):
        super(TruthCollectionsFixerBlock, self).__init__()
        self.addOption(
            "truthContainersToFix",
            [
                "TruthBoson",
                "TruthBosonsWithDecayParticles",
                "TruthElectrons",
                "TruthMuons",
                "TruthPhotons",
                "TruthNeutrinos",
                "TruthTaus",
                "TruthTausWithDecayParticles",
                "TruthTop",
                "TruthBottom",
                "TruthCharm",
                "TruthHFWithDecayParticles",
                "TruthForwardProtons",
                "TruthPileupParticles",
                "BornLeptons",
            ],
            type=list,
            info="list of input DAOD truth containers to fix",
        )
        self.addOption("fixDAODTruthRecord", True, type=bool,
                       info="older derivations have the old HepMC barcodes and need to be fixed, otherwise we get "
                       "a crash on 'missing ::uid'. Schedules an instance of TruthCollectionsFixerBlock. "
                       "Not needed for recent derivations.")

    def makeAlgs(self, config):

        # we need this boolean because the responsibility of scheduling the whole block is passed onto CommonServicesConfig
        if not self.fixDAODTruthRecord: return

        if config.dataType() is DataType.Data: return

        containers = self.truthContainersToFix
        # not all containers are available in PHYSLITE
        missing_in_physlite = ["TruthTausWithDecayParticles","TruthCharm","TruthHFWithDecayParticles","TruthPileupParticles"]
        if config.isPhyslite():
            containers = list(set(self.truthContainersToFix) - set(missing_in_physlite))

        # in Athena, we have to rename the containers. In AnalysisBase, we can just overwrite in place
        if DualUseConfig.isAthena:
            # replicate the behaviour of AddressRemappingCfg
            ars = config.createService("AddressRemappingSvc", "AddressRemappingSvc")
            pps = config.createService("ProxyProviderSvc", "ProxyProviderSvc")
            if "AddressRemappingSvc" not in pps.ProviderNames:
                pps.ProviderNames += ["AddressRemappingSvc"]
            for container in containers:
                ars.TypeKeyRenameMaps += [
                    f"xAOD::TruthParticleContainer#{container}->InFile{container}",
                    f"xAOD::AuxContainerBase#{container}Aux.->InFile{container}Aux.",
                ]

        # the actual fix for old DAOD truth schema
        for container in containers:
            alg = config.createAlgorithm(
                "xAODMaker::TruthParticleFixerAlg",
                "TruthParticleFixerAlg_" + container,
                reentrant=True,
            )
            alg.InputContainer = (
                container if not DualUseConfig.isAthena else f"InFile{container}"
            )
            alg.OutputContainer = container

            # in Athena, we further need to remap relevant ElementLinks for containers that have them
            containers_without_parent_child_links = ["TruthBosonsWithDecayParticles", "TruthTausWithDecayParticles", "BornLeptons", "TruthPileupParticles", "TruthForwardProtons"]
            if DualUseConfig.isAthena and container not in containers_without_parent_child_links:
                alg.LinkPrefixToRemove = "InFile"
                alg.ParticleLinks = ["parentLinks", "childLinks"]
