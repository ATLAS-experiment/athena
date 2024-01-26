# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Compatibility layer allowing to convert between new (as of 2020) Gaudi Configurable2
# and old Configurable classes

from AthenaCommon import CfgMgr, CFElements
from AthenaCommon.AlgSequence import AthSequencer
from AthenaCommon.Configurable import Configurable, ConfigurableCABehavior
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory, isComponentAccumulatorCfg
from AthenaConfiguration.ComponentAccumulator import ConfigurationError
from AthenaConfiguration.Deduplication import DeduplicationFailed

import GaudiConfig2
import collections.abc


def _indent( indent = ""):
    """For indendation of debug output"""
    return indent + "  "

def _isOldConfigurable(c):
    """Is old-style Configurable"""
    return isinstance(c, Configurable)


# Tuple of semantics helpers that need special treatment to retrieve their value
_semanticsHelpers = (GaudiConfig2.semantics._ListHelper,
                     GaudiConfig2.semantics._DictHelper,
                     GaudiConfig2.semantics._SetHelper)

def _conf2HelperToBuiltin(value):
    """Recursively convert GaudiConfig2 semantics helpers"""
    if isinstance(value, GaudiConfig2.semantics._ListHelper):
        return [_conf2HelperToBuiltin(item) for item in value.data]
    if isinstance(value, GaudiConfig2.semantics._DictHelper):
        return dict((k,_conf2HelperToBuiltin(v)) for k,v in value.data.items())
    if isinstance(value, GaudiConfig2.semantics._SetHelper):
        return set(_conf2HelperToBuiltin(item) for item in value.data)
    return value


def _setProperties( dest, src, indent="" ):
    """Set properties from src (GaudiConfig2) on dest configurable"""

    _log = logging.getLogger( "_setProperties" )

    for pname, pvalue in src._properties.items():
        if dest.__class__.__name__ == 'AthSequencer' and pname == 'Members':
            continue

        propType = src._descriptors[pname].cpp_type
        if "PrivateToolHandleArray" in propType:
            setattr( dest, pname,
                     [conf2toConfigurable( tool, indent=_indent( indent ),
                                           parent = f"{src.getName()}.{pname}",
                                           propType = propType ) for tool in pvalue] )
            _log.debug( "%sSetting private tool array property %s of %s",
                        indent, pname, dest.name() )

        elif ("PrivateToolHandle" in propType or
              "GaudiConfig2.Configurables" in propType or
              "ServiceHandle" in propType):

            _log.debug( "%sSetting property %s of %s",
                        indent, pname, dest.name() )
            try: # sometimes it is not printable
                _log.debug("%sComponent: %s", indent, pvalue)
            except Exception:
                pass
            if pvalue is not None:
                setattr( dest, pname,
                         conf2toConfigurable( pvalue, indent=_indent( indent ),
                                              parent = f"{src.getName()}.{pname}",
                                              propType = propType ) )
            else:
                setattr( dest, pname, pvalue )

        else: # plain data
            if isinstance(pvalue, _semanticsHelpers):
                pvalue = _conf2HelperToBuiltin(pvalue)
            try: # sometimes values are not printable
                _log.debug( "%sSetting property %s to value %s", indent, pname, pvalue )
            except Exception:
                pass
            setattr( dest, pname, pvalue )


def _fetchOldSeq(name=""):
    with ConfigurableCABehavior(target_state=False):
        seq = AthSequencer(name)
    return seq

def _mergeSequences( currentConfigurableSeq, conf2Sequence, _log, indent="" ):
    """Merge conf2sequence into currentConfigurableSeq"""

    sequence = CFElements.findSubSequence( currentConfigurableSeq, conf2Sequence.name )
    if not sequence:
        sequence = _fetchOldSeq( conf2Sequence.name )
        _setProperties( sequence, conf2Sequence, indent=_indent( indent ) )
        currentConfigurableSeq += sequence
        _log.debug( "%sCreated missing AthSequencer %s and added to %s",
                    _indent( indent ), sequence.name(), currentConfigurableSeq.name() )

    for el in conf2Sequence.Members:
        if el.__class__.__name__ in ["AthSequencer"]:
            _mergeSequences( sequence, el, _log, _indent( indent ) )
        elif el.getGaudiType() == "Algorithm":
            # We will get an error if there are duplicate algs in a sequence, indicating misconfiguration
            # HLT requires algs to be in many sequences, cannot afford to veto them all here
            toadd = conf2toConfigurable( el, indent=_indent( indent ))
            if (
                toadd is not None
                # SGInputLoader has to be in TopAlg but doesn't have to be anywhere else
                and not ( toadd.name()=='SGInputLoader' and sequence.name() not in ['TopAlg','HLTBeginSeq'])
            ):  
                sequence += toadd
                _log.debug( "%sAlgorithm %s and added to the sequence %s",
                            _indent( indent ),  el.getFullJobOptName(), sequence.name() )


def conf2toConfigurable( comp, indent="", parent="", servicesOfThisCA=[], suppressDupes=False, propType="" ):
    """
    Method converts from Conf2 ( comp argument ) to old Configurable
    If the Configurable of the same name exists, the properties merging process is invoked
    """
    _log = logging.getLogger( "conf2toConfigurable" )

    if _isOldConfigurable(comp):
        _log.debug( "%sComponent is already OLD Configurable object %s, no conversion",
                    indent, CFElements.compName(comp) )
        return comp

    if isinstance(comp, str):
        if comp and 'ServiceHandle' not in propType:  # warning for non-empty string
            _log.warning( "%sComponent '%s' in '%s' is of type string, no conversion, "
                          "some properties possibly not set?", indent, comp, parent)
        return comp

    if comp.getType() == 'AthSequencer':
        _log.debug( "%sComponent is a sequence %s, attempt to merge",
                    indent, CFElements.compName(comp))
        oldsequence = _fetchOldSeq(CFElements.compName(comp))
        _mergeSequences(oldsequence, comp, _log, indent)
        return oldsequence

    _log.debug( "%sConverting from GaudiConfig2 object %s type %s, parent %s",
                indent, CFElements.compName(comp), comp.__class__.__name__ , parent)

    def _alreadyConfigured( comp, parent ):
        instanceName = comp.getName()
        for conf in Configurable.allConfigurables.values():
            conf_name = ''
            try:
                conf_name=conf.name()
            except TypeError:
                # Is a string
                conf_name=conf

            if conf_name==instanceName:
                if conf.getParent() == parent:
                    _log.debug( "%s Matched component: '%s' with parent %s with same from allConfigurables match.",
                                indent, instanceName, parent if parent else "[not set]" )
                    return conf
                else:
                    _log.debug( "%sComponent: '%s' had parent %s whilst this allConfigurables match had parent %s.",
                                indent, instanceName, parent if parent else "[not set]", conf.getParent() )
        return None

    def _createConf2Object( name ):
        typename, instanceName = name.split("/") if "/" in name else (name,name)
        return CompFactory.getComp( typename.replace("__", "::") )( instanceName )

    def _configurableToConf2( comp, indent="" ):
        _log.debug( "%sConverting Conf2 to Configurable class %s, type %s", indent, comp.getFullName(), type(comp) )
        conf2Object = _createConf2Object( comp.getFullName() )
        _getProperties( comp, conf2Object, _indent( indent ) )
        return conf2Object

    def _getProperties( src, dest, indent="" ):
        """Read properties on src and set them on dest (GaudiConfig2) configurable"""
        for prop, value in src.getProperties().items():
            _log.debug( "%sDealing with class %s property %s value type %s",
                        indent, src.getFullJobOptName(), prop, type(value) )
            if "ServiceHandle" in str( type( value ) ):
                instance = _alreadyConfigured(value, src.getName())
                if instance:
                    setattr( dest, prop, _configurableToConf2(instance, _indent(indent)) )
            else:
                if isinstance(value, _semanticsHelpers):
                    value=value.data
                setattr( dest, prop, value )

    def _findConfigurableClass( name ):
        """Find old-style Configurable class for name"""
        if "::" in name: # cure namespaces
            name = name.replace("::","__")

        if "<" in name:
            name=name.replace("<","_")
            name=name.replace(">","_")
            name=name.replace(", ","_")

        classObj = getattr( CfgMgr, name )

        if not classObj:
            raise ConfigurationError(f"CAtoGlobalWrapper could not find the component of type {name}")

        return classObj

    def _areSettingsSame( conf1, conf2, indent="",servicesOfThisCA=[] ):
        """Are the properties the same between old-style conf1 and new-style conf2 instance?"""
        from AthenaCommon.AppMgr import ToolSvc

        _log.debug( "%sChecking if settings are the same %s (%s) old(new)",
                    indent, conf1.getFullName(), conf2.getFullJobOptName() )

        if conf1.getType() != conf2.__cpp_type__:
            raise ConfigurationError("Old/new ({} | {}) cpp types are not the same for ({} | {}) !".format(
                                    conf1.getType(), conf2.__cpp_type__,
                                    conf1.getFullName(), conf2.getFullJobOptName() ) )

        alreadySetProperties = conf1.getValuedProperties().copy()

        _log.debug( "%sExisting properties: %s", indent, alreadySetProperties )
        _log.debug( "%sNew properties: %s", indent, conf2._properties )

        for pname, pvalue in conf2._properties.items():

            if _isOldConfigurable( pvalue ):
                _log.warning( "%sNew configuration object %s property %s has legacy configuration "
                              "components assigned to it %s. Skipping comparison, no guarantees "
                              "about configuration consistency.",
                              indent, CFElements.compName(conf2), pname, CFElements.compName(pvalue) )
                continue

            propType = conf2._descriptors[pname].cpp_type
            _log.debug( "%sComparing type: %s for: %s in: %s", indent, propType, pname, conf1.getFullJobOptName() )

            if  "PrivateToolHandleArray" in propType:
                toolDict = {_.getName(): _ for _ in alreadySetProperties[pname]}
                _log.debug('Private tool properties? %s', toolDict)
                newCdict = {_.getName() : _ for _ in pvalue}
                oldCset = set(toolDict); newCset = set(newCdict)
                _log.debug('Private tool property names? %s %s', oldCset, newCset)
                if ( not (oldCset == newCset) ):
                    # This is allowed, and happens normally in a number of
                    # cases.  Don't warn by default here.
                    _log.debug('%s PrivateToolHandleArray %s of %s does not have the same named components',indent, pname, conf1.getFullJobOptName() )
                    _log.debug('%s Old (conf1) %s for %s',indent, sorted(oldCset), conf1.getFullJobOptName())
                    _log.debug('%s New (conf2) %s for %s',indent, sorted(newCset), conf2.getFullJobOptName())
                    _log.debug('%s Will try to merge them, but this might go wrong!',indent)
                for oldC in oldCset & newCset:
                    _areSettingsSame( toolDict[oldC], newCdict[oldC], _indent(indent),servicesOfThisCA)

                # And now just the new properties in conf2 (the stuff just in conf1 is already in the objec)
                for newC in sorted(newCset-oldCset):
                    className = newCdict[newC].getFullJobOptName().split( "/" )[0]
                    _log.debug('%s%s not in old config. Will try to create conf1 instance using '
                               'this className: %s, and merge.',indent, newC, className)
                    configurableClass = _findConfigurableClass( className )
                    # Do not create with existing name, or it will try to get an existing public tool, if available
                    # (and public tools cannot be added to a PrivateToolHandleArray)
                    tmpName = newC + className + str(len(indent))
                    instance = configurableClass( tmpName )

                    # Now give it the correct name and fix Configurable DB
                    instance._name = newCdict[newC].name
                    if hasattr(instance, '_jobOptName'):
                        instance._jobOptName = instance._name

                    instance.allConfigurables[instance._name] = instance.allConfigurables.pop(tmpName)
                    instance.configurables[instance._name] = instance.configurables.pop(tmpName)

                    _setProperties( instance, newCdict[newC], _indent( indent ) )
                    _log.debug('%s will now add %s to array.',indent, instance)
                    conf1 += instance # Makes a copy with a correctly set parent and name
                    del instance
                    alreadySetProperties[pname].append(conf1.getChildren()[-1])

            elif "PublicToolHandleArray" in propType:
                toolSet = {_.getName() for _ in alreadySetProperties[pname]}
                _log.debug('Public tool handle array properties? %s %s', toolSet, pvalue)
                # strings?
                for newC in pvalue:
                    if isinstance(newC, str):
                        pubtoolclass, pubtoolname = newC.split('/')
                        if pubtoolname not in toolSet:
                            klass = _findConfigurableClass( pubtoolclass )
                            instance = klass(pubtoolname)
                            ToolSvc += instance
                            alreadySetProperties[pname].append(instance)
                    else:
                        _log.warning('Not handling actual Configurable2s for public tool merging yet')
                        raise Exception()

            elif ("PrivateToolHandle" in propType or
                  "GaudiConfig2.Configurables" in propType or
                  "ServiceHandle" in propType):
                existingVal = getattr(conf1, pname)
                if isinstance( pvalue, str ):
                    #Try getting the component from the known services:
                    if "ServiceHandle" in propType and pvalue in servicesOfThisCA:
                        _log.debug("%sThe service %s is part of the CA. Consistency checks will be performed when the service is merged") 
                    else:
                        # debug only, because we already get a warning from conf2toConfigurable for this
                        _log.debug("%sThe %s '%s' of GaudiConfig2 component %s.%s is a string, "
                                   "skipping deeper checks",
                                   indent, propType, pvalue, conf2.name, pname)
                elif pvalue is None:
                    _log.debug("%sThe property value for %s of %s is None. Skipping.", indent, pname, conf2.name )
                    continue
                elif str(existingVal) == "":
                    className = pvalue.getFullJobOptName().split( "/" )[0]
                    pvalueCompName = pvalue.getFullJobOptName().split( "/" )[1]
                    _log.debug("%sThe existing value for %s of %s is an empty handle. "
                               "Will try to create conf1 instance using this className: %s, and merge.",
                               indent, pname, conf2.name, className )
                    configurableClass = _findConfigurableClass( className )
                    # Do not create with existing name, or it will try to get an existing public tool, if available
                    # (and public tools cannot be added to a PrivateToolHandle)
                    tmpName = pvalueCompName + className + str(len(indent))
                    instance = configurableClass( tmpName )
                    # Now give it the correct name, assign to the conf1 property, and merge
                    instance._name = pvalueCompName
                    if hasattr(instance, '_jobOptName'):
                        instance._jobOptName = instance._name

                    instance.allConfigurables[instance._name] = instance.allConfigurables.pop(tmpName)
                    instance.configurables[instance._name] = instance.configurables.pop(tmpName)

                    setattr(conf1, pname, instance)
                    existingVal = getattr(conf1, pname)
                    _areSettingsSame( existingVal, pvalue, indent,servicesOfThisCA)
                else:
                    _log.debug( "%sSome kind of handle and, object type %s existing %s",
                                indent, type(pvalue), type(existingVal) )
                    _areSettingsSame( existingVal, pvalue, indent,servicesOfThisCA)
            else:
                if isinstance(pvalue, _semanticsHelpers):
                    pvalue = _conf2HelperToBuiltin(pvalue)

                if pname not in alreadySetProperties:
                    _log.debug( "%sAdding property: %s for %s", indent, pname, conf2.getName() )
                    try:
                        setattr(conf1, pname, pvalue)
                    except AttributeError:
                        _log.info("%sCould not set attribute. Type of conf1 %s.",indent, type(conf1) )
                        raise

                elif alreadySetProperties[pname] != pvalue:
                    # Old configuration writes some properties differently e.g. ConditionStore+TileBadChannels
                    # instead of just TileBadChannels. So check if this isn't a false positive before continuing.
                    merge = True

                    # Could be strings e.g. alreadySetProperties[pname]==RPCCablingDbTool
                    # and pvalue == RPCCablingDbTool/RPCCablingDbTool
                    if (isinstance(pvalue, str) and isinstance(alreadySetProperties[pname], str)):
                        if ('/' in pvalue \
                               and pvalue.split('/')[-1] == alreadySetProperties[pname]):
                            # Okay. so they probably are actually the same. Can't check type.
                            merge = False
                            _log.warning( "%sProperties here are strings and not exactly the same. "
                                          "ASSUMING they match types but we cannot check. %s for %s",
                                          indent, pname, conf2.getName() )
                    try:
                        if ('+' in alreadySetProperties[pname].toStringProperty() and
                            alreadySetProperties[pname].toStringProperty().split('+')[-1] == pvalue):
                            # Okay. so they ARE actually the same
                            merge = False
                    except AttributeError :
                        # This is fine - it just means it's not e.g. a DataHandle and doesn't have toStringProperty
                        pass

                    # Okay, not the same ... let's merge
                    if merge:
                        _log.debug( "%sMerging property: %s for new config: %s", indent, pname, conf2.getName() )
                        # create surrogate
                        clone = conf2.getInstance("Clone")
                        setattr(clone, pname, alreadySetProperties[pname])
                        try:
                            updatedPropValue = _conf2HelperToBuiltin( conf2._descriptors[pname].semantics.merge(
                                getattr(conf2, pname), getattr(clone, pname)) )
                        except (TypeError, ValueError):
                            err_message = f"Failed merging new config value ({getattr(conf2, pname)}) and old config value ({getattr(clone, pname)}) for the ({pname}) property of {conf1.getFullJobOptName() } ({conf2.getFullJobOptName()}) old (new)."
                            _log.fatal( err_message )
                            raise ConfigurationError(err_message)

                        _log.debug("existingConfigurable.name: %s, pname: %s, updatedPropValue: %s",
                                   conf1.name(), pname, updatedPropValue )

                        setattr(conf1, pname, updatedPropValue)
                        del clone
                        _log.debug("%sInvoked GaudiConf2 semantics to merge the %s and the %s to %s "
                                   "for property %s of %s",
                                   indent, alreadySetProperties[pname], pvalue, pname,
                                   updatedPropValue, existingConfigurable.getFullName())

    _log.debug( "%sConf2 Full name: %s ", indent, comp.getFullJobOptName() )
    existingConfigurable = _alreadyConfigured( comp, parent )

    if existingConfigurable: # if configurable exists we try to merge with it
        _log.debug( "%sPre-existing configurable %s was found, checking if has the same properties",
                    indent, existingConfigurable.getFullJobOptName() )
        _areSettingsSame( existingConfigurable, comp, indent,  servicesOfThisCA )
        _log.debug( "%sPre-existing configurable %s was found to have the same properties",
                    indent, comp.name )
        instance = existingConfigurable if not suppressDupes else None

    else: # create new configurable
        _log.debug( "%sExisting Conf1 not found. Creating component configurable %s",
                    indent, comp.getFullJobOptName() )
        configurableClass = _findConfigurableClass( comp.getFullJobOptName().split( "/" )[0] )
        instance = configurableClass( comp.name )
        _setProperties( instance, comp, _indent( indent ) )

    return instance


def CAtoGlobalWrapper(cfgFunc, flags, **kwargs):
    """Execute the cfgFunc CA with the given flags and arguments and run appendCAtoAthena.
    Return the result of cfgFunc."""
    if isComponentAccumulatorCfg():
        raise RuntimeError("CAtoGlobalWrapper should not be called in pure CA config")

    if not callable(cfgFunc):
        raise TypeError("CAtoGlobalWrapper must be called with a configuration-function as parameter")

    with ConfigurableCABehavior():
        result = cfgFunc(flags, **kwargs)
        if isinstance(result, tuple):
            ca = result[0]
        else:
            ca = result

    appendCAtoAthena(ca)
    return result

def appendCAtoAthena(ca):
    from AthenaCommon.AppMgr import (ServiceMgr, ToolSvc, theApp,
                                     athCondSeq, athOutSeq, athAlgSeq, topSequence)
    _log = logging.getLogger( "conf2toConfigurable" )
    _log.debug( "Merging ComponentAccumulator into global configuration" )


    servicesOfThisCA=[svc.getFullJobOptName() for svc in ca.getServices()]

    if len( ca.getPublicTools() ) != 0:
        for comp in ca.getPublicTools():
            instance = conf2toConfigurable( comp, indent="  ", parent="ToolSvc", servicesOfThisCA=servicesOfThisCA )
            if instance not in ToolSvc:
                ToolSvc += instance

    if len(ca.getServices()) != 0:
        for comp in ca.getServices():
            instance = conf2toConfigurable( comp, indent="  ", servicesOfThisCA=servicesOfThisCA )
            if instance not in ServiceMgr:
                ServiceMgr += instance
        for svcName in ca._servicesToCreate:
            if svcName not in theApp.CreateSvc:
                theApp.CreateSvc += [svcName]

    if  len(ca._conditionsAlgs) != 0:
        for comp in ca._conditionsAlgs:
            instance = conf2toConfigurable( comp, indent="  ", servicesOfThisCA=servicesOfThisCA )
            if instance not in athCondSeq:
                athCondSeq += instance

    if len( ca.getAppProps() ) != 0:
        for propName, propValue in ca.getAppProps().items():
            # Same logic as in ComponentAccumulator.setAppProperty()
            if not hasattr(theApp, propName):
                setattr(theApp, propName, propValue)
            else:
                origPropValue = getattr(theApp, propName)
                if origPropValue == propValue:
                    _log.info("ApplicationMgr property '%s' already set to '%s'.", propName, propValue)
                elif isinstance(origPropValue, collections.abc.Sequence) and not isinstance(origPropValue, str):
                    propValue =  origPropValue + [el for el in propValue if el not in origPropValue]
                    _log.info("ApplicationMgr property '%s' already set to '%s'. Overwriting with %s",
                              propName, origPropValue, propValue)
                    setattr(theApp, propName, propValue)
                else:
                    raise DeduplicationFailed(f"ApplicationMgr property {propName} set twice: "
                                              "{origPropValue} and {propValue}")

    preconfigured = [athCondSeq,athOutSeq,athAlgSeq,topSequence]

    for seq in ca._allSequences:

        # For legacy (serial) support, the AthAlgSeq needs to be mapped into TopAlg
        # because that's where old-style job options add their algorithms.
        # Otherwise we can end up with the wrong order, see e.g. ATLASRECTS-7078.
        if seq.getName() == "AthAlgSeq":
            seq.name = "TopAlg"

        merged = False
        for pre in preconfigured:
            if seq.getName() == pre.getName():
                _log.debug( "%sfound sequence %s to have the same name as predefined %s",
                            _indent(), seq.getName(),  pre.getName() )
                _mergeSequences( pre, seq, _log )
                merged = True
                break
            if CFElements.findSubSequence( pre, seq.name ):
                _log.debug( "%sfound sequence %s in predefined %s",
                            _indent(), seq.getName(),  pre.getName() )
                _mergeSequences( pre, seq, _log )
                merged = True
                break

        if not merged:
            _log.debug( "%snot found sequence %s merging it to AthAlgSeq", _indent(), seq.name )
            _mergeSequences( athAlgSeq, seq, _log )

    ca.wasMerged()
    _log.debug( "Merging of CA to global done" )
