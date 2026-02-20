# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def TrigBhhComboHypoMonitoring(flags, name):
    montool = GenericMonitoringTool(flags, name)

    montool.defineHistogram('nMuon', type='TH1F', path='EXPERT', title="number of muons", xbins=10, xmin=0, xmax=10)
    montool.defineHistogram('nTrk', type='TH1F', path='EXPERT', title="number of tracks in superRoI", xbins=200, xmin=0, xmax=200)
    montool.defineHistogram('nBPhysObject', type='TH1F', path='EXPERT', title="number of fitted BPhysObjects", xbins=100, xmin=0, xmax=100)

    return montool
