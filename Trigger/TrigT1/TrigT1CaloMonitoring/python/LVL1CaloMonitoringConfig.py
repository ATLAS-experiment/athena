#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

class L1CaloMonitorCfgHelper(object):

    """
    This class is designed to handle registering all of L1Calo's monitoring histograms and trees in a
    coherent way. It will also generate the han config file for all the histograms.
    """



    from collections import defaultdict
    hanConfigs = {} # nested structure as required, keys will be either "dir XXX" or "hist YYY"
    hanAlgConfigs = {}
    hanThresholdConfigs = {}
    xmlConfigs = {} # flat dictionary of histograms that are in the shifter folders, so are used for online monitoring
    embargoed = [] # list of embargoed dqdm plots

    SIGNATURES = ["gJ","gLJ","gLJRho","gXEJWOJ","gXEJWOJMHT","gXEJWOJMST","gTEJWOJ","gXENC","gTENC","gXERHO","gTERHO","jJ","jEM","jTAU","jXE","jTE","eTAU","eEM"]
    HELPURL = "" #"https://codimd.web.cern.ch/s/678H65Tk9" No longer adding a help url

    @staticmethod
    def createXmls():
        # First create the DQAlgorithm config file
        import config
        db = config.Configuration()
        db.create_db("l1calo_gen_phase1.data.xml",["daq/schema/dqm.schema.xml","daq/sw/dqm-algorithms.data.xml","daq/sw/repository.data.xml","daq/sw/dqm-default-streams.data.xml"])

        # first ensure all the algorithms exist
        for confName,conf in L1CaloMonitorCfgHelper.hanAlgConfigs.items():
            # only need algorithms for Shifter histograms
            algUsed = False
            for histPath,value in L1CaloMonitorCfgHelper.xmlConfigs.items():
                if value.get("algorithm","") == confName:
                    algUsed = True
                    break
            if not algUsed: continue
            updated = False
            try:
                dqAlgo = db.get_dal("DQAlgorithm",conf["name"])
            except RuntimeError:
                # algorithm doesn't exist, create it ...
                updated=True
                db.create_obj("DQAlgorithm",conf["name"])
                dqAlgo = db.get_dal("DQAlgorithm",conf["name"])
                dqAlgo.LibraryName = conf["libname"]
            # ensure all parameters appear in ParametersNames, and thresholds in ThresholdsNames
            for par,val in conf.items():
                if par=="thresholds" and val in L1CaloMonitorCfgHelper.hanThresholdConfigs:
                    for thresh,vals in L1CaloMonitorCfgHelper.hanThresholdConfigs[val].items():
                        if thresh not in dqAlgo.ThresholdsNames:
                            updated=True
                            dqAlgo.ThresholdsNames += [thresh]
                elif par not in ["name","libname","thresholds"]+dqAlgo.ParametersNames:
                    updated=True
                    dqAlgo.ParametersNames += [par]
            if updated:
                db.update_dal(dqAlgo)


        # now create DQParameter object for each histogram with a DQ algorithm
        # need to have them all in a DQRegion that belongs to a DQLayout
        db.create_obj("DQShape","L1CaloShapeDeveloper")
        db.create_obj("DQLayout","L1CaloLayoutDeveloper")
        db.create_obj("DQRegion","L1CaloRegionDeveloper")
        dqLayout = db.get_dal("DQLayout","L1CaloLayoutDeveloper")
        dqLayout.DefaultChildrenShape = db.get_dal("DQShape","L1CaloShapeDeveloper")
        db.update_dal(dqLayout)
        dqRegion = db.get_dal("DQRegion","L1CaloRegionDeveloper")
        dqRegion.Description = "L1Calo Monitoring Development Folder"
        dqRegion.Label = "Developer"
        dqRegion.DQLayout = dqLayout
        dqRegion.DQSummaryMaker = db.get_dal("DQSummaryMaker","WorstCaseSummary")

        for histPath,value in L1CaloMonitorCfgHelper.xmlConfigs.items():
            if "algorithm" not in value.keys(): continue # only add histograms that have an algorithm

            name = "_".join(histPath.split("/"))
            db.create_obj("DQParameter",name)
            dqPar = db.get_dal("DQParameter",name)
            dqPar.InputDataSource = ["Histogramming.l1calo-athenaHLT-Phase1./"+histPath]
            dqPar.Description = value["description"]

            if value["algorithm"] in L1CaloMonitorCfgHelper.hanAlgConfigs:
                algConf = L1CaloMonitorCfgHelper.hanAlgConfigs[value["algorithm"]]
                dqPar.DQAlgorithm = db.get_dal("DQAlgorithm",algConf["name"])
                for par,val in algConf.items():
                    if par in dqPar.DQAlgorithm.ParametersNames:
                        dqPar.Parameters += [f"{par}={val}"]
                if "thresholds" in algConf:
                    for thresh,vals in L1CaloMonitorCfgHelper.hanThresholdConfigs[algConf["thresholds"]].items():
                        dqPar.GreenThresholds += [f"{thresh}={vals['warning']}"]
                        dqPar.RedThresholds += [f"{thresh}={vals['error']}"]
            else:
                dqPar.DQAlgorithm = db.get_dal("DQAlgorithm",value["algorithm"]) # catches cases of e.g. predefined parameterless alg like Histogram_Empty

            dqRegion.DQParameters +=  [dqPar]
            db.update_dal(dqPar)

        db.update_dal(dqRegion)
        db.commit("Autogenerated")


    @staticmethod
    def printHanConfig(filename="collisions_run.config"):
        L1CaloMonitorCfgHelper.createXmls()
        from contextlib import redirect_stdout
        from PathResolver import PathResolver
        header = PathResolver.FindCalibFile("TrigT1CaloMonitoring/hanConfig_header.txt")
        with open(filename,'w') as f:
            # copy the header if we have one
            if header != "":
                with open(header,'r') as fh:
                    for line in fh: f.write(line)
            with redirect_stdout(f):
                # Note: common configs (see common dir in DataQualityConfiguration) provides following algorithms by default:
                #     algorithm All_Bins_Filled
                #     algorithm Histogram_Effective_Empty
                #     algorithm Histogram_Empty
                #     algorithm Histogram_Not_Empty
                #     algorithm No_OverFlows
                #     algorithm No_UnderFlows

                outputs = set()
                algorithms = set()
                def printConf(d,prefix="",currentOutput=""):
                    for key,value in d.items():
                        if type(value)==dict:
                            print(prefix,key,"{")
                            newCurrentOutput=str(currentOutput)
                            if(key=="dir detail" or key=="dir Developer"):
                                print(prefix+"  algorithm = GatherData") # define GatherData as default algo for all of these hists
                            if key.startswith("dir ") and any([x.startswith("hist ") for x in value.keys()]):
                                # this is a dir with hists, so specify output as the path .. take from the first hist child
                                for childKey,childVal in value.items():
                                    if childKey.startswith("hist ") and "output" in childVal:
                                        print(prefix+"  output = "+childVal["output"])
                                        newCurrentOutput = str(childVal["output"])
                                        break
                            printConf(value,prefix + "  ",newCurrentOutput)
                            print(prefix,"}")
                        else:
                            # save all output paths to add to output block (don't need to print here b.c. specified at dir level
                            # exception is if plot was embargoed!
                            if key == "output": outputs.add(value)
                            if key != "output" or value != currentOutput: print(prefix,key,"=",value)
                            if key == "algorithm": algorithms.add(value)


                print("#inputs")
                print("dir L1Calo {")
                printConf(L1CaloMonitorCfgHelper.hanConfigs," ")
                print("}")
                print("#outputs")
                print("output top_level {")
                print("  output L1Calo {")

                # go through outputs set, build a nested dictionary of paths
                outputsDict = {}
                for o in outputs:
                    theDict = outputsDict
                    for p in o.split("/"):
                        if p not in theDict: theDict[p] = {}
                        theDict = theDict[p]
                def printOutputs(d,prefix=""):
                     for key,value in d.items():
                         print(prefix,"output",key,"{")
                         if key=="detail" or key=="Developer": print(prefix,"  algorithm = L1Calo_AlwaysUndefinedSummary")
                         printOutputs(value,prefix + "  ")
                         print(prefix,"}")
                printOutputs(outputsDict["L1Calo"],"   ")
                print("  }")
                print("}")
                # include example of adding algorithms and thresholds
                print("""
#algorithms
algorithm AnyNonZeroBinIsError {
  # Use this algo if want error on any non-zero bin content
  libname = libdqm_summaries.so
  name = Bins_NotEqual_Threshold
  BinThreshold = 0.
  thresholds = th_AnyBinIsError
}
algorithm L1Calo_AlwaysUndefinedSummary {
    libname = libdqm_summaries.so
    name = AlwaysUndefinedSummary
}
""")
                thresholds = set()
                for algName,algProps in L1CaloMonitorCfgHelper.hanAlgConfigs.items():
                    if algName not in algorithms: continue # only print the algorithms that were used
                    print(f"algorithm {algName} {{")
                    for propName,propVal in algProps.items():
                        if propName == "thresholds": thresholds.add(propVal)
                        print(f"  {propName} = {propVal}")
                    print("  }")
                print("""
#thresholds
thresholds th_AnyBinIsError {
    limits NBins {
      warning = 0
      error = 1
    }
}
""")
                for threshName,threshProps in L1CaloMonitorCfgHelper.hanThresholdConfigs.items():
                    if threshName not in thresholds: continue # only print used thresholds
                    print(f"thresholds {threshName} {{")
                    for parName,parLims in threshProps.items():
                        print(f"  limits {parName} {{")
                        for limName,limVal in parLims.items():
                            print(f"    {limName} = {limVal}")
                        print("  }")
                    print("}")



    def __init__(self, flags, algClassOrObj = None, name = None, *args, **kwargs):
        '''
        Create the configuration helper.

        Arguments:
        flags -- the configuration flag object
        algClassOrObj -- the name you want to assign the family of algorithms
        '''
        from AthenaMonitoring import AthMonitorCfgHelper
        self.helper = AthMonitorCfgHelper(flags,name)
        self.alg = self.helper.addAlgorithm(algClassOrObj,name,*args, **kwargs) if algClassOrObj is not None else None
        self.fillGroups = {}
        self.dqEnv = flags.DQ.Environment # used to decide if should defineTree or not ...

    def defineDQAlgorithm(self,name,hanConfig,thresholdConfig=None):

        """

        :param name: name of algorithm
        :param hanConfig: dict of algo properties
        :param thresholdConfig: dict of thresholds, key = ParName, value = pair of thresholds [warning,error]
        :return:
        """

        # note: this method will replace any existing alg definition

        #thresNum = len(self.hanThresholdConfigs)
        if thresholdConfig is not None:
            hanConfig["thresholds"] = f"{name}Thresholds"
            threshDict = {}
            for parName,limVals in thresholdConfig.items():
                if len(limVals) != 2:
                    raise Exception("must specify two limits: warning and error")
                if parName not in threshDict: threshDict[parName] = {}
                threshDict[parName]["warning"] = limVals[0]
                threshDict[parName]["error"] = limVals[1]
            # see if any existing thresholds are identical, if so we can reuse
            for threshName,thresh in self.hanThresholdConfigs.items():
                if str(thresh)==str(threshDict):
                    threshDict = None
                    hanConfig["thresholds"] = threshName
                    break
            if threshDict is not None: self.hanThresholdConfigs[hanConfig["thresholds"]] = threshDict
        self.hanAlgConfigs[name] = hanConfig

        return


    def defineHistogram(self,*args,fillGroup=None,hanConfig={},paths=[],**kwargs):
        '''

        :param path:
        :param fillGroup:
        :param args:
        :param kwargs:
        :return:
        '''

        hanConfig = dict(hanConfig) # create a copy since will modify below (otherwise we end up modifying the default empty dict)

        if paths != []:
            for path in paths:
                # create a copy of the histogram in each of the extra locations
                self.defineHistogram(*args,fillGroup=fillGroup,hanConfig=hanConfig,paths=[],path=path,**kwargs)
            return None

        argsCopy = list(args) # need to convert tuple to list to convert it
        if ";" not in args[0]:
            argsCopy[0] += ";h_" + argsCopy[0].replace(":","_")

        if kwargs.get("path",None) is None:
            # put in the Developer path, under the name of the algorithm
            kwargs["path"] = "Developer/" + self.alg.name
        elif kwargs["path"][-1] == '/':
            kwargs["path"] = kwargs["path"][:-1] # strip trailing slash
        # verify path obeys convention
        splitPath = kwargs["path"].split("/")
        if splitPath[0] not in ["Shifter","Expert","Developer"]:
            raise Exception("Path of histogram invalid, does not start with one of the allowed audiences (Shifter,Expert,Developer)")

        # don't bother registering Shifter plots in tier0 ... only needed in p1 and user environments
        if splitPath[0]=="Shifter" and self.dqEnv == 'tier0':
            return None

        # require a hanConfig not in Developer or a detail dir
        if splitPath[0] != "Developer" and splitPath[-1] != "detail" and ("algorithm" not in hanConfig) and (splitPath[0]+"/algorithm" not in hanConfig):
            # will default to using GatherData as long as there is a description
            if "description" not in hanConfig and splitPath[0]+"/description" not in hanConfig:
                raise Exception("Must specify a hanConfig for a Shifter or Expert (non-detail) histogram")
            else:
                hanConfig["algorithm"] = "GatherData" # must have an algo, otherwise wont be valid han config
        elif "algorithm" in hanConfig and hanConfig["algorithm"] not in self.hanAlgConfigs and hanConfig["algorithm"] not in ["All_Bins_Filled","Histogram_Effective_Empty","Histogram_Empty","Histogram_Not_Empty","No_OverFlows","No_UnderFlows"]:
            # if algorithm specified, must have been defined
            raise Exception(f'DQ Algorithm {hanConfig["algorithm"]} for histogram {args[0]} not defined. Please use defineDQAlgorithm method to define it')
        elif splitPath[0]+"/algorithm" in hanConfig and hanConfig[splitPath[0]+"/algorithm"] not in self.hanAlgConfigs and hanConfig[splitPath[0]+"/algorithm"] not in ["All_Bins_Filled","Histogram_Effective_Empty","Histogram_Empty","Histogram_Not_Empty","No_OverFlows","No_UnderFlows"]:
            # if algorithm specified, must have been defined
            raise Exception(f'{splitPath[0]} DQ Algorithm {hanConfig[splitPath[0]+"/algorithm"]} for histogram {args[0]} not defined. Please use defineDQAlgorithm method to define it')

        if fillGroup is None: fillGroup = self.alg.name + "_fillGroup"
        if fillGroup not in self.fillGroups:
            self.fillGroups[fillGroup] = self.helper.addGroup(self.alg,fillGroup,topPath="L1Calo")


        if "merge" not in kwargs and kwargs.get("type","") !="TEfficiency":
            kwargs["merge"] = "merge" # ensures we don't get a warning about not specifying merge method
        out = self.fillGroups[fillGroup].defineHistogram(*argsCopy,**kwargs)
        histName = argsCopy[0].split(";")[-1]

        # add help link for all expert plots
        if splitPath[0] == "Expert" and self.HELPURL!="":
            linkUrl = self.HELPURL + "#" + "".join(splitPath[1:]+[histName])
            linkUrl = f"<a href=\"{linkUrl}\">Help</a>"
            if "description" not in hanConfig and "Expert/descripton" not in hanConfig: hanConfig["description"] = linkUrl
            elif "description" in hanConfig: hanConfig["description"] += " - " + linkUrl
            else: hanConfig["Expert/description"] += " - " + linkUrl

        # only add to hanConfig if in Expert or Developer folder
        if splitPath[0] == "Expert" or splitPath[0] == "Developer":
            splitPathWithHist = splitPath + [histName]
            # check if hist is in the embargoed list
            embargo = ("/".join(splitPathWithHist) in L1CaloMonitorCfgHelper.embargoed)
            if embargo:
                if "Expert/description" not in hanConfig: hanConfig["description"] = "<b>EMBARGOED PLOT</b> - " + (hanConfig.get("description",""))
                else: hanConfig["Expert/description"] = "<b>EMBARGOED PLOT</b> - " + (hanConfig.get("Expert/description",""))
            if "description" in hanConfig and hanConfig["description"]=="":
                del hanConfig["description"]
            if splitPath[0]+"/description" in hanConfig and hanConfig[splitPath[0]+"/description"]=="":
                del hanConfig[splitPath[0]+"/description"]
            x = L1CaloMonitorCfgHelper.hanConfigs
            for i,p in enumerate(splitPathWithHist):
                key = ("dir " if i!=len(splitPathWithHist)-1 else "hist ") + p
                if key not in x:
                    x[key] = {}
                x = x[key]
            hanConfig["output"] = "/".join(["L1Calo"]+[splitPath[0] if not embargo else "Developer"]+splitPath[1:]) #embargoed plots appear in developer folder in DQDM, but come from Expert folder in ROOT file
            # create a copy of the hanConfig and remove any keys beginning with "Shifter/", which means its a shifter-only config attribute
            myConfig = {}
            for k,v in hanConfig.items():
                if k.startswith("Shifter/"): continue
                # strip Expert/ prefix if it exists
                myConfig[str(k).replace("Expert/","")] = v
            if "algorithm" in myConfig and myConfig["algorithm"] in self.hanAlgConfigs:
                # if there are any string parameters in the config, add them to the description
                # need this until WebDisplay shows string parameters
                for pName,pVal in self.hanAlgConfigs[myConfig["algorithm"]].items():
                    if len(str(pVal))>0 and str(pVal)[0]=="\"":
                        myConfig["description"] += f"<br>{pName} : {pVal[1:-1]}"
            x.update(myConfig)
        elif splitPath[0] == "Shifter": # record shifter histograms in another map, for generating xmls
            # create a copy of the hanConfig and remove any keys beginning with "Expert/", which means its a expert-only config attribute
            myConfig = {}
            for k,v in hanConfig.items():
                if k.startswith("Expert/"): continue
                # strip Shifter/ prefix if it exists
                myConfig[str(k).replace("Shifter/","")] = v
            L1CaloMonitorCfgHelper.xmlConfigs["/".join(["L1Calo"]+splitPath+[histName])] = {}
            L1CaloMonitorCfgHelper.xmlConfigs["/".join(["L1Calo"]+splitPath+[histName])].update(myConfig) # copy in the config

        return out

    def defineTree(self,*args,fillGroup=None,**kwargs):

        if ";" not in args[0]:
            raise Exception("Must specify a tree name using ';name' suffix")
        treeName = args[0].split(";")[-1]

        if "," in args[1]: # catch a subtle typo that can screw up monitoring
            raise Exception("Should not have comma in list of branch names and types")

        if kwargs.get("path",None) is None:
            # put in the Developer path, under the name of the algorithm
            kwargs["path"] = "Developer/" + self.alg.name

        # verify path obeys convention
        splitPath = kwargs["path"].split("/")
        if splitPath[0] not in ["Developer"]:
            raise Exception("Path of tree invalid, must be in the audience=Developer directory")


        if fillGroup is None: fillGroup = self.alg.name + "_fillGroup"
        if fillGroup not in self.fillGroups:
            self.fillGroups[fillGroup] = self.helper.addGroup(self.alg,fillGroup,topPath="L1Calo")

        # always define a histogram to go with the tree, which will indicate how many entries there are
        # this also ensures we satisfy the requirement that every fillGroup has an object defined
        # (in the cases of running online/tier0 the defineTrees are blocked, but we still need a hist then)
        histName = "h_" + treeName + "_entries"
        argsCopy = list(args)
        argsCopy[0] = argsCopy[0].replace(";"+treeName,";"+histName)
        kwargsCopy = dict(kwargs)
        kwargsCopy["title"] = f"Number of Entries in {treeName} TTree" + ";" + ";".join(kwargsCopy.get("title","").split(";")[1:])
        kwargsCopy["opt"] = ['kCanRebin','kAddBinsDynamically','kAlwaysCreate']
        kwargsCopy["merge"] = "merge"
        is2d = (kwargsCopy["title"].count(";")>1)
        self.defineHistogram(argsCopy[0],type="TH2I" if is2d else "TH1I",xbins=1,xmin=0,xmax=1,ybins=1 if is2d else None,ymin=0,ymax=1,fillGroup=fillGroup,**kwargsCopy)
        if not any([x in self.dqEnv for x in ['tier0','online']]):
            out = self.fillGroups[fillGroup].defineTree(*args,**kwargs)
        else:
            out = None
        return out

    def result(self):
        return self.helper.result()


def LVL1CaloMonitoringConfig(flags):
    '''Function to call l1calo DQ monitoring algorithms'''
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.Enums import Format
    import logging

    # local printing
    local_logger = logging.getLogger('AthenaMonitoringCfg')
    info = local_logger.info
    info('In LVL1CaloMonitoringConfig')

    result = ComponentAccumulator()

    # If we're not putting trigger objects in event store, can't monitor them
    if not flags.Trigger.Online.isPartition:
        if not flags.DQ.triggerDataAvailable:
            return result

    isData = not flags.Input.isMC

    # check if validation requested
    validation=flags.DQ.Steering.LVL1Calo.doValidation

    # monitoring algorithm configs
    # do not run on MC or  RAW->ESD(tier0), or AOD-only
    if not validation and isData and flags.DQ.Environment not in ('tier0Raw', 'AOD'):

        from TrigT1CaloMonitoring.PprMonitorAlgorithm import PprMonitoringConfig
        from TrigT1CaloMonitoring.JepJemMonitorAlgorithm import JepJemMonitoringConfig

        # Use metadata to check Run3 compatible trigger info is available  
        from AthenaConfiguration.AutoConfigFlags import GetFileMD
        md = GetFileMD(flags.Input.Files)
        inputContainsRun3FormatConfigMetadata = ("metadata_items" in md and any(('TriggerMenuJson' in key) for key in md["metadata_items"].keys()))
        result.merge(PprMonitoringConfig(flags))
        result.merge(JepJemMonitoringConfig(flags))
        if flags.Input.Format is not Format.POOL or inputContainsRun3FormatConfigMetadata:
            # L1 menu available in the POOL file

            # since the legacy system started getting turned off in 2024, use detMask to determine
            # which things are included, and therefore need monitoring ...
            import eformat
            detMask=eformat.helper.DetectorMask(f'{md.get("detectorMask",[0x0])[0]:032x}') #DetectorMask constructor swallows two 64bit ints
            hasCPM = detMask.is_set(eformat.helper.SubDetector.TDAQ_CALO_CLUSTER_PROC_DAQ)
            hasJEP = detMask.is_set(eformat.helper.SubDetector.TDAQ_CALO_JET_PROC_DAQ)

            if hasCPM:
                from TrigT1CaloMonitoring.CpmMonitorAlgorithm import CpmMonitoringConfig
                from TrigT1CaloMonitoring.CpmSimMonitorAlgorithm import CpmSimMonitoringConfig
                result.merge(CpmMonitoringConfig(flags))
                result.merge(CpmSimMonitoringConfig(flags))

            if hasJEP:
                from TrigT1CaloMonitoring.JepCmxMonitorAlgorithm import JepCmxMonitoringConfig
                result.merge(JepCmxMonitoringConfig(flags))

            from TrigT1CaloMonitoring.OverviewMonitorAlgorithm import OverviewMonitoringConfig
            from TrigT1CaloMonitoring.PPMSimBSMonitorAlgorithm import PPMSimBSMonitoringConfig
            result.merge(PPMSimBSMonitoringConfig(flags))
            result.merge(OverviewMonitoringConfig(flags))

            if not hasCPM:
                # CPM was disabled for run 480893 onwards, so stop monitoring that part
                OverviewMonAlg = result.getEventAlgo("OverviewMonAlg")
                OverviewMonAlg.CPMErrorLocation = ""
                OverviewMonAlg.CPMMismatchLocation = ""

            if  flags.Input.TriggerStream == "physics_Mistimed":
                from TrigT1CaloMonitoring.MistimedStreamMonitorAlgorithm import MistimedStreamMonitorConfig
                result.merge(MistimedStreamMonitorConfig(flags))

        # For running on bytestream data
        if flags.Input.Format is Format.BS:
            from TrigT1CaloByteStream.LVL1CaloRun2ByteStreamConfig import LVL1CaloRun2ReadBSCfg
            result.merge(LVL1CaloRun2ReadBSCfg(flags))

        # Phase 1 monitoring of inputs and sim-vs-hw
        if flags.Trigger.enableL1CaloPhase1 and flags.Input.Format is not Format.POOL:

            # run the L1Calo simulation (causes conflicts with DAOD)
            from L1CaloFEXSim.L1CaloFEXSimCfg import L1CaloFEXSimCfg
            result.merge(L1CaloFEXSimCfg(flags))

            #efex monitoring
            if flags.Trigger.L1.doeFex:
                from TrigT1CaloMonitoring.EfexInputMonitorAlgorithm import EfexInputMonitoringConfig
                result.merge(EfexInputMonitoringConfig(flags))

                # monitoring of simulation vs hardware
                from TrigT1CaloMonitoring.EfexSimMonitorAlgorithm import EfexSimMonitoringConfig
                result.merge(EfexSimMonitoringConfig(flags))

            #gfex monitoring 
            if flags.Trigger.L1.dogFex:
                #gfex input monitoring 
                from TrigT1CaloMonitoring.GfexInputMonitorAlgorithm import GfexInputMonitoringConfig
                result.merge(GfexInputMonitoringConfig(flags))

                from TrigT1CaloMonitoring.GfexSimMonitorAlgorithm import GfexSimMonitoringConfig
                result.merge(GfexSimMonitoringConfig(flags))

            #jfex monitoring 
            if flags.Trigger.L1.dojFex:
                #jfex monitoring for input data
                from TrigT1CaloMonitoring.JfexInputMonitorAlgorithm import JfexInputMonitoringConfig
                result.merge(JfexInputMonitoringConfig(flags))
            
                #jfex monitoring for Data Vs Simulation
                from TrigT1CaloMonitoring.JfexSimMonitorAlgorithm import JfexSimMonitoringConfig
                JfexSimMonitoring = JfexSimMonitoringConfig(flags)
                result.merge(JfexSimMonitoring)

            if flags.Trigger.L1.doTopo and isData:
                #L1TopoSimulation (with monitoring Off to avoid clash with next call)
                from L1TopoSimulation.L1TopoSimulationConfig import L1TopoSimulationCfg
                result.merge(L1TopoSimulationCfg(flags,readMuCTPI=True,doMonitoring=False, useMuonDecoder=True, writeMuonRoIs = False))
                #L1TopoOnlineMonitoring specific for L1Calo DQPlots
                from L1TopoOnlineMonitoring.L1TopoOnlineMonitoringConfig import Phase1TopoMonitoringCfg
                result.merge(Phase1TopoMonitoringCfg(flags))

    # run FEX output monitoring if doing validation or running on data not @ tier0 or on AOD
    if validation or (isData and flags.DQ.Environment not in ('tier0Raw', 'AOD')):

        if validation:
            # only run this monitoring if doing validation
            from TrigT1CaloMonitoring.L1CaloLegacyEDMMonitorAlgorithm import L1CaloLegacyEDMMonitoringConfig
            result.merge(L1CaloLegacyEDMMonitoringConfig(flags))
        
        #efex monitoring
        if flags.Trigger.L1.doeFex:
            from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringConfig
            result.merge(EfexMonitoringConfig(flags))
            from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringHistConfig
            result.merge(EfexMonitoringHistConfig(flags,result.getEventAlgo('EfexMonAlg')))

        #gfex monitoring
        if flags.Trigger.L1.dogFex:
            from TrigT1CaloMonitoring.GfexMonitorAlgorithm import GfexMonitoringConfig
            result.merge(GfexMonitoringConfig(flags))
        
        #jfex monitoring
        if flags.Trigger.L1.dojFex:
            from TrigT1CaloMonitoring.JfexMonitorAlgorithm import JfexMonitoringConfig
            result.merge(JfexMonitoringConfig(flags))

        # jet efficiency monitoring -- needs either gfex or jfex to be worth scheduling
        if flags.Trigger.L1.dojFex or flags.Trigger.L1.dogFex:
            from TrigT1CaloMonitoring.JetEfficiencyMonitorAlgorithm import JetEfficiencyMonitoringConfig
            result.merge(JetEfficiencyMonitoringConfig(flags))

    result.printConfig( withDetails= True )

    return result

if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaMonitoring.DQConfigFlags import DQDataType
    flags = initConfigFlags()
    flags.DQ.useTrigger = False
    flags.DQ.Environment = "user"
    flags.DQ.DataType = DQDataType.MC
    flags.DQ.enableLumiAccess = False
    flags.lock()
    h = L1CaloMonitorCfgHelper(flags,CompFactory.JfexSimMonitorAlgorithm,"MyAlg")
    h.defineHistogram("x,y;h_myHist",type='TH2D',path="Shifter/Blah",hanConfig={"algorithm":"blah"})
    h.defineHistogram("x,y;h_myHis2t",type='TH2D',path="Shifter/Blah",hanConfig={"algorithm":"blah2"})
    h.defineHistogram("x;anotherHist",type='TH1D',path="Developer/whatever",hanConfig={"aaa":2})
    h.defineTree("x,y,z,a,b;myTree","x/i:y/i:z/i:a/I:b/I",path="Developer/whatever")

    # example of an algorithm requiring all bins to be empty
    h.defineDQAlgorithm("MyAlgo",
                        hanConfig={"libname":"libdqm_summaries.so","name":"Bins_NotEqual_Threshold","BinThreshold":"0."},
                        thresholdConfig={"NBins":[1,1]}) # first number if warning threshold, second is error threshold

    L1CaloMonitorCfgHelper.printHanConfig()
