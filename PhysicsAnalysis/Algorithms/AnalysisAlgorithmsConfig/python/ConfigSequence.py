# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AnaAlgorithm.Logging import logging
logCPAlgCfgSeq = logging.getLogger('CPAlgCfgSeq')

from functools import wraps
from random import randrange

def groupBlocks(func):
    """
    Decorates a configSequence or function with 'seq' as a
    arguement.

    Sets groupName to the name of the decorated funtion  or
    calss plus and integer for each ConfigBlock in the configSequence.

    Blocks with the same groupName can be configured together.
    """
    @wraps(func)
    def wrapper(**kwargs):
        func(**kwargs)
        groupName = f"{func.__name__}_{randrange(10**8):08}"
        for block in kwargs['seq']:
            block.setOptionValue('groupName', groupName)
    return wrapper

class ConfigSequence:
    """a sequence of ConfigBlock objects

    This could in principle just be a simple python list, and maybe we
    change it to that at some point (10 Mar 22).  Having it as its own
    class allows to implement some helper functions.

    This implements an interface similar to ConfigBlock, but it
    doesn't derive from it, as ConfigBlock will likely gain
    functionality in the future that wouldn't work for a sequence (or
    wouldn't work in the same way).

    """

    def __init__ (self) :
        self._blocks = []


    def append (self, block) :
        """append a configuration block to the sequence"""
        self._blocks.append (block)


    def makeAlgs (self, config) :
        """call makeAlgs() on all blocks

        This will create the actual algorithm configurables based on
        how the blocks are configured right now.
        """
        for block in self._blocks:
            if not block.isUsedForConfig(config):
                continue
            config.setAlgPostfix(block.instanceName())
            block.makeAlgs (config)
        config.setAlgPostfix('')  # reset algPostfix after all blocks are configured


    def applyConfigOverrides(self, config):
        """
        Apply any properties that were set in the block's
        'propertyOverrides' option.
        """
        for block in self._blocks:
            if not block.isUsedForConfig(config):
                continue
            config.setAlgPostfix(block.instanceName())
            block.applyConfigOverrides(config)
        config.setAlgPostfix('')  # reset algPostfix after all blocks are configured

    def reorderAlgs(self):
        """
        Check for blocks with dependencies.

        If a block required another block that is not present, will
        throw an error; Otherwise, will move block immediately after
        required block. If dependency is not required, will move
        after other block, if it is present.

        Note: this implementation can only move blocks forward.
        """
        def moveBlock(blocks):
            for i, block in enumerate(blocks):
                # the 'ignoreDependencies' option is added with a dep.
                ignore = block.getOptionValue('ignoreDependencies')
                if block.hasDependencies():
                    depIdx = i
                    for dep in block.getDependencies():
                        if dep in ignore:
                            continue
                        # find dep with largest idx
                        if dep in blocks:
                            lastIdx = max(index for index,value in enumerate(blocks) if value == dep.blockName)
                            if lastIdx > depIdx:
                                depIdx = lastIdx
                        elif dep.required:
                            raise ValueError(f"{dep} block is required"
                                f" for {block} but was not found.")
                    # check to see if block is already infront of deps
                    if depIdx > i:
                        logCPAlgCfgSeq.info(f"Moving {block} after {blocks[depIdx]}")
                        # depIdx > i so after pop, depIdx -= 1 -> depIdx is after dep
                        blocks.insert(depIdx, blocks.pop(i))
                        return False
            # nothing to move
            return True
        MAXTRIES = 1000
        for _ in range(MAXTRIES):
            if moveBlock(self._blocks):
                # sorted
                break
        else:
            raise Exception("Could not order blocks based on dependencies"
                            f" in {MAXTRIES} moves.")


    def fullConfigure (self, config) :
        """do the full configuration on this sequence

        This sequence needs to be the only sequence, i.e. it needs to
        contain all the blocks that will be configured, as it will
        perform all configuration steps at once.
        """
        self.reorderAlgs()
        self.makeAlgs (config)
        config.nextPass ()
        self.makeAlgs (config)
        self.applyConfigOverrides(config)


    def setOptionValue (self, name, value, **kwargs) :
        """set the given option on the sequence

        The name should generally be of the form
        "groupName.optionName" to identify what group the option
        belongs to.

        For simplicity I also allow a ".optionName" here, which will
        then set the property in the last group added.  That makes it
        fairly straightforward to add new blocks, set options on them,
        and then move on to the next blocks.  Please note that this
        mechanism ought to be viewed as strictly as a temporary
        convenience, and this short cut may go away once better
        alternatives are available.

        WARNING: The backend to option handling is slated to be
        replaced at some point.  This particular function may change
        behavior, interface or be removed/replaced entirely.
        """
        names = name.split('.')
        # <optionName>
        optionName = names.pop(-1)
        # <groupName>.<optionName>, or
        # .<optionName> (backwards compatability)
        groupName = names.pop(0) if names else ''
        if names:
            raise ValueError(f'Option name can be either <groupName>.<optionName>'
                f' or <optionName> not {name}')
        blocks = self._blocks
        # check if last block added has an instance name
        if not groupName:
            groupName = blocks[-1].getOptionValue('groupName')
        if groupName:
            used = False
            # set optionName for all blocks with groupName
            for block in blocks:
                if ( block.getOptionValue('groupName') == groupName
                        and block.hasOption(optionName) ):
                    block.setOptionValue (optionName, value, **kwargs)
                    used = True
            if not used:
                raise ValueError(f'{optionName} not found in blocks with '
                    f'group name {groupName}')
        else:
            # set opyion for last added block
            blocks[-1].setOptionValue (optionName, value, **kwargs)


    def printOptions(self, verbose=False):
        """
        Prints options and their values for each config block in a config sequence
        """
        for config in self:
            logCPAlgCfgSeq.info(config.__class__.__name__)
            config.printOptions(verbose=verbose)


    def getOptions(self):
        """get information on options for last block in sequence"""
        # get groupName for last added block
        groupName = self._blocks[-1].getOptionValue('groupName')
        blocks = [self._blocks[-1]]
        # get all blocks with the same groupName
        if groupName:
            for block in self._blocks[:-1]:
                if block.getOptionValue('groupName') == groupName:
                    blocks.append(block)
        options = []
        # get options for all blocks with same groupName
        for block in blocks:
            for name, o in block.getOptions().items():
                val = getattr(block, name)
                valDefault = o.default
                valType = o.type
                valRequired = o.required
                noneAction = o.noneAction
                options.append({'name': name, 'defaultValue': valDefault,
                    'type': valType, 'required': valRequired,
                    'noneAction': noneAction, 'value': val})
        return options


    def setOptions(self, options):
        """Set options for a ConfigBlock"""
        algOptions = self.getOptions()
        for opt in algOptions:
            name = opt['name']
            if name in options:
                self.setOptionValue (f'.{name}', options[name])
                logCPAlgCfgSeq.info(f"    {name}: {options[name]}")
            else:
                if opt['required']:
                    raise ValueError(f'{name} is required but not included in config')
                # add default used to config
                defaultVal = opt['value'] if opt['value'] else opt['defaultValue']
                # do not overwright groupName unless set by user
                if name != 'groupName':
                    options[name] = defaultVal
                logCPAlgCfgSeq.info(f"    {name}: {defaultVal}")
        return algOptions


    def groupBlocks(self, groupName=''):
        """
        Assigns all blocks in configSequence groupName. If no name is
        provided, the name is set to group_ plus an integer.

        Blocks with the same groupName can be configured together.
        """
        if not groupName:
            groupName = f"group_{randrange(10**8):08}"
        for block in self._blocks:
            block.setOptionValue('groupName', groupName)


    def __iadd__( self, sequence, index = None ):
        """Add another sequence to this one

        This function is used to add another sequence to this sequence
        using the '+=' operator.
        """
        # Check that the received object is of the right type:
        if not isinstance( sequence, ConfigSequence ):
            raise TypeError( 'The received object is not of type ConfigSequence' )

        for block in sequence._blocks :
            self._blocks.append (block)

        # Return the modified object:
        return self


    def __iter__( self ):
        """Create an iterator over all the configurations in this sequence

        This is to allow for a Python-like iteration over all
        configuration blocks that are part of the sequence.
        """
        # Create the iterator to process the internal list of algorithms:
        return self._blocks.__iter__()
