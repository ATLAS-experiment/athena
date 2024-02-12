# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @file: Configurables.py
# @purpose: a set of Configurables for the PyAthena components
# @author: Sebastien Binet <binet@cern.ch>, Frank Winklmeier
# @author: Frank Winklmeier (rewrite for GaudiConfig2)

import GaudiConfig2
from AthenaConfiguration.ComponentAccumulator import isComponentAccumulatorCfg
from AthenaCommon.Configurable import (Configurable as LegacyConfigurable,
                                       ConfigurableAlgorithm,
                                       ConfigurableService,
                                       ConfigurableAlgTool,
                                       ConfigurableAuditor)


### 
class PyComponents(object):
    """@c PyComponents is a placeholder where all factories for the python
    components will be collected and stored for easy look-up and call from
    the C++ side.
    The @a PyComponents.instances dictionary will store the instance
    e.g.:
     PyComponents.instances[ 'alg1' ] = <PyAthena::Alg/alg1 instance>
    All this boilerplate code will of course be done automatically for the user
    as soon as she uses the (python) @PyConfigurable classes.
    """
    instances = {}
    pass

### helper methods ------------------------------------------------------------
def _get_prop_value(pycomp, propname):
    v = pycomp.properties()[propname]
    if v == pycomp.propertyNoValue:
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        if propname == 'OutputLevel' and hasattr (svcMgr, 'MessageSvc'):
            # special case of OutputLevel...
            v = getattr(svcMgr.MessageSvc, propname)
        else:
            v = pycomp.getDefaultProperty(propname)
            pass
    return v


### Configurable base class for all Py compmonents ----------------------------
class CfgPyComponent:
    def __init__(self, name, **kw):
        self.__dict__['__cpp_type__'] = self.getType()
        for n,v in kw.items():
            setattr(self, n, v)

    def getDlls(self):
        return 'AthenaPython'

    @property
    def msg(self):
        import AthenaCommon.Logging as _L
        return _L.logging.getLogger( self.getName() )

    def getHandle(self):
        return None

    def __getstate__(self):
        state = super().__getstate__()
        state.update(self.__dict__)
        return state

    def __setstate__(self, state):
        self.__dict__.update(state)
        super().__setstate__(state)

    def setup(self):
        if isinstance(self, LegacyConfigurable):
            from AthenaCommon.AppMgr import ServiceMgr as svcMgr
            if not hasattr( svcMgr, 'PyComponentMgr' ):
                from AthenaPython.AthenaPythonCompsConf import PyAthena__PyComponentMgr
                svcMgr += PyAthena__PyComponentMgr('PyComponentMgr')

            ## special case of the OutputLevel: take the value from the
            ## svcMgr.MessageSvc if none already set by user
            setattr(self, 'OutputLevel', _get_prop_value (self, 'OutputLevel') )

        ## populate the PyComponents instances repository
        o = PyComponents.instances.get(self.getName(), None)
        if not (o is None) and not (o is self):
            err = "A python component [%r] has already been "\
                  "registered with the PyComponents registry !" % o
            raise RuntimeError(err)
        PyComponents.instances[self.getName()] = self

    def merge(self, other):
        """Basic merge for Python components.
        Checks that all attributes/properties are identical.
        """
        if self is other:
            return self

        if type(self) is not type(other):
            raise TypeError(f"cannot merge instance of {type(other).__name__} into "
                            f"an instance of { type(self).__name__}")

        if self.name != other.name:
            raise ValueError(f"cannot merge configurables with different names ({self.name} and {other.name})")

        for prop in other.__dict__:
            if (hasattr(self, prop) and getattr(self, prop) == getattr(other, prop)):
                continue
            else:
                raise ValueError(f"conflicting settings for property {prop} of {self.name}: "
                                 f"{getattr(self,prop)} vs {getattr(other,prop)}")
        return self


### Variable base classes to support legacy and GaudiConfig2
# Note that this only works for jobs that are either legacy or CA. Mixing both
# configuration types for Python components is not supported.
#
# Once the legacy classes are no longer needed all this can be greatly simplified.
#
if isComponentAccumulatorCfg():
    _alg_base = _svc_base = _tool_base = _aud_base = GaudiConfig2.Configurable
else:
    _alg_base = ConfigurableAlgorithm
    _svc_base = ConfigurableService
    _tool_base = ConfigurableAlgTool
    _aud_base = ConfigurableAuditor


### Configurable base class for PyAlgorithms ----------------------------------
class CfgPyAlgorithm( CfgPyComponent, _alg_base ):
    def __init__( self, name, **kw ):
        if isinstance(self, LegacyConfigurable):
            _alg_base.__init__(self, name)
        else:
            _alg_base.__init__(self, name, **kw)
        CfgPyComponent.__init__(self, name, **kw)

    def getGaudiType( self ): return 'Algorithm'
    def getType(self):        return 'PyAthena::Alg'

    def setup(self):
        ## store in registry
        from AthenaPython import PyAthena
        setattr(PyAthena.algs, self.getName(), self)

        ## base class setup
        CfgPyComponent.setup(self)
        if isinstance(self, LegacyConfigurable):
            ConfigurableAlgorithm.setup(self)


### Configurable base class for PyServices ------------------------------------
class CfgPyService( CfgPyComponent, _svc_base ):
    def __init__( self, name, **kw ):
        if isinstance(self, LegacyConfigurable):
            _svc_base.__init__(self, name)
        else:
            _svc_base.__init__(self, name, **kw)
        CfgPyComponent.__init__(self, name, **kw)

    def getGaudiType( self ): return 'Service'
    def getType(self):        return 'PyAthena::Svc'

    def setup(self):
        ## store in registry
        from AthenaPython import PyAthena
        setattr(PyAthena.services, self.getName(), self)

        ## base class setup
        CfgPyComponent.setup(self)
        if isinstance(self, LegacyConfigurable):
            _svc_base.setup(self)


### Configurable base class for PyAlgTools ------------------------------------
class CfgPyAlgTool( CfgPyComponent, _tool_base ):
    def __init__( self, name, **kw ):
        if isinstance(self, LegacyConfigurable):
            _tool_base.__init__(self, name)
        else:
            _tool_base.__init__(self, name, **kw)
        CfgPyComponent.__init__(self, name, **kw)

    def getGaudiType( self ): return 'AlgTool'
    def getType(self):        return 'PyAthena::Tool'

    def setup(self):
        CfgPyComponent.setup(self)
        if isinstance(self, LegacyConfigurable):
            _tool_base.setup(self)


### Configurable base class for PyAud -----------------------------------------
class CfgPyAud( CfgPyComponent, _aud_base ):
    def __init__( self, name, **kw ):
        _aud_base.__init__(self, name)
        CfgPyComponent.__init__(self, name, **kw)

    def getGaudiType( self ): return 'Auditor'
    def getType(self):        return 'PyAthena::Aud'

    def setup(self):
        CfgPyComponent.setup(self)
        if isinstance(self, LegacyConfigurable):
            _aud_base.setup(self)


### -----
class _PyCompHandle(object):
    """a class to mimic the gaudi C++ {Tool,Svc}Handle classes: automatic call
    to `initialize` when __getattr__ is called on the instance.
    """
    def __init__(self, parent, attr_name):
        msg = parent.msg.verbose
        try:
            parentName=parent.name()
        except TypeError:
            parentName=parent.name
        msg('installing py-comp-handle for [%s.%s]...',
            parentName, attr_name)
        self.__dict__.update({
            '_parent': parent,
            '_name': attr_name,
            '_attr': getattr(parent, attr_name),
            '_init_called': False,
            })
        msg('installing py-comp-handle for [%s.%s]... [done]',
            parentName, attr_name)
        return

    def _init_once(self, obj):
        if self.__dict__['_init_called']:
            return
        parent = self.__dict__['_parent']
        # FIXME: should we raise something in case initialize failed ?
        obj.initialize()
        self.__dict__['_init_called'] = True
        # replace the handle with the proxied object
        setattr(parent, self.__dict__['_name'], obj)

    def __getattribute__(self, n):
        if n.startswith('_'):
            return super().__getattribute__(n)
        obj = self.__dict__['_attr']
        self._init_once(obj)
        return getattr(obj, n)

    def __setattr__(self, n, v):
        if n.startswith('_'):
            return super().__setattr__(n,v)
        obj = self.__dict__['_attr']
        self._init_once(obj)
        return setattr(obj, n, v)


def _is_pycomp(obj):
    return isinstance(
        obj,
        (CfgPyAlgorithm, CfgPyService, CfgPyAlgTool, CfgPyAud)
        )

def _install_fancy_attrs():
    """loop over all pycomponents, inspect their attributes and install
    a handle in place of (sub) pycomponents to trigger the auto-initialize
    behaviour of (C++) XyzHandles.
    """
    import PyUtils.Logging as L
    msg = L.logging.getLogger('PyComponentMgr').verbose
    comps  = PyComponents.instances.items()
    ncomps = len(comps)
    msg('installing fancy getattrs... (%i)', ncomps)
    for k,comp in comps:
        msg('handling [%s]...', k)
        for attr_name, attr in comp.__dict__.items():
            if _is_pycomp(attr):
                msg(' ==> [%s]...', attr_name)
                setattr(comp, attr_name,
                        _PyCompHandle(parent=comp, attr_name=attr_name))
                msg(' ==> [%s]... [done]', attr_name)
    msg('installing fancy getattrs... (%i) [done]',ncomps)
