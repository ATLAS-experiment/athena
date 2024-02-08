# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @file: Configurables.py
# @purpose: a set of Configurables for the PyAthena components
# @author: Sebastien Binet <binet@cern.ch>

from AthenaCommon.Configurable import (ConfigurableAlgorithm,
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
        for n,v in kw.items():
            setattr(self, n, v)

    # pickling support
    def __getstate__( self ):
        dic = super().__getstate__()
        dic.update(self.__dict__)
        if 'msg' in dic:
            del dic['msg'] # logger cannot be pickled
        return dic

    def getDlls(self):
        return 'AthenaPython'

    @property
    def msg(self):
        import AthenaCommon.Logging as _L
        return _L.logging.getLogger( self.getJobOptName() )

    def getHandle(self):
        return None

    def _register(self):
        ## populate the PyComponents instances repository
        name = self.getJobOptName()
        o = PyComponents.instances.get(name, None)
        if not (o is None) and not (o is self):
            err = "A python component [%r] has already been "\
                  "registered with the PyComponents registry !" % o
            raise RuntimeError(err)
        PyComponents.instances[name] = self

    def setup(self):
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        if not hasattr( svcMgr, 'PyComponentMgr' ):
            from AthenaPython.AthenaPythonCompsConf import PyAthena__PyComponentMgr
            svcMgr += PyAthena__PyComponentMgr('PyComponentMgr')

        ## special case of the OutputLevel: take the value from the
        ## svcMgr.MessageSvc if none already set by user
        setattr(self, 'OutputLevel', _get_prop_value (self, 'OutputLevel') )

        self._register()

    def setup2(self): # for CA-based configurations
        self._register()


### Configurable base class for PyAlgorithms ----------------------------------
class CfgPyAlgorithm( ConfigurableAlgorithm, CfgPyComponent ):
    def __init__( self, name, **kw ):
        ConfigurableAlgorithm.__init__(self, name)
        CfgPyComponent.__init__(self, name, **kw)

    def getGaudiType( self ): return 'Algorithm'
    def getType(self):        return 'PyAthena::Alg'

    def setup(self):
        from AthenaPython import PyAthena
        setattr(PyAthena.algs, self.getJobOptName(), self)
        ConfigurableAlgorithm.setup(self)
        CfgPyComponent.setup(self)

    def setup2(self):
        from AthenaPython import PyAthena
        setattr(PyAthena.algs, self.getJobOptName(), self)
        CfgPyComponent.setup2(self)


### Configurable base class for PyServices ------------------------------------
class CfgPyService( ConfigurableService, CfgPyComponent ):
    def __init__( self, name, **kw ):
        ConfigurableService.__init__(self, name)
        CfgPyComponent.__init__(self, name, **kw)

    def getGaudiType( self ): return 'Service'
    def getType(self):        return 'PyAthena::Svc'

    def setup(self):
        from AthenaPython import PyAthena
        setattr(PyAthena.services, self.getJobOptName(), self)
        ConfigurableService.setup(self)
        CfgPyComponent.setup(self)

    def setup2(self):
        from AthenaPython import PyAthena
        setattr(PyAthena.services, self.getJobOptName(), self)
        CfgPyComponent.setup2(self)


### Configurable base class for PyAlgTools ------------------------------------
class CfgPyAlgTool( ConfigurableAlgTool, CfgPyComponent ):
    def __init__( self, name, **kw ):
        ConfigurableAlgTool.__init__(self, name)
        CfgPyComponent.__init__(self, name, **kw)

    def getGaudiType( self ): return 'AlgTool'
    def getType(self):        return 'PyAthena::Tool'

    def setup(self):
        ConfigurableAlgTool.setup(self)
        CfgPyComponent.setup(self)


### Configurable base class for PyAud -----------------------------------------
class CfgPyAud( ConfigurableAuditor, CfgPyComponent ):
    def __init__( self, name, **kw ):
        ConfigurableAuditor.__init__(self, name)
        CfgPyComponent.__init__(self, name, **kw)

    def getGaudiType( self ): return 'Auditor'
    def getType(self):        return 'PyAthena::Aud'

    def setup(self):
        ConfigurableAuditor.setup(self)
        CfgPyComponent.setup(self)


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
