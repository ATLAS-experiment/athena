# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# This file defines a factory method that can create a configuration
# block sequence based on a passed in name.  This avoids having to
# import all the various config block sequence makers in the
# configuration code, and also would make it easier to create them
# from a text configuration file.

# This relies heavily on the blocks exposing all configurable
# parameters as options, since there is no other mechanism to
# configure them through this interface.

# The implementation itself is probably not the best possible, it
# lacks all extensibility, gathers all information in a single place,
# etc.  Still for now (08 Dec 22) this ought to be good enough.


import inspect
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence

from AnaAlgorithm.Logging import logging
logCPAlgCfgFactory = logging.getLogger('CPAlgCfgFactory')


def getDefaultArgs(func):
    """return dict(par, val) with all func parameters with defualt values"""
    signature = inspect.signature(func)
    return {
        k: v.default
        for k, v in signature.parameters.items()
        if v.default is not inspect.Parameter.empty
    }


def getFuncArgs(func):
    """return list of input parameters"""
    if inspect.isclass(func):
        args = list(inspect.signature(func.__init__).parameters.keys())
        args.remove('self')
    else:
        args = list(inspect.signature(func).parameters.keys())
    return args


# class for config block information
class FactoryBlock():
    """
    """
    def __init__(self, alg, algName, options, defaults, subAlgs=None):
        self.alg = alg
        self.algName = algName
        self.options = options
        self.defaults = defaults
        if subAlgs is None:
            self.subAlgs = {}
        else:
            self.subAlgs = subAlgs


    def makeConfig(self, funcOptions):
            """
            Parameters
            ----------
            funcName: str
                name associated with the algorithm. This name must have been added to the
                list of available algorithms
            funcOptions: dict
                dictionary containing options for the algorithm read from the YAML file

            Returns
            -------
                configSequence
            """
            configSeq = ConfigSequence()

            func = self.alg
            funcName = self.algName
            funcDefaults = getDefaultArgs(func)
            defaults = self.defaults

            args = {}
            # loop over all options for the function
            for arg in getFuncArgs(func):
                # supplied from config file
                if arg in funcOptions:
                    args[arg] = funcOptions[arg]
                # defaults set in function def
                elif arg in funcDefaults:
                    args[arg] = funcDefaults[arg]
                # defaults provided when func was added
                elif defaults is not None and arg in defaults:
                    args[arg] = defaults[arg]
                elif arg == 'seq':
                    # 'seq' should be first arg of func (not needed for class)
                    args[arg] = configSeq
                elif arg == 'kwargs':
                    # cannot handle arbitrary parameters
                    continue
                else:
                    raise ValueError(f"{arg} is required for {funcName}")
            if inspect.isclass(func):
                configSeq.append(func(**args))
            else:
                func(**args)
            return configSeq, args.keys()


class ConfigFactory():
    """This class provides a configuration manager that is intended to allow the user to:
        - define and configure functions that return an algSequence(?) object
    """
    def __init__(self, addDefaultBlocks=True):
        self.ROOTNAME = 'root' # constant
        self._algs = {}
        self._order = {self.ROOTNAME: []}
        if addDefaultBlocks:
            self.addDefaultAlgs()


    def addAlgConfigBlock(self, algName, alg, defaults=None, pos=None, superBlocks=None):
        """Add class to list of available algorithms"""
        if not callable(alg):
            raise ValueError(f"{algName} is not a callable.")
        opts = getFuncArgs(alg)

        if superBlocks is None:
            superBlocks = [self.ROOTNAME]
        elif not isinstance(superBlocks, list):
            superBlocks = [superBlocks]

        # add new alg block to subAlgs dict of super block
        for block in superBlocks:
            if block not in self._order:
                self._order[block] = []
            order = self._order[block]

            if block == self.ROOTNAME:
                algs = self._algs
            else:
                if block not in self._algs:
                    raise ValueError(f"{block} not added")
                algs = self._algs[block].subAlgs

            if alg in algs:
                raise ValueError(f"{algName} has already been added.")

            # create FactoryBlock with alg information
            algs[algName] = FactoryBlock(
                alg=alg,
                algName=algName,
                options=opts,
                defaults=defaults,
                subAlgs={}
            )
            # insert into order (list)
            if pos is None:
                order.append(algName)
            elif pos in order:
                order.insert(order.index(pos), algName)
            else:
                raise ValueError(f"{pos} does not exit in already added config blocks")
        return


    def printAlgs(self, printOpts=True):
        """Prints algorithms exposed to configuration"""
        printed = [] # some subblocks exist for multiple superblocks
        def printAlg(algs):
            for alg, algInfo in algs.items():
                algName = algInfo.alg.__name__
                algOptions = algInfo.options
                if algName not in printed:
                    printed.append(algName)
                    logCPAlgCfgFactory.info(f"\033[4m{alg}\033[0m -> \033[4m{algName}\033[0m")
                    if printOpts:
                        try:
                            if inspect.isclass(algInfo.alg):
                                # block
                                algInfo.alg().printOptions(verbose=printOpts)
                            else:
                                # make function
                                seq = ConfigSequence()
                                algInfo.alg(seq=seq)
                                seq.printOptions(verbose=printOpts)
                        except Exception:
                            # either a TypeError or something else due to missing args
                            # try to print something for casses with required args
                            for opt in algOptions:
                                logCPAlgCfgFactory.info(f"    {opt}")
                printAlg(algInfo.subAlgs)
        printAlg(self._algs)
        return


    def makeConfig(self, name, **kwargs):
        """
        Returns:
            configSeq: configSequence object
        """
        try:
            if '.' in name:
                algContext, algName = name.split('.')
                block = self._algs[algContext].subAlgs[algName]
            else:
                block = self._algs[name]
        except KeyError:
            raise ValueError(f"{name} config block not found. Make sure context is correct.")
        configSeq, _ = block.makeConfig(kwargs)
        return configSeq


    def addDefaultAlgs(self):
        """add algorithms and options"""

        # CommonServices
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import CommonServicesConfig
        self.addAlgConfigBlock(algName="CommonServices", alg=CommonServicesConfig)

        # pileup reweighting
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import PileupReweightingBlock
        self.addAlgConfigBlock(algName="PileupReweighting", alg=PileupReweightingBlock)

        # event cleaning
        from AsgAnalysisAlgorithms.EventCleaningConfig import EventCleaningBlock
        self.addAlgConfigBlock(algName="EventCleaning", alg=EventCleaningBlock)

        # trigger
        from TriggerAnalysisAlgorithms.TriggerAnalysisConfig import Trigger
        self.addAlgConfigBlock(algName="Trigger", alg=Trigger)
        from TriggerAnalysisAlgorithms.TriggerAnalysisSFConfig import TriggerAnalysisSFBlock
        self.addAlgConfigBlock(algName="TriggerMatching", alg=TriggerAnalysisSFBlock)

        # jets
        from JetAnalysisAlgorithms.JetAnalysisConfig import makeJetAnalysisConfig
        self.addAlgConfigBlock(algName="Jets", alg=makeJetAnalysisConfig)
        from JetAnalysisAlgorithms.JetJvtAnalysisConfig import JetJvtAnalysisConfig
        self.addAlgConfigBlock(algName="JVT", alg=JetJvtAnalysisConfig,
            superBlocks="Jets")
        from JetAnalysisAlgorithms.BJetCalibAnalysisConfig import BJetCalibAnalysisConfig
        self.addAlgConfigBlock(algName="BJetCalib", alg=BJetCalibAnalysisConfig,
                               superBlocks="Jets")
        from FTagAnalysisAlgorithms.FTagAnalysisConfig import FTagConfig
        self.addAlgConfigBlock(algName="FlavourTagging", alg=FTagConfig,
            defaults={'selectionName': ''},
            superBlocks="Jets")
        from FTagAnalysisAlgorithms.FTagSFAnalysisConfig import FlavourTaggingEventSF
        self.addAlgConfigBlock(algName="FlavourTaggingEventSF",
                               alg=FlavourTaggingEventSF,
                               defaults={'selectionName': ''},
                               superBlocks="Jets")
        from FTagAnalysisAlgorithms.XbbAnalysisConfig import XbbConfig
        self.addAlgConfigBlock(algName="XbbTagging", alg=XbbConfig,
                               superBlocks="Jets")

        # electrons
        from EgammaAnalysisAlgorithms.ElectronAnalysisConfig import ElectronCalibrationConfig
        self.addAlgConfigBlock(algName="Electrons", alg=ElectronCalibrationConfig)
        from EgammaAnalysisAlgorithms.ElectronAnalysisConfig import ElectronWorkingPointConfig
        self.addAlgConfigBlock(algName="WorkingPoint", alg=ElectronWorkingPointConfig,
            superBlocks="Electrons")
        from EgammaAnalysisAlgorithms.ElectronAnalysisConfig import ElectronLRTMergedConfig
        self.addAlgConfigBlock(algName="LRTMerging", alg=ElectronLRTMergedConfig,
                               superBlocks="Electrons")
        from EgammaAnalysisAlgorithms.ElectronAnalysisConfig import ElectronTriggerAnalysisSFBlock
        self.addAlgConfigBlock(algName="TriggerSF", alg=ElectronTriggerAnalysisSFBlock,
                               superBlocks="Electrons")

        # photons
        from EgammaAnalysisAlgorithms.PhotonAnalysisConfig import PhotonCalibrationConfig
        self.addAlgConfigBlock(algName="Photons", alg=PhotonCalibrationConfig)
        from EgammaAnalysisAlgorithms.PhotonAnalysisConfig import PhotonWorkingPointConfig
        self.addAlgConfigBlock(algName="WorkingPoint", alg=PhotonWorkingPointConfig,
            superBlocks="Photons")
        from EgammaAnalysisAlgorithms.PhotonExtraVariablesConfig import PhotonExtraVariablesBlock
        self.addAlgConfigBlock(algName="ExtraVariables", alg=PhotonExtraVariablesBlock,
            superBlocks="Photons")

        # muons
        from MuonAnalysisAlgorithms.MuonAnalysisConfig import MuonCalibrationConfig
        self.addAlgConfigBlock(algName="Muons", alg=MuonCalibrationConfig)
        from MuonAnalysisAlgorithms.MuonAnalysisConfig import MuonWorkingPointConfig
        self.addAlgConfigBlock(algName="WorkingPoint", alg=MuonWorkingPointConfig,
            superBlocks="Muons")
        from MuonAnalysisAlgorithms.MuonAnalysisConfig import MuonTriggerAnalysisSFBlock
        self.addAlgConfigBlock(algName="TriggerSF", alg=MuonTriggerAnalysisSFBlock,
                               superBlocks="Muons")
        from MuonAnalysisAlgorithms.MuonAnalysisConfig import MuonLRTMergedConfig
        self.addAlgConfigBlock(algName="LRTMerging", alg=MuonLRTMergedConfig,
                               superBlocks="Muons")
        from MuonAnalysisAlgorithms.MuonAnalysisConfig import MuonContainerMergingConfig
        self.addAlgConfigBlock(algName="ContainerMerging", alg=MuonContainerMergingConfig,
                               superBlocks="Muons")

        # tauJets
        from TauAnalysisAlgorithms.TauAnalysisConfig import TauCalibrationConfig
        self.addAlgConfigBlock(algName="TauJets", alg=TauCalibrationConfig)
        from TauAnalysisAlgorithms.TauAnalysisConfig import TauWorkingPointConfig
        self.addAlgConfigBlock(algName="WorkingPoint", alg=TauWorkingPointConfig,
            superBlocks="TauJets")
        from TauAnalysisAlgorithms.TauAnalysisConfig import TauTriggerAnalysisSFBlock
        self.addAlgConfigBlock(algName="TriggerSF", alg=TauTriggerAnalysisSFBlock,
                               superBlocks="TauJets")

        # SystObjectLink
        from AsgAnalysisAlgorithms.SystObjectLinkConfig import SystObjectLinkBlock
        self.addAlgConfigBlock(algName="SystObjectLink", alg=SystObjectLinkBlock,
            superBlocks=[self.ROOTNAME, "Jets", "Electrons", "Photons", "Muons", "TauJets"])

        # Particle-level truth algorithms
        from TruthParticleLevelAnalysisAlgorithms.ParticleLevelElectronsConfig import ParticleLevelElectronsBlock
        self.addAlgConfigBlock(algName="PL_Electrons", alg=ParticleLevelElectronsBlock)
        from TruthParticleLevelAnalysisAlgorithms.ParticleLevelMuonsConfig import ParticleLevelMuonsBlock
        self.addAlgConfigBlock(algName="PL_Muons", alg=ParticleLevelMuonsBlock)
        from TruthParticleLevelAnalysisAlgorithms.ParticleLevelNeutrinosConfig import ParticleLevelNeutrinosBlock
        self.addAlgConfigBlock(algName="PL_Neutrinos", alg=ParticleLevelNeutrinosBlock)
        from TruthParticleLevelAnalysisAlgorithms.ParticleLevelJetsConfig import ParticleLevelJetsBlock
        self.addAlgConfigBlock(algName="PL_Jets", alg=ParticleLevelJetsBlock)
        from TruthParticleLevelAnalysisAlgorithms.ParticleLevelTausConfig import ParticleLevelTausBlock
        self.addAlgConfigBlock(algName="PL_Taus", alg=ParticleLevelTausBlock)
        from TruthParticleLevelAnalysisAlgorithms.ParticleLevelPhotonsConfig import ParticleLevelPhotonsBlock
        self.addAlgConfigBlock(algName="PL_Photons", alg=ParticleLevelPhotonsBlock)
        from TruthParticleLevelAnalysisAlgorithms.ParticleLevelMissingETConfig import ParticleLevelMissingETBlock
        self.addAlgConfigBlock(algName="PL_MissingET", alg=ParticleLevelMissingETBlock)
        from TruthParticleLevelAnalysisAlgorithms.ParticleLevelOverlapRemovalConfig import ParticleLevelOverlapRemovalBlock
        self.addAlgConfigBlock(algName="PL_OverlapRemoval", alg=ParticleLevelOverlapRemovalBlock)

        # IFF truth classification
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import IFFLeptonDecorationBlock
        self.addAlgConfigBlock(algName="IFFClassification", alg=IFFLeptonDecorationBlock,
            superBlocks=["Electrons", "Muons",
                         "PL_Electrons", "PL_Muons"])
        # MCTC truth classification
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import MCTCLeptonDecorationBlock
        self.addAlgConfigBlock(algName="MCTCClassification", alg=MCTCLeptonDecorationBlock,
            superBlocks=["Electrons", "Muons", "TauJets",
                         "PL_Electrons", "PL_Muons", "PL_Taus"])

        # generator level analysis
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import GeneratorAnalysisBlock
        self.addAlgConfigBlock(algName="GeneratorLevelAnalysis", alg=GeneratorAnalysisBlock)

        # pT/Eta Selection
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import PtEtaSelectionBlock
        self.addAlgConfigBlock(algName="PtEtaSelection", alg=PtEtaSelectionBlock,
            defaults={'selectionName': ''},
            superBlocks=[self.ROOTNAME,
                         "Jets", "Electrons", "Photons", "Muons", "TauJets",
                         "PL_Jets", "PL_Electrons", "PL_Photons", "PL_Muons", "PL_Taus", "PL_Neutrinos"])

        # met
        from MetAnalysisAlgorithms.MetAnalysisConfig import MetAnalysisConfig
        self.addAlgConfigBlock(algName="MissingET", alg=MetAnalysisConfig)

        # overlap removal
        from AsgAnalysisAlgorithms.OverlapAnalysisConfig import OverlapAnalysisConfig
        self.addAlgConfigBlock(algName="OverlapRemoval", alg=OverlapAnalysisConfig,
            defaults={'configName': 'OverlapRemoval'})

        # object-based cutflow
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import ObjectCutFlowBlock
        self.addAlgConfigBlock(algName='ObjectCutFlow', alg=ObjectCutFlowBlock)

        # jet reclustering
        from JetAnalysisAlgorithms.JetReclusteringConfig import JetReclusteringBlock
        self.addAlgConfigBlock(algName="JetReclustering", alg=JetReclusteringBlock)

        # jet reclustering calibration
        from JetAnalysisAlgorithms.ReclusteredJetCalibrationConfig import ReclusteredJetCalibrationBlock
        self.addAlgConfigBlock(algName="ReclusteredJetCalibration", alg=ReclusteredJetCalibrationBlock)

        # event selection
        from EventSelectionAlgorithms.EventSelectionConfig import makeMultipleEventSelectionConfigs
        self.addAlgConfigBlock(algName='EventSelection', alg=makeMultipleEventSelectionConfigs)

        # event-based cutflow
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import EventCutFlowBlock
        self.addAlgConfigBlock(algName='EventCutFlow', alg=EventCutFlowBlock,
            defaults={'containerName': 'EventInfo', 'selectionName': ''})

        # bootstraps
        from AsgAnalysisAlgorithms.BootstrapGeneratorConfig import BootstrapGeneratorConfig
        self.addAlgConfigBlock(algName='Bootstraps', alg=BootstrapGeneratorConfig)

        # per-event scale factor calculation
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import PerEventSFBlock
        self.addAlgConfigBlock(algName='PerEventSF', alg=PerEventSFBlock)

        # per-event unified lepton scale factor calculation
        from AsgAnalysisAlgorithms.LeptonSFCalculatorConfig import LeptonSFCalculatorBlock
        self.addAlgConfigBlock(algName='LeptonSF', alg=LeptonSFCalculatorBlock)

        # thinning
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import OutputThinningBlock
        self.addAlgConfigBlock(algName="Thinning", alg=OutputThinningBlock,
            defaults={'configName': 'Thinning'})

        # selection decorations
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import SelectionDecorationBlock
        self.addAlgConfigBlock(algName='SelectionDecoration',
                               alg=SelectionDecorationBlock)

        # di-tau mass calculator
        from TauAnalysisAlgorithms.DiTauMassConfig import DiTauMassBlock
        self.addAlgConfigBlock(algName="DiTauMMC", alg=DiTauMassBlock)

        # IFF fake background estimator
        from AsgAnalysisAlgorithms.FakeBkgConfig import FakeBkgBlock
        self.addAlgConfigBlock(algName='FakeBkgCalculator', alg=FakeBkgBlock)

        # VGamma overlap removal
        from AsgAnalysisAlgorithms.VGammaORConfig import VGammaORBlock
        self.addAlgConfigBlock(algName='VGammaOR', alg=VGammaORBlock)

        # output
        from AsgAnalysisAlgorithms.OutputAnalysisConfig import OutputAnalysisConfig
        self.addAlgConfigBlock(algName="Output", alg=OutputAnalysisConfig,
            defaults={'configName': 'Output'})

        # IOStats printouts
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import IOStatsBlock
        self.addAlgConfigBlock(algName="IOStats", alg=IOStatsBlock)

        # configuration printer
        from AsgAnalysisAlgorithms.PrintToolConfigAlgConfig import PrintToolConfigAlgBlock
        self.addAlgConfigBlock(algName="PrintConfiguration", alg=PrintToolConfigAlgBlock)

        # Pixel dE/dx equalization tool
        from TrackingAnalysisAlgorithms.TrackingAnalysisConfig import PixelDEdxEqualizationBlock
        self.addAlgConfigBlock(algName="PixelDEdxEqualization", alg=PixelDEdxEqualizationBlock)
        
        return
