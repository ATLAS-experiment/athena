# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhysValJetToolCfg(flags, name="PhysValJetTool", **kwargs):
    '''Following the logic defined for JetMonitoring JetMonitoringStandard.py but adding
    collections that were present in the old PhysVal config''' 
    acc = ComponentAccumulator()

    # create a list of JetMonitoringAlg specifications
    jetcollections = [
                        "AntiKt4LCTopoJets",
                        "AntiKt4EMTopoJets",
                        "AntiKt4EMPFlowJets",
                        "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                        "AntiKtVR30Rmax4Rmin02PV0TrackJets",
                    ]
    
    if flags.Input.isMC:
        jetcollections +=[
            "AntiKt4TruthJets",
            "AntiKt10TruthSoftDropBeta100Zcut10Jets",
        ]
    
    fillers = []
    for col in jetcollections:
        truthJetCollection = ''
        if flags.Input.isMC and 'Truth' not in col:
            if col == 'AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets':
                truthJetCollection = 'AntiKt10TruthTrimmedPtFrac5SmallR20Jets'
            elif col == 'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets':
                truthJetCollection = 'AntiKt10TruthSoftDropBeta100Zcut10Jets'
            elif 'AntiKt4' in col:
                truthJetCollection = 'AntiKt4TruthJets'

        fillers += [ acc.popToolsAndMerge(JetMonToolCfg(flags, JetContainer=col,
                                                        refcontainer=truthJetCollection)) ]

    kwargs.setdefault("HistoTools", fillers)
    kwargs.setdefault("IntervalType", 8)
    acc.setPrivateTools(CompFactory.JetMonitoringTool(name, **kwargs))

    return acc

def JetMonToolCfg(flags, name="HistoFiller",
                  refcontainer='', onlyKinematics=False, globalSelection='',
                  **kwargs):
    acc = ComponentAccumulator()
    filler = CompFactory.JetContainerHistoFiller(kwargs["JetContainer"]+name, **kwargs)

    if globalSelection !='':
        print("WARNING global selection is not yet supported in CA, returning plots with no selection.")

    filler.HistoTools = [ 
       acc.popToolsAndMerge(JetKinematicHistosCfg(flags, 'kinematics',
                                                  PlotOccupancy=False,
                                                  PlotAveragePt=False,
                                                  PlotNJet=True))
    ]
  
    if onlyKinematics: #TODO add JetValidation flags for these
        acc.setPrivateTools(filler)
        return acc

    filler.HistoTools += [
        acc.popToolsAndMerge(JetHistogramsAndSelectionCfg(flags, selection="leadingjet", histos=["basickinematics"])),
        acc.popToolsAndMerge(JetHistogramsAndSelectionCfg(flags, selection="subleadingjet", histos=["basickinematics"]))
    ]


    from JetValidation.JetValidationHistoDefs import GetJetVariables
    vars = GetJetVariables(kwargs["JetContainer"], refcontainer)

    for var in vars:
        if "kinematics" in var:
            tool = acc.popToolsAndMerge(JetKinematicHistosCfg(flags, var))
        elif "leadingjettrel" in var:
            tool = acc.popToolsAndMerge(LeadingJetsRelationsCfg(flags, var))
        elif "effresponse" in var:
            tool = acc.popToolsAndMerge(JetEfficiencyResponseHistosCfg(flags, var, RefContainer=refcontainer))
        else:
            from JetMonitoring.JetHistoTools import compactSpecification
            spec = compactSpecification[var]
            if len(spec) == 2:
                binning, attributeInfo = spec
                tool = acc.popToolsAndMerge(Create1DHistoToolCfg(flags, var,
                                                                 binning, attributeInfo))
            if len(spec) == 3:
                binning, attributeInfo1, attributeInfo2 = spec
                doTProfile = var.beginswith("Prof_")
                tool = acc.popToolsAndMerge(Create2DHistoToolCfg(flags, var,
                                                                 binning, attributeInfo1, attributeInfo2,
                                                                 DoTProfile=doTProfile))

        filler.HistoTools += [ tool ]

    acc.setPrivateTools(filler)
    return acc

def JetKinematicHistosCfg(flags, name, **kwargs):
    acc = ComponentAccumulator()

    if "emscale" in name:
        kwargs.setdefault("JetScale", "JetEMScaleMomentum")
    elif "constscale" in name:
        kwargs.setdefault("JetScale", "JetConstitScaleMomentum")

    acc.setPrivateTools(CompFactory.JetKinematicHistos(name, **kwargs))

    return acc

def JetHistogramsAndSelectionCfg(flags, name="hjsel",
                                 selection="alljet", histos=[],
                                 **kwargs):
    acc = ComponentAccumulator()

    name_tool = name + "_" + selection

    if "alljet" == selection:
        kwargs.setdefault("SelectionType", 0)
    elif "leadingjet" == selection:
        kwargs.setdefault("SelectionType", 1)
    elif "subleadingjet" == selection:
        kwargs.setdefault("SelectionType", 2)

    else:
        kwargs.setdefault("SelectionType", 3)
        selTool = acc.popToolsAndMerge(AddSelectorCfg(flags, selectString=selection))
        name_tool = name + "_" + selTool.name
        kwargs.setdefault("JetSelectorTool", selTool)
        kwargs.setdefault("HistoTitleSuffix", '('+selection+')')
        kwargs.setdefault("HistoNameSuffix", selTool.name)

    histotools = []
    for histo in histos:
        if "kinematics" in histo:
            tool = acc.popToolsAndMerge(JetKinematicHistosCfg(flags, histo))
        else:
            tool = acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, histo))
        histotools += [ tool ]
    kwargs.setdefault("HistoTools", histotools)

    acc.setPrivateTools(CompFactory.HistosForJetSelection(name_tool, **kwargs))
    return acc


def JetEfficiencyResponseHistosCfg(flags, name, **kwargs):
    acc = ComponentAccumulator()

    tool = CompFactory.EfficiencyResponseHistos(name, **kwargs)

    tool.HistoDef = [
        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='erhEfficiencyR1',
                                                   title="Jet p_{T} Efficiency #DeltaR = 0.1;p_{T}^{Truth} (GeV);Efficiency",
                                                   nbinsx=50, xlow=0, xup=100)),
        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='erhEfficiencyR2',
                                                   title="Jet p_{T} Efficiency #DeltaR = 0.2;p_{T}^{Truth} (GeV);Efficiency",
                                                   nbinsx=50, xlow=0, xup=100)),
        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='erhEfficiencyR3',
                                                   title="Jet p_{T} Efficiency #DeltaR = 0.3;p_{T}^{Truth} (GeV);Efficiency",
                                                   nbinsx=50, xlow=0, xup=100)),

        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='erhResponse',
                                                   title="Jet p_{T} Response;#frac{p_{T}^{Jet} - p_{T}^{Truth}}{p_{T}^{Truth}};Number of jets",
                                                   nbinsx=50, xlow=-1, xup=1)),
        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='erhResponseVsEta',
                                                   title="Jet p_{T} Response vs #eta;#eta of jet;#frac{p_{T}^{Jet} - p_{T}^{Truth}}{p_{T}^{Truth}}",
                                                   nbinsx=50, xlow=-5, xup=5)),
        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='erhResponseVsPt',
                                                   title="Jet p_{T} Response vs p_{T};p_{T}^{Truth} of jet;#frac{p_{T}^{Jet} - p_{T}^{Truth}}{p_{T}^{Truth}}",
                                                   nbinsx=50, xlow=0, xup=1000)),

        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='erhResponse_noShift',
                                                   title="Jet p_{T} Response;#frac{p_{T}^{Jet}}{p_{T}^{Truth}};Number of jets",
                                                   nbinsx=50, xlow=0, xup=2)),
        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='erhResponseVsEta_noShift',
                                                   title="Jet p_{T} Response vs #eta;#eta of jet;#frac{p_{T}^{Jet}}{p_{T}^{Truth}}",
                                                   nbinsx=50, xlow=-5, xup=5)),
        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='erhResponseVsPt_noShift',
                                                   title="Jet p_{T} Response vs p_{T};p_{T}^{Truth} of jet;#frac{p_{T}^{Jet}}{p_{T}^{Truth}}",
                                                   nbinsx=50, xlow=0, xup=1000)),

        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='erhDeltaR',
                                                   title="#DeltaR between Jet and closest Truth Jet;#DeltaR;Number of jets",
                                                   nbinsx=50, xlow=0, xup=4)),
    ]

    acc.setPrivateTools(tool)
    return acc

def LeadingJetsRelationsCfg(flags, name="leadingjetrel", **kwargs):
    acc = ComponentAccumulator()

    tool = CompFactory.LeadingJetsRelations(name, **kwargs)

    tool.HistoDef = [
        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='ljrDeltaEta',
                                                   title="#Delta #eta (lead, sublead);#Delta#eta;Entries",
                                                   nbinsx=100, xlow=-10, xup=10)),
        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='ljrDeltaPhi',
                                                   title="#Delta #Phi (lead, sublead);#Delta#Phi;Entries",
                                                   nbinsx=100, xlow=0, xup=3.142)),
        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='ljrDeltaR',
                                                   title="#Delta R (lead, sublead);#Delta R;Entries",
                                                   nbinsx=100, xlow=0, xup=10)),
        acc.popToolsAndMerge(CreateHistoDefToolCfg(flags, name='ljrFrac',
                                                   title="(sublead Pt)/(lead Pt);ratio;Entries",
                                                   nbinsx=100, xlow=0, xup=1.))
    ]

    acc.setPrivateTools(tool)
    return acc

def CreateHistoDefToolCfg(flags, name, **kwargs):
    """Short cut to return a HistoDefinitionTool from a compact list of arguments"""   
    acc = ComponentAccumulator()

    kwargs.setdefault("title", name)
    kwargs.setdefault("hname", name)

    # All of those are default which can be overriden from config call
    kwargs.setdefault("nbinsx", 10)
    kwargs.setdefault("xlow", 10.0)
    kwargs.setdefault("xup", 1.0)
    kwargs.setdefault("nbinsy", 10)
    kwargs.setdefault("ylow", 0.0)
    kwargs.setdefault("yup", 1.0)

    name = "hdef_"+name # athena can causes conflicts when tools of different types have same names
    acc.setPrivateTools(CompFactory.HistoDefinitionTool(name, **kwargs))
    return acc

def Create1DHistoToolCfg(flags, name,
                         binning=None, attributeInfo=None,
                         **kwargs):
    acc = ComponentAccumulator()

    from JetMonitoring.JetAttributeHistoManager import unpackto3, findSelectIndex, sanitizeName
    attName, attType, attGeV = unpackto3(attributeInfo)
    attName, selectIndex = findSelectIndex(attName) # 'JVF[1]' --> 'JVF', 1

    #hname = name if selectIndex==-1 else (name+'_'+str(selectIndex))
    hname = sanitizeName(name) # remove [ and ] which can be problematic in histo names

    kwargs.setdefault("AttributeTypes", [ attType ])
    kwargs.setdefault("AttributeNames", [ attName ])
    kwargs.setdefault("AttributeInGeV", [ bool(attGeV) ])
    kwargs.setdefault("SelectIndex", selectIndex)

    bin_args = {}
    bin_args["title"]  = binning[0]
    bin_args["nbinsx"] = binning[1]
    bin_args["xlow"]   = binning[2]
    bin_args["xup"]    = binning[3]
    kwargs.setdefault("HistoDef", acc.popToolsAndMerge(
        CreateHistoDefToolCfg(flags, hname, **bin_args)))

    acc.setPrivateTools(CompFactory.JetAttributeHisto(name, **kwargs))
    return acc

def Create2DHistoToolCfg(flags, name,
                         binning=None, attributeInfo1=None, attributeInfo2=None,
                         **kwargs):
    acc = ComponentAccumulator()

    from JetMonitoring.JetAttributeHistoManager import unpackto3, findSelectIndex, sanitizeName
    attName1, attType1, attGeV1 = unpackto3(attributeInfo1)
    attName1, selectIndex1 = findSelectIndex(attName1)

    attName2, attType2, attGeV2 = unpackto3(attributeInfo2)
    attName2, selectIndex2 = findSelectIndex(attName2)

    # currently support only vector<float> vs float, so there can be only one selected index.
    selectIndex = max ( selectIndex1, selectIndex2)

    #hname = name if selectIndex==-1 else (name+'_'+str(selectIndex))
    hname = sanitizeName(name) # remove [ and ] which can be problematic in histo names

    kwargs.setdefault("AttributeTypes", [ attType1, attType2 ])
    kwargs.setdefault("AttributeNames", [ attName1, attName2 ])
    kwargs.setdefault("AttributeInGeV", [ bool(attGeV1), bool(attGeV2) ])
    kwargs.setdefault("SelectIndex", selectIndex)

    bin_args = {}
    bin_args["title"]  = binning[0]
    bin_args["nbinsx"] = binning[1]
    bin_args["xlow"]   = binning[2]
    bin_args["xup"]    = binning[3]
    bin_args["nbinsy"] = binning[4]
    bin_args["ylow"]   = binning[5]
    bin_args["yup"]    = binning[6]
    kwargs.setdefault("HistoDef", acc.popToolsAndMerge(
        CreateHistoDefToolCfg(flags, hname, **bin_args)))
    
    acc.setPrivateTools(CompFactory.JetAttributeHisto(name, **kwargs))
    return acc

def AddSelectorCfg(flags, name="", selectString="", typ="float"):
    acc = ComponentAccumulator()

    from JetMonitoring.JetAttributeHistoManager import interpretSelStr,findSelectIndex
    cmin, att, cmax = interpretSelStr(selectString)
    att, ind = findSelectIndex(att)
    if ind>-1 and 'vector' not in typ :
        typ = 'vector<'+typ+'>'

    if name == "":
        # try to build a unique name
        name = selectString.replace('<','_inf_')
        name = name.replace('[','_')
        name = name.replace(']','_')
        name = name.replace('.','_')
        name = 'sel_'+name

    tool = CompFactory.JetSelectorAttributeRunII(name, Attribute=att, AttributeType=typ, VectorIndex=ind)
    if cmin is not None: tool.CutMin = cmin
    if cmax is not None: tool.CutMax = cmax
    acc.setPrivateTools(tool)
    return acc

