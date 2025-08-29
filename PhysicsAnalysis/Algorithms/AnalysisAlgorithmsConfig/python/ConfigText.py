# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Joseph Lambert

import yaml
import json
import os
import sys
import importlib
import pathlib
import warnings

from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory
from AnalysisAlgorithmsConfig.ConfigAccumulator import deprecationWarningCategory

from AnaAlgorithm.Logging import logging
logCPAlgTextCfg = logging.getLogger('CPAlgTextCfg')


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
    def __init__(self, yamlPath=None, *, config=None, addDefaultBlocks=True):
        super().__init__(addDefaultBlocks=False)

        if yamlPath and config:
            raise ValueError("Cannot specify both yamlPath and config. Use one or the other.")

        # Block to add new blocks to this object
        self.addAlgConfigBlock(algName="AddConfigBlocks", alg=self._addNewConfigBlocks,
            defaults={'self': self})
        # add default blocks
        if addDefaultBlocks:
            self.addDefaultAlgs()
        # load yaml
        self._config = {}
        # do not allow for loading multiple yaml files
        self.__loadedYaml = False
        if yamlPath is not None or config is not None:
            self.loadConfig(yamlPath, configDict=config)
        # last is used for setOptionValue when using addBlock
        self._last = None


    def setConfig(self, config):
        """Print YAML configuration file."""
        if self._config:
            raise ValueError("Configuration has already been loaded.")
        self._config = config
        return

    # Less-than-ideal fix introduced in !76767
    def preprocessConfig(self, config, algs):
        """
        Preprocess the configuration dictionary.
        Ensure blocks with only sub-blocks are initialized with an empty dictionary.
        """
        def processNode(node, algs):
            if not isinstance(node, dict):
                return  # Base case: not a dictionary
            for blockName, blockContent in list(node.items()):
                # If the block name is recognized in algs
                if blockName in algs:
                    # If the block only defines sub-blocks, initialize it
                    if isinstance(blockContent, dict) and not any(
                            key in algs[blockName].options for key in blockContent
                    ):
                        # Ensure parent block is initialized as an empty dictionary
                        node[blockName] = {'__placeholder__': True, **blockContent}
                    # Recurse into sub-blocks
                    processNode(node[blockName], algs[blockName].subAlgs)

        # Start processing from the root of the configuration
        processNode(config, algs)

    # Less-than-ideal fix introduced in !76767
    def cleanupPlaceholders(self, config):
        """
        Remove placeholder markers after initialization.
        """
        if not isinstance(config, dict):
            return
        if "__placeholder__" in config:
            del config["__placeholder__"]
        for key, value in config.items():
            self.cleanupPlaceholders(value)

    def loadConfig(self, yamlPath=None, *, configDict=None):
        """
        read a YAML file. Will combine with any config blocks added using python
        """
        if self.__loadedYaml or isinstance(yamlPath, list):
            raise NotImplementedError("Mering multiple yaml files is not implemented.")
        self.__loadedYaml = True

        def merge(config, algs, path=''):
            """Add to config block-by-block"""
            if not isinstance(config, list):
                config = [config]
            # loop over list of blocks with same block name
            for blocks in config:
                # deal with case where empty dict is config
                if blocks == {} and path:
                    self.addBlock(path)
                    return
                # remove any subBlocks from block config
                subBlocks = {}
                for blockName in algs:
                    if blockName in blocks:
                        subBlocks[blockName] = blocks.pop(blockName)
                # anything left should be a block and it's configuration
                if blocks:
                    self.addBlock(path, **blocks)
                # add in any subBlocks
                for subName, subBlock in subBlocks.items():
                    newPath = f'{path}.{subName}' if path else subName
                    merge(subBlock, algs[subName].subAlgs, newPath)
            return

        logCPAlgTextCfg.info(f'loading {yamlPath}')
        if configDict is not None:
            # if configDict is provided, use it directly
            config = configDict
        else:
            config = readYaml(yamlPath)
        # check if blocks are defined in yaml file
        if "AddConfigBlocks" in config:
           self._configureAlg(self._algs["AddConfigBlocks"], config["AddConfigBlocks"])

        # Preprocess the configuration dictionary (see !76767)
        self.preprocessConfig(config, self._algs)

        merge(config, self._algs)

        # Cleanup placeholders (see !76767)
        self.cleanupPlaceholders(config)

        return


    def printConfig(self, sort=False, jsonFormat=False):
        """Print YAML configuration file."""
        if self._config is None:
            raise ValueError("No configuration has been loaded.")
        printYaml(self._config, sort, jsonFormat)
        return


    def saveYaml(self, filePath='config.yaml', default_flow_style=False,
            **kwargs):
        """
        Convert dictionary representation to yaml and save
        """
        logCPAlgTextCfg.info(f"Saving configuration to {filePath}")
        config = self._config
        with open(filePath, 'w') as outfile:
            yaml.dump(config, outfile, default_flow_style=False, **kwargs)
        return


    def addBlock(self, name, **kwargs):
        """
        Create entry into dictionary representing the text configuration
        """
        def setEntry(name, config, opts):
            if '.' not in name:
                if name not in config:
                    config[name] = opts
                elif isinstance(config[name], list):
                    config[name].append(opts)
                else:
                    config[name] = [config[name], opts]
                # set last added block for setOptionValue
                self._last = opts
            else:
                name, rest = name[:name.index('.')], name[name.index('.') + 1:]
                config = config[name]
                if isinstance(config, list):
                    config = config[-1]
                setEntry(rest, config, opts)
            return
        setEntry(name, self._config, dict(kwargs))
        return


    def setOptions(self, **kwargs):
        """
        Set option(s) for the lsat block that was added. If an option
        was added previously, will update value
        """
        if self._last is None:
            raise TypeError("Cannot set options before adding a block")
        # points to dict with opts for last added block
        self._last.update(**kwargs)


    def configure(self):
        """Process YAML configuration file and confgure added algorithms."""
        # make sure all blocks in yaml file are added (otherwise they would be ignored)
        for blockName in self._config:
            if blockName not in self._order[self.ROOTNAME]:
                if not blockName:
                    blockName = list(self._config[blockName].keys())[0]
                raise ValueError(f"Unkown block {blockName} in yaml file")

        # configure blocks
        configSeq = ConfigSequence()
        for blockName in self._order[self.ROOTNAME]:
            if blockName == "AddConfigBlocks":
                continue

            assert blockName in self._algs

            # order only applies to root blocks
            if blockName in self._config:
                blockConfig = self._config[blockName]
                alg = self._algs[blockName]
                self._configureAlg(alg, blockConfig, configSeq)
            else:
                continue
        return configSeq


    def _addNewConfigBlocks(self, modulePath, functionName,
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


    def _configureAlg(self, block, blockConfig, configSeq=None, containerName=None,
                      extraOptions=None):
        if not isinstance(blockConfig, list):
            blockConfig = [blockConfig]

        for options in blockConfig:
            # Special case: propogate containerName down to subAlgs
            if 'containerName' in options:
                containerName = options['containerName']
            elif containerName is not None and 'containerName' not in options:
                options['containerName'] = containerName
            # will check which options are associated alg and not options
            logCPAlgTextCfg.info(f"Configuring {block.algName}")
            seq, funcOpts = block.makeConfig(options)
            if not seq._blocks:
                continue
            algOpts = seq.setOptions(options)
            # If containerName was not set explicitly, we can now retrieve
            # its default value
            if containerName is None:
                for opt in algOpts:
                    if 'name' in opt and opt['name'] == 'containerName':
                        containerName = opt.get('value', None)
                        break  # Exit the loop as we've found the key

            if configSeq is not None:
                configSeq += seq

            # propagate special extra options to subalgs
            if extraOptions is None:
                extraOptionsList = ["skipOnData", "skipOnMC", "onlyForDSIDs"]
                for i in algOpts:
                    if i['name'] in extraOptionsList and i['defaultValue'] != i['value']:
                        if extraOptions is None:
                            extraOptions = {}
                        extraOptions[i['name']] = i['value']
            else:
                algOpts = seq.setOptions(extraOptions.copy())

            # check to see if there are unused parameters
            algOpts = [i['name'] for i in algOpts]
            expectedOptions = set(funcOpts)
            expectedOptions |= set(algOpts)
            expectedOptions |= set(block.subAlgs)

            difference = set(options.keys()) - expectedOptions
            difference.discard('__placeholder__')
            if difference:
                difference = "\n".join(difference)
                raise ValueError(f"There are options set that are not used for "
                                 f"{block.algName}:\n{difference}\n"
                                 "Please check your configuration.")

            # check for sub-blocks and call this function recursively
            for alg in self._order.get(block.algName, []):
                if alg in options:
                    subAlg = block.subAlgs[alg]
                    self._configureAlg(subAlg, options[alg], configSeq, containerName, extraOptions)
        return configSeq


def makeSequence(configPath, *, flags=None, algSeq=None, noSystematics=None, dataType=None, geometry=None, autoconfigFromFlags=None, isPhyslite=None, noPhysliteBroken=False):
    """
    """

    # Historically we have used the identifier
    # `autoconfigFromFlags`, but in the rest of the code base
    # `flags` is used. So for now we allow either, and can hopefully
    # at some point remove the former (21 Aug 25).
    if autoconfigFromFlags is not None:
        if flags is not None:
            raise ValueError("Cannot pass both flags and autoconfigFromFlags arguments")
        flags = autoconfigFromFlags
        warnings.warn ('Using autoconfigFromFlags parameter is deprecated, use flags instead', category=deprecationWarningCategory, stacklevel=2)
    elif flags is None:
        warnings.warn ('it is deprecated to configure meta-data for analysis configuration manually, please read the configuration flags via the meta-data reader', category=deprecationWarningCategory, stacklevel=2)

    from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator

    config = TextConfig(configPath)

    logCPAlgTextCfg.info("Configuration file read in:")
    config.printConfig()

    logCPAlgTextCfg.info("Default algorithms:")
    config.printAlgs(printOpts=True)

    logCPAlgTextCfg.info("Configuring algorithms based on YAML file:")
    configSeq = config.configure()

    # defaults are added to config as algs are configured
    logCPAlgTextCfg.info("Configuration used:")
    config.printConfig()

    # compile
    configAccumulator = ConfigAccumulator(algSeq=algSeq, dataType=dataType, isPhyslite=isPhyslite, geometry=geometry, autoconfigFromFlags=autoconfigFromFlags, flags=flags, noSystematics=noSystematics)
    configSeq.fullConfigure(configAccumulator)

    # blocks can be reordered during configSeq.fullConfigure
    logCPAlgTextCfg.info("ConfigBlocks and their configuration:")
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
    combined = False

    # if this isn't an iterable there's nothing to combine
    if isinstance(local, dict):
        to_combine = local.values()
    elif isinstance(local, list):
        to_combine = local
    else:
        return combined

    # otherwise descend into all the entries here
    for sub in to_combine:
        combined = combineConfigFiles(sub, config_path, fragment_key=fragment_key) or combined

    # if there are no fragments to include we're done
    if fragment_key not in local:
        return combined

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


    # if we came to here we merged a fragment, so return True
    return True


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
