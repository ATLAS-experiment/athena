# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

class ConfigBlockOption:
    """the information for a single option on a configuration block"""

    def __init__ (self, type, info, duplicateAction, required=False) :
        self.type = type
        self.info = info
        self.required = required
        self.duplicateAction = duplicateAction



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

    def __init__ (self, groupName = '') :
        self._groupName = groupName
        self._options = {}


    def groupName (self) :
        """the configuration group we belong to

        This is generally either 'ObjectName' or
        'ObjectName.SelectionName', and can be used to identify blocks
        on which to set options.  This name should not change after
        the block has been created, i.e. not depend on any options
        itself.

        WARNING: The backend to option handling is slated to be
        replaced at some point.  This particular function may change
        behavior, interface or be removed/replaced entirely.
        """
        return self._groupName


    def addOption (self, name, defaultValue,
                   *, type, info='', duplicateAction='set', required=False) :
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
        if duplicateAction not in ['skip', 'set', 'error'] :
            raise ValueError (f'unknown duplicateAction: {duplicateAction}')
        setattr (self, name, defaultValue)
        self._options[name] = ConfigBlockOption (type=type, info=info, duplicateAction=duplicateAction, required=required)


    def setOptionValue (self, name, value,
                        *, noneAction='error', isDuplicate=False) :
        """set the given option on the configuration block

        NOTE: The backend to option handling is slated to be replaced
        at some point.  This particular function should essentially
        stay the same, but some behavior may change.
        """

        noneActions = ['error', 'set', 'ignore']
        if noneAction not in noneActions :
            raise ValueError (f'invalid noneAction: {noneAction} [allowed values: {noneActions}]')

        if name not in self._options :
            raise KeyError (f'unknown option "{name}" in block "{self.__class__.__name__}"')
        option = self._options[name]

        if isDuplicate :
            if option.duplicateAction == 'set' :
                pass
            elif option.duplicateAction == 'skip' :
                return
            elif option.duplicateAction == 'error' :
                raise Exception (f"can't have two options with the same name: {self._groupName}.{name}")

        if value is not None or noneAction == 'set' :
            setattr (self, name, value)
        elif noneAction == 'ignore' :
            pass
        elif noneAction == 'error' :
            raise ValueError (f'passed None for setting option {name} with noneAction=error')
        else :
            raise Exception ('should not get here')


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
