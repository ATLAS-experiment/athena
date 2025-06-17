
"""ComponentAccumulator configuration for SQLiteDBSvc

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
def SQLiteDBSvcCfg(flags, name="SQLiteDBSvc", dbPath=":memory:", **kwargs):
    acc = ComponentAccumulator()
    acc.addService(CompFactory.SQLiteDBSvc(name=name, DatabasePath=dbPath, **kwargs))
    return acc
