# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

class ConfigBlockOption:
    """the information for a single option on a configuration block"""

    def __init__ (self, type=None, info='', noneAction='ignore', required=False) :
        self.type = type
        self.info = info
        self.required = required
        self.noneAction = noneAction



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
        return f'ConfigBlcokDependency(blockName="{self.blockName}", required={self.required})'


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

    The child class needs to implement two methods,
    `collectReferences` and `makeAlgs` which are each given a single
    `ConfigAccumulator` type argument.  The first is for the first
    configuration step, and should only collect references to the
    containers to be used.  The second is for the second configuration
    step, and should create the actual algorithms.

    """

    def __init__ (self) :
        self._blockName = ''
        self._dependencies = []
        self._options = {}
        # used with block configuration to set arbitrary option
        self.addOption('groupName', '', type=str,
            info=('Used to specify this block when setting an'
                ' option at an arbitrary location.'))


    def setBlockName(self, name):
        """Set blockName"""
        self._blockName = name

    def getBlockName(self, name):
        """Get blockName"""
        return self._blockName

    def addDependency(self, dependencyName, required=True):
        """
        Add a dependency for the block. Dependency is corresponds to the
        blockName of another block. If requried is True, will throw an
        error if dependency is not present; otherwise will move this
        block after the required block. If required is False, will do
        nothing if required block is not present; otherwise, it will
        move block after required block.
        """
        self._dependencies.append(ConfigBlockDependency(dependencyName, required))
        # add option to block ignore dependencies
        self.addOption('ignoreDependencies', [], type=list,
            info='List of dependencies defined in the ConfigBlock to ignore.')

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
            noneAction=noneAction, required=required)


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
