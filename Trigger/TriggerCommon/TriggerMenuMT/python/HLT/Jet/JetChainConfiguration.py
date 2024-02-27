# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import re
from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from ..Config.ChainConfigurationBase import ChainConfigurationBase
from ..Config.MenuComponents import ChainStep

from .JetMenuSequencesConfig import (
    jetCaloHypoMenuSequence,
    jetRoITrackJetTagHypoMenuSequence,
    jetFSTrackingHypoMenuSequence,
    jetCaloRecoMenuSequence, 
    jetCaloPreselMenuSequence,
)
from .ExoticJetSequencesConfig import jetEJsMenuSequence, jetCRMenuSequence,jetCROldMenuSequence

from . import JetRecoCommon
from . import JetPresel

import copy

#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class JetChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        # we deliberately don't call base class constructore, since this assumes a single chain part
        # which is not the case for jets

        self.dict = copy.deepcopy(chainDict)
        
        self.chainName = self.dict['chainName']
        self.chainL1Item = self.dict['L1item']

        self.chainPart = self.dict['chainParts']
        self.L1Threshold = ''
        self.mult = 1 # from the framework point of view I think the multiplicity is 1, internally the jet hypo has to figure out what to actually do

        # these properties are in the base class, but I don't think we need them for jets
        #self.chainPartName = ''
        #self.chainPartNameNoMult = ''
        #self.chainPartNameNoMultwL1 = ''

        # expect that the L1 seed is the same for all jet parts, otherwise we have a problem
        jChainParts = JetRecoCommon.jetChainParts(self.chainPart)
        # Register if this is a performance chain, in which case the HLT should be exactly j0_perf
        self.isPerf = False
        # Exotic hypo (emerging-jets, trackless jets)
        self.exotHypo = ''
        # Check if we intend to preselect events with calo jets in step 1
        self.trkpresel = "nopresel"

        for ip,p in enumerate(jChainParts):
            # Check if there is exactly one exotic hypothesis defined
            if len(p['exotHypo']) > 1:
                raise RuntimeError('emerging chains currently not configurable with more than one emerging selection!')
            if p['exotHypo']:
                self.exotHypo = p['exotHypo'][0]

            if 'perf' in p['addInfo']:
                # Need to ensure a few conditions to ensure there is no selection bias:
                # No preselection, no special hypo
                # Only one chainPart is permitted
                verify_null_str = [ p[k]!='' for k in ['jvt','momCuts','timing','bsel','bTag']]
                verify_null_list = [ p[k]!=[] for k in ['prefilters','exotHypo'] ]
                if (
                    p['trkpresel']!='nopresel' or p['hypoScenario']!='simple'
                    or any(verify_null_str) or any(verify_null_list)
                    or p['smc']!='nosmc'
                    or ip>0
                ):
                    raise RuntimeError(f'Invalid jet \'perf\' chain "{self.chainName}": No additional selection is permitted!')
                self.isPerf = True
            l1th = p['L1threshold']
            if self.L1Threshold != '' and self.L1Threshold != l1th:
                raise RuntimeError('Cannot configure a jet chain with different L1 thresholds')
            self.L1Threshold = l1th

            # Verify that the preselection is defined only once
            if p["trkpresel"]!="nopresel" and ip+1!=len(jChainParts): # Last jet chainPart, presel should go here
                log.error("Likely inconsistency encountered in preselection specification for %s",self.chainName)
                raise RuntimeError("Preselection %s specified earlier than in the last chainPart!",p["trkpresel"])

        self.trkpresel = JetPresel.extractPreselection(self.dict)
        self.trkpresel_parsed_reco = {key:p[key] for key in ['recoAlg']} #Storing here the reco options from last chain part that we want to propagate to preselection (e.g. jet radius)

        self.recoDict = JetRecoCommon.extractRecoDict(jChainParts)

        self._setJetName()


    # ----------------------
    # Assemble jet collection name based on reco dictionary
    # ----------------------
    def _setJetName(self):
        from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitChainDict
        from JetRecConfig.JetDefinition import buildJetAlgName, xAODType
        subJetChainDict = {}
        for subChainDict in splitChainDict(self.dict):
            for part in subChainDict["chainParts"]:
                if part['signature'] in ["Jet", "Bjet", "Beamspot"]:
                    subJetChainDict = subChainDict
                    break
        if not subJetChainDict:
            raise ValueError("sub Jet Chain dictionary is empty. Cannot define jet collection name on empty dictionary")
        clustersKey = JetRecoCommon.getClustersKey(self.recoDict)
        prefix = JetRecoCommon.getHLTPrefix()
        suffix = "_"+self.recoDict["jetCalib"]
        if JetRecoCommon.jetDefNeedsTracks(self.recoDict):
            suffix += "_{}".format(self.recoDict["trkopt"])
        inputDef = JetRecoCommon.defineJetConstit(self.recoDict, clustersKey = clustersKey, pfoPrefix=prefix+self.recoDict["trkopt"])
        jetalg, jetradius, jetextra = JetRecoCommon.interpretRecoAlg(self.recoDict["recoAlg"])
        actualradius = float(jetradius)/10
        self.jetName = prefix+buildJetAlgName("AntiKt", actualradius)+inputDef.label+"Jets"+suffix
        if inputDef.basetype == xAODType.CaloCluster:
             # Omit cluster origin correction from jet name
             # Keep the origin correction explicit because sometimes we may not
             # wish to apply it, whereas PFlow corrections are applied implicitly
             self.jetName = self.jetName.replace("Origin","")
        

    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):                            
        log.debug("Assembling chain %s", self.chainName)

        # --------------------
        # define here the names of the steps and obtain the chainStep configuration 
        # --------------------
        # Only one step for now, but we might consider adding steps for
        # reclustering and trimming workflows
        chainSteps = []
        if self.recoDict["ionopt"]=="ion":
            jetCollectionName, jetDef, jetHICaloHypoStep = self.getJetHICaloHypoChainStep(flags)
            chainSteps.append( jetHICaloHypoStep )
        elif self.recoDict["trkopt"]=="roiftf":
            # Can't work w/o presel jets to seed RoIs
            if self.trkpresel=="nopresel":
                raise RuntimeError("RoI FTF jet tracking requested with no jet preselection to provide RoIs")
            # Set up preselection step first
            clustersKey, preselJetDef, jetPreselStep = self.getJetCaloPreselChainStep(flags)
            chainSteps.append( jetPreselStep )
            # Standard tracking step, configure the tracking instance differently
            # Later we should convert this to a preselection-style hypo

            jetRoITrackJetTagHypoStep = self.getJetRoITrackJetTagHypoChainStep(flags, preselJetDef.fullname())
            chainSteps.append( jetRoITrackJetTagHypoStep )
        elif self.recoDict["trkopt"]=="ftf":
            if self.trkpresel=="nopresel":
                clustersKey, caloRecoStep = self.getJetCaloRecoChainStep(flags)
                chainSteps.append( caloRecoStep )
                #Add empty step to align with preselection step
                roitrkPreselStep = self.getEmptyStep(2, 'RoIFTFEmptyStep')
            else:
                clustersKey, preselJetDef, jetPreselStep = self.getJetCaloPreselChainStep(flags)
                chainSteps.append( jetPreselStep )
                if re.match(r'.*(b\d\d|bg\d\d|bgtwo\d\d)|.*Z', self.trkpresel):
                    roitrkPreselStep = self.getJetRoITrackJetTagPreselChainStep(flags, preselJetDef.fullname())
                else:
                    roitrkPreselStep=self.getEmptyStep(2, 'RoIFTFEmptyStep')
            chainSteps.append(roitrkPreselStep)
            jetCollectionName, jetDef, jetFSTrackingHypoStep = self.getJetFSTrackingHypoChainStep(flags, clustersKey)
            chainSteps.append( jetFSTrackingHypoStep )
        else:
            jetCollectionName, jetDef, jetCaloHypoStep = self.getJetCaloHypoChainStep(flags)
            chainSteps.append( jetCaloHypoStep )

        # Add exotic jets hypo
        if self.exotHypo != '' and ("emerging" in self.exotHypo or "trackless" in self.exotHypo):
            EJsStep = self.getJetEJsChainStep(flags, jetCollectionName, self.exotHypo)
            chainSteps+= [EJsStep]
        elif self.exotHypo != '' and ("calratio" in self.exotHypo):
             CRStep = self.getJetCRChainStep(flags, self.jetName, self.exotHypo)
             chainSteps+= [ CRStep]
        elif self.exotHypo != '' and ("calrtold" in self.exotHypo):
            CRStep = self.getJetCROldChainStep(flags,self.jetName, self.exotHypo)
            chainSteps+= [self.getEmptyStep(2, 'RoIFTFEmptyStep'), CRStep]

        myChain = self.buildChain(chainSteps)

        return myChain
        

    # --------------------
    # Configuration of steps
    # --------------------
    def getJetCaloHypoChainStep(self, flags):
        stepName = f"MainStep_jet_{self.recoDict['jetDefStr']}"
        if self.isPerf:
            stepName += '_perf'
        jetSeq, jetDef = jetCaloHypoMenuSequence(
            flags, isPerf=self.isPerf, **self.recoDict
        )
        jetCollectionName = jetDef.fullname()

        return jetCollectionName, jetDef, ChainStep(stepName, [jetSeq], multiplicity=[1], chainDicts=[self.dict])

    def getJetHICaloHypoChainStep(self, flags):
        stepName = "MainStep_HIjet"
        if self.isPerf:
            stepName += '_perf'


        from .JetMenuSequencesConfig import jetHICaloHypoMenuSequence
        jetSeq, jetDef = jetHICaloHypoMenuSequence(
            flags, isPerf=self.isPerf, **self.recoDict
        )


        jetCollectionName = jetDef.fullname()

        return jetCollectionName, jetDef, ChainStep(stepName, [jetSeq], multiplicity=[1], chainDicts=[self.dict])

    def getJetRoITrackJetTagHypoChainStep(self, flags, jetsInKey):
        stepName = "RoIFTFStep_jet_sel_"+self.recoDict['jetDefStr']
        jetSeq = jetRoITrackJetTagHypoMenuSequence(
            flags, jetsIn=jetsInKey, isPresel=False, **self.recoDict
        )
        return ChainStep(stepName, [jetSeq], multiplicity=[1], chainDicts=[self.dict])

    def getJetFSTrackingHypoChainStep(self, flags, clustersKey):
        stepName = "MainStep_jet_"+self.recoDict['jetDefStr']
        if self.isPerf:
            stepName += '_perf'
        jetSeq, jetDef = jetFSTrackingHypoMenuSequence(
            flags, clustersKey=clustersKey,
            isPerf=self.isPerf,
            **self.recoDict
        )
        jetCollectionName = jetDef.fullname()
        return jetCollectionName, jetDef, ChainStep(stepName, [jetSeq], multiplicity=[1], chainDicts=[self.dict])

    def getJetCaloRecoChainStep(self, flags):
        stepName = "CaloRecoPTStep_jet_"+self.recoDict["clusterCalib"]
        jetSeq, clustersKey = jetCaloRecoMenuSequence(
            flags, clusterCalib=self.recoDict["clusterCalib"]
        )

        return str(clustersKey), ChainStep(stepName, [jetSeq], multiplicity=[1], chainDicts=[self.dict])

    def getJetCaloPreselChainStep(self, flags):

        #Find if a a4 or a10 calo jet needs to be used in the pre-selection from the last chain dict
        assert 'recoAlg' in self.trkpresel_parsed_reco.keys(), "Impossible to find \'recoAlg\' key in last chain dictionary for preselection"
        #Want to match now only a4 and a10 in the original reco algorithm. We don't want to use a10sd or a10t in the preselection
        matched_reco = re.match(r'^a\d?\d?', self.trkpresel_parsed_reco['recoAlg'])
        assert matched_reco is not None, "Impossible to get matched reco algorithm for jet trigger preselection The reco expression {0} seems to be impossible to be parsed.".format(self.trkpresel_parsed_reco['recoAlg'])

        #Getting the outcome of the regex reco option (it should correspond to a4 or a10 depending by which chain you are configuring)
        preselRecoDict = JetPresel.getPreselRecoDict(matched_reco.group())

        stepName = "PreselStep_jet_"+preselRecoDict['jetDefStr']
        jetSeq, jetDef, clustersKey = jetCaloPreselMenuSequence( flags, **preselRecoDict )

        return str(clustersKey), jetDef, ChainStep(stepName, [jetSeq], multiplicity=[1], chainDicts=[self.dict])

    def getJetRoITrackJetTagPreselChainStep(self, flags, jetsInKey):

        #Find if a a4 or a10 calo jet needs to be used in the pre-selection from the last chain dict
        assert 'recoAlg' in self.trkpresel_parsed_reco.keys(
        ), "Impossible to find \'recoAlg\' key in last chain dictionary for preselection"
        #Want to match now only a4 and a10 in the original reco algorithm. We don't want to use a10sd or a10t in the preselection
        matched_reco = re.match(
            r'^a\d?\d?', self.trkpresel_parsed_reco['recoAlg'])
        assert matched_reco is not None, "Impossible to get matched reco algorithm for jet trigger preselection The reco expression {0} seems to be impossible to be parsed.".format(
            self.trkpresel_parsed_reco['recoAlg'])

        #Getting the outcome of the regex reco option (it should correspond to a4 or a10 depending by which chain you are configuring)
        preselRecoDict = JetPresel.getPreselRecoDict(matched_reco.group(),roiftf=True)

        assert preselRecoDict['trkopt'] == 'roiftf', 'getJetRoITrackJetTagPreselChainStep: you requested a RoI tracking preselection but the reco dictionary has \'trkopt\' set to {0}'.format(preselRecoDict['trkopt'])

        stepName = "RoIFTFStep_jet_"+self.recoDict['jetDefStr']
        jetSeq = jetRoITrackJetTagHypoMenuSequence(flags, jetsIn=jetsInKey, isPresel=True, **preselRecoDict)

        return ChainStep(stepName, [jetSeq], multiplicity=[1], chainDicts=[self.dict])

    def getJetEJsChainStep(self, flags, jetCollectionName, exotdictstring):

        # Must be configured similar to : emergingPTF0p0dR1p2 or tracklessdR1p2
        if 'emerging' in exotdictstring and ('dR' not in exotdictstring \
           or 'PTF' not in exotdictstring):
            log.error('Misconfiguration of exotic jet chain - need dR and PTF options')
            exit(1)
        if 'trackless' in exotdictstring and 'dR' not in exotdictstring:
            log.error('Misconfiguration of trackless exotic jet chain - need dR option')
            exit(1)

        trackless = int(0)
        if 'emerging' in exotdictstring:
            ptf = float(exotdictstring.split('PTF')[1].split('dR')[0].replace('p', '.'))
            dr  = float(exotdictstring.split('dR')[1].split('_')[0].replace('p', '.'))
        elif 'trackless' in exotdictstring:
            trackless = int(1)
            ptf = 0.0
            dr = float(exotdictstring.split('dR')[1].split('_')[0].replace('p', '.'))
        else:
            log.error('Misconfiguration of trackless exotic jet chain - need emerging or trackless selection')
            exit(1)

        log.debug("Running exotic jets with ptf: " + str(ptf) + "\tdR: " + str(dr) + "\ttrackless: " + str(trackless) + "\thypo: " + exotdictstring)

        stepName = "EJsStep_"
        jetSeq = jetEJsMenuSequence(flags, jetsIn=jetCollectionName)
        chainStep = ChainStep(stepName, [jetSeq], multiplicity=[1], chainDicts=[self.dict])

        return chainStep

    def getJetCRChainStep(self, flags, jetCollectionName, exotdictstring):
        
        if 'calratio' in exotdictstring:
            MinjetlogR = 1.2
            doBIBremoval = int(0)
        else:
            log.error('Misconfiguration of trackless exotic jet chain - need calratio selection')
            exit(1)

        log.debug("Running exotic jets with MinjetlogR: " + str(MinjetlogR) + "\t BIB rm " + str(doBIBremoval) + "\thypo: " + exotdictstring)

        stepName = "CRStep_"+self.chainName
        jetSeq = jetCRMenuSequence(flags, jetsIn=jetCollectionName)
        chainStep = ChainStep(stepName, [jetSeq], multiplicity=[1], chainDicts=[self.dict])

        return chainStep

    def getJetCROldChainStep(self, flags, jetCollectionName, exotdictstring):
        
        if 'calrtold' in exotdictstring:
            MinjetlogR = 1.2
            doBIBremoval = int(0)
        else:
            log.error('Misconfiguration of trackless exotic jet chain - need calrtold selection')
            exit(1)

        log.debug("Running exotic jets with MinjetlogR: " + str(MinjetlogR) + "\t BIB rm " + str(doBIBremoval) + "\thypo: " + exotdictstring)

        stepName = "CRPldStep_"+self.chainName
        jetSeq = jetCROldMenuSequence(flags, jetsIn=jetCollectionName)
        chainStep = ChainStep(stepName, [jetSeq], multiplicity=[1], chainDicts=[self.dict])

        return chainStep
