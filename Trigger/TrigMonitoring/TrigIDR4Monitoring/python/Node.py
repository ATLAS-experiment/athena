## noqa: ATL902
##
##    @file  Node.py
##
##           Node class to represent flags as an actual tree.
##           When requesting a feature (node or flag) from a node, 
##           the precendence is:
##
##            1. Node or flag from this current local node
##            2. Dynamically loaded node or flag on this local node
##            3. Node or flag on the top level node 
##            4. Dynamically loaded node or flag on the top level node
##
##           because of this, a local node with the same name as a
##           top level node will always mask the top level node, so if you 
##           have a local node and you want to explicitly access that top 
##           level node, then use the idiom
##
##              topnode(flags).MyNode
##
##           or, should you have a lot of such nodes
##
##              topflags = topnode(Mode)
##              topflasgs.MyNode
##
##           If you need to configure a sub-algorithm where that algorithm
##           configuration lives outside the domain of the current node, then
##           you can do things like
##
##              # on node flags.SomeDomain, 
##              AlgConfig( flags.TopDomain.NewDomain ) 
##
##   @author  sutt
##   @date    Tue 11 Aug 2026 19:04:50 BST
##
##  $Id: Node.py, v0.0   Tue 11 Aug 2026 19:04:50 BST  sutt $
##
## Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
##

import warnings

# locked state, allows transitions, locked forever,
# means it can not later be unlocked
# dynamic flag loading is possibly even if locked forever
# since existing nodes do not need to be unlocked

_UNLOCKED = 0
_LOCKED   = 1
_LOCKED_FOREVER = 2

class Node:
    """
    Basic node class, so we can get all the egregious flags from a
    flags object, and use, and pass round, the individual nodes, as
    actual nodes without all the nonsense limitations of using the
    flags directly
    """   
    
    def __init__(self, name="root", flags=None, root=None ):
        object.__setattr__(self, "_lockstate", _UNLOCKED)
        object.__setattr__(self, "_name", name)
        object.__setattr__(self, "_children", {})
        object.__setattr__(self, "_flags", flags)
        object.__setattr__(self, "_root", root)

    def __setattr__(self, name, value):
        self._fail_if_locked()
        if name in self._children : raise AttributeError(f"node {name} already exists")  
        current = self.__dict__.get(name)
        if type(current).__name__ == "CfgFlag":
            current.set(value)
        else:
            object.__setattr__(self, name, value) 
            
    def __getattribute__(self, name):
        value = object.__getattribute__(self, name)        
        if type(value).__name__ == "CfgFlag": 
            flags = object.__getattribute__(self, "_flags")
            return value.get(flags)
        return value

    def __getattr__(self, name):
        """ will attempt a dynamic load from flags if need be """
        if name in self._children : return self._children[name]
        if self._root :
            if self._root._loadflags( f"{self.name()}.{name}" ) :
                if name in self._children : return self._children[name]
            if name in self._root._children : return self._root._children[name]
            if self._root._loadflags( name ) :
                if name in self._root._children : return self._root._children[name]
        else:
            if self._loadflags( name ) :
                if name in self._children : return self._children[name]
                
        raise AttributeError(f"no attribute {name}")

    def __contains__(self, name):
        return self.has(name)

    def __copy__(self):
        return self.clone()

    def __delattr__(self, name):
        self.delete(name)

    def __delitem__(self, name):
        self.delete(name)

    def lock(self, lockstate=_LOCKED):
        for node in self.nodes() : node.lock(lockstate)
        object.__setattr__(self, "_lockstate", lockstate)
                           
    def unlock(self):
        self._fail_if_locked_forever()
        if self._lockstate == _LOCKED:
             object.__setattr__(self, "_lockstate", _UNLOCKED)
             for node in self.nodes(): node.unlock()
                     
    def lock_forever(self):
        self.lock(_LOCKED_FOREVER) 

    def islocked(self):
        return self._lockstate != _UNLOCKED

    def locked(self):
        return self.islocked()

    def _fail_if_locked_forever(self):
        if self._lockstate == _LOCKED_FOREVER : raise RuntimeError(f"Node {self._name} is permanently locked; ")

    def _fail_if_locked(self):
        if self._lockstate == _LOCKED : raise RuntimeError(f"Node {self._name} is locked; ")
        if self._lockstate == _LOCKED_FOREVER : raise RuntimeError(f"Node {self._name} is permanently locked; ")

    def _loadflags(self, name ) :
        """ load dynamic flags - can only load flags on the root node """

        if self._root : return self._root._loadflags( name )

        flags = self._flags
        if flags is None : raise AttributeError(f"no attribute {name}")
        lockstate = self._lockstate
        object.__setattr__(self, "_lockstate", _UNLOCKED)
        try:
            if name in flags._loaded    : return self._stragglers(flags)
            if name in flags._dynaflags : return self._addDynaFlags(name)
            for key in flags._dynaflags :
                if name in key : return self._addDynaFlags(key)
            return False
        finally:
            # ensure the lock state is restored if something goes wrong ...
            if lockstate != _UNLOCKED : self.lock(lockstate)

    def _addDynaFlags(self, name) :
        flags = self._flags
        self._flags._loadDynaFlags(name)
        return self._stragglers(flags)
        
    def _stragglers(self, flags, dbg="") :
        for key, val in self._flags._flagdict.items() :
            if self._hasFlagPath(key) : continue
            self._addFlagUnlock( key, val, flags )
        return True
        
    def _rootnode(self):
        if self._root : return self._root
        return self

    def _setrootnode(self, rootnode ):
        if self._root is rootnode : return 
        object.__setattr__(self, "_root", rootnode )
        for node in self.nodes() : node._setrootnode(rootnode)
            
    def name(self) :
        return self._name

    def add(self, attr_name, value):
        """ add an attribute to this node """
        setattr(self, attr_name, value)

    def _addFlagImpl(self, tag, value, flags=None, unlock=False):
        """ addFlag method like that in the egregious flags """
        node = self
        if flags is not None and self._flags is None : node._flags = flags
        names = tag.split(".")
        for name in names[:-1]:
            node = node._addNewNode(name, flags )
            if unlock : object.__setattr__( node, "_lockstate", _UNLOCKED )
        if flags is not None : node._flags = flags
        if names[-1] in node.__dict__ : return False
        # just use a value at the moment, can use CfgFlag(value, type) if we really have to(
        setattr(node, names[-1], value)
        return True

    def addFlag(self, tag, value, flags=None):
        if not self._addFlagImpl( tag, value, flags, False ):
            raise  KeyError(f"flag {tag} already exists")

    def _addFlagUnlock(self, tag, value, flags=None):
        self._addFlagImpl( tag, value, flags, True )

    def addNode( self, item, flags=None):
        if isinstance(item, Node) : return self._addNode(item)
        if isinstance(item, str)  : return self._addNewNode(item, flags)
        raise TypeError(f"Cannot construct node from type {type(item).__name__}, value {item}")
        
    def _addNewNode(self, name : str, flags=None):
        """ add a new child node """
        self._fail_if_locked()
        if name in self.__dict__: raise AttributeError(f"attribute {name} already exists")
        rootnode = self._rootnode()
        if name not in self._children :
            self._children[name] = ( node := Node(name, root=rootnode) )
            if flags is not None : node._flags = flags
        return self._children[name]

    def _addNode(self, node ):
        """ an en existing node as a child node """
        self._fail_if_locked()
        name = node.name()
        if name in self.__dict__: raise AttributeError(f"attribute {name} already exists")
        rootnode = self._rootnode()
        if name not in self._children :
            if node._root and rootnode is not node._root :
                raise RuntimeError(f"cannot add node {name} from a different tree")
            self._children[node.name()] = node
            if not node._root : node._setrootnode( rootnode )
        return  self._children[node.name()]

    def has(self, name ):
        return hasattr( self, name )

    def hasFlag(self, name ):
        return name in self.__dict__

    def hasNode(self, name):
        return name in self._children

    def _hasFlagPath(self, name):
        parts = name.split('.')
        node = self._rootnode()       
        for part in parts[:-1]:
            if not node.hasNode(part) : return False
            node = node.node(part)
        return node.hasFlag(parts[-1])

    def nodes(self):
        return self._children.values()

    def node_flags(self):
        for key, value in self.__dict__.items():
            if not key.startswith("_"):
                yield key, getattr(self, key)
      
    def node(self, name):
        if self.hasNode(name) : return self._children[name]
        raise RuntimeError(f"So such child: {name}")

    def delete(self, name):
        self._fail_if_locked()
        if name in self._children  : del  self._children[name]
        elif name in self.__dict__ : object.__delattr__(self, name) # delattr( self, name )
        else : raise AttributeError(f"Node {self._name} has no member {name}")

    def rename( self, name, newname ):
        if newname in self._children or newname in self.__dict__ :
            raise RuntimeError(f"name {newname} already in node")

        if name in self.__dict__: setattr(self, newname, self.__dict__.pop(name) )
        elif name in self._children :
            self._children[newname] = ( node := self._children.pop(name) )
            node._name = newname
    
    def clone(self, name=None):
        """Return an independent copy" of this node"""

        if name is not None : newnode = Node(name)
        else:                 newnode = Node(self._name)

        for key, value in self.__dict__.items():
            if key in {"_name", "_children", "_lockstate", "_root" } : continue
            setattr(newnode, key, value)

        for node in self.nodes():
            child = node.clone()
            child._root = None
            newnode._addNode(child)

        return newnode

    
    def print(self, spacer=""):
        print( f"{spacer}{self._name} (Node)" )
        if    spacer=="" : spacer = "|__ "
        else: spacer=f"    {spacer}"
        for key, value in self.__dict__.items():
            if key in {"_name","_children","_lockstate", "_flags", "_root"} : continue 
            print( f"{spacer}{key}\t:  {value}" )
        for node in self.nodes() : node.print( spacer )

        
    def dump(self): self.print()

        
    def cloneAndReplace(self, targetname, flagname ) :
        # we really, really, should not have this ...
        warnings.warn( "cloneAndReplace() is unnecessary for Node flags; "
                       "use the appropriate Node directly",
                       RuntimeWarning,  stacklevel=2 )
        # eventually, instead of delegation, should raise an exception ...
        # raise RuntimeError( "cloneAndReplace() should not be used for node flags" )
        return self._rootnode()._flags.cloneAndReplace( targetname, flagname )

        
def notDefined(flags, name):
    """ check whether the flags have an actual item with this name
        NB using hasFlag(name) is really dengerous, as that seems to 
           actually evaluate any actual functions ? shouldn't 
           something like that be called testFlag() ? 
    """
    return name not in flags._flagdict



def decode_flags( flags, domain=None ) :
    """ take an egregious  flags list and convert to a lovely tree
        or, if it is a lovely tree already, just  hand it back 
    """

    if isinstance( flags, Node ) : return flags
    
    root = Node("root")

    for tag, value in flags._flagdict.items():
        root.addFlag( tag, value, flags )

    return root




def update_flags( flags, node, domain="" ) :
    """ take a lovely tree and convert to an egregious flags list, 
        only adding items that are not in the original flags list """

    for key, value in node.__dict__.items():
        if key in {"_name","_children","_lockstate", "_flags", "_root" } : continue
        name = f"{domain}.{key}"
        if notDefined(flags,name) : flags.addFlag( name, value )
        
    for n in node.nodes() :
        if domain=="" : update_flags( flags, n, f"{n.name()}" )
        else:           update_flags( flags, n, f"{domain}.{n.name()}" )

        
        
def topnode(flags) :
    """ Escape hatch in case the user needs to access a node 
        from the top node that has the Same Name, as a node 
        on the local node""" 
    return flags._rootnode()
