# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from AthenaCommon.SystemOfUnits import GeV

def _IncTool(flags, name, monGroups, threshold, sel, tool=None):


    if not tool:
        from AthenaConfiguration.ComponentFactory import CompFactory
        tool = CompFactory.TrigEgammaPrecisionCaloHypoTool(name)

    if hasattr(tool, "MonTool"):

        doValidationMonitoring = flags.Trigger.doValidationMonitoring # True to monitor all chains for validation purposes

        if (any('egammaMon:online' in group for group in monGroups) or doValidationMonitoring):
            monTool = GenericMonitoringTool(flags, "MonTool_"+name,
                                            HistPath = 'PrecisionCaloHypo/'+tool.getName())
            monTool.defineHistogram('dEta', type='TH1F', path='EXPERT', title="PrecisionCalo Hypo #Delta#eta_{L2 L1}; #Delta#eta_{L2 L1}", xbins=80, xmin=-0.01, xmax=0.01)
            monTool.defineHistogram('dPhi', type='TH1F', path='EXPERT', title="PrecisionCalo Hypo #Delta#phi_{L2 L1}; #Delta#phi_{L2 L1}", xbins=80, xmin=-0.01, xmax=0.01)
            monTool.defineHistogram('Eta', type='TH1F', path='EXPERT', title="PrecisionCalo Hypo entries per Eta;Eta", xbins=100, xmin=-2.5, xmax=2.5)
            monTool.defineHistogram('Phi', type='TH1F', path='EXPERT', title="PrecisionCalo Hypo entries per Phi;Phi", xbins=128, xmin=-3.2, xmax=3.2)
            monTool.defineHistogram('Et_em', type='TH1F', path='EXPERT', title="PrecisionCalo Hypo cluster E_{T}^{EM};E_{T}^{EM} [MeV]", xbins=50, xmin=-2000, xmax=100000)

            cuts=['Input','#Delta #eta L2-L1', '#Delta #phi L2-L1','eta','E_{T}^{EM}']

            monTool.defineHistogram('CutCounter', type='TH1I', path='EXPERT', title="PrecisionCalo Hypo Passed Cuts;Cut", xbins=5, xmin=0, xmax=5,  opt="kCumulative", xlabels=cuts)

            tool.MonTool = monTool


    tool.EtaBins        = [0.0, 0.6, 0.8, 1.15, 1.37, 1.52, 1.81, 2.01, 2.37, 2.47]
    def same( val ):
        return [val]*( len( tool.EtaBins ) - 1 )

    tool.ETthr          = same( float(threshold)*GeV )
    tool.dETACLUSTERthr = 0.1
    tool.dPHICLUSTERthr = 0.1
    tool.ET2thr         = same( 90.0*GeV )

    if sel == 'nocut':
        tool.AcceptAll = True
        tool.ETthr          = same( float( threshold )*GeV ) 
        tool.dETACLUSTERthr = 9999.
        tool.dPHICLUSTERthr = 9999.

    if sel == 'etcut' or sel == 'nopid' or sel == 'ion':
        tool.ETthr          = same( float( threshold )*GeV )
        tool.dETACLUSTERthr = 9999.
        tool.dPHICLUSTERthr = 9999.

 
    return tool


def TrigEgammaPrecisionCaloHypoToolFromDict( flags, d, tool=None ):
    """ Use menu decoded chain dictionary to configure the tool """
    cparts = [i for i in d['chainParts'] if ((i['signature']=='Electron') or (i['signature']=='Photon'))]

    def __th(cpart):
        return cpart['threshold']
    
    def __sel(cpart):
        return 'ion' if 'ion' in cpart['extra'] else (cpart['addInfo'][0] if cpart['addInfo'] else cpart['IDinfo'])

    return _IncTool(flags, d['chainName'], d['monGroups'], __th( cparts[0]),  __sel( cparts[0] ) , tool=tool)
