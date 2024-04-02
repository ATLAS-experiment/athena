#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''@file JetTagDQAConfig.py
@author T. Strebler
@date 2022-06-16
@brief Main CA-based python configuration for JetTagDQA
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def PhysValBTagCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("DetailLevel", 10)
    kwargs.setdefault("isData", not flags.Input.isMC)

    import ROOT
    path = ROOT.PathResolver.find_file( 'JetTagDQA/PhysValBtag_VariablesMenu.json', 'DATAPATH' )
    from PhysValMonitoring.PhysValUtils import getHistogramDefinitions
    definitions = getHistogramDefinitions(path, 'PHYSVAL', 'ALL')

    # Run-dependent histo definitions
    path_Run = ROOT.PathResolver.find_file( \
                'JetTagDQA/PhysValBtag_VariablesMenu_Run3.json' if flags.GeoModel.Run <= LHCPeriod.Run3 \
                else 'JetTagDQA/PhysValBtag_VariablesMenu_Run4.json',
                'DATAPATH' )
    definitions_Run = getHistogramDefinitions(path_Run, 'PHYSVAL', 'ALL')

    kwargs.setdefault("HistogramDefinitions", definitions + definitions_Run)
    kwargs.setdefault("JetEtaCut", 2.5 if flags.GeoModel.Run <= LHCPeriod.Run3 else 4.0)
    kwargs.setdefault("JetContainerEMTopo", "" if flags.GeoModel.Run <= LHCPeriod.Run3 else "AntiKt4EMTopoJets")

    if "trackTruthOriginTool" not in kwargs:
        from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import InDetTrackTruthOriginToolCfg
        kwargs.setdefault("trackTruthOriginTool", acc.popToolsAndMerge(
            InDetTrackTruthOriginToolCfg(flags)))

    if flags.GeoModel.Run >= LHCPeriod.Run4:
        kwargs.setdefault("dipsTaggerName",    "dipsrun420221008")
        kwargs.setdefault("DL1dv01TaggerName", "DL1drun420221017")
        kwargs.setdefault("GN2v01TaggerName", "")

    tool = CompFactory.JetTagDQA.PhysValBTag(**kwargs)
    acc.setPrivateTools(tool)
    return acc
