# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from collections import defaultdict
from copy import copy, deepcopy
from difflib import get_close_matches
from enum import EnumMeta
import glob
import importlib
from AthenaCommon.Logging import logging
from PyUtils.moduleExists import moduleExists
from PyUtils.Decorators import deprecate

_msg = logging.getLogger('AthConfigFlags')

def isGaudiEnv():
    """Return whether or not this is a gaudi-based (athena) environment"""

    return moduleExists('Gaudi')

class CfgFlag(object):
    """The base flag object.

    A flag can be set to either a fixed value or a callable, which computes
    the value based on other flags.
    """

    __slots__ = ['_value', '_setDef', '_type', '_help']

    _compatibleTypes = {
        (int, float),   # int can be assigned to float flag
    }

    def __init__(self, default, type=None, help=None):
        """Initialise the flag with the default value.

        Optionally set the type of the flag value and the help string.
        """
        if default is None:
            raise ValueError("Default value of a flag must not be None")
        self._type = type
        self._help = help
        self.set(default)
        return

    def set(self, value):
        """Set the value of the flag.

        Can be a constant value or a callable.
        """
        if callable(value):
            self._value=None
            self._setDef=value
        else:
            self._value=value
            self._setDef=None
            self._validateType(self._value)
        return

    def get(self, flagdict=None):
        """Get the value of the flag.

        If the currently set value is a callable, a dictionary of all available
        flags needs to be provided.
        """

        if self._value is not None:
            return deepcopy(self._value)

        # For cases where the value is intended to be None
        # i.e. _setDef applied this value, we should not progress
        if self._setDef is None:
            return None

        if not flagdict:
            raise RuntimeError("Flag is using a callable but all flags are not available.")

        # Have to call the method to obtain the default value, and then reuse it in all next accesses
        if flagdict.locked():
            # optimise future reads, drop possibility to update this flag ever
            self._value = self._setDef(flagdict)
            self._setDef = None
            value = self._value
        else:
            # use function for as long as the flags are not locked
            value = self._setDef(flagdict)

        self._validateType(value)
        return deepcopy(value)

    def __repr__(self):
        if self._value is not None:
            return repr(self._value)
        else:
            return "[function]"

    def _validateType(self, value):
        if (self._type is None or value is None or
            isinstance(value, self._type) or
            (type(value), self._type) in self._compatibleTypes):
            return
        # Type mismatch
        raise TypeError(f"Flag is of type '{self._type.__name__}', "
                        f"but value '{value}' of type '{type(value).__name__}' set.")


def _asdict(iterator):
    """Flags to dict converter

    Used by both FlagAddress and AthConfigFlags. The input must be an
    iterator over flags to be included in the dict.

    """
    outdict = {}
    for key, item in iterator:
        x = outdict
        subkeys = key.split('.')
        for subkey in subkeys[:-1]:
            x = x.setdefault(subkey,{})
        x[subkeys[-1]] = item
    return outdict

class FlagAddress(object):
    def __init__(self, flag, name):
        if isinstance(flag, AthConfigFlags):
            self._flags = flag
            self._name = name

        elif isinstance(flag, FlagAddress):
            self._flags = flag._flags
            self._name  = f"{flag._name}.{name}"

        else:
            raise TypeError(f"Cannot create FlagAddress for object {name} of type {type(flag)}")

        # Handle renames
        self._name = self._flags._renames.get(self._name, self._name)
        if self._name is None:
            raise AttributeError(f"Accessing category {name} has been blocked by cloneAndReplace")


    def __getattr__(self, name):
        return getattr(self._flags, self._name + "." + name)

    def __setattr__( self, name, value ):
        if name.startswith("_"):
            return object.__setattr__(self, name, value)
        merged = self._name + "." + name

        if not self._flags.hasFlag( merged ): # flag is misisng, try loading dynamic ones
            self._flags._loadDynaFlags( merged )

        if not self._flags.hasFlag( merged ):
            raise RuntimeError( "No such flag: {}  The name is likely incomplete.".format(merged) )
        return self._flags._set( merged, value )

    def __delattr__(self, name):
        del self[name]

    def __cmp__(self, other):
        raise RuntimeError( "No such flag: "+ self._name+".  The name is likely incomplete." )
    __eq__ = __cmp__
    __ne__ = __cmp__
    __lt__ = __cmp__
    __le__ = __cmp__
    __gt__ = __cmp__
    __ge__ = __cmp__

    def __bool__(self):
        raise RuntimeError( "No such flag: "+ self._name+".  The name is likely incomplete." )

    def __getitem__(self, name):
        return getattr(self, name)

    def __setitem__(self, name, value):
        setattr(self, name, value)

    def __delitem__(self, name):
        merged = self._name + "." + name
        del self._flags[merged]

    def __iter__(self):
        self._flags.loadAllDynamicFlags()
        rmap = self._flags._renamed_map()
        used = set()
        for flag in self._flags._flagdict.keys():
            if flag.startswith(self._name.rstrip('.') + '.'):
                for newflag in rmap[flag]:
                    ntrim = len(self._name) + 1
                    n_dots_in = flag[:ntrim].count('.')
                    remaining = newflag.split('.')[n_dots_in]
                    if remaining not in used:
                        yield remaining
                        used.add(remaining)

    def _subflag_itr(self):
        """Subflag iterator specialized for this address

        """
        self._flags.loadAllDynamicFlags()
        address = self._name
        rename = self._flags._renamed_map()
        for key in self._flags._flagdict.keys():
            if key.startswith(address.rstrip('.') + '.'):
                ntrim = len(address) + 1
                remaining = key[ntrim:]
                for r in rename[key]:
                    yield r, getattr(self, remaining)

    def asdict(self):
        """Convert to a python dictionary

        Recursively convert this flag and all subflags into a
        structure of nested dictionaries. All dynamic flags are
        resolved in the process.

        The resulting data structure should be easy to serialize as
        json or yaml.

        """
        d = _asdict(self._subflag_itr())
        for k in self._name.split('.'):
            d = d[k]
        return d


class AthConfigFlags(object):

    # A list of all flags instances for which we've returned a hash.
    # We can't allow them to be deleted; otherwise, we might get new
    # flags object with the same hash.
    _hashedFlags = []

    def __init__(self):
        self._flagdict=dict()
        self._locked=False
        self._dynaflags = dict()
        self._loaded    = set()      # dynamic dlags that were loaded
        self._categoryCache = set()  # cache for already found categories
        self._hash = None
        self._parser = None
        self._args = None # user args from parser
        self._renames = {}

    def athHash(self):
        if self._locked is False:
            raise RuntimeError("Cannot calculate hash of unlocked flag container")
        elif self._hash is None:
            self._hash = self._calculateHash()
        return self._hash

    def __hash__(self):
        raise DeprecationWarning("__hash__ method in AthConfigFlags is deprecated. Probably called from function decorator, use AccumulatorCache decorator instead.")

    def _calculateHash(self):
        # Once we've hashed a flags instance, we need to be sure that
        # it never goes away.  Otherwise, since we base the hash
        # on just the id of the dictionary, if a flags object is deleted
        # and a new one created, the hash of the new one could match the
        # hash of the old, even if contents are different.
        # See ATLASRECTS-8070.
        AthConfigFlags._hashedFlags.append (self)
        return hash( (frozenset({k: v for k, v in self._renames.items() if k != v}), id(self._flagdict)) )

    def __getattr__(self, name):
        # Avoid infinite recursion looking up our own attributes
        _flagdict = object.__getattribute__(self, "_flagdict")

        # First try to get an already loaded flag or category
        if name in _flagdict:
            return self._get(name)

        if self.hasCategory(name):
            return FlagAddress(self, name)

        # Reaching here means that we may need to load a dynamic flag
        self._loadDynaFlags(name)

        # Try again
        if name in _flagdict:
            return self._get(name)

        if self.hasCategory(name):
            return FlagAddress(self, name)

        # Reaching here means that it truly isn't something we know about
        raise AttributeError(f"No such flag: {name}")

    def __setattr__(self, name, value):
        if name.startswith("_"):
            return object.__setattr__(self, name, value)

        if name in self._flagdict:
            return self._set(name, value)
        raise RuntimeError( "No such flag: "+ name+". The name is likely incomplete." )

    def __delattr__(self, name):
        del self[name]

    def __getitem__(self, name):
        return getattr(self, name)

    def __setitem__(self, name, value):
        setattr(self, name, value)

    def __delitem__(self, name):
        self._tryModify()
        self.loadAllDynamicFlags()
        for key in list(self._flagdict):
            if key.startswith(name):
                del self._flagdict[key]
        self._categoryCache.clear()

    def __iter__(self):
        self.loadAllDynamicFlags()
        rmap = self._renamed_map()
        used = set()
        for flag in self._flagdict:
            for r in rmap[flag]:
                first = r.split('.',1)[0]
                if first not in used:
                    yield first
                    used.add(first)

    def asdict(self):
        """Convert to a python dictionary

        This is identical to the `asdict` in FlagAddress, but for all
        the flags.

        """
        return _asdict(self._subflag_itr())


    def _renamed_map(self):
        """mapping from the old names to the new names

        This is the inverse of _renamed, which maps new names to old
        names
        
        Returns a list of the new names corresponding to the old names
        (since cloneAndReplace may or may not disable access to the old name,
        it is possible that an old name renames to multiple new names)
        """        
        revmap = defaultdict(list)
        
        for new, old in self._renames.items():
            if old is not None:
                revmap[old].append(new)
        
        def rename(key):
            for old, newlist in revmap.items():
                if key.startswith(old + '.'):
                    stem = key.removeprefix(old)
                    return [ f'{new}{stem}' if new else '' for new in newlist ]
            return [ key ]
        
        return {x:rename(x) for x in self._flagdict.keys()}

    def _subflag_itr(self):
        """Subflag iterator for all flags

        This is used by the asdict() function.
        """
        self.loadAllDynamicFlags()

        for old, newlist in self._renamed_map().items():
            for new in newlist:
                # Lots of modules are missing in analysis releases. I
                # tried to prevent imports using the _addFlagsCategory
                # function which checks if some module exists, but this
                # turned in to quite a rabbit hole. Catching and ignoring
                # the missing module exception seems to work, even if it's
                # not pretty.
                try:
                    yield new, getattr(self, old)
                except ModuleNotFoundError as err:
                    _msg.debug(f'missing module: {err}')
                    pass

    def addFlag(self, name, setDef, type=None, help=None):
        self._tryModify()
        if name in self._flagdict:
            raise KeyError("Duplicated flag name: {}".format( name ))
        self._flagdict[name]=CfgFlag(setDef, type, help)
        return

    def addFlagsCategory(self, path, generator, prefix=False):
        """
        The path is the beginning of the flag name (e.g. "X" for flags generated with name "X.*").
        The generator is a function that returns a flags container, the flags have to start with the same path.
        When the prefix is True the flags created by the generator are prefixed by "path".

        Supported calls are then:
         addFlagsCategory("A", g) - where g is function creating flags  is f.addFlag("A.x", someValue)
         addFlagsCategory("A", g, True) - when flags are defined in g like this: f.addFalg("x", somevalue),
        The latter option allows to share one generator among flags that are later loaded in different paths.
        """
        self._tryModify()
        _msg.debug("Adding flag category %s", path)
        self._dynaflags[path] = (generator, prefix)

    def needFlagsCategory(self, name):
        """ public interface for _loadDynaFlags """
        self._loadDynaFlags( name )

    def _loadDynaFlags(self, name):
        """
        loads the flags of the form "A.B.C" first attempting the path "A" then "A.B" and then "A.B.C"
        """

        def __load_impl( flagBaseName ):
            if flagBaseName in self._loaded:
                _msg.debug("Flags %s already loaded",flagBaseName  )
                return
            if flagBaseName in self._dynaflags:
                _msg.debug("Dynamically loading the flags under %s", flagBaseName )
                # Retain locked status and hash
                isLocked = self._locked
                myHash = self._hash
                self._locked = False
                generator, prefix = self._dynaflags[flagBaseName]
                self.join( generator(), flagBaseName if prefix else "" )
                self._locked = isLocked
                self._hash = myHash
                del self._dynaflags[flagBaseName]
                self._loaded.add(flagBaseName)

        pathfrags = name.split('.')
        for maxf in range(1, len(pathfrags)+1):
            __load_impl( '.'.join(pathfrags[:maxf]) )

    def loadAllDynamicFlags(self):
        """Force load all the dynamic flags """
        while len(self._dynaflags) != 0:
            # Need to convert to a list since _loadDynaFlags may change the dict.
            for prefix in list(self._dynaflags.keys()):
                self._loadDynaFlags( prefix )

    def hasCategory(self, name):
        # We cache successfully found categories
        if name in self._categoryCache:
            return True

        if (re_name := self._renames.get(name)) is not None and re_name != name:
            return self.hasCategory(re_name)
        
        # If not found do search through all keys.
        # TODO: could be improved by using a trie for _flagdict
        for f in self._flagdict.keys():
            if f.startswith(name+'.'):
                self._categoryCache.add(name)
                return True
        for c in self._dynaflags.keys():
            if c.startswith(name):
                self._categoryCache.add(name)
                return True

        return False

    def hasFlag(self, name):
        return name in [y for x in self._renamed_map().values() for y in x]

    def _set(self,name,value):
        self._tryModify()
        try:
            self._flagdict[name].set(value)
        except KeyError:
            closestMatch = get_close_matches(name,self._flagdict.keys(),1)
            raise KeyError(f"No flag with name '{name}' found" +
                           (f". Did you mean '{closestMatch[0]}'?" if closestMatch else ""))

    def _get(self,name):
        try:
            return self._flagdict[name].get(self)
        except KeyError:
            closestMatch = get_close_matches(name,self._flagdict.keys(),1)
            raise KeyError(f"No flag with name '{name}' found" +
                           (f". Did you mean '{closestMatch[0]}'?" if closestMatch else ""))

    @deprecate("Use '[...]' rather than '(...)' to access flags", print_context=True)
    def __call__(self,name):
        return self._get(name)

    def lock(self):
        if not self._locked:
            # before locking, parse args if a parser was defined
            if self._args is None and self._parser is not None: self.fillFromArgs()
            self._locked = True
        return

    def locked(self):
        return self._locked

    def _tryModify(self):
        if self._locked:
            raise RuntimeError("Attempt to modify locked flag container")
        else:
            # if unlocked then invalidate hash
            self._hash = None

    def clone(self):
        """Return an unlocked copy of self (dynamic flags are not loaded)"""
        cln = AthConfigFlags()
        cln._flagdict = deepcopy(self._flagdict)
        cln._dynaflags = copy(self._dynaflags)
        cln._renames = deepcopy(self._renames)
        return cln


    def cloneAndReplace(self,subsetToReplace,replacementSubset, keepOriginal=False):
        """
        This is to replace subsets of configuration flags like

        Example:
        newflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon')
        """

        _msg.debug("cloning flags and replacing %s by %s", subsetToReplace, replacementSubset)

        self._loadDynaFlags( subsetToReplace )
        self._loadDynaFlags( replacementSubset )

        subsetToReplace = subsetToReplace.strip(".")
        replacementSubset = replacementSubset.strip(".")

        #Sanity check: Don't replace a by a
        if (subsetToReplace == replacementSubset):
            raise RuntimeError(f'Can not replace flags {subsetToReplace} with themselves')

        # protect against subsequent remaps within remaps: clone = flags.cloneAndReplace('Y', 'X').cloneAndReplace('X.b', 'X.a')
        for alias,src in self._renames.items():
            if src is None: continue
            if src+"." in subsetToReplace:
                raise RuntimeError(f'Can not replace flags {subsetToReplace} by {replacementSubset} because of already present replacement of {alias} by {src}')


        newFlags = copy(self) # shallow copy
        newFlags._renames = deepcopy(self._renames) #maintains renames

        if replacementSubset in newFlags._renames: #and newFlags._renames[replacementSubset]:
            newFlags._renames[subsetToReplace] = newFlags._renames[replacementSubset]
        else:
            newFlags._renames[subsetToReplace] = replacementSubset
        
        if not keepOriginal:
            if replacementSubset not in newFlags._renames or newFlags._renames[replacementSubset] == replacementSubset:
                newFlags._renames[replacementSubset] = None # block access to original flags
            else:
                del newFlags._renames[replacementSubset]
                #If replacementSubset was a "pure renaming" of another set of flags,
                #the original set of flags gets propagated down to its potential further renamings:
                #no need to worry about maintaining the intermediate steps in the renaming.
        else:
            if replacementSubset not in newFlags._renames:
                newFlags._renames[replacementSubset] = replacementSubset
                #For _renamed_map to know that these flags still work.
        newFlags._hash = None
        return newFlags


    def join(self, other, prefix=''):
        """
        Merges two flag containers
        When the prefix is passed each flag from the "other" is prefixed by "prefix."
        """
        self._tryModify()

        for (name,flag) in other._flagdict.items():
            fullName = prefix+"."+name if prefix != "" else name
            if fullName in self._flagdict:
                raise KeyError("Duplicated flag name: {}".format( fullName ) )
            self._flagdict[fullName]=flag

        for (name,loader) in other._dynaflags.items():
            fullName = prefix+"."+name if prefix != "" else name
            if fullName in self._dynaflags:
                raise KeyError("Duplicated dynamic flags name: {}".format( fullName ) )
            _msg.debug("Joining dynamic flags with %s", fullName)
            self._dynaflags[fullName] = loader
        return

    def dump(self, pattern=".*", evaluate=False, formatStr="{:40} : {}", maxLength=None):
        import re
        compiled = re.compile(pattern)
        def truncate(s): return s[:maxLength] + ("..." if maxLength and len(s)>maxLength else "")
        reverse_renames = {value: key for key, value in self._renames.items() if value is not None} # new name to old
        for name in sorted(self._flagdict):
            renamed = name
            if any([name.startswith(r) for r in reverse_renames.keys() if r is not None]):
                for oldprefix, newprefix in reverse_renames.items():
                    if name.startswith(oldprefix):
                        renamed = name.replace(oldprefix, newprefix)
                        break
            if compiled.match(renamed):
                if evaluate:
                    try:
                        rep = repr(self._flagdict[name] )
                        val = repr(self._flagdict[name].get(self))
                        if val != rep:
                            print(formatStr.format(renamed,truncate("{} {}".format( val, rep )) ))
                        else:
                            print(formatStr.format(renamed, truncate("{}".format(val)) ) )
                    except Exception as e:
                        print(formatStr.format(renamed, truncate("Exception: {}".format( e )) ))
                else:
                    print(formatStr.format( renamed, truncate("{}".format(repr(self._flagdict[name] ) )) ))

        if len(self._dynaflags) != 0 and any([compiled.match(x) for x in self._dynaflags.keys()]):
            print("Flag categories that can be loaded dynamically")
            print("{:25} : {:>30} : {}".format( "Category","Generator name", "Defined in" ) )
            for name,gen_and_prefix in sorted(self._dynaflags.items()):
                if compiled.match(name):
                    print("{:25} : {:>30} : {}".format( name, gen_and_prefix[0].__name__, '/'.join(gen_and_prefix[0].__code__.co_filename.split('/')[-2:]) ) )
        if len(self._renames):
            print("Flag categories that are redirected by the cloneAndReplace")
            for alias,src in self._renames.items():
                print("{:30} points to {:>30} ".format( alias, src  if src else "nothing") )
            

    def initAll(self):
        """
        Mostly a self-test method
        """
        for n,f in list(self._flagdict.items()):
            f.get(self)
        return


    def getArgumentParser(self, **kwargs):
        """
        Scripts calling AthConfigFlags.fillFromArgs can extend this parser, and pass their version to fillFromArgs
        """
        import argparse
        from AthenaCommon.AthOptionsParser import getArgumentParser
        parser = getArgumentParser(**kwargs)
        parser.add_argument("---",dest="terminator",action='store_true', help=argparse.SUPPRESS) # special hidden option required to convert option terminator -- for --help calls

        return parser

    def parser(self):
        if self._parser is None: self._parser = self.getArgumentParser()
        return self._parser

    def args(self):
        return self._args


    def fillFromString(self, flag_string):
        """Fill the flags from a string of type key=value"""
        import ast

        try:
            key, value = flag_string.split("=")
        except ValueError:
            raise ValueError(f"Cannot interpret argument {flag_string}, expected a key=value format")

        key = key.strip()
        value = value.strip()

        # also allow key+=value to append
        oper = "="
        if (key[-1]=="+"):
            oper = "+="
            key = key[:-1]

        if not self.hasFlag(key):
            self._loadDynaFlags( '.'.join(key.split('.')[:-1]) ) # for a flag A.B.C dymanic flags from category A.B
        if not self.hasFlag(key):
            raise KeyError(f"{key} is not a known configuration flag")

        flag_type = self._flagdict[key]._type
        if flag_type is None:
            # Regular flag
            try:
                ast.literal_eval(value)
            except Exception:  # Can't determine type, assume we got an un-quoted string
                value=f"\"{value}\""

        elif isinstance(flag_type, EnumMeta):
            # Flag is an enum, so we need to import the module containing the enum
            ENUM = importlib.import_module(flag_type.__module__)  # noqa: F841 (used in exec)
            value=f"ENUM.{value}"

        # Set the value (this also does the type checking if needed)
        exec(f"self.{key}{oper}{value}")


    # parser argument must be an ArgumentParser returned from getArgumentParser()
    def fillFromArgs(self, listOfArgs=None, parser=None, return_unknown=False):
        """
        Used to set flags from command-line parameters, like flags.fillFromArgs(sys.argv[1:])

        if return_unknown=False, returns: args 
                       otherwise returns: args, uknown_args
             where unknown_args is the list of arguments that did not correspond to one of the flags 
        """
        import sys

        self._tryModify()

        if parser is None:
            parser = self.parser()
        self._parser = parser # set our parser to given one
        argList = listOfArgs or sys.argv[1:]
        do_help = False
        # We will now do a pre-parse of the command line arguments to propagate these to the flags
        # the reason for this is so that we can use the help messaging to display the values of all
        # flags as they would be *after* any parsing takes place. This is nice to see e.g. the value
        # that any derived flag (functional flag) will take after, say, the filesInput are set
        import argparse
        unrequiredActions = []
        if "-h" in argList or "--help" in argList:
            do_help = True
            if "-h" in argList: argList.remove("-h")
            if "--help" in argList: argList.remove("--help")
            # need to unrequire any required arguments in order to do a "pre parse"
            for a in parser._actions:
                if a.required:
                    unrequiredActions.append(a)
                    a.required = False
        (args,leftover)=parser.parse_known_args(argList)
        for a in unrequiredActions: a.required=True

        # remove the leftovers from the argList ... for later use in the do_help
        argList = [a for a in argList if a not in leftover]

        # First, handle athena.py-like arguments (if available in parser):
        def arg_set(dest):
            """Check if dest is available in parser and has been set"""
            return vars(args).get(dest, None) is not None

        if arg_set('debug'):
            self.Exec.DebugStage=args.debug

        if arg_set('evtMax'):
            self.Exec.MaxEvents=args.evtMax

        if arg_set('interactive'):
            self.Exec.Interactive=args.interactive

        if arg_set('skipEvents'):
            self.Exec.SkipEvents=args.skipEvents

        if arg_set('filesInput'):
            self.Input.Files = [] # remove generic
            for f in args.filesInput.split(","):
                found = glob.glob(f)
                # if not found, add string directly
                self.Input.Files += found if found else [f]

        if "-l" in argList or "--loglevel" in argList: # different check b.c. has a default value so will always be in args
            from AthenaCommon import Constants
            self.Exec.OutputLevel = getattr(Constants, args.loglevel)

        if arg_set('config_only') and args.config_only is not False:
            from os import environ
            environ["PICKLECAFILE"] = "" if args.config_only is True else args.config_only

        if arg_set('threads'):
            self.Concurrency.NumThreads = args.threads
            #Work-around a possible inconsistency of NumThreads and NumConcurrentEvents that may
            #occur when these values are set by the transforms and overwritten by --athenaopts ..
            #See also ATEAM-907
            if args.concurrent_events is None and self.Concurrency.NumConcurrentEvents==0:
                self.Concurrency.NumConcurrentEvents = args.threads

        if arg_set('concurrent_events'):
            self.Concurrency.NumConcurrentEvents = args.concurrent_events

        if arg_set('nprocs'):
            self.Concurrency.NumProcs = args.nprocs

        if arg_set('perfmon'):
            from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
            setPerfmonFlagsFromRunArgs(self, args)

        if arg_set('mtes'):
            self.Exec.MTEventService = args.mtes

        if arg_set('mtes_channel'):
            self.Exec.MTEventServiceChannel = args.mtes_channel

        if arg_set('profile_python'):
            from AthenaCommon.Debugging import dumpPythonProfile
            import atexit, cProfile, functools
            cProfile._athena_python_profiler = cProfile.Profile()
            cProfile._athena_python_profiler.enable()

            # Save stats to file at exit
            atexit.register(functools.partial(dumpPythonProfile, args.profile_python))


        # All remaining arguments are assumed to be key=value pairs to set arbitrary flags:
        unknown_args = []
        for arg in leftover:
            if arg=='--':
                argList += ["---"]
                continue # allows for multi-value arguments to be terminated by a " -- "
            if do_help and '=' not in arg:
                argList += arg.split(".") # put arg back back for help (but split by sub-categories)
                continue
            try:
                self.fillFromString(arg)
            except KeyError as e:
                if return_unknown:
                    unknown_args += [arg]
                else:
                    raise e

        if do_help:
            if parser.epilog is None: parser.epilog=""
            parser.epilog += "  Note: Specify additional flags in form <flagName>=<value>."
            subparsers = {"":[parser,parser.add_subparsers(help=argparse.SUPPRESS)]} # first is category's parser, second is subparsers (effectively the category's subcategories)
            # silence logging while evaluating flags
            logging.root.setLevel(logging.ERROR)
            def getParser(category): # get parser for a given category
                if category not in subparsers.keys():
                    cat1,cat2 = category.rsplit(".",1) if "." in category else ("",category)
                    p,subp = getParser(cat1)
                    if subp.help==argparse.SUPPRESS:
                        subp.help = "Flag subcategories:"
                    newp = subp.add_parser(cat2,help="{} flags".format(category),
                                           formatter_class = argparse.ArgumentDefaultsHelpFormatter,usage=argparse.SUPPRESS)
                    newp._positionals.title = "flags"
                    subparsers[category] = [newp,newp.add_subparsers(help=argparse.SUPPRESS)]
                return subparsers[category]
            self.loadAllDynamicFlags()
            for name in sorted(self._flagdict):
                category,flagName = name.rsplit(".",1) if "." in name else ("",name)
                flag = self._flagdict[name]
                try:
                    val = repr(flag.get(self))
                except Exception:
                    val = None
                if flag._help != argparse.SUPPRESS:
                    helptext = ""
                    if flag._help is not None:
                        helptext = f": {flag._help}"
                    if flag._type is not None:
                        helptext += f' [type: {flag._type.__name__}]'
                    if val is not None and helptext == "":
                        helptext = ": " # ensures default values are displayed even if there's no help text
                    getParser(category)[0].add_argument(name, nargs='?', default=val, help=helptext)

            parser._positionals.title = 'flags and positional arguments'
            parser.parse_known_args(argList + ["--help"])

        self._args = args

        if return_unknown:
            return args,unknown_args
        else:
            return args



