# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Joseph Lambert

import yaml
import json
import os
import sys
import importlib
import pathlib

from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

def readYaml(yamlPath):
    """Loads YAML file into a dictionary"""
    if not os.path.isfile(yamlPath):
        raise ValueError(f"{yamlPath} is not a file.")
    with open(yamlPath, 'r') as f:
        textConfig = yaml.safe_load(f)
    return textConfig


def printYaml(d, sort=False, jsonFormat=False):
    """Prints a dictionary as YAML"""
    print(yaml.dump(d, default_flow_style=jsonFormat, sort_keys=sort))


class TextConfig(ConfigFactory):
    def __init__(self, yamlPath=None):
        # will add default blocks in ConfigFactory init
        super().__init__(addDefaultBlocks=False)
        self._textConfig = {}
        if yamlPath is not None:
            self.loadConfig(yamlPath)
        # Block to add new blocks to this object
        self.addAlgConfigBlock(algName="AddConfigBlocks", alg=self.addNewConfigBlocks,
            defaults={'self': self})
        self.addDefaultAlgs()


    def setConfig(self, config):
        """Print YAML configuration file."""
        if self._textConfig:
            # should this be an error?
            #raise ValueError("Configuration has already been loaded.")
            print("WARNING: Overwritting existing configuration.")
        # should check to make sure that config is type dict or OrderedDict
        self._textConfig = config
        return


    def loadConfig(self, yamlPath):
        """read a YAML file in to a dictionary and return it"""
        if not os.path.isfile(yamlPath):
            raise ValueError(f"{yamlPath} is not a file")
        self._textConfig = readYaml(yamlPath)
        return


    def printConfig(self, sort=False, jsonFormat=False):
        """Print YAML configuration file."""
        if self._textConfig is None:
            raise ValueError("No configuration has been loaded.")
        printYaml(self._textConfig, sort, jsonFormat)
        return


    def addNewConfigBlocks(self, modulePath, functionName,
        algName, defaults=None, pos=None, superBlocks=None):
        """
        Load <functionName> from <modulePath>
        """
        try:
            module = importlib.import_module(modulePath)
            fxn = getattr(module, functionName)
        except ModuleNotFoundError as e:
            raise ModuleNotFoundError(f"{e}\nFailed to load {functionName} from {modulePath}")
        else:
            sys.modules[functionName] = fxn
        # add new algorithm to available algorithms
        self.addAlgConfigBlock(algName=algName, alg=fxn,
            defaults=defaults,
            superBlocks=superBlocks,
            pos=pos)
        return


    def configure(self):
        """Process YAML configuration file and confgure added algorithms."""
        def configureAlg(configSeq, block, blockConfig, containerName=None):
            if not isinstance(blockConfig, list):
                blockConfig = [blockConfig]

            for options in blockConfig:
                # Special case: propogate containerName down to subAlgs
                if 'containerName' in options:
                    containerName = options['containerName']
                elif containerName is not None and 'containerName' not in options:
                    options['containerName'] = containerName
                # will check which options are associated alg and not options
                print(f"Configuring {block.algName}")
                seq, funcOpts = block.makeConfig(options)
                if not seq._blocks:
                    continue
                algOpts = seq.setOptions(options)
                configSeq += seq

                # check to see if there are unused parameters
                algOpts = [i['name'] for i in algOpts]
                expectedOptions = set(funcOpts)
                expectedOptions |= set(algOpts)
                expectedOptions |= set(block.subAlgs)

                difference = set(options.keys()) - expectedOptions
                if difference:
                    difference = "\n".join(difference)
                    raise ValueError(f"There are options set that are not used for "
                                     f"{block.algName}:\n{difference}\n"
                                     "Please check your configuration.")


                # check for sub-blocks and call this function recursively
                for alg in self._order.get(block.algName, []):
                    if alg in options:
                        subAlg = block.subAlgs[alg]
                        configureAlg(configSeq, subAlg, options[alg], containerName)
            return configSeq


        ### configure starts here ###
        configSeq = ConfigSequence()
        # check if blocks are defined in yaml file
        if "AddConfigBlocks" in self._textConfig:
            blockConfig = self._textConfig["AddConfigBlocks"]
            alg = self._algs["AddConfigBlocks"]
            configureAlg(configSeq, alg, blockConfig)

        # make sure all blocks in yaml file are added (otherwise they would be ignored)
        for blockName in self._textConfig:
            if blockName not in self._order[self.ROOTNAME]:
                raise ValueError(f"Unkown block {blockName} in yaml file")

        # configure blocks
        for blockName in self._order[self.ROOTNAME]:
            if blockName == "AddConfigBlocks":
                continue

            assert blockName in self._algs

            # order only applies to root blocks
            if blockName in self._textConfig:
                blockConfig = self._textConfig[blockName]
                alg = self._algs[blockName]
                configureAlg(configSeq, alg, blockConfig)
            else:
                continue
        return configSeq


def makeSequence(configPath, dataType, algSeq, geometry=None, autoconfigFromFlags=None,
                 isPhyslite=False, noPhysliteBroken=False, noSystematics=None):
    """
    """

    from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator

    config = TextConfig(configPath)

    print("\n>>> Configuration file read in:")
    config.printConfig()

    print("\n>>> Default algorithms:")
    config.printAlgs(printOpts=True)

    print("\n>>> Configuring algorithms based on YAML file:")
    configSeq = config.configure()

    # defaults are added to config as algs are configured
    print("\n>>> Configuration used:")
    config.printConfig()

    # compile
    configAccumulator = ConfigAccumulator(algSeq, dataType, isPhyslite=isPhyslite, geometry=geometry, autoconfigFromFlags=autoconfigFromFlags, noSystematics=noSystematics)
    configSeq.fullConfigure(configAccumulator)

    # blocks can be reordered during configSeq.fullConfigure
    print("\n>>> ConfigBlocks and their configuration:")
    configSeq.printOptions()

    from AnaAlgorithm.DualUseConfig import isAthena, useComponentAccumulator
    if isAthena and useComponentAccumulator:
        return configAccumulator.CA
    else:
        return None


# Combine configuration files
#
# See the README for more info on how this works
#
def combineConfigFiles(local, config_path, fragment_key="include"):

    # if this isn't an iterable there's nothing to combine
    if isinstance(local, dict):
        to_combine = local.values()
    elif isinstance(local, list):
        to_combine = local
    else:
        return

    # otherwise descend into all the entries here
    for sub in to_combine:
        combineConfigFiles(sub, config_path, fragment_key=fragment_key)

    # if there are no fragments to include we're done
    if fragment_key not in local:
        return

    fragment_path = _find_fragment(
        pathlib.Path(local[fragment_key]),
        config_path)

    with open(fragment_path) as fragment_file:
        # once https://github.com/yaml/pyyaml/issues/173 is resolved
        # pyyaml will support the yaml 1.2 spec, which is compatable
        # with json. Until then yaml and json behave differently, so
        # we have this override.
        if fragment_path.suffix == '.json':
            fragment = json.load(fragment_file)
        else:
            fragment = yaml.safe_load(fragment_file)

    # fill out any sub-fragments, looking in the parent path of the
    # fragment for local sub-fragments.
    combineConfigFiles(
        fragment,
        fragment_path.parent,
        fragment_key=fragment_key
    )

    # merge the fragment with this one
    _merge_dicts(local, fragment)

    # delete the fragment so we don't stumble over it again
    del local[fragment_key]


def _find_fragment(fragment_path, config_path):
    paths_to_check = [
        fragment_path,
        config_path / fragment_path,
        *[x / fragment_path for x in os.environ["DATAPATH"].split(":")]
    ]
    for path in paths_to_check:
        if path.exists():
            return path

    raise FileNotFoundError(fragment_path)


def _merge_dicts(local, fragment):
    # in the list case append the fragment to the local list
    if isinstance(local, list):
        local += fragment
        return
    # In the dict case, append only missing values to local: the local
    # values take precidence over the fragment ones.
    if isinstance(local, dict):
        for key, value in fragment.items():
            if key in local:
                _merge_dicts(local[key], value)
            else:
                local[key] = value
        return
