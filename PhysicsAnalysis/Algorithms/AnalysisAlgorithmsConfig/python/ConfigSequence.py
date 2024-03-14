# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

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
            block.makeAlgs (config)


    def fullConfigure (self, config) :
        """do the full configuration on this sequence

        This sequence needs to be the only sequence, i.e. it needs to
        contain all the blocks that will be configured, as it will
        perform all configuration steps at once.
        """
        self.makeAlgs (config)
        config.nextPass ()
        self.makeAlgs (config)


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


    def printOptions(self):
        """
        Prints options and their values for each config block in a config sequence
        """
        for config in self:
            print(config)
            for opt in config.getOptions():
                print(f"    {opt}: {config.getOptionValue(opt)}")


    def getOptions(self):
        """get information on options for last block in sequence"""
        options = []
        for name, o in self._blocks[-1].getOptions().items():
            val = getattr(self._blocks[-1], name)
            valType = o.type
            valRequired = o.required 
            options.append({'name':name, 'defaultValue':val, 'type': valType, 
                'required': valRequired})
        return options

    
    def setOptions(self, options):
        """Set options for a ConfigBlock"""
        algOptions = self.getOptions()
        for opt in algOptions:
            name = opt['name']
            valType = opt['type']
            valDefault = opt['defaultValue']
            valRequired = opt['required']

            if valRequired and name not in options:
                raise ValueError(f'{name} is required but not included in config')

            if name in options:
                opt_type = type(options[name])
                # does not check type if expected type is None
                if valType is not None and opt_type != valType:
                    raise ValueError(f'{name} should be of type {valType} not {opt_type}')
                self.setOptionValue (f'.{name}', options[name])
                print(f"    {name}: {options[name]}")
            else:
                # add default used to config
                options[name] = valDefault
                print(f"    {name}: {valDefault}")
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
