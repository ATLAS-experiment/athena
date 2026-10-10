#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def ITkPixelCablingAlgCfg(flags, name = 'ITkPixelCablingAlg', **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    acc = ComponentAccumulator()
    acc.merge(addFolders(flags,"/ITk/Pixel/Identifier<ctag>ITkPixModIDMap-RUN4-00-01-TEST</ctag>","INDET_OFL",className="AthenaAttributeList"))
    # "INDET_OFL" is ignored by CREST
    acc.addEventAlgo(CompFactory.ITkPixelCablingAlg(name, **kwargs))
    return acc