# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

import textwrap
import inspect

from AnaAlgorithm.Logging import logging
logCPAlgCfgBlock = logging.getLogger('CPAlgCfgBlock')

from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
import re

def filter_dsids (filterList, config) :
    """check whether the sample being run passes a"""
    """possible DSID filter on the block"""
    if len(filterList) == 0:
        return True
    for dsid_filter in filterList:
        # Check if the pattern is enclosed in regex delimiters (e.g., starts with '^' or contains regex metacharacters)
        if any(char in dsid_filter for char in "^$*+?.()|[]{}\\"):
            pattern = re.compile(dsid_filter)
            if pattern.match(str(config.dsid())):
                return True
        else:
            # Otherwise it's an exact DSID (but could be int or string)
            if str(dsid_filter) == str(config.dsid()):
                return True
    return False


class ConfigBlockOption:
    """the information for a single option on a configuration block"""

    def __init__ (self, type=None, info='', noneAction='ignore', required=False,
            default=None) :
        self.type = type
        self.info = info
        self.required = required
        self.noneAction = noneAction
        self.default = default



class ConfigBlockDependency():
    """Class encoding a blocks dependence on other blocks."""

    def __init__(self, blockName, required=True):
        self.blockName = blockName
        self.required = required


    def __eq__(self, name):
        return self.blockName == name


    def __str__(self):
        return self.blockName


    def __repr__(self):
        return f'ConfigBlockDependency(blockName="{self.blockName}", required={self.required})'


class ConfigBlock:
    """the base class for classes implementing individual blocks of
    configuration

    A configuration block is a sequence of one or more algorithms that
    should always be scheduled together, e.g. the muon four momentum
    corrections could be a single block, muon selection could then be
    another block.  The blocks themselves generally have their own
    configuration options/properties specific to the block, and will
    perform a dynamic configuration based on those options as well as
    the overall job.

    The actual configuration of the algorithms in the block will
    depend on what other blocks are scheduled before and afterwards,
    most importantly some algorithms will introduce shallow copies
    that subsequent algorithms will need to run on, and some
    algorithms will add selection decorations that subquent algorithms
    should use as preselections.

    The algorithms get created in a multi-step process (that may be
    extended in the future): As a first step each block retrieves
    references to the containers it uses (essentially marking its spot
    in the processing chain) and also registering any shallow copies
    that will be made.  In the second/last step each block then
    creates the fully configured algorithms.

    One goal is that when the algorithms get created they will have
    their final configuration and there needs to be no
    meta-configuration data attached to the algorithms, essentially an
    inversion of the approach in AnaAlgSequence in which the
    algorithms got created first with associated meta-configuration
    and then get modified in susequent configuration steps.

    For now this is mostly an empty base class, but another goal of
    this approach is to make it easier to build another configuration
    layer on top of this one, and this class will likely be extended
    and get data members at that point.

    The child class needs to implement the method `makeAlgs` which is
    given a single `ConfigAccumulator` type argument. This is meant to
    create the sequence of algorithms that this block configures. This
    is currently (28 Jul 2025) called twice and should do the same thing
    during both calls, but the plan is to change that to a single call.

    The child class should also implement the method `getInstanceName`
    which should return a string that is used to distinguish between
    multiple instances of the same block. This is used to append the
    instance name to the names of all algorithms created by this block,
    and may in the future also be used to distinguish between multiple
    instances of the block.
    """

    # Class-level dictionary to keep track of instance counts for each derived class
    instance_counts = {}

    def __init__ (self) :
        self._blockName = ''
        self._dependencies = []
        self._options = {}
        # used with block configuration to set arbitrary option
        self.addOption('groupName', '', type=str,
            info=('Used to specify this block when setting an'
                ' option at an arbitrary location.'))
        self.addOption('skipOnData', False, type=bool,
            info=('User option to prevent the block from running'
                  ' on data. This only affects blocks that are'
                  ' intended to run on data.'))
        self.addOption('skipOnMC', False, type=bool,
            info=('User option to prevent the block from running'
                  ' on MC. This only affects blocks that are'
                  ' intended to run on MC.'))
        self.addOption('onlyForDSIDs', [], type=list,
            info=('Used to specify which MC DSIDs to allow this'
                  ' block to run on. Each element of the list'
                  ' can be a full DSID (e.g. 410470), or a regex'
                  ' (e.g. 410.* to select all 410xxx DSIDs, or'
                  ' ^(?!410) to veto them). An empty list means no'
                  ' DSID restriction.'))
        self.addOption('propertyOverrides', {}, type=None,
            info=('EXPERT USE ONLY: A dictionary of properties to'
                  ' override at the end of configuration. This should'
                  ' take the form'
                  ' {"algName.toolName.propertyName": value, ...},'
                  ' without any automatically applied postfixes for'
                  ' the algorithm name. THIS IS MEANT TO BE EXPERT'
                  ' USAGE ONLY. Properties that need to be set by'
                  ' the user should be declared as options on the'
                  ' block itself. EXPERT USE ONLY!'))
        # Increment the instance count for the current class
        cls = type(self)  # Get the actual class of the instance (also derived!)
        if cls not in ConfigBlock.instance_counts:
            ConfigBlock.instance_counts[cls] = 0
        # Note: we do need to check in the call stack that we are
        # in a real makeConfig situation, and not e.g. printAlgs
        stack = inspect.stack()
        for frame_info in stack:
            # Get the class name (if any) from the frame
            parent_cls = frame_info.frame.f_locals.get('self', None)
            if parent_cls is None or not isinstance(parent_cls, ConfigBlock):
                # If the frame does not belong to an instance of ConfigBlock, it's an external caller
                if frame_info.function == "makeConfig":
                    ConfigBlock.instance_counts[cls] += 1
                    break


    def setBlockName(self, name):
        """Set blockName"""
        self._blockName = name

    def getBlockName(self):
        """Get blockName"""
        return self._blockName

    def instanceName(self):
        """Get the name of the instance

        The name of the instance is used to distinguish between multiple
        instances of the same block. Most importantly, this will be
        appended to the names of all algorithms created by this block.
        This defaults to an empty string, but block implementations
        should override it with an appropriate name based on identifying
        options set on this instance. A typical example would be the
        name of the (main) container, plus potentially the selection or
        working point.

        Ideally all blocks should override this method, but for backward
        compatibility (28 Jul 25) it defaults to an empty string.
        """
        return ''

    def isUsedForConfig(self, config):
        """
        whether this block should be used for the given configuration

        This is used by `ConfigSequence` to determine whether this block
        should be included in the configuration.
        """
        if self.skipOnData and config.dataType() is DataType.Data:
            return False
        if self.skipOnMC and config.dataType() is DataType.MC:
            return False

        if self.skipOnData and config.dataType() is DataType.Data:
            return False
        if self.skipOnMC and config.dataType() is not DataType.Data:
            return False
        if not filter_dsids(self.onlyForDSIDs, config):
            return False
        return True

    def applyConfigOverrides(self, config):
        """
        Apply any configuration overrides specified in the block's
        `propertyOverrides` option. This is meant to be called at the
        end of the configuration process, after all algorithms have been
        created and configured.
        """
        for key, value in self.propertyOverrides.items():
            # Split the key into algorithm name, tool name, and property name
            parts = key.split('.')
            if len(parts) < 2:
                raise Exception(f"Invalid override key format: {key}")
            alg = config.getAlgorithm(parts[0])
            if alg is None:
                raise Exception(f"Algorithm {parts[0]} not found in config for override: {key}")
            for name in parts[1:-1]:
                # Navigate through tools if necessary
                if hasattr(alg, name):
                    alg = getattr(alg, name)
                else:
                    raise Exception(f"Tool {name} not found for override: {key}")
            # Set the property on the algorithm/tool. This is probably a
            # horrible hack, but `setattr` didn't work for me.
            alg.__setattr__(parts[-1], value)

    def addDependency(self, dependencyName, required=True):
        """
        Add a dependency for the block. Dependency is corresponds to the
        blockName of another block. If required is True, will throw an
        error if dependency is not present; otherwise will move this
        block after the required block. If required is False, will do
        nothing if required block is not present; otherwise, it will
        move block after required block.
        """
        if not self.hasDependencies():
            # add option to block ignore dependencies
            self.addOption('ignoreDependencies', [], type=list,
                           info='List of dependencies defined in the ConfigBlock to ignore.')
        self._dependencies.append(ConfigBlockDependency(dependencyName, required))

    def hasDependencies(self):
        """Return True if there is a dependency."""
        return bool(self._dependencies)

    def getDependencies(self):
        """Return the list of dependencies. """
        return self._dependencies

    def addOption (self, name, defaultValue, *,
            type, info='', noneAction='ignore', required=False) :
        """declare the given option on the configuration block

        This should only be called in the constructor of the
        configuration block.

        NOTE: The backend to option handling is slated to be replaced
        at some point.  This particular function should essentially
        stay the same, but some behavior may change.
        """
        if name in self._options :
            raise KeyError (f'duplicate option: {name}')
        if type not in [str, bool, int, float, list, None] :
            raise TypeError (f'unknown option type: {type}')
        noneActions = ['error', 'set', 'ignore']
        if noneAction not in noneActions :
            raise ValueError (f'invalid noneAction: {noneAction} [allowed values: {noneActions}]')
        setattr (self, name, defaultValue)
        self._options[name] = ConfigBlockOption(type=type, info=info,
            noneAction=noneAction, required=required, default=defaultValue)


    def setOptionValue (self, name, value) :
        """set the given option on the configuration block

        NOTE: The backend to option handling is slated to be replaced
        at some point.  This particular function should essentially
        stay the same, but some behavior may change.
        """

        if name not in self._options :
            raise KeyError (f'unknown option "{name}" in block "{self.__class__.__name__}"')
        noneAction = self._options[name].noneAction
        if value is not None or noneAction == 'set' :
            # check type if specified
            optType = self._options[name].type
            # convert int to float to prevent crash
            if optType is float and type(value) is int:
                value = float(value)
            if optType is not None and optType != type(value):
                raise ValueError(f'{name} for block {self.__class__.__name__} should '
                    f'be of type {optType} not {type(value)}')
            setattr (self, name, value)
        elif noneAction == 'ignore' :
            pass
        elif noneAction == 'error' :
            raise ValueError (f'passed None for setting option {name} with noneAction=error')


    def getOptionValue(self, name):
        """Returns config option value, if present; otherwise return None"""
        if name in self._options:
            return getattr(self, name)


    def getOptions(self):
        """Return a copy of the options associated with the block"""
        return self._options.copy()


    def printOptions(self, verbose=False, width=60, indent="    "):
        """
        Prints options and their values
        """
        def printWrap(text, width=60, indent="    "):
            wrapper = textwrap.TextWrapper(width=width, initial_indent=indent,
                subsequent_indent=indent)
            for line in wrapper.wrap(text=text):
                logCPAlgCfgBlock.info(line)

        for opt, vals in self.getOptions().items():
            if verbose:
                logCPAlgCfgBlock.info(indent + f"\033[4m{opt}\033[0m: {self.getOptionValue(opt)}")
                logCPAlgCfgBlock.info(indent*2 + f"\033[4mtype\033[0m: {vals.type}")
                logCPAlgCfgBlock.info(indent*2 + f"\033[4mdefault\033[0m: {vals.default}")
                logCPAlgCfgBlock.info(indent*2 + f"\033[4mrequired\033[0m: {vals.required}")
                logCPAlgCfgBlock.info(indent*2 + f"\033[4mnoneAction\033[0m: {vals.noneAction}")
                printWrap(f"\033[4minfo\033[0m: {vals.info}", indent=indent*2)
            else:
                logCPAlgCfgBlock.info(indent + f"{ opt}: {self.getOptionValue(opt)}")


    def hasOption (self, name) :
        """whether the configuration block has the given option

        WARNING: The backend to option handling is slated to be
        replaced at some point.  This particular function may change
        behavior, interface or be removed/replaced entirely.
        """
        return name in self._options


    def __eq__(self, blockName):
        """
        Implementation of == operator. Used for seaching configSeque.
        E.g. if blockName in configSeq:
        """
        return self._blockName == blockName


    def __str__(self):
        return self._blockName


    @classmethod
    def get_instance_count(cls):
        # Access the current count for this class
        return ConfigBlock.instance_counts.get(cls, 0)
