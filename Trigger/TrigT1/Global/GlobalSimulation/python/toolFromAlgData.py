#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


# This module contains code to process AlgData objects in order to
# configure AlgTools that run L1Topo Algorithms.
#
# entrance point: toolFromAlgData

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

# needed to translate symbolic  names in the menu to numric values
import L1TopoHardware.L1TopoHardware as HW 

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)
from AthenaCommon.Constants import VERBOSE
logger.setLevel(VERBOSE)

import sys

do_dump = True  # flag to enable dump information to the local run directory

# writeHandleKeys specifies how the L1Topo AlgTools are interconnected
# via the event store.
writeHandleKeys = {}  


def toolFromAlgData(flags, alg_data):

    if alg_data is None:
        logger.error('Attempting to build an AlgTool from None')
        sys.exit(1)
        
    return {'INPUT': inputToolFromAlgData,
            'COUNT': countToolFromAlgData,
            'SORT': sortToolFromAlgData,
            'DECISION': decisionToolFromAlgData,
            }[alg_data.globalL1AlgType](flags, alg_data)


def klass(ad):
    return ad.menu['klass']


def fixedValue(ad, var):
    """helper function to extract values from a L1MenuAccessor object"""
    
    # Menu has entries like
    # ('MaxTob1',
    #            OrderedDict([('value',
    #              ':eEmOutputWidthSelect:'),
    #             ('position', 2)])),
    # The numeric values for "XXX: values are found in L1TopoHardware

    val = ad.menu[
        'fixedParameters'][
            'generics'][
                var][
                    'value']
    if isinstance(val, str):
        if val.startswith(':') and val.endswith(':'):
            val = getattr(HW, val[1:-1]).value
    return val
        

def variableValue(ad, var):
    """helper function to extract values from a L1MenuAccessor object"""

    vps = ad.menu['variableParameters']
    for vp in vps:
        if vp['name'] == var:
            return vp['value']

        

def countAlgValue(ad, var):
    """helper function to extract values from a L1MenuAccessor object"""

    return  ad.menu[var]


        
def inputToolFromAlgData(flags, ad):
    """Input Algs read in FEX outputs and output TOBs of various types."""

    # partial dump of an AlgData object with globalL1AlgType set to INPUT
    #
    # --
    # 'globalL1AlgType: INPUT\n'
    # 'inputs: []\n'
    # "outputs: ['cTAU']\n"
    # 'input_sns: []\n'
    # 'sn: 240\n'
    # 'numResultBits: None\n'
    # threshold: None
    # threshold_flavour: None

    # tool to run the concrete input alg
    tool = None
    out0 = ad.outputs[0]
    write_handle  = '_'.join([ad.klass, out0, str(ad.sn)])
    writeHandleKeys[ad.sn] =write_handle

    # The following if - elseif structure specifies the Input Algorithms
    # currently implemented
    if out0 == 'jJetTobs':
        name = 'jJetInputAlgTool_'+ out0
        tool = CompFactory.GlobalSim.jJetInputAlgTool(name)

    elif  out0 == 'eEmTobs':
        name = 'eEmInputAlgTool_'+ out0
        tool = CompFactory.GlobalSim.eEmInputAlgTool(name)
    elif out0 == 'cTAU':
        name = 'cTauInputAlgTool_' + out0
        tool =  CompFactory.GlobalSim.cTauInputAlgTool(name)
    elif out0 == 'jXE':
        name = 'jXEInputAlgTool_' + out0
        tool =  CompFactory.GlobalSim.jXEInputAlgTool(name)
        monTool = GenericMonitoringTool(flags, "MonTool_" + ad.name)

        monTool.defineHistogram('jXETOBPt', path='EXPERT', type='TH1I',
                                title='jXE TOB Pt;p_{T} [GeV];',
                                xbins=200, xmin=0, xmax=2000)
        
        monTool.defineHistogram('jXETOBPhi', path='EXPERT', type='TH1I',
                                title='jXE TOB Phi;#phi;',
                                xbins=64, xmin=-3.2, xmax=3.2)

        tool.monTool = monTool
    else:
        logger.error('Unsupported input alg type ' + str(out0))
        raise RuntimeError(
            'toolFromAlgData: Unsupported input alg type ' + str(out0))

    tool.TOBArrayWriteKey = writeHandleKeys[ad.sn]
    return tool

def countToolFromAlgData(flags, ad):
    """Count Algorithms write out a Count object determined
    from TOBS produced by a single Input Algorithm."""

    # The following dictionary specifies the Count Algorithms currently
    # implemented
    countToolSelector = {
        'cTauMultiplicity': CompFactory.GlobalSim.cTauMultiplicityAlgTool,
        ('EnergyThreshold', 'jXE'): CompFactory.GlobalSim.EnergyThresholdAlgTool_jXE,
    }
    
    # Note: the name of the owning component
    #  will be prepended to the AlgTool name
    key = None
    if ad.klass == 'EnergyThreshold':
        key = (ad.klass, ad.inputs[0]) # eg ('EnergyThreshold', 'jXE')

    else:
        key = ad.klass
        
    tool = countToolSelector[key](ad.name)
    tool.alg_instance_name = ad.name
    tool.nbits = int(countAlgValue(ad, 'nbits'))
    tool.TOBArrayReadKey = writeHandleKeys[ad.input_sns[0]]
    tool.CountWriteKey = ad.name + '_Count_' + str(ad.sn) 
    writeHandleKeys[ad.sn] = tool.CountWriteKey
    
    if ad.klass in ('EnergyThreshold',):

        # magic numbers from the original C++ calculation
        tool.hundredMeVThreshold = int(ad.threshold)*10 + 5

    if ad.klass == 'cTauMultiplicity':
        tool.do_dump = do_dump
        monTool = GenericMonitoringTool(flags, "MonTool_" + ad.name)

        threshold_name = countAlgValue(ad, 'threshold')
        title = "cTauMultiplicity_accept #eta Et_" + threshold_name

        monTool.defineHistogram("accept_eta,accept_et",
                                path="EXPERT",
                                type="TH2F",
                                title=title,
                                xbins=200, xmin=-200, xmax=200,
                                ybins=100, ymin=0, ymax=100)

        title_stub =  "cTauMultiplicityc "  + threshold_name + " "
        title = title_stub + "counts"
        monTool.defineHistogram("counts",
                                path="EXPERT",
                                type="TH1F",
                                title=title,
                                xbins=15, xmin=0, xmax=15)

        title = title_stub + "TOB Et"
        monTool.defineHistogram("Et",
                                path="EXPERT",
                                type="TH1F",
                                title=title,
                                xbins=200, xmin=0, xmax=400)

        
        title =  title_stub + "#phi #eta"
        monTool.defineHistogram("phi,eta",
                                path="EXPERT",
                                type="TH2F",
                                title=title,
                                xbins=200, xmin=0, xmax=400,
                                ybins=128, ymin=0, ymax=128)

     
        title =  title_stub + "Et #eta"
        monTool.defineHistogram("Et,eta",
                                path="EXPERT",
                                type="TH2F",
                                title=title,
                                xbins=200, xmin=0, xmax=200,
                                ybins=200, ymin=0, ymax=400)

        title =  title_stub + "loose partial isolation"
        monTool.defineHistogram("iso_loose",
                                path="EXPERT",
                                type="TH1F",
                                title=title,
                                xbins=200, xmin=0, xmax=10)

        title =  title_stub + "medium partial isolation"
        monTool.defineHistogram("iso_medium",
                                path="EXPERT",
                                type="TH1F",
                                title=title,
                                xbins=200, xmin=0, xmax=10)


        title =  title_stub + "tight partial isolation"
        monTool.defineHistogram("iso_tight",
                                path="EXPERT",
                                type="TH1F",
                                title=title,
                                xbins=200, xmin=0, xmax=10)

        tool.monTool = monTool

    return tool

def sortToolFromAlgData(flags, ad):
    """Sort Algorithms write out a GenericTOBArray produced by sorting,
    and possibly selecting, TOBs produced by an Input Algorithm"""


    # The following dictionary specifies the Sort Algorithms currently
    # implemented
    sortToolSelector = {
        'eEmSelect': CompFactory.GlobalSim.eEmSelectAlgTool,
        'jJetSelect': CompFactory.GlobalSim.jJetSelectAlgTool,
    }

    
    # Note: owning component name will be prepended to the AlgTool name
    tool = sortToolSelector[ad.klass](ad.name)
    out0 = ad.outputs[0]  # node number (int)
    tool.alg_instance_name = ad.name
    tool.TOBArrayReadKey = writeHandleKeys[ad.input_sns[0]]
    writeHandleKeys[ad.sn] = ad.klass+ '_' + out0 + '_' + str(ad.sn)
    tool.TOBArrayWriteKey = writeHandleKeys[ad.sn]
    if klass(ad) == 'eEmSelect':
        tool.InputWidth = int(fixedValue(ad, 'InputWidth'))
        tool.MinET = int(variableValue(ad, 'MinET'))
        tool.REtaMin = int(variableValue(ad,'REtaMin'))
        tool.RHadMin = int(variableValue(ad, 'RHadMin'))
        tool.WsTotMin = int(variableValue(ad, 'WsTotMin'))

    if klass(ad) == 'jJetSelect':
        tool.InputWidth = int(fixedValue(ad, 'InputWidth'))
        tool.MinET = int(variableValue(ad, 'MinET'))
        tool.MinEta = int(variableValue(ad,'MinEta'))
        tool.MaxEta = int(variableValue(ad,'MaxEta'))
    
    # tool.sortAlgTool = sortAlgTool

    return tool

def decisionToolFromAlgData(flags, ad):
    """Decision Algorithms all write out a L1Topo Decision object
    determined by processing the outputs of 1 or more Sort Algorithms.
    They also write out a vector of GenericTOBs."""

    
    # The following dictionary specifies the Decision Algorithms currently
    # implemented
 
    decisionToolSelector = {
        'DeltaRSqrIncl2': CompFactory.GlobalSim.DeltaRSqrIncl2AlgTool,
        'SimpleCone': CompFactory.GlobalSim.SimpleConeAlgTool,
    }

    tool = decisionToolSelector[ad.klass](ad.name)
    tool.alg_instance_name = ad.name

    assert len(ad.outputs) == 1
    writeHandleKeys[ad.sn] = ad.klass + '_'+ ad.outputs[0] + '_' + str(ad.sn)
    tool.TOBArrayVectorWriteKey = writeHandleKeys[ad.sn]
    tool.DecisionWriteKey =  ad.name + '_Decision_' + str(ad.sn)

    if klass(ad) == 'DeltaRSqrIncl2':
        tool.TOBArrayReadKey0 = writeHandleKeys[ad.input_sns[0]]
        tool.TOBArrayReadKey1 = writeHandleKeys[ad.input_sns[1]]

        tool.MaxTOB1 = int(fixedValue(ad, 'InputWidth1')) 
        tool.MaxTOB2 = int(fixedValue(ad, 'InputWidth2'))
        tool.NumResultBits = fixedValue(ad, 'NumResultBits')

        tool.MinET1 = [int(variableValue(ad, 'MinET1'))]
        tool.MinET2 = [int(variableValue(ad, 'MinET2'))]
        tool.DeltaRMin = [int(variableValue(ad, 'DeltaRMin'))]
        tool.DeltaRMax = [int(variableValue(ad, 'DeltaRMax'))]

        # unclear how the menu stores cut values when there is more
        # than one result bit. Examined menus all have 1 for this variable
        assert tool.NumResultBits == 1

        tool.NumResultBits = int(fixedValue(ad, 'NumResultBits'))

        monTool = GenericMonitoringTool(flags, "MonTool_" + ad.name)
        for i in range(tool.NumResultBits):

            label = ad.name + "_pass_by_bit_" + str(i)
            monTool.defineHistogram(label,
                                    path="EXPERT", type="TH1F",
                                    title="DeltaR pass, bit " + str(i),
                                    xbins=10, xmin=0, xmax=100.)
            
            label = ad.name + "_fail_by_bit_" + str(i)
            monTool.defineHistogram(label,
                                    path="EXPERT", type="TH1F",
                                    title="DeltaR fail, bit " + str(i),
                                    xbins=10, xmin=0, xmax=100.)
        tool.monTool = monTool
        tool.do_dump = do_dump

 

    if klass(ad) == 'SimpleCone':

        tool.TOBArrayReadKey = writeHandleKeys[ad.input_sns[0]]

        tool.InputWidth = int(fixedValue(ad, 'InputWidth'))
        tool.MaxRSqr = int(variableValue(ad, 'MaxRSqr'))
        tool.MinET = int(variableValue(ad, 'MinET'))

        # in some previous version of the menu NumResultBits was set to 1.
        # not sure how the list of cuts is specified if NumResultBits > 1
        # now hardwire NumResultBits=1
        numResultBits = 1

        
        tool.MinSumET = [int(variableValue(ad, 'MinSumET'))]
        
   
        monTool = GenericMonitoringTool(flags, "MonTool_" + ad.name)
        for i in range(numResultBits):
            monTool.defineHistogram(ad.name+"_pass_by_bit_" + str(i),
                                    path="EXPERT", type="TH1F",
                                    title="ET pass, bit " + str(i),
                                    xbins=10, xmin=0, xmax=100.)
            
            monTool.defineHistogram(ad.name+"_fail_by_bit_" + str(i),
                                    path="EXPERT", type="TH1F",
                                    title="ET fail, bit " + str(i),
                                    xbins=10, xmin=0, xmax=100.)
        tool.monTool = monTool

    
    return tool

