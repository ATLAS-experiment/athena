# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

# Default name of HitDV output
hitDVName = "HLT_HitDV"

def TrigHitDVHypoAlgCfg(flags : AthConfigFlags, name : str) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Setup the hypothesis algorithm
    theHitDVHypo = CompFactory.TrigHitDVHypoAlg(name)

    from TrigEDMConfig.TriggerEDM import recordable
    theHitDVHypo.HitDV = recordable(hitDVName)

    theHitDVHypo.isMC = flags.Input.isMC

    # monitoring
    monTool = GenericMonitoringTool(flags, "IM_MonTool"+name,
                                    HistPath = 'HitDVHypoAlg')

    monTool.defineHistogram('jet_pt',        type='TH1F', path='EXPERT', title="p_{T}^{jet} [GeV];p_{T}^{jet} [GeV];Nevents", xbins=50, xmin=0, xmax=200)
    monTool.defineHistogram('jet_eta',       type='TH1F', path='EXPERT', title="#eta^{jet};#eta^{jet};Nevents", xbins=50, xmin=-5.0, xmax=5.0)
    #
    monTool.defineHistogram('n_dvtrks',      type='TH1F', path='EXPERT', title="Nr of HitDVTrks;N HitDVTrks size;Nevents", xbins=50, xmin=0, xmax=1000)
    monTool.defineHistogram('n_dvsps',       type='TH1F', path='EXPERT', title="Nr of HitDVSPs;N HitDVSPs size;Nevents", xbins=50, xmin=0, xmax=100000)
    monTool.defineHistogram('n_jetseeds',    type='TH1F', path='EXPERT', title="Nr of Jet Seeds;N jet seeds;Nevents", xbins=25, xmin=0, xmax=25)
    monTool.defineHistogram('n_jetseedsdel', type='TH1F', path='EXPERT', title="Nr of deleted jet seeds;N jet seeds;Nevents", xbins=25, xmin=0, xmax=25)
    monTool.defineHistogram('n_spseeds',     type='TH1F', path='EXPERT', title="Nr of Ly6/Ly7 SP-doublet Seeds;N SP seeds;Nevents", xbins=25, xmin=0, xmax=25)
    monTool.defineHistogram('n_spseedsdel',  type='TH1F', path='EXPERT', title="Nr of deleted Ly6/Ly7 SP-doublet seeds;N SP seeds;Nevents", xbins=25, xmin=0, xmax=25)
    monTool.defineHistogram('average_mu',    type='TH1F', path='EXPERT', title="Average mu;Average mu;Nevents", xbins=50, xmin=0, xmax=100)
    #

    # Layer histograms for both eta bins
    for i in range(8):
        monTool.defineHistogram(f'ly{i}_spfr;eta1_ly{i}_spfr', type='TH1F', path='EXPERT',
                                title=f"Layer#{i} hit fraction (|#eta|<1);Hit fraction;Nevents",
                                xbins=50, xmin=0.0, xmax=1.0, cutmask='cutEta1')
        monTool.defineHistogram(f'ly{i}_spfr;1eta2_ly{i}_spfr', type='TH1F', path='EXPERT',
                                title=f"Layer#{i} hit fraction (1<|#eta|<2);Hit fraction;Nevents",
                                xbins=50, xmin=0.0, xmax=1.0, cutmask='cut1Eta2')

    # |eta|<1
    monTool.defineHistogram('n_qtrk;eta1_n_qtrk', type='TH1F', path='EXPERT', title="Nr of quality tracks (|#eta|<1);Nr of quality tracks;Nevents", xbins=20, xmin=0, xmax=20, cutmask='cutEta1')
    monTool.defineHistogram('bdtscore;eta1_bdtscore', type='TH1F', path='EXPERT', title="BDT score (|#eta|<1);BDT score;Nevents", xbins=50, xmin=-1.0, xmax=1.0, cutmask='cutEta1')

    # 1<|eta|<2
    monTool.defineHistogram('n_qtrk;1eta2_n_qtrk', type='TH1F', path='EXPERT', title="Nr of quality tracks (1<|#eta|<2);Nr of quality tracks;Nevents", xbins=20, xmin=0, xmax=20, cutmask='cut1Eta2')
    monTool.defineHistogram('bdtscore;1eta2_bdtscore', type='TH1F', path='EXPERT', title="BDT score (1<|#eta|<2);BDT score;Nevents", xbins=50, xmin=-1.0, xmax=1.0, cutmask='cut1Eta2')

    theHitDVHypo.MonTool = monTool
    theHitDVHypo.jFexSRJetRoI = "L1_jFexSRJetRoI"

    useNewLayerNumberScheme = False
    from TrigFastTrackFinder.TrigFastTrackFinderConfig import TrigSpacePointConversionToolCfg
    spTool = acc.popToolsAndMerge(TrigSpacePointConversionToolCfg(flags,
                                                                  UseNewLayerScheme=useNewLayerNumberScheme,
                                                                  DoPhiFiltering        = False,
                                                                  UseBeamTilt           = False,                                                                                                       ))

    theHitDVHypo.SpacePointProviderTool = spTool

    acc.addEventAlgo(theHitDVHypo)
    return acc


def TrigHitDVHypoToolFromDict( flags, chainDict ):

    log = logging.getLogger('TrigHitDVHypoTool')

    """ Use menu decoded chain dictionary to configure the tool """
    cparts = [i for i in chainDict['chainParts'] if i['signature']=='UnconventionalTracking']
    thresholds = sum([ [cpart['threshold']]*int(cpart['multiplicity']) for cpart in cparts], [])

    name = chainDict['chainName']
    from AthenaConfiguration.ComponentFactory import CompFactory
    tool = CompFactory.TrigHitDVHypoTool(name)

    # set thresholds

    strThr = ""

    thresholds = [ float(THR) for THR in thresholds]

    for THR in thresholds:
        strThr += str(THR)+", "

    log.debug("Threshold Values are: %s",strThr)

    tool.cutJetPtGeV = thresholds

    jetEta=[]
    doSPseed=[]
    effBDT=[]

    for cpart in cparts:
        if cpart['IDinfo'] =="loose":
            log.debug("Loose ID working point is set")
            jetEta.append(2.0)
            doSPseed.append(True)
            effBDT.append(0.9)
        elif cpart['IDinfo'] =="tight":
            log.debug("Tight ID working point is set")
            jetEta.append(1.0)
            doSPseed.append(False)
            effBDT.append(0.75)
        else:
            if cpart['IDinfo'] =="medium":
                log.debug("Medium ID working point is set")
            else:
                log.info("no working point specificed. setting medium working point")
            jetEta.append(2.0)
            doSPseed.append(False)
            effBDT.append(0.75)

    tool.cutJetEta = jetEta
    tool.doSPseed  = doSPseed
    tool.effBDT    = effBDT

    return tool
