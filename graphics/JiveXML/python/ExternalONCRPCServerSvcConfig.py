# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ExternalONCRPCServerSvcCfg(flags):

    acc = ComponentAccumulator()

    serverService = CompFactory.JiveXML.ExternalONCRPCServerSvc(name="ExternalONCRPCServerSvc", Hostname = "pc-tdq-mon-29")

    acc.addService(serverService)

    return acc
