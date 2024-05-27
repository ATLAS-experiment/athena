# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def StandardFieldSvcCfg(flags, name="StandardField", **kwargs):
    result = ComponentAccumulator()
    result.addService(CompFactory.StandardFieldSvc(name, **kwargs), primary=True)
    return result


def ForwardFieldSvcCfg(flags, name="ForwardField", **kwargs):
    result = ComponentAccumulator()
    # FIXME Once it exists this version should use the new MagField Service defined in ForwardRegionMgField
    # kwargs.setdefault("MagneticFieldSvc", result.getService("AtlasFieldSvc"))
    # kwargs.setdefault("FieldOn", True)
    kwargs.setdefault("UseMagFieldSvc", True)
    result.addService(CompFactory.StandardFieldSvc(name, **kwargs), primary=True)
    return result


def ForwardRegionFieldSvcCfg(flags, name="ForwardRegionFieldSvc", **kwargs):
    result = ComponentAccumulator()
    from ForwardRegionProperties.ForwardRegionPropertiesConfig import ForwardRegionPropertiesCfg
    kwargs.setdefault("ForwardRegionProperties", result.addPublicTool(result.popToolsAndMerge(ForwardRegionPropertiesCfg(flags))))
    result.addService(CompFactory.MagField.ForwardRegionFieldSvc(name, **kwargs),
                      primary=True)
    return result


def Q1FwdG4FieldSvcCfg(flags, name='Q1FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q1",
                              Magnet=0,
                              MQXA_DataFile="MQXA_NOMINAL.dat")).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q2FwdG4FieldSvcCfg(flags, name='Q2FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q2",
                              Magnet=1,
                              MQXA_DataFile="MQXA_NOMINAL.dat")).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q3FwdG4FieldSvcCfg(flags, name='Q3FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q3",
                              Magnet=2,
                              MQXA_DataFile="MQXA_NOMINAL.dat")).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def D1FwdG4FieldSvcCfg(flags, name='D1FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="D1",
                              Magnet=3)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def D2FwdG4FieldSvcCfg(flags, name='D2FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="D2",
                              Magnet=4)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q4FwdG4FieldSvcCfg(flags, name='Q4FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q4",
                              Magnet=5)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q5FwdG4FieldSvcCfg(flags, name='Q5FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q5",
                              Magnet=6)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q6FwdG4FieldSvcCfg(flags, name='Q6FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q6",
                              Magnet=7)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q7FwdG4FieldSvcCfg(flags, name='Q7FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q7",
                              Magnet=8)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q1HKickFwdG4FieldSvcCfg(flags, name='Q1HKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q1HKick",
                              Magnet=9)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


# note is lower case "v" in ForwardRegionMgFieldConfig.py
def Q1VKickFwdG4FieldSvcCfg(flags, name='Q1VKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q1VKick",
                              Magnet=10)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q2HKickFwdG4FieldSvcCfg(flags, name='Q2HKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q2HKick",
                              Magnet=11)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q2VKickFwdG4FieldSvcCfg(flags, name='Q2VKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q2VKick",
                              Magnet=12)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q3HKickFwdG4FieldSvcCfg(flags, name='Q3HKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q3HKick",
                              Magnet=13)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q3VKickFwdG4FieldSvcCfg(flags, name='Q3VKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q3VKick",
                              Magnet=14)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q4VKickAFwdG4FieldSvcCfg(flags, name='Q4VKickAFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q4VKickA",
                              Magnet=15)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q4HKickFwdG4FieldSvcCfg(flags, name='Q4HKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q4HKick",
                              Magnet=16)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q4VKickBFwdG4FieldSvcCfg(flags, name='Q4VKickBFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q4VKickB",
                              Magnet=17)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q5HKickFwdG4FieldSvcCfg(flags, name='Q5HKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q5HKick",
                              Magnet=18)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q6VKickFwdG4FieldSvcCfg(flags, name='Q6VKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("MagneticFieldSvc",
                      result.getPrimaryAndMerge(
                          ForwardRegionFieldSvcCfg(
                              flags,
                              name="Q6VKick",
                              Magnet=19)).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result
