# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
from AthenaConfiguration.Enums import Format
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from TrigConfIO.L1TriggerConfigAccess import L1MenuAccess
from TrigConfigSvc.TrigConfigSvcCfg   import getL1MenuFileName
from libpyeformat_helper import SourceIdentifier, SubDetector

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
    lvl1name = getL1MenuFileName(flags)
    lvl1access  = L1MenuAccess(lvl1name)
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
    lvl1name = getL1MenuFileName(flags)
    lvl1access  = L1MenuAccess(lvl1name)
    topo_triglines_dict = lvl1access.connector(topoModule)['triggerlines']

    for topo_trigline in topo_triglines_dict:
        topo_trigline_name = topo_trigline['name']
        bit_id = topo_trigline['startbit']
        topo_trigline_labels[bit_id] = topo_trigline_name
    
    return topo_trigline_labels


def getL1TopoPhase1OnlineMonitor(flags, name='L1TopoOnlineMonitor', doSimMon=True, doHwMonCtp=False, doHwMon=False, doComp=False, doMultComp=False, forceCtp=False, logLevel = None):
    # Placeholder for phase-1 implementation
    #raise RuntimeError('L1Topo phase-1 online monitoring not yet implemented')
    alg = CompFactory.L1TopoOnlineMonitor("L1TopoMonitoringTool",
                                          doHwMon = doHwMon,
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


        
def getL1TopoLegacyOnlineMonitor(flags, name='L1TopoLegacyOnlineMonitor', configBS = True, logLevel = None):
    alg = CompFactory.L1TopoLegacyOnlineMonitor()
    if logLevel : alg.OutputLevel=logLevel
    alg.MonTool = GenericMonitoringTool(flags, 'MonTool')
    alg.MonTool.HistPath = name
    configureLegacyHistograms(alg, flags)

    # Disable ByteStream monitoring when running on MC
    alg.doRawMon = not flags.Input.isMC
    alg.doCnvMon = not flags.Input.isMC

    def getAlgProp(prop):
        return getattr(alg, prop) if hasattr(alg, prop) \
               else alg.getDefaultProperty(prop)

    # Commented out because TrigEDMConfig.TriggerEDMRun3.recordable() currently enforces
    # that all names start with HLT_ which doesn't make sense for L1TopoErrorFlags
    # alg.ErrorFlagsKey = recordable(str(getAlgProp('ErrorFlagsKey')))

    if getAlgProp('PrescaleDAQROBAccess') >= 1 and flags.Input.Format is Format.BS and configBS:
        # Add BS converter for DAQ L1Topo RDO collection
        from L1TopoByteStream.L1TopoByteStreamConfig import L1TopoRDOCollectionBSCnvCfg
        CAtoGlobalWrapper(L1TopoRDOCollectionBSCnvCfg, flags)
        if getAlgProp('doSimMon') and not flags.Trigger.doLVL1:
            # Add BS converter for CTP_RDO
            from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
            CAtoGlobalWrapper(ByteStreamReadCfg, flags, type_names=['CTP_RDO/CTP_RDO'])

    return alg

def configureLegacyHistograms(alg, flags):
    '''All histogram definitions for L1TopoLegacyOnlineMonitor'''
    # ==========================================================================
    rob_src_id_labels = []
    module_ids = []
    module_ids += alg.DAQROBModuleIds
    module_ids += alg.RoIBROBModuleIds
    for module_id in module_ids:
        sid = SourceIdentifier(SubDetector.TDAQ_CALO_TOPO_PROC, module_id)
        rob_src_id_labels.append('{:#010x}'.format(sid.code()))

    alg.MonTool.defineHistogram('ROB_src_IDs_fromROB', path='EXPERT', type='TH1F',
                                title='L1Topo ROB source IDs received direct from ROBs;ROB ID;Entries',
                                xlabels=rob_src_id_labels, xbins=len(rob_src_id_labels),
                                xmin=0, xmax=len(rob_src_id_labels))
    alg.MonTool.defineHistogram('ROB_src_IDs_fromCnv', path='EXPERT', type='TH1F',
                                title='L1Topo ROB source IDs received via converters;ROB ID;Entries',
                                xlabels=rob_src_id_labels, xbins=len(rob_src_id_labels),
                                xmin=0, xmax=len(rob_src_id_labels))
    # ==========================================================================
    tob_type_labels = ['EM','TAU','MU','0x3','JETc1','JETc2','ENERGY','0x7',
                       'L1TOPO','0x9','0xa','0xb','HEADER','FIBRE','STATUS','0xf']
    alg.MonTool.defineHistogram('TOBtype_fromROIROB', path='EXPERT', type='TH1F',
                                title='4-bit TOB type via ROI ROB;TOB type;Entries',
                                xlabels=tob_type_labels, xbins=len(tob_type_labels),
                                xmin=0, xmax=len(tob_type_labels))
    alg.MonTool.defineHistogram('TOBtype_fromDAQROB', path='EXPERT', type='TH1F',
                                title='4-bit TOB type via DAQ ROB;TOB type;Entries',
                                xlabels=tob_type_labels, xbins=len(tob_type_labels),
                                xmin=0, xmax=len(tob_type_labels))
    # ==========================================================================
    alg.MonTool.defineHistogram('ROI_ROB_payload_size', path='EXPERT', type='TH1F',
                                title='L1Topo ROI ROB payload size;number of words;Entries',
                                xbins=300, xmin=0, xmax=300)
    alg.MonTool.defineHistogram('DAQ_ROB_payload_size', path='EXPERT', type='TH1F',
                                title='L1Topo DAQ ROB payload size;number of words;Entries',
                                xbins=300, xmin=0, xmax=300)
    # ==========================================================================
    alg.MonTool.defineHistogram('CTP_sig_part_fromROICnv', path='EXPERT', type='TH1F',
                                title='4-bit CTP signal part from ROI via converter;CTP signal part;Entries',
                                xbins=16, xmin=0, xmax=16)
    # ==========================================================================
    input_link_crc_labels = ['EM', 'Tau', 'Muon', 'Jet', 'Energy']
    alg.MonTool.defineHistogram('InputLinkCRCs', path='EXPERT', type='TH1F',
                                title='CRC flags for input links, from ROI via converter;;Entries',
                                xlabels=input_link_crc_labels, xbins=len(input_link_crc_labels),
                                xmin=0, xmax=len(input_link_crc_labels))
    # ==========================================================================
    topo_trigline_labels = getL1TopoLabels(flags)

    def defineBitsHistogram(var, title):
        alg.MonTool.defineHistogram(var, path='EXPERT', type='TH1F', title=title+';;Entries',
                                    xlabels=topo_trigline_labels, xbins=len(topo_trigline_labels),
                                    xmin=0, xmax=len(topo_trigline_labels))

    defineBitsHistogram('CTP_Trigger_fromROICnv', 'L1Topo CTP signal trigger bits from ROI via converter')
    defineBitsHistogram('CTP_Overflow_fromROICnv', 'L1Topo CTP signal overflow bits from ROI via converter')
    defineBitsHistogram('SimResults', 'L1Topo simulation accepts, events with no overflows')
    defineBitsHistogram('HdwResults', 'L1Topo hardware accepts, events with no overflows')
    defineBitsHistogram('SimNotHdwResult', 'L1Topo events with simulation accept and hardware fail, events with no overflows')
    defineBitsHistogram('HdwNotSimResult', 'L1Topo events with hardware accept and simulation fail, events with no overflows')
    ylabels = ['#frac{HdwNotSim}{Hdw}','#frac{SimNotHdw}{Sim}','#frac{HdwAndSim}{HdwOrSim}','#frac{Hdw}{Sim}']
    for topo in [(0,'1a'),(1,'1b'),(2,'2a'),(3,'2b')]:
        name = f'LegacyTopoTrigger_{topo[0]},LegacyTopoMissMatch_{topo[0]};LegacyTopo{topo[1]}'
        title = f'Legacy Topo{topo[1]} Miss/Matches Summary'
        alg.MonTool.defineHistogram(name, path='EXPERT', type='TH2F',
                                    title=title,xbins=32,ybins=4,
                                    weight=f'LegacyTopoWeight_{topo[0]}',
                                    xlabels=topo_trigline_labels[topo[0]*32:(topo[0]+1)*32],
                                    ylabels=ylabels,
                                    xmin=0, xmax=32,
                                    ymin=0, ymax=len(ylabels))
        
    defineBitsHistogram('Hdw_vs_Sim_Events', 'L1Topo decisions hardware XOR simulation event-by-event differences, events with no overflows')
    defineBitsHistogram('SimDaqRobResults', 'L1Topo simulation accepts, events with no overflows (DAQ ROB)')
    defineBitsHistogram('HdwDaqRobResults', 'L1Topo hardware accepts, events with no overflows (DAQ ROB)')
    defineBitsHistogram('SimNotHdwDaqRobResult', 'L1Topo events with simulation accept and hardware fail, events with no overflows (DAQ ROB)')
    defineBitsHistogram('HdwNotSimDaqRobResult', 'L1Topo events with hardware accept and simulation fail, events with no overflows (DAQ ROB)')
    defineBitsHistogram('SimOverflows', 'L1Topo simulation overflows')
    defineBitsHistogram('HdwOverflows', 'L1Topo hardware overflows')
    defineBitsHistogram('SimNotHdwOverflow', 'L1Topo events with overflow simulation=1 and hardware=0')
    defineBitsHistogram('HdwNotSimOverflow', 'L1Topo events with overflow hardware=1 and simulation=0')
    defineBitsHistogram('Hdw_vs_Sim_EventOverflow', 'L1Topo overflow hardware XOR simulation event-by-event differences')
    defineBitsHistogram('DAQ_ROB_Hdw_vs_L1Topo_Hdw_Events', 'L1Topo decisions hardware XOR DAQ ROB hardware event-by-event differences')
    defineBitsHistogram('DAQ_ROB_Hdw_vs_CTP_Hdw_Events', 'L1Topo DAQ ROB hardware XOR CTP TIP hardware event-by-event differences')
    defineBitsHistogram('CTP_Hdw_vs_L1Topo_Hdw_Events', 'L1Topo decisions hardware (trigger|overflow) XOR CTP TIP hardware event-by-event differences, events with no overflows')
    defineBitsHistogram('CTP_Hdw_vs_Sim_Events', 'L1Topo decisions CTP hardware XOR simulation event-by-event differences, events with no overflows')
    # ==========================================================================
    alg.MonTool.defineHistogram('CRC_fromDAQCnv', path='EXPERT', type='TH1F',
                                title='non zero payload CRCs via converter;Payload CRC;Entries',
                                xbins=256, xmin=0, xmax=256)  # CRC is 8 bits
    # ==========================================================================
    alg.MonTool.defineHistogram('NonZero_FibreStatus_fromDAQCnv', path='EXPERT', type='TH1F',
                                title='L1Topo Non-zero Fibre status flags from DAQ via converter;fibre status flags;Entries',
                                xbins=85, xmin=0, xmax=85, opt='kVec')
    # ==========================================================================
    alg.MonTool.defineHistogram('DAQ_ROB_rel_bx_fromCnv', path='EXPERT', type='TH1F',
                                title='L1Topo DAQ ROB relative bunch crossings sent via converter;relative bunch crossing;Entries',
                                xbins=10, xmin=-5, xmax=5)
    # ==========================================================================
    problem_labels = ['ROI no RDO', 'ROI converter error', 'ROI payload empty', 'ROI invalid TOB type',
                      'DAQ no RDO', 'DAC collection empty', 'DAQ payload empty', 'DAQ invalid TOB type',
                      'Fibre overflow', 'Fibre CRC']
    alg.MonTool.defineHistogram('Problems', path='EXPERT', type='TH1F',
                                title='Counts of various problems;;Entries',
                                xlabels=problem_labels, xbins=len(problem_labels),
                                xmin=0, xmax=len(problem_labels))
    # ==========================================================================
    mon_failure_labels = ['doRawMon', 'doCnvMon', 'doSimMon', 'doOverflowSimMon', 'doSimDaq']
    alg.MonTool.defineHistogram('MonitoringFailures', path='EXPERT', type='TH1F',
                                title='Counts of mon functions returning failure;;Entries',
                                xlabels=mon_failure_labels, xbins=len(mon_failure_labels),
                                xmin=0, xmax=len(mon_failure_labels))
    # ==========================================================================
