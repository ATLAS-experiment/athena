# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from typing import Optional
import importlib, re, string

from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

# _maxAllowedCustomCH: this variable keeps track of the number of custom ComboHypo that are named differently as expected from the ControlFlow rule (which sets one ComboHypo per step, both named the same). These violations are already investigated and allowed because the ComboHypos are able to handle the decisions internally. Any change of this parameter needs to be discussed with experts
_maxAllowedCustomCH = 10

def calibCosmicMonSignatures():
    return ['Streaming','Monitor','Beamspot','Cosmic', 'Calib', 'EnhancedBias']

def combinedSignatures():
    return ['MinBias','Electron','Photon','Muon','Tau','Jet', 'Bjet','MET','UnconventionalTracking','HeavyIon']

def jointSignatures():
    return ['Bjet', 'Egamma', 'Combined']

def defaultSignatures():
    return ['Streaming']

def testSignatures():
    return ['Test']

def bphysicsSignatures():
    return ['Bphysics']

def allSignatures():
    return set(calibCosmicMonSignatures() + combinedSignatures() + jointSignatures() + bphysicsSignatures() + defaultSignatures() + testSignatures())


class FilterChainsToGenerate(object):
    """Standard chain filter"""
    def __init__(self, flags):
        self.enabledSignatures  = flags.Trigger.enabledSignatures  if flags.hasFlag("Trigger.enabledSignatures") else []
        self.disabledSignatures = flags.Trigger.disabledSignatures if flags.hasFlag("Trigger.disabledSignatures") else []
        self.selectChains       = flags.Trigger.selectChains       if flags.hasFlag("Trigger.selectChains") else []
        self.disableChains      = flags.Trigger.disableChains      if flags.hasFlag("Trigger.disableChains") else []          

    def __call__(self, signame, chain):            
        return ((signame in self.enabledSignatures and signame not in self.disabledSignatures) and \
            (not self.selectChains or chain in self.selectChains) and chain not in self.disableChains)

    def __str__(self) -> str:
        return f'FilterChainsToGenerate(enabledSignatures={self.enabledSignatures}, disabledSignatures={self.disabledSignatures}, selectChains={self.selectChains}, disableChains={self.disableChains})'
  

class Singleton(type):
    _instances = {}
    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls._instances[cls]

    def clear(cls):
        cls._instances.clear()


class GenerateMenuMT(metaclass=Singleton):
    """Singleton class for the Trigger Menu"""
    def __init__(self):
        self.base_menu_name: str = ''

        self.chainsInMenu = {}  # signature : [chains]

        self.allChainsForAlignment = []
        self.chainDicts = []
        self.combinationsInMenu = []
        self.alignmentGroupsToAlign = set()
        self.configLengthDict = {}
        
        self.signaturesOverwritten = False
        self.L1Prescales = None
        self.HLTPrescales = None

        self.chainFilter = None
        self.availableSignatures = []

        self.sigDicts = {}

        self.chainDefModule = {}   # Generate[SIG]ChainDefs module for each SIGnature


    # Define which signatures (folders) are required for each slice
    def getRequiredSignatures(theslice):
        allSigs = allSignatures()
        signatureDeps = {sig:[sig] for sig in allSigs}
        # Special cases
        signatureDeps.update({
            # Bjet always requires jet
            'Bjet': ['Bjet','Jet'],
            # Egamma contains two signatures
            'Egamma': ['Electron','Photon'],
            'Combined': combinedSignatures(),
        })
        return set(signatureDeps[theslice]+defaultSignatures()) # always allow streamers


    def setChainFilter(self, f):
        """Set chain filter for menu generation.

           This can be any callable object taking two
           arguments for signature and chain name and returning a boolean.
           E.g. to only generate Egamma chains:
                menu.setChainFilter(lambda slice,chain : slice=='Egamma').

           In the special case that f is a functor with the list attributes
           selectChains and/or disableChains, the contents will be explicitly
           checked to be in the menu.
        """
        fname = f.__class__.__name__ if isinstance(f,object) else f.__name__
        import inspect
        if len(inspect.signature(f).parameters)!=2:
            log.error('%s is not a valid chain filter. Function/callable needs take two arguments '
                      'for signature and chain name and return a boolean', fname)
        else:
            log.info('Setting chain filter to: %s', f)
            self.chainFilter = f


    def getChainDicts(self, flags):

        def validSignature(currentSig, chainSig):
            """Check if chain is assigned to the correct signature"""
            reqd = GenerateMenuMT.getRequiredSignatures(currentSig)
            isValid = chainSig.issubset( reqd )
            log.debug("Chain signatures: %s, required signatures: %s",chainSig,reqd)
            if not isValid:
                log.error("Chain signatures %s not a subset of required signatures %s",set(chainSig),reqd)
            return isValid

        from TriggerMenuMT.HLT.Config.Utility.DictFromChainName import dictFromChainName

        chainCounter = 0
        invalid = False
        for sig, chains in self.chainsInMenu.items():
            for chain in chains:
                log.debug("Now processing chain: %s from signature %s", chain, sig)
                chainCounter += 1
                chainDict = dictFromChainName(flags, chain)
                chainDict['chainCounter'] = chainCounter
                chainDict['prescale'] = 1  # set default chain prescale

                # Pick out the folder and subsignature directories to import
                for sigfo, subsig in chainDict['sigDicts'].items():
                    if sigfo not in self.sigDicts:
                        self.sigDicts[sigfo] = subsig
                    else:
                        for ss in subsig:
                            if ss not in self.sigDicts[sigfo]:
                                self.sigDicts[sigfo].append(ss)

                self.chainDicts.append(chainDict)

                if not validSignature(sig, set(chainDict['signatures'])):
                    invalid=True
                    log.error('Chain %s assigned to signature %s but creates %s',
                              chainDict['chainName'], sig, set(chainDict['signatures']))
        if invalid:
            raise RuntimeError('Incorrect assignment of chains to slices -- see preceding messages.')

    def importSignaturesToGenerate(self):
        """check if all the signature files can be imported and then import them"""

        for sig, subSigs in self.sigDicts.items():
            try:
                for ss in subSigs:
                    import_module = 'TriggerMenuMT.HLT.' + sig +'.Generate' + ss + 'ChainDefs'
                    self.chainDefModule[ss] = importlib.import_module(import_module)

                    if ss not in self.availableSignatures:
                        self.availableSignatures.append(ss)

            except ImportError:
                log.exception('Problems when importing ChainDef generating code for %s', sig)
                import traceback
                traceback.print_exc()

        log.info('Available signature(s) for chain generation: %s', self.availableSignatures)

        return

    def generateChains(self, flags):       
        all_chains = []
        combinations_in_menu = []
        alignmentGroups_to_align = set()
        length_of_configs = {}
        
        nchainDicts = len(self.chainDicts)
        notify_increment = max(int(nchainDicts / 10),1)
        for ichainDict, chainDict in enumerate(self.chainDicts):
            log.debug("Next: getting chain configuration for chain %s ", chainDict['chainName'])
            if ichainDict % notify_increment==0:
                log.info("Generating HLT chain %d / %d", ichainDict+1, nchainDicts)
            chainConfig,lengthOfChainConfigs = self.__generateChainConfig(flags, chainDict)
            all_chains += [(chainDict,chainConfig,lengthOfChainConfigs)]
            
            #update the alignment group length dictionary if we have a longer number of steps
            #or the signature isn't registered in the dictionary yet
            for config_length, config_grp in lengthOfChainConfigs:
                if config_grp in length_of_configs:
                    if config_length > length_of_configs[config_grp]:
                        length_of_configs[config_grp] = config_length
                else:
                    length_of_configs[config_grp] = config_length

            # find the chains that contain more than one alignment group, to keep track
            # of what combinations do we need to deal with.
            # we're using sets here so we don't end up with duplicates
            if len(set(chainDict['alignmentGroups'])) > 1:  
                combinations_in_menu += [list(set(chainDict['alignmentGroups']))]
                for align_group in list(set(chainDict['alignmentGroups'])):
                    alignmentGroups_to_align.update([align_group])
                    
        self.allChainsForAlignment = all_chains
        self.combinationsInMenu = combinations_in_menu
        self.alignmentGroupsToAlign = alignmentGroups_to_align
        self.configLengthDict = length_of_configs

        return 


    def generateAllChainConfigs(self, flags):
        """
        == Obtains chain configs for all chains in menu
        """

        from TriggerMenuMT.HLT.Config.Utility.MenuAlignmentTools import MenuAlignment
        from TriggerMenuMT.HLT.CommonSequences import EventBuildingSequences, TLABuildingSequences

        # get all chain names from menu
        log.info("Will now get chains from the menu")
        self.getChainsFromMenu(flags)

        # decoding of the chain name
        log.info("Will now get chain dictionaries for each chain")
        self.getChainDicts(flags)

        if flags.Trigger.disableCPS:
            log.warning('Removing all CPS group because the flag Trigger.disableCPS is set')
            for chainDict in self.chainDicts:
                chainDict['groups'] = [g for g in chainDict['groups'] if not g.startswith('RATE:CPS_')]

        #import the necessary signatures
        log.debug("Importing the necessary signatures")
        self.importSignaturesToGenerate()

        log.info("Will now generate the chain configuration for each chain")
        self.generateChains(flags)

        log.info("Will now calculate the alignment parameters")
        #dict of signature: set it belongs to
        #e.g. {'Electron': ['Electron','Muon','Photon']}        
        menuAlignment = MenuAlignment(self.combinationsInMenu,
                                      self.alignmentGroupsToAlign,
                                      self.configLengthDict)
        menuAlignment.analyse_combinations()

        # alignmentGroups_to_align = menuAlignment.groupsToAlign
        # lengthOfChainConfigs = self.configLengthDict
        # combinationsInMenu = menuAlignment.combinationsInMenu
        # alignmentGroup_sets_to_align = menuAlignment.setsToAlign

        log.info('Aligning the following signatures: %s',sorted(menuAlignment.sets_to_align))
        log.debug('Length of each of the alignment groups: %s',self.configLengthDict)

        chainConfigs = []

        for chainDict,chainConfig,lengthOfChainConfigs in self.allChainsForAlignment:

              # start by ordering electron, photon, muon by having e+mu, g+mu, e+g chains
              # desired ordering: electron, photon, muon, tau, jet, met, b-jet
              
              # lengthOfChainConfigs is something like this: [(4, 'Photon'), (5, 'Muon')]
              # needs to match up with the maximum number of steps in a signature in the menu (length_of_configs)
              # start with electron! Only need to add post-steps for combined electron chains if the max length in a combined chain
              # is greater than the number of electron steps combined chain. Assume that the max length of an electron chain occurs 
              # in a combined chain.

              log.debug("[generateAllChainConfigs] chain %s has config lengths %s and alignment groups %s", chainDict['chainName'], lengthOfChainConfigs, chainDict['alignmentGroups'])
 
              alignmentGroups = chainDict['alignmentGroups']
            
              #parallel-merged single-signature chains or single signature chains. Anything that needs no splitting!
              if len(set(alignmentGroups)) == 1: 
                  alignedChainConfig = menuAlignment.single_align(chainDict, chainConfig)
                  HLTMenuConfig.registerChain( chainDict )
                  chainConfigs.append( alignedChainConfig )

              elif len(alignmentGroups) >= 2:
                  alignedChainConfig = menuAlignment.multi_align(chainDict, chainConfig, lengthOfChainConfigs)
                  HLTMenuConfig.registerChain( chainDict )
                  chainConfigs.append( alignedChainConfig )

              else: 
                  log.error("Menu can't deal with combined chains with more than two alignmentGroups at the moment. oops...")
                  raise NotImplementedError("more than three alignment groups still needs implementing in ChainMerging.py, ATR-22206")

              if not HLTMenuConfig.isChainRegistered(chainDict['chainName']):
                log.error("Chain %s has not been registered in the menu!", chainDict['chainName'])
                import pprint
                pp = pprint.PrettyPrinter(indent=4, depth=8)
                log.error('The chain dictionary is: %s', pp.pformat(chainDict))
                raise Exception("Please fix the menu or the chain.")

        # align event building sequences
        log.info("[generateAllChainConfigs] general alignment complete, will now align TLA chains")
        TLABuildingSequences.alignTLASteps(chainConfigs, HLTMenuConfig.dicts())
        log.info("[generateAllChainConfigs] general and TLA alignment complete, will now align PEB chains")
        EventBuildingSequences.alignEventBuildingSteps(chainConfigs, HLTMenuConfig.dicts())

        log.info("[generateAllChainConfigs] all chain configurations have been generated.")
        return chainConfigs


    def getChainsFromMenu(self, flags):
        """
        == Returns the list of chain names that are in the menu
        """

        self.base_menu_name = re.match(r'\w*_v\d*', flags.Trigger.triggerMenuSetup).group(0)
        log.info(f'Menu name: {flags.Trigger.triggerMenuSetup}')
        log.debug('Base menu name: %s', self.base_menu_name)
        
        # Generate the list of chains from the basic menu (terminated in a version number)
        try:
            menu_module = importlib.import_module(f'TriggerMenuMT.HLT.Menu.{self.base_menu_name}')
        except Exception as e:
            log.fatal(f'Failed to import menu module "{self.base_menu_name}" inferred from menu "{flags.Trigger.triggerMenuSetup}"')
            raise e

        # Load Menu
        self.chainsInMenu = menu_module.setupMenu()

        # Filter chains if requested
        if self.chainFilter is not None:
            self.signaturesOverwritten = True

            # Verify that if the chain filter has lists of chains
            # they are all in the menu
            chainsToCheck = []
            if hasattr(self.chainFilter,'selectChains'):
                chainsToCheck += self.chainFilter.selectChains
            if hasattr(self.chainFilter,'disableChains'):
                chainsToCheck += self.chainFilter.disableChains
            for chain in chainsToCheck:
                inMenu = False
                for signame in self.chainsInMenu:
                    if chain in [c.name for c in self.chainsInMenu[signame]]:
                        inMenu = True
                        break
                if not inMenu:
                    raise RuntimeError(f'Request to enable/disable chain {chain} that is not in menu')

            for signame in self.chainsInMenu:
                self.chainsInMenu[signame] = [c for c in self.chainsInMenu[signame]
                                              if self.chainFilter(signame, c.name)]

        if not self.chainsInMenu:
            log.warning("There seem to be no chains in the menu - please check")
        elif log.isEnabledFor(logging.DEBUG):
            import pprint
            log.debug("The following chains were found in the menu:")
            pprint.pprint(self.chainsInMenu)


    def __generateChainConfig(self, flags, mainChainDict):
        """
        # Assembles the chain configuration and returns a chain object with (name, L1see and list of ChainSteps)
        """

        from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitInterSignatureChainDict
        from TriggerMenuMT.HLT.Config.Utility.ComboHypoHandling import addTopoInfo, comboConfigurator, topoLegIndices, anomdetWPIndices
        from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs
        from TriggerMenuMT.HLT.CommonSequences import EventBuildingSequences, TLABuildingSequences

        # split the the chainDictionaries for each chain and print them in a pretty way
        chainDicts = splitInterSignatureChainDict(mainChainDict)

        if log.isEnabledFor(logging.DEBUG):
            import pprint
            pp = pprint.PrettyPrinter(indent=4, depth=8)
            log.debug('dictionary is: %s', pp.pformat(chainDicts))

        # Loop over all chainDicts and send them off to their respective assembly code
        listOfChainConfigs = []
        perSig_lengthOfChainConfigs = []

        for chainPartDict in chainDicts:
            chainPartConfig = None
            currentSig = chainPartDict['signature']
            currentAlignGroup = None
            if len(chainPartDict['chainParts']) == 1:
                currentAlignGroup = chainPartDict['chainParts'][0]['alignmentGroup']
            
            chainName = chainPartDict['chainName']
            log.debug('Checking chainDict for chain %s in signature %s, alignment group %s' , chainName, currentSig, currentAlignGroup)

            if currentSig in self.availableSignatures:
                try:
                    log.debug("[__generateChainConfigs] Trying to get chain config for %s", currentSig)
                    if currentSig in ['Electron', 'Photon', 'Muon', 'Tau', 'Bphysics'] :
                        chainPartConfig, perSig_lengthOfChainConfigs = self.chainDefModule[currentSig].generateChainConfigs(flags, chainPartDict, perSig_lengthOfChainConfigs)
                    else:
                        chainPartConfig = self.chainDefModule[currentSig].generateChainConfigs(flags, chainPartDict)
                        if currentSig == 'Test' and isinstance(chainPartConfig, tuple):
                            chainPartConfig = chainPartConfig[0]
                except Exception:
                    log.error('[__generateChainConfigs] Problems creating ChainDef for chain %s ', chainName)
                    log.error('[__generateChainConfigs] I am in chain part\n %s ', chainPartDict)
                    log.exception('[__generateChainConfigs] Full chain dictionary is\n %s ', mainChainDict)
                    raise Exception('[__generateChainConfigs] Stopping menu generation. Please investigate the exception shown above.')
            else:
                log.error('Chain %s cannot be generated - Signature "%s" not available', chainPartDict['chainName'], currentSig)
                log.error('Available signature(s): %s', self.availableSignatures)
                raise Exception('Stopping the execution. Please correct the configuration.')

            log.debug("Chain %s \n chain config: %s",chainPartDict['chainName'],chainPartConfig)

            listOfChainConfigs.append(chainPartConfig)
            log.debug("[__generateChainConfigs] adding to the perSig_lengthOfChainConfigs list (%s, %s)",chainPartConfig.nSteps,chainPartConfig.alignmentGroups)
            perSig_lengthOfChainConfigs.append((chainPartConfig.nSteps,chainPartConfig.alignmentGroups))
                
        # this will be a list of lists for inter-sig combined chains and a list with one 
        # multi-element list for intra-sig combined chains
        # here, we flatten it accordingly (works for both cases!)
        lengthOfChainConfigs = []
        for nSteps, aGrps in perSig_lengthOfChainConfigs:
            if len(nSteps) != len(aGrps):
                log.error("Chain part has %s steps and %s alignment groups - these don't match!",nSteps,aGrps)
            else:
                for a,b in zip(nSteps,aGrps):
                    lengthOfChainConfigs.append((a,b))
            
       
        # This part is to deal with combined chains between different signatures
        try:
            if len(listOfChainConfigs) == 0:
                raise Exception('[__generateChainConfigs] No Chain Configuration found for {0}'.format(mainChainDict['chainName']))                    
            else:
                if len(listOfChainConfigs)>1:
                    log.debug("Merging strategy from dictionary: %s", mainChainDict["mergingStrategy"])
                    theChainConfig, perSig_lengthOfChainConfigs = mergeChainDefs(listOfChainConfigs, mainChainDict, perSig_lengthOfChainConfigs)
                    lengthOfChainConfigs = []
                    for nSteps, aGrps in perSig_lengthOfChainConfigs:
                        if len(nSteps) != len(aGrps):
                            log.error("Post-merged chain part has %s steps and %s alignment groups - these don't match!",nSteps,aGrps)
                        else:
                            for a,b in zip(nSteps,aGrps):
                                lengthOfChainConfigs.append((a,b))
                else:
                    theChainConfig = listOfChainConfigs[0]
                
                for topoID in range(len(mainChainDict['extraComboHypos'])):
                    thetopo = mainChainDict['extraComboHypos'][topoID].strip(string.digits).rstrip(topoLegIndices)  
                    
                    
                    if "anomdet" in thetopo:
                        thetopo = thetopo.rstrip(anomdetWPIndices)
                                      
                    theChainConfig.addTopo((comboConfigurator[thetopo],thetopo))
                                    
                # Now we know where the topos should go, we can insert them in the right steps
                if len(theChainConfig.topoMap) > 0:                    
                    log.debug("Trying to add extra ComboHypoTool for %s",mainChainDict['extraComboHypos'])
                    addTopoInfo(theChainConfig,mainChainDict,listOfChainConfigs,lengthOfChainConfigs)
        except RuntimeError:
            log.error('[__generateChainConfigs] Problems creating ChainDef for chain %s ', chainName)
            log.error('[__generateChainConfigs] I am in the extraComboHypos section, for %s ', mainChainDict['extraComboHypos'])
            log.exception('[__generateChainConfigs] Full chain dictionary is\n %s ', mainChainDict)
            raise Exception('[__generateChainConfigs] Stopping menu generation. Please investigate the exception shown above.')
        except AttributeError:                    
            raise Exception('[__generateChainConfigs] Stopping menu generation. Please investigate the exception shown above.')

        # Configure event building strategy
        eventBuildType = mainChainDict['eventBuildType']
        TLAEventBuildTypes = ('PhysicsTLA', 'FTagPEBTLA', 'EgammaPEBTLA', 'DarkJetPEBTLA')
        if eventBuildType:
            try:
                if any(ebtype in eventBuildType for ebtype in TLAEventBuildTypes):
                    log.debug("Adding TLA Step for chain %s", mainChainDict['chainName'])
                    TLABuildingSequences.addTLAStep(flags, theChainConfig, mainChainDict)
                log.debug('Configuring event building sequence %s for chain %s', eventBuildType, mainChainDict['chainName'])
                EventBuildingSequences.addEventBuildingSequence(flags, theChainConfig, eventBuildType, mainChainDict)
            except TypeError as ex:
                    log.error(ex)
                    raise Exception('[__generateChainConfigs] Stopping menu generation for EventBuilding/TLA sequences. Please investigate the exception shown above.')

        log.debug('[__generateChainConfigs] lengthOfChainConfigs %s, ChainConfigs  %s ', lengthOfChainConfigs, theChainConfig)
        return theChainConfig,lengthOfChainConfigs
 
    
    def resolveEmptySteps(self,chainConfigs):
        max_steps = max([len(cc.steps) for cc in chainConfigs], default=0)    
        steps_are_empty = [True for i in range(0,max_steps)]
        emptySteps = []
        for cc in chainConfigs:
            for istep, the_step in enumerate(cc.steps):
                if not the_step.isEmpty:
                    steps_are_empty[istep] = False
                else: 
                    emptySteps.append(the_step)
                            
        log.debug("Are there any fully empty steps? %s", steps_are_empty)
        log.debug("The empty step(s) and associated chain(s) are: %s", emptySteps)
        empty_step_indices = [i for i,is_empty in enumerate(steps_are_empty) if is_empty]
        
        if len(empty_step_indices) == 0:
            return chainConfigs
        
        special_test_menu = self.chainFilter and (  getattr(self.chainFilter, "selectChains", False) or \
                                                    getattr(self.chainFilter, "disableChains", False) or \
                                                    getattr(self.chainFilter, "disabledSignatures", False) or \
                                                    getattr(self.chainFilter, "enabledSignatures", False) )
                                                
        
        if len(self.availableSignatures) != 1 and not special_test_menu:                
            raise Exception("[resolveEmptySteps] Please find the reason for this empty step and resolve it / remove it from the menu: %s", emptySteps)  

        log.info("Will now delete steps %s (indexed from zero)",empty_step_indices)
        
        for cc in chainConfigs:
            new_steps = []
            #only add non-empty steps to the new steps list!
            for istep,step in enumerate(cc.steps):
                if istep not in empty_step_indices:
                    new_steps += [step]
            cc.steps = new_steps

        return chainConfigs 

    def generatePrescales(self, flags, prescale_set: Optional[str] = '__auto__'):
        '''Add prescales for disabling items (e.g. MC production)'''

        menu_name = flags.Trigger.triggerMenuSetup
        if prescale_set == '__auto__':
            if menu_name.endswith('_prescale'):
                # Get the prescale set name from the Menu
                prescale_set = menu_name.removeprefix(f'{self.base_menu_name}_').removesuffix('_prescale')
            else:
                prescale_set = None

        if prescale_set:
            from TriggerMenuMT.HLT.Menu.MenuPrescaleConfig import menu_prescale_set_gens
            if prescale_set not in menu_prescale_set_gens:
                raise RuntimeError(f'Unknown menu prescale set for menu {flags.Trigger.triggerMenuSetup}')

            gen = menu_prescale_set_gens[prescale_set]
        else:
            from TriggerMenuMT.HLT.Config.Utility.MenuPrescaleSet import AutoPrescaleSetGen
            gen = AutoPrescaleSetGen()

        # Generate prescale set
        log.info(f'Generating automatic prescale set: {prescale_set}')
        ps_set = gen.generate(flags, store=True)
        self.L1Prescales = ps_set.l1_prescales
        self.HLTPrescales = ps_set.hlt_prescales

 

def generateMenuMT(flags): 
    """
    == Main function to generate the L1, L1Topo and HLT menu configs and CA, using the GenerateMenuMT class
    """         
    # Generate L1 menu
    # The L1Menu json file is produced here
    from TrigConfigSvc.TrigConfigSvcCfg import generateL1Menu
    generateL1Menu(flags)


    menu = GenerateMenuMT()

    # Apply generation filter (enable/disable chains and signatures)
    chains_gen_filter = FilterChainsToGenerate(flags)
    menu.setChainFilter(chains_gen_filter)
    log.debug('Filtering chains: %d', menu.chainFilter is not None)

    # Generate all chains configuration
    finalListOfChainConfigs = menu.generateAllChainConfigs(flags)

    log.info('Number of configured chains: %d', len(finalListOfChainConfigs))
    from TriggerMenuMT.HLT.Config import MenuComponents
    if len(MenuComponents._CustomComboHypoAllowed)> _maxAllowedCustomCH:
        log.error(f'Found {len(MenuComponents._CustomComboHypoAllowed)} ComboHypo algorithms  violating the one-CH-per-step rule, only {_maxAllowedCustomCH} are allowed (which are BLS ComboHypos). This is the list of current violations: {MenuComponents._CustomComboHypoAllowed}. Please consolidate your choice of ComboHypo, by checking that it is able to handle decisions internally; if yes eventually increase the limit set by _maxAllowedCustomCH, after discussing with experts')
    # Generate and apply the automatic prescale sets (e.g. for disabling items in an MC production)
    menu.generatePrescales(flags)
    

    # Remove any remaining steps that are fully empty in all chains
    finalListOfChainConfigs = menu.resolveEmptySteps(finalListOfChainConfigs)
    log.debug("finalListOfChainConfig: %s", finalListOfChainConfigs)

    # Make the HLT configuration tree
    # The HLTMenu json file is produced here.
    log.info("Making the HLT configuration tree")
    menuAcc, CFseq_list = makeHLTTree(flags, finalListOfChainConfigs)

    # Configure ChainFilters for ROBPrefetching
    from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching
    if ROBPrefetching.InitialRoI in flags.Trigger.ROBPrefetchingOptions:
        from TrigGenericAlgs.TrigGenericAlgsConfig import prefetchingInitialRoIConfig
        menuAcc.merge(prefetchingInitialRoIConfig(flags, CFseq_list), 'HLTBeginSeq')


    # Post-generation checks:
    log.info("Checking the L1HLTConsistency...")
    from TriggerMenuMT.HLT.Config.Validation.CheckL1HLTConsistency import checkL1HLTConsistency
    checkL1HLTConsistency(flags)

    log.info("Checking the Coherent Prescale assignments...")
    from TriggerMenuMT.HLT.Config.Validation.CheckCPSGroups import checkCPSGroups
    checkCPSGroups(HLTMenuConfig.dictsList())


    # Cleanup menu singletons to allow garbage collection (ATR-28855)
    GenerateMenuMT.clear()
    from TriggerMenuMT.HLT.Config import MenuComponents
    MenuComponents._ComboHypoPool.clear() 
    MenuComponents._CustomComboHypoAllowed.clear()

    return menuAcc
    

def makeHLTTree(flags, chainConfigs):
    """
    Generate appropriate Control Flow Graph wiht all HLT algorithms
    """
    from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFConfig import decisionTreeFromChains, sequenceScanner
    from TriggerJobOpts.TriggerConfig import collectViewMakers
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaCommon.CFElements import seqAND

    acc = ComponentAccumulator()    
    steps = seqAND('HLTAllSteps')
    finalDecisions, CFseq_list, menuAcc = decisionTreeFromChains(flags, steps, chainConfigs, HLTMenuConfig.dictsList())
    if log.getEffectiveLevel() <= logging.DEBUG:
        menuAcc.printConfig()

    acc.merge(menuAcc)
    successful_scan = sequenceScanner( steps )
    if not successful_scan:
        raise Exception("[makeHLTTree] At least one sequence is expected in more than one step. Check error messages and fix!")    

    flatDecisions=[]
    for step in finalDecisions:
        flatDecisions.extend (step)
 
    viewMakers = collectViewMakers(steps)
    viewMakerMap = {vm.name:vm for vm in viewMakers}
    for vmname, vm in viewMakerMap.items():
        log.debug(f"[makeHLTTree] {vmname} InputMakerOutputDecisions: {vm.InputMakerOutputDecisions}")
        if vmname.endswith("_probe"):
            try:
                log.debug(f"Setting InputCachedViews on {vmname} to read decisions from tag leg {vmname[:-6]}: {vm.InputMakerOutputDecisions}")
                vm.InputCachedViews = viewMakerMap[vmname[:-6]].InputMakerOutputDecisions
            except KeyError: # We may be using a probe leg that has different reco from the tag
                log.debug(f"Tag leg does not match probe: '{vmname[:-6]}', will not use cached views")

    
    # Generate JSON representation of the config
    from TriggerMenuMT.HLT.Config.JSON.HLTMenuJSON import generateJSON
    generateJSON(flags, HLTMenuConfig.dictsList(), menuAcc.getSequence("HLTAllSteps"))

    # Store the HLTMonitoring json file
    from TriggerMenuMT.HLT.Config.JSON.HLTMonitoringJSON import generateDefaultMonitoringJSON
    generateDefaultMonitoringJSON(flags, HLTMenuConfig.dictsList())

    return acc, CFseq_list
