# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def TrigmuRoIMonitoring(flags, name = "TrigLateMuonRoIs"):

    montool = GenericMonitoringTool(flags, name, HistPath = name)

    montool.defineHistogram('Rpc_OutOfTimeBCShift', type='TH1F', path='EXPERT', title="RPC RoIs: Difference of the BCID of the out-of-time RoI to the BCID of the event",xbins=11, xmin=-5.5, xmax=5.5)
    montool.defineHistogram('Tgc_OutOfTimeBCShift', type='TH1F', path='EXPERT', title="TGC RoIs: Difference of the BCID if the out-of-time RoI to the BCID of the event",xbins=11, xmin=-5.5, xmax=5.5)
    montool.defineHistogram('OutOfTimeRoI_Eta, OutOfTimeRoI_Phi', type='TH2F', path='EXPERT', title="Eta vs Phi of the late-muon out-of-time RoIs; Eta; Phi", xbins=108, xmin=-2.7, xmax=2.7, ybins=96, ymin=-3.1416, ymax=3.1416 )

    return montool
