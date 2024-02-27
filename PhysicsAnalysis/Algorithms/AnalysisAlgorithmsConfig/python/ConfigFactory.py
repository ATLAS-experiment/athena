# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

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
    signature = inspect.signature(func)
    return list(signature.parameters.keys())


def getClassArgs(func):
    """return list of args used i=ton initialize class"""
    args = list(inspect.signature(func.__init__).parameters.keys())
    args.remove('self')
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
            self.subAlgs = []
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
                    raise ValueError(f"{arg} is requried for {funcName}")
            if type(func) == type(dict):
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
        if type(alg) == type(dict):
            opts = getClassArgs(alg)
        else:
            opts = getFuncArgs(alg)    

        if superBlocks is None:
            superBlocks = [self.ROOTNAME]
        elif type(superBlocks) != list:
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


    def printAlgs(self, printOpts=False):
        """Prints algorithms exposed to configuration"""
        algs = self._algs
        for alg, algInfo in algs.items():
            algName = algInfo.alg.__name__
            algOptions = algInfo.options
            algDefaults = algInfo.defaults
            print(f"{alg} -> {algName}")
            if printOpts and algOptions:
                for opt in algOptions:
                    if algDefaults and opt in algDefaults:
                        print(f"    {opt}: {algDefaults[opt]}")
                    else:
                        print(f"    {opt}")
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


    def addDefaultAlgs(config):
        """add algorithms and options"""

        # CommonServices
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import makeCommonServicesConfig
        config.addAlgConfigBlock(algName="CommonServices", alg=makeCommonServicesConfig)

        # pileup reweighting
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import PileupReweightingBlock
        config.addAlgConfigBlock(algName="PileupReweighting", alg=PileupReweightingBlock)

        # event cleaning
        from AsgAnalysisAlgorithms.EventCleaningConfig import EventCleaningBlock
        config.addAlgConfigBlock(algName="EventCleaning", alg=EventCleaningBlock)

        # jets
        from JetAnalysisAlgorithms.JetAnalysisConfig import makeJetAnalysisConfig
        config.addAlgConfigBlock(algName="Jets", alg=makeJetAnalysisConfig)
        from JetAnalysisAlgorithms.JetJvtAnalysisConfig import JetJvtAnalysisConfig
        config.addAlgConfigBlock(algName="JVT", alg=JetJvtAnalysisConfig,
            superBlocks="Jets")
        from FTagAnalysisAlgorithms.FTagAnalysisConfig import makeFTagAnalysisConfig
        config.addAlgConfigBlock(algName="FlavourTagging", alg=makeFTagAnalysisConfig,
            defaults={'selectionName': ''},
            superBlocks="Jets")

        # electrons
        from EgammaAnalysisAlgorithms.ElectronAnalysisConfig import ElectronCalibrationConfig 
        config.addAlgConfigBlock(algName="Electrons", alg=ElectronCalibrationConfig)
        from EgammaAnalysisAlgorithms.ElectronAnalysisConfig import ElectronWorkingPointConfig
        config.addAlgConfigBlock(algName="WorkingPoint", alg=ElectronWorkingPointConfig,
            superBlocks="Electrons")

        # photons
        from EgammaAnalysisAlgorithms.PhotonAnalysisConfig import PhotonCalibrationConfig
        config.addAlgConfigBlock(algName="Photons", alg=PhotonCalibrationConfig)
        from EgammaAnalysisAlgorithms.PhotonAnalysisConfig import PhotonWorkingPointConfig
        config.addAlgConfigBlock(algName="WorkingPoint", alg=PhotonWorkingPointConfig,
            superBlocks="Photons")

        # muons
        from MuonAnalysisAlgorithms.MuonAnalysisConfig import MuonCalibrationConfig
        config.addAlgConfigBlock(algName="Muons", alg=MuonCalibrationConfig)
        from MuonAnalysisAlgorithms.MuonAnalysisConfig import MuonWorkingPointConfig
        config.addAlgConfigBlock(algName="WorkingPoint", alg=MuonWorkingPointConfig,
            superBlocks="Muons")

        # tauJets
        from TauAnalysisAlgorithms.TauAnalysisConfig import TauCalibrationConfig
        config.addAlgConfigBlock(algName="TauJets", alg=TauCalibrationConfig)
        from TauAnalysisAlgorithms.TauAnalysisConfig import TauWorkingPointConfig
        config.addAlgConfigBlock(algName="WorkingPoint", alg=TauWorkingPointConfig,
            superBlocks="TauJets")

        # SystObjectLink
        from AsgAnalysisAlgorithms.SystObjectLinkConfig import makeSystObjectLinkConfig
        config.addAlgConfigBlock(algName="SystObjectLink", alg=makeSystObjectLinkConfig,
            superBlocks=[config.ROOTNAME, "Jets", "Electrons", "Photons", "Muons", "TauJets"])

        # IFF truth classification
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import IFFLeptonDecorationBlock
        config.addAlgConfigBlock(algName="IFFClassification", alg=IFFLeptonDecorationBlock,
            superBlocks=["Electrons","Muons"])

        # generator level analysis
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import makeGeneratorAnalysisConfig 
        config.addAlgConfigBlock(algName="GeneratorLevelAnalysis", alg=makeGeneratorAnalysisConfig)

        # pT/Eta Selection
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import makePtEtaSelectionConfig
        config.addAlgConfigBlock(algName="PtEtaSelection", alg=makePtEtaSelectionConfig,
            defaults={'selectionName': ''},
            superBlocks=[config.ROOTNAME, "Jets", "Electrons", "Photons", "Muons", "TauJets"])

        # met
        from MetAnalysisAlgorithms.MetAnalysisConfig import MetAnalysisConfig
        config.addAlgConfigBlock(algName="MissingET", alg=MetAnalysisConfig)

        # overlap removal
        from AsgAnalysisAlgorithms.OverlapAnalysisConfig import OverlapAnalysisConfig
        config.addAlgConfigBlock(algName="OverlapRemoval", alg=OverlapAnalysisConfig,
            defaults={'configName': 'OverlapRemoval'})

        # object-based cutflow
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import ObjectCutFlowBlock
        config.addAlgConfigBlock(algName='ObjectCutFlow', alg=ObjectCutFlowBlock)

        # thinning
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import OutputThinningBlock
        config.addAlgConfigBlock(algName="Thinning", alg=OutputThinningBlock,
            defaults={'configName': 'Thinning'},
            superBlocks=[config.ROOTNAME, "Jets", "Electrons", "Photons", "Muons", "TauJets"])

        # trigger
        from TriggerAnalysisAlgorithms.TriggerAnalysisConfig import TriggerAnalysisBlock
        config.addAlgConfigBlock(algName="Trigger", alg=TriggerAnalysisBlock,
            defaults={'configName': 'Trigger'})

        # event selection
        from EventSelectionAlgorithms.EventSelectionConfig import makeMultipleEventSelectionConfigs
        config.addAlgConfigBlock(algName='EventSelection', alg=makeMultipleEventSelectionConfigs)

        # event-based cutflow
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import EventCutFlowBlock
        config.addAlgConfigBlock(algName='EventCutFlow', alg=EventCutFlowBlock,
            defaults={'containerName': 'EventInfo', 'selectionName': ''})

        # bootstraps
        from AsgAnalysisAlgorithms.BootstrapGeneratorConfig import BootstrapGeneratorConfig
        config.addAlgConfigBlock(algName='Bootstraps', alg=BootstrapGeneratorConfig)

        # per-event scale factor calculation
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import PerEventSFBlock
        config.addAlgConfigBlock(algName='PerEventSF', alg=PerEventSFBlock)

        # output
        from AsgAnalysisAlgorithms.OutputAnalysisConfig import OutputAnalysisConfig
        config.addAlgConfigBlock(algName="Output", alg=OutputAnalysisConfig,
            defaults={'configName': 'Output'})

        return
