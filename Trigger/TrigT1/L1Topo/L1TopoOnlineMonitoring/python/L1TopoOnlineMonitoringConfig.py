# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from TrigConfigSvc.TriggerConfigAccess import getL1MenuAccess


def getL1TopoOnlineMonitorHypo(flags):
    hypo = CompFactory.L1TopoOnlineMonitorHypo()
    return hypo

def L1TopoOnlineMonitorHypoToolGen(chainDict):
    tool = CompFactory.L1TopoOnlineMonitorHypoTool(
        chainDict['chainName'],
        # Select error flags to accept events
        ErrorFlagsKey = 'L1TopoErrorFlags',
        AcceptTrivialFlag = True)

    return tool

def getL1TopoLabels(flags,connectors = {0: 'LegacyTopo0', 1: 'LegacyTopo1'}, bmax = 128):
    topo_trigline_labels = ["" for i in range(bmax)]
    lvl1access  = getL1MenuAccess(flags)
    for connector_id, connectorKey in connectors.items():
        topo_triglines_dict = lvl1access.connector(connectorKey)['triggerlines']
        if not isinstance(topo_triglines_dict, list):
            for fpga_id in [0,1]:
                topo_fpga = topo_triglines_dict['fpga{:d}'.format(fpga_id)]
                for clock_id in [0,1]:
                    topo_clock = topo_fpga['clock{:d}'.format(clock_id)]
                    for topo_trigline in topo_clock:
                        topo_trigline_name = topo_trigline['name']
                        bit_id = topo_trigline['startbit']
                        topo_trigline_index = 64*connector_id + 32*fpga_id + 2*bit_id + clock_id
                        topo_trigline_labels[topo_trigline_index] = topo_trigline_name
        else:
            for topo_trigline in topo_triglines_dict:
                topo_trigline_name = topo_trigline['name']
                bit_id = topo_trigline['startbit']
                fpga_id = topo_trigline['fpga']
                clock_id = topo_trigline['clock']
                topo_trigline_index = 64*connector_id + 32*fpga_id + 2*bit_id + clock_id
                topo_trigline_labels[topo_trigline_index] = topo_trigline_name

    return topo_trigline_labels

def getMultiplicityLabels(flags,topoModule):
    topo_trigline_labels = ["" for i in range(128)]
    lvl1access  = getL1MenuAccess(flags)
    topo_triglines_dict = lvl1access.connector(topoModule)['triggerlines']

    for topo_trigline in topo_triglines_dict:
        topo_trigline_name = topo_trigline['name']
        bit_id = topo_trigline['startbit']
        topo_trigline_labels[bit_id] = topo_trigline_name
    
    return topo_trigline_labels

def Phase1TopoMonitoringCfg(flags):
    # placeholder for new phase-1 implementation, used in L1CaloPhase1Monitoring
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    return result

def getL1TopoPhase1OnlineMonitor(flags, name='L1TopoOnlineMonitor', doSimMon=True, doHwMonCtp=False, doHwMon=False, doComp=False, doMultComp=False, forceCtp=False, logLevel = None, toolName="L1TopoMonitoringTool"):
    # Placeholder for phase-1 implementation
    #raise RuntimeError('L1Topo phase-1 online monitoring not yet implemented')
    alg = CompFactory.L1TopoOnlineMonitor(toolName,
                                          doHwMon = doHwMon,
                                          doHwErrorMon = False,
                                          doSimMon = doSimMon,
                                          doHwMonCTP = doHwMonCtp,
                                          doComp = doComp,
                                          doMultComp = doMultComp,
                                          forceCTPasHdw=forceCtp)
    if logLevel : alg.OutputLevel=logLevel
    alg.MonTool = GenericMonitoringTool(flags, 'MonTool')
    alg.MonTool.HistPath = name
    configureHistograms(alg, flags, doHwMonCtp, doHwMon, doComp, doMultComp)

    return alg

def configureHistograms(alg, flags, doHwMonCtp, doHwMon, doComp, doMultComp):

    label_topo_all = []
    for cable in range(2):
        name = 'CableElec_'+str(cable+2)
        name += ';Topo'+str(cable+2)+'El'
        title = f'Topo Electric Cable {cable+2}'
        labels = getL1TopoLabels(flags,{0: f'Topo{cable+2}El'},64)
        label_topo_all += labels
        alg.MonTool.defineHistogram(name, path='EXPERT', type='TH1I',
                                    title=title, xbins=64, xlabels=labels,
                                    xmin=0, xmax=64)

    for cable in range(4):
        topoName = 'Topo1Opt'+str(cable)
        name = 'CableOpti_'+str(cable)
        weight = name+'_weight'
        name += f';{topoName}'
        title = f'Topo Optical Cable {cable}'
        labels = getMultiplicityLabels(flags=flags,topoModule=topoName)
        xlabels = [x for x in labels if x]
        alg.MonTool.defineHistogram(name, path='EXPERT', type='TH1I',
                                    title=title, xbins=len(xlabels), xlabels=xlabels,
                                    weight=weight,
                                    xmin=0, xmax=len(xlabels))
    for cable in range(4):
        topoName = 'Topo1Opt'+str(cable)
        name = 'HdwTopo1Opt'+str(cable)
        weight = name+'_weight'
        name += f';{topoName}_data'
        title = f'Topo Optical Cable {cable} (Data)'
        labels = getMultiplicityLabels(flags=flags,topoModule=topoName)
        xlabels = [x for x in labels if x]
        alg.MonTool.defineHistogram(name, path='EXPERT', type='TH1I',
                                    title=title, xbins=len(xlabels), xlabels=xlabels,
                                    weight=weight,
                                    xmin=0, xmax=len(xlabels))

    for cable in range(4):
        ylabels = ['#splitline{Sim counts <}{Hdw counts}','#splitline{Sim counts >}{Hdw counts}','#splitline{Sim counts =}{Hdw counts}']
        topoName = 'Topo1Opt'+str(cable)
        name = 'MultiplicityTopo1Opt'+str(cable)+',MultiplicityMatchTopo1Opt'+str(cable)
        name += f';{topoName}_mismatch'
        title = f'Topo Optical Cable {cable} Miss/Matches Summary'
        labels = getMultiplicityLabels(flags=flags,topoModule=topoName)
        xlabels = [x for x in labels if x]
        alg.MonTool.defineHistogram(name, path='EXPERT', type='TH2F',
                                    title=title, xbins=len(xlabels), ybins=3,
                                    xlabels=xlabels,ylabels=ylabels,
                                    xmin=0, xmax=len(xlabels),
                                    ymin=0, ymax=len(ylabels))

    for cable in range(4):
        topoName = 'Topo1Opt'+str(cable)
        labels = getMultiplicityLabels(flags=flags,topoModule=topoName)
        labels = [x for x in labels if x]
        for i,label in enumerate(labels):
            name = f'Topo1Opt{cable}_{i}_Sim,Topo1Opt{cable}_{i}_Hdw;Topo1Opt{cable}_{label}'
            title = f'Topo1Opt{cable}_{label};Simulation Counts;Hardware Counts'
            alg.MonTool.defineHistogram(name, path='EXPERT', type='TH2F',
                                        title=title,xbins=10,ybins=10,
                                        xmin=0, xmax=10,
                                        ymin=0, ymax=10)

    alg.MonTool.defineHistogram('TopoSim', path='EXPERT', type='TH1I',
                                    title='Simulation Results for L1Topo', xbins=128, xlabels=label_topo_all,
                                    xmin=0, xmax=128)
    alg.MonTool.defineHistogram('TopoSim_overflows', path='EXPERT', type='TH1I',
                                    title='Overflow Simulation Results for L1Topo', xbins=128, xlabels=label_topo_all,
                                    xmin=0, xmax=128)
    alg.MonTool.defineHistogram('TopoSim_ambiguity', path='EXPERT', type='TH1I',
                                    title='Ambiguity Results for L1Topo', xbins=128, xlabels=label_topo_all,
                                    xmin=0, xmax=128)

    if doHwMonCtp:
        alg.MonTool.defineHistogram('TopoCTP', path='EXPERT', type='TH1I',
                                    title='CTP Results for L1Topo', xbins=128, xlabels=label_topo_all,
                                    xmin=0, xmax=128)

    if doComp:
        alg.MonTool.defineHistogram('SimNotHdwL1TopoResult', path='EXPERT', type='TH1I',
                                    title='L1Topo events with simulation accept and hardware fail',
                                    xbins=128, xlabels=label_topo_all,
                                    xmin=0, xmax=128)
        alg.MonTool.defineHistogram('HdwNotSimL1TopoResult', path='EXPERT', type='TH1I',
                                    title='L1Topo events with hardware accept and simulation fail',
                                    xbins=128, xlabels=label_topo_all,
                                    xmin=0, xmax=128)
        alg.MonTool.defineHistogram('Ambiguity_SimANDHdwDecisions', path='EXPERT', type='TH1I',
                                    title='L1Topo ambiguity events with simulation accept and hardware accept',
                                    xbins=128, xlabels=label_topo_all,
                                    xmin=0, xmax=128)
        alg.MonTool.defineHistogram('Ambiguity_DecisionMismatches', path='EXPERT', type='TH1I',
                                    title='L1Topo ambiguity events with mismatches between simulation and hardware',
                                    xbins=128, xlabels=label_topo_all,
                                    xmin=0, xmax=128)

    if doMultComp:                             
        ylabels = ['#frac{HdwNotSim}{Hdw}','#frac{SimNotHdw}{Sim}','#frac{HdwAndSim}{HdwOrSim}','#frac{Hdw}{Sim}']
        for topo in [(0,'2a'),(1,'2b'),(2,'3a'),(3,'3b')]:
            name = f'Phase1TopoTrigger_{topo[0]},Phase1TopoMissMatch_{topo[0]};Ph1Topo{topo[1]}'
            title = f'Phase1 Topo{topo[1]} Miss/Matches Summary'
            alg.MonTool.defineHistogram(name, path='EXPERT', type='TH2F',
                                        title=title,xbins=32,ybins=4,
                                        weight=f'Phase1TopoWeight_{topo[0]}',
                                        xlabels=label_topo_all[topo[0]*32:(topo[0]+1)*32],
                                        ylabels=ylabels,
                                        xmin=0, xmax=32,
                                        ymin=0, ymax=len(ylabels))
        ylabelsOF = ['#frac{HdwOFnotSimOF}{HdwOF}','#frac{SimOFnotHdwOF}{SimOF}','#frac{HdwOFandSimOF}{HdwOForSimOF}','#frac{HdwOF}{SimOF}']
        for topo in [(0,'2a'),(1,'2b'),(2,'3a'),(3,'3b')]:
            name_OF = f'Phase1TopoTrigger_{topo[0]},Phase1TopoMissMatch_{topo[0]};Ph1Topo{topo[1]}_overflows'
            title_OF = f'Phase1 Topo{topo[1]} Overflow Miss/Matches Summary'
            alg.MonTool.defineHistogram(name_OF, path='EXPERT', type='TH2F',
                                        title=title_OF,xbins=32,ybins=4,
                                        weight=f'Phase1TopoOFWeight_{topo[0]}',
                                        xlabels=label_topo_all[topo[0]*32:(topo[0]+1)*32],
                                        ylabels=ylabelsOF,
                                        xmin=0, xmax=32,
                                        ymin=0, ymax=len(ylabelsOF))

    if doHwMon:
        alg.MonTool.defineHistogram('HdwResults', path='EXPERT', type='TH1I',
                                    title='Hardware Results for L1Topo', xbins=128, xlabels=label_topo_all,
                                    xmin=0, xmax=128)
        alg.MonTool.defineHistogram('OverflowResults', path='EXPERT', type='TH1I',
                                    title='Overflow Results for L1Topo', xbins=128, xlabels=label_topo_all,
                                    xmin=0, xmax=128)
        rod_errors_labels = ["CT", "pc", "hc", "pe", "lm", "hm", "pt"]
        alg.MonTool.defineHistogram('ROD_Errors', path='EXPERT', type='TH1I', 
                                    title='Counts of ROD errors', xbins=len(rod_errors_labels), xlabels=rod_errors_labels, 
                                    xmin=0, xmax=len(rod_errors_labels))
        fpga_errors_labels = ["CT", "sm", "pe", "lm", "hm", "pt"]
        fpga_indexes = ["topo1fpga1", "topo1fpga0", "topo2fpga1", "topo2fpga0", "topo3fpga1", "topo3fpga0"]
        alg.MonTool.defineHistogram('FPGA_Errors, FPGA_Labels; FPGA_Errors', path='EXPERT', type='TH2I',
                                        title='Counts of FPGA errors',xbins=len(fpga_errors_labels),ybins=len(fpga_indexes),
                                        xlabels=fpga_errors_labels,
                                        ylabels=fpga_indexes,
                                        xmin=0, xmax=len(fpga_errors_labels),
                                        ymin=0, ymax=len(fpga_indexes))
        

    mon_failure_labels = ['doHwMon', 'doSimMon', 'doHwMonCTP', 'doComp', 'doMultComp']
    alg.MonTool.defineHistogram('MonitoringFailures', path='EXPERT', type='TH1F',
                                title='Counts of mon functions returning failure;;Entries',
                                xlabels=mon_failure_labels, xbins=len(mon_failure_labels),
                                xmin=0, xmax=len(mon_failure_labels))
