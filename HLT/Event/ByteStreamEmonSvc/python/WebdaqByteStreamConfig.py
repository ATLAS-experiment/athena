#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

# NB: This file is similar and has a corresponding role to ByteStreamCnvSvc.ByteStreamConfig
# and TrigByteStreamCnvSvc.TrigByteStreamConfig

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SGComps.SGInputLoaderConfig import SGInputLoaderCfg

def WebdaqByteStreamCfg(flags, type_names=[]):
    acc = ComponentAccumulator()

    bytestream_conversion = CompFactory.ByteStreamCnvSvc()
    acc.addService(bytestream_conversion, primary=True)

    bytestream_input = CompFactory.ByteStreamWebdaqInputSvc("ByteStreamInputSvc")
    acc.addService(bytestream_input)

    event_selector = CompFactory.EventSelectorByteStream(
        name='EventSelector',
        ByteStreamInputSvc=bytestream_input.name,
        FileBased=False)
    acc.addService(event_selector)
    acc.setAppProperty("EvtSel", event_selector.name)

    event_persistency = CompFactory.EvtPersistencySvc(
        name="EventPersistencySvc",
        CnvServices=[bytestream_conversion.name])
    acc.addService(event_persistency)

    address_provider = CompFactory.ByteStreamAddressProviderSvc(
        TypeNames=type_names)
    acc.addService(address_provider)

    proxy = CompFactory.ProxyProviderSvc()
    proxy.ProviderNames += [address_provider.name]
    acc.addService(proxy)

    acc.merge(SGInputLoaderCfg(flags, address_provider.TypeNames))

    return acc


def replaceByteStreamInputWithWebdaq(flags, acc, **kwargs):
    """Replace the file based bytestream input of an already configured job
    with the ByteStreamWebdaqInputSvc.

    This is meant to be called on the final ComponentAccumulator, after
    all other configuration (e.g. RecoSteering or detector specific decoding
    configs) has been merged. Those configs typically merge
    ByteStreamCnvSvc.ByteStreamConfig.ByteStreamReadCfg themselves, which
    installs a ByteStreamEventStorageInputSvc as 'ByteStreamInputSvc'.
    Since the component accumulator refuses to merge two services with the
    same name and different types, the service is swapped here after the fact.

    Any keyword arguments are passed as properties to ByteStreamWebdaqInputSvc.
    """
    services = acc.getServices()
    old_inputs = [s for s in services if s.name == "ByteStreamInputSvc"]

    if not old_inputs:
        # No bytestream input configured at all, add the full setup
        acc.merge(WebdaqByteStreamCfg(flags))
        services = acc.getServices()
        old_inputs = [s for s in services if s.name == "ByteStreamInputSvc"]

    for s in old_inputs:
        services.remove(s)

    acc.addService(CompFactory.ByteStreamWebdaqInputSvc("ByteStreamInputSvc", **kwargs))

    # The event selector must not try to open any files
    event_selector = acc.getService("EventSelector")
    event_selector.FileBased = False
    event_selector.Input = []
    event_selector.ByteStreamInputSvc = "ByteStreamInputSvc"

    # Used by ByteStreamWebdaqInputSvc, not added by WebdaqByteStreamCfg
    acc.addService(CompFactory.ROBDataProviderSvc())

    return acc
