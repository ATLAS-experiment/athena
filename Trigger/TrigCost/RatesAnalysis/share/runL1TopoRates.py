#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
CA module to configure the (standalone) HLT for athena.
"""

# This entry point is only used when running in athena

if __name__ == "__main__":
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()
   parser = flags.getArgumentParser()
   # Add additional args
    #
   parser.add_argument('--disableHistograms', action='store_false', help='Turn off histograming')
   parser.add_argument('--disableGlobalGroups', action='store_false', help='Turn off global groups')
   parser.add_argument('--disableTriggerGroups', action='store_false', help='Turn off per-trigger groups')
   parser.add_argument('--disableExpressGroup', action='store_false', help='Turn off express stream rates')
   parser.add_argument('--disableUniqueRates', action='store_false', help='Turn off unique rates (much faster!)')
   parser.add_argument('--disableLumiExtrapolation', action='store_false', help='Turn off luminosity extrapolation')
   #
   parser.add_argument('--MCDatasetName', default='', type=str, help='For MC input: Name of the dataset, can be used instead of MCCrossSection, MCFilterEfficiency')
   parser.add_argument('--MCCrossSection', default=0.0, type=float, help='For MC input: Cross section of process in nb')
   parser.add_argument('--MCFilterEfficiency', default=1.0, type=float, help='For MC input: Filter efficiency of any MC filter (0.0 - 1.0)')
   parser.add_argument('--MCKFactor', default=1.0, type=float, help='For MC input: Additional multiplicitive fudge-factor to the supplied cross section.')
   parser.add_argument('--MCIgnoreGeneratorWeights', action='store_true', help='For MC input: Flag to disregard any generator weights.')
   #
   parser.add_argument('--outputHist', default='RatesHistograms.root', type=str, help='Histogram output ROOT file')
   #
   parser.add_argument('--targetLuminosity', default=2e34, type=float)
   #
   parser.add_argument('--doRatesVsPositionInTrain', action='store_true', help='Study rates vs BCID position in bunch train')
   parser.add_argument('--vetoStartOfTrain', default=0, type=int, help='Number of BCIDs at the start of the train to veto, implies doRatesVsPositionInTrain')
   #
   parser.add_argument('--maxEvents', type=int, help='Maximum number of events to process')
   parser.add_argument('flags', nargs='*', help='Config flag overrides')

   parser.add_argument('--ratesJson', type=str, help='Input JSON of triggers for rates')
   args, _ = parser.parse_known_args()

   if args.ratesJson:
        import json
        with open(args.ratesJson, 'r') as f:
            triggers_data = json.load(f)
        L1_items = triggers_data.get("L1_items", [])
        
        userDefinedL1items = triggers_data.get("userDefinedL1items", [])
        userDefinedNames = [item["name"] for item in userDefinedL1items if "name" in item] 
        userDefinedDefinitions = [item["definition"] for item in userDefinedL1items if "definition" in item]
   else:
        L1_items = []
        userDefinedNames = []
        userDefinedDefinitions = []
   # Act on additional args, e.g. run over ttbar if no --filesInput argument is specified
   # Runs over Enhanced bias data
   if not args.filesInput:
      from TrigValTools.TrigValSteering import Input
      flags.Input.Files = Input.load_input_json()['data']['paths']
   
   # Set explicit flags
   flags.Trigger.enabledSignatures=['Calib']
   flags.Trigger.doLVL1=True
   flags.Concurrency.NumConcurrentEvents = 1
   flags.Concurrency.NumThreads = 1
   flags.Exec.MaxEvents = 1000

   useBunchCrossingData = (args.doRatesVsPositionInTrain or args.vetoStartOfTrain > 0)
   
   # Fill flags from command line
   try:
        flags.fillFromArgs(parser=parser)
   except Exception as e:
        print(f"Error filling flags: {e}")
        import sys
        sys.exit(1)
   if (flags.Input.isMC):
        flags.IOVDb.GlobalTag="OFLCOND-MC23-SDR-RUN3-05"

   from TriggerJobOpts.runHLT import athenaCfg
   acc = athenaCfg(flags,parser=parser)
   from AthenaConfiguration.ComponentFactory import CompFactory

   from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
   tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))

   from TrigConfxAOD.TrigConfxAODConfig import getxAODConfigSvc
   cfgsvc = acc.getPrimaryAndMerge(getxAODConfigSvc(flags))

   histSvc = CompFactory.THistSvc()
   histSvc.Output += ["RATESTREAM DATAFILE='RatesHistograms.root' OPT='RECREATE'"]
   acc.addService(histSvc)
   
   # If the dataset name is in the input files path, then it will be fetched from there
   # Note to enable autolookup, first run "lsetup pyami; voms-proxy-init -voms atlas" and enter your grid pass phrase
   xsec = args.MCCrossSection
   fEff = args.MCFilterEfficiency
   dset = args.MCDatasetName
   if flags.Input.isMC and xsec == 0: # If the input file is MC then make sure we have the needed info
     from RatesAnalysis.GetCrossSectionAMITool import GetCrossSectionAMI
     amiTool = GetCrossSectionAMI()
     if dset == "": # Can we get the dataset name from the input file path?
       dset = amiTool.getDatasetNameFromPath(flags.Input.Files[0])
     amiTool.queryAmi(dset)
     xsec = amiTool.crossSection
     fEff = amiTool.filterEfficiency

   ebw = CompFactory.EnhancedBiasWeighter('EnhancedBiasRatesTool')
   ebw.RunNumber = flags.Input.RunNumbers[0]
   ebw.UseBunchCrossingData = False
   ebw.IsMC = flags.Input.isMC
   # Assuming ttbar
   #ebw.MCCrossSection = 0.73207
   #ebw.MCFilterEfficiency = 4.385184E-01
   # The following three are only needed if isMC == true
   ebw.MCCrossSection = xsec
   ebw.MCFilterEfficiency = fEff
   ebw.MCKFactor = args.MCKFactor
   ebw.MCIgnoreGeneratorWeights = args.MCIgnoreGeneratorWeights

   acc.addPublicTool(ebw)

   tdt = CompFactory.Trig.TrigDecisionTool('TrigDecisionTool')
   tdt.TrigConfigSvc = cfgsvc
   tdt.HLTSummary = "HLTNav_Summary_OnlineSlimmed"
   tdt.NavigationFormat = "TrigComposite"
   acc.addPublicTool(tdt)

   rates = CompFactory.L1TopoRatesCalculator()
   if L1_items:
        try:
            rates.m_L1_items_json = L1_items  
        except AttributeError:
            print("Failed to set L1_items, method not found")
   
   if userDefinedNames and userDefinedDefinitions:
        rates.m_userDefinedNames = userDefinedNames  
        rates.m_userDefinedDefinitions = userDefinedDefinitions

   rates.DoTriggerGroups = True
   rates.DoGlobalGroups = True
   rates.DoExpressRates = True
   rates.DoUniqueRates = True
   rates.DoHistograms = True
   rates.UseBunchCrossingData = False
   rates.TargetLuminosity = 2e34
   rates.VetoStartOfTrain = False
   rates.EnableLumiExtrapolation = True
   rates.EnhancedBiasRatesTool = ebw
   rates.TrigDecisionTool = tdt
   rates.TrigConfigSvc = cfgsvc
   acc.addEventAlgo(rates)

   # Add aditional algorithms, e.g. the rates algorithm
   #acc.addEventAlgo( ... )

   import sys
   sys.exit(acc.run().isFailure())
