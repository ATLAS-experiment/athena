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

    SIGNATURES = ["gJ","gLJ","gLJRho","gXEJWOJ","gXENC","gXERHO","jJ","jLJ","jEM","jTAU","jXE","jTE","eTAU","eEM"]

    @staticmethod
    def printHanConfig(filename="collisions_run.config"):
        from contextlib import redirect_stdout
        with open(filename,'w') as f:
            with redirect_stdout(f):
                # Note: common configs (see common dir in DataQualityConfiguration) provides following algorithms by default:
                #     algorithm All_Bins_Filled
                #     algorithm Histogram_Effective_Empty
                #     algorithm Histogram_Empty
                #     algorithm Histogram_Not_Empty
                #     algorithm No_OverFlows
                #     algorithm No_UnderFlows

                outputs = set()
                def printConf(d,prefix=""):
                    for key,value in d.items():
                        if type(value)==dict:
                            print(prefix,key,"{")
                            if(key=="dir detail" or key=="dir Developer"):
                                print(prefix+"  algorithm = GatherData") # define GatherData as default algo for all of these hists
                            printConf(value,prefix + "  ")
                            print(prefix,"}")
                        else:
                            print(prefix,key,"=",value)
                            # save all output paths to add to output block
                            if key == "output": outputs.add(value)

                print("#inputs")
                print("dir L1Calo {")
                printConf(L1CaloMonitorCfgHelper.hanConfigs," ")
                print("}")
                print("#outputs")
                print("output top_level {")
                def printOutputs(d,prefix=""):
                    for key,value in d.items():
                        if(key.startswith("hist ")):
                            pass # do nothing
                        elif type(value)==dict:
                            print(prefix,key.replace("dir ","output "),"{")
                            printOutputs(value,prefix + "  ")
                            print(prefix,"}")
                print("  output L1Calo {")
                printOutputs(L1CaloMonitorCfgHelper.hanConfigs,"   ")
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
#thresholds
thresholds th_AnyBinIsError {
    limits NBins {
      warning = 0
      error = 1
    }
}
""")




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

    def defineHistogram(self,*args,fillGroup=None,hanConfig={},paths=[],**kwargs):
        '''

        :param path:
        :param fillGroup:
        :param args:
        :param kwargs:
        :return:
        '''

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

        # require a hanConfig not in Developer or a detail dir
        if splitPath[0] != "Developer" and splitPath[-1] != "detail" and "algorithm" not in hanConfig:
            raise Exception("Must specify a hanConfig for a Shifter or Expert (non-detail) histogram")


        if fillGroup is None: fillGroup = self.alg.name + "_fillGroup"
        if fillGroup not in self.fillGroups:
            self.fillGroups[fillGroup] = self.helper.addGroup(self.alg,fillGroup,topPath="L1Calo")

        if "merge" not in kwargs and kwargs.get("type","") !="TEfficiency":
            kwargs["merge"] = "merge" # ensures we don't get a warning about not specifying merge method
        out = self.fillGroups[fillGroup].defineHistogram(*argsCopy,**kwargs)
        histName = argsCopy[0].split(";")[-1]

        splitPathWithHist = splitPath + [histName]
        x = L1CaloMonitorCfgHelper.hanConfigs
        for i,p in enumerate(splitPathWithHist):
            key = ("dir " if i!=len(splitPathWithHist)-1 else "hist ") + p
            if key not in x:
                x[key] = {}
            x = x[key]
        hanConfig["output"] = "/".join(["L1Calo"]+splitPath)
        x.update(hanConfig)

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
        kwargsCopy["opt"] = ['kCanRebin']
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
            from TrigT1CaloMonitoring.CpmMonitorAlgorithm import CpmMonitoringConfig
            from TrigT1CaloMonitoring.CpmSimMonitorAlgorithm import CpmSimMonitoringConfig
            from TrigT1CaloMonitoring.JepCmxMonitorAlgorithm import JepCmxMonitoringConfig
            from TrigT1CaloMonitoring.OverviewMonitorAlgorithm import OverviewMonitoringConfig
            from TrigT1CaloMonitoring.PPMSimBSMonitorAlgorithm import PPMSimBSMonitoringConfig
            
            result.merge(CpmMonitoringConfig(flags))
            result.merge(CpmSimMonitoringConfig(flags))
            result.merge(JepCmxMonitoringConfig(flags))
            result.merge(PPMSimBSMonitoringConfig(flags))
            result.merge(OverviewMonitoringConfig(flags))

            if  flags.Input.TriggerStream == "physics_Mistimed":
                from TrigT1CaloMonitoring.MistimedStreamMonitorAlgorithm import MistimedStreamMonitorConfig
                result.merge(MistimedStreamMonitorConfig(flags))

        # For running on bytestream data
        if flags.Input.Format is Format.BS:
            from TrigT1CaloByteStream.LVL1CaloRun2ByteStreamConfig import LVL1CaloRun2ReadBSCfg
            result.merge(LVL1CaloRun2ReadBSCfg(flags))

        # Phase 1 monitoring
        if flags.Trigger.enableL1CaloPhase1 and flags.Input.Format is not Format.POOL:
            #efex monitoring
            from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringConfig
            EfexMonitorCfg = EfexMonitoringConfig(flags)
            result.merge(EfexMonitorCfg)

            #  Need to pass the algorithm to the histogram booking
            EfexMonAlg = result.getEventAlgo('EfexMonAlg')  
            from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringHistConfig
            EfexMonitorHistCfg = EfexMonitoringHistConfig(flags,EfexMonAlg)
            result.merge(EfexMonitorHistCfg)

            #gfex monitoring 
            from TrigT1CaloMonitoring.GfexMonitorAlgorithm import GfexMonitoringConfig
            result.merge(GfexMonitoringConfig(flags))

            # run the L1Calo simulation (causes conflicts with DAOD)
            from L1CaloFEXSim.L1CaloFEXSimCfg import L1CaloFEXSimCfg
            result.merge(L1CaloFEXSimCfg(flags))

            # monitoring of simulation vs hardware
            from TrigT1CaloMonitoring.EfexSimMonitorAlgorithm import EfexSimMonitoringConfig
            result.merge(EfexSimMonitoringConfig(flags))

            from TrigT1CaloMonitoring.EfexInputMonitorAlgorithm import EfexInputMonitoringConfig
            result.merge(EfexInputMonitoringConfig(flags))

            from TrigT1CaloMonitoring.GfexSimMonitorAlgorithm import GfexSimMonitoringConfig
            result.merge(GfexSimMonitoringConfig(flags))

            #gfex input monitoring 
            from TrigT1CaloMonitoring.GfexInputMonitorAlgorithm import GfexInputMonitoringConfig
            result.merge(GfexInputMonitoringConfig(flags))
            
            #############################
            #           jFEX
            #############################
            
            #jfex monitoring for input data
            maybeMissingRobs = []
            decoderTools = []
            
            from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import jFexInputByteStreamToolCfg
            inputjFexTool = result.popToolsAndMerge(jFexInputByteStreamToolCfg(flags, 'jFexInputBSDecoderTool'))  
                      
            for module_id in inputjFexTool.ROBIDs:
                maybeMissingRobs.append(module_id)

            decoderTools += [inputjFexTool]
            from AthenaConfiguration.ComponentFactory import CompFactory
            decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder", DecoderTools=[inputjFexTool], MaybeMissingROBs=maybeMissingRobs)
            result.addEventAlgo(decoderAlg)     
                
            from L1CaloFEXSim.L1CaloFEXSimCfg import L1CaloFEXSimCfg
            result.merge(L1CaloFEXSimCfg(flags))              
            
            from TrigT1CaloMonitoring.JfexInputMonitorAlgorithm import JfexInputMonitoringConfig
            result.merge(JfexInputMonitoringConfig(flags))
            
            #jfex monitoring for Data Vs Simulation
            from TrigT1CaloMonitoring.JfexSimMonitorAlgorithm import JfexSimMonitoringConfig
            JfexSimMonitoring = JfexSimMonitoringConfig(flags)
            result.merge(JfexSimMonitoring)
            
            #jfex monitoring for Data Tobs
            from TrigT1CaloMonitoring.JfexMonitorAlgorithm import JfexMonitoringConfig
            JfexMonitoring = JfexMonitoringConfig(flags)
            result.merge(JfexMonitoring)
            
            # jet monitoring
            from TrigT1CaloMonitoring.JetEfficiencyMonitorAlgorithm import JetEfficiencyMonitoringConfig
            result.merge(JetEfficiencyMonitoringConfig(flags))
            
            result.printConfig( withDetails= True )


    # algorithms for validation checks
    if validation:
        from TrigT1CaloMonitoring.L1CaloLegacyEDMMonitorAlgorithm import L1CaloLegacyEDMMonitoringConfig
        result.merge(L1CaloLegacyEDMMonitoringConfig(flags))
        # Phase 1 systems
        from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringConfig
        EfexMonitorCfg = EfexMonitoringConfig(flags)
        result.merge(EfexMonitorCfg)
        # algorithm required for dynamic histogram booking
        EfexMonAlg = result.getEventAlgo('EfexMonAlg')
        EfexMonAlg.eFexEMTobKeyList = ['L1_eEMRoI', 'L1_eEMxRoI']
        EfexMonAlg.eFexTauTobKeyList = ['L1_eTauRoI', 'L1_eTauxRoI'] 
        from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringHistConfig
        EfexMonitorHistCfg = EfexMonitoringHistConfig(flags,EfexMonAlg)
        result.merge(EfexMonitorHistCfg)
        #
        from TrigT1CaloMonitoring.GfexMonitorAlgorithm import GfexMonitoringConfig
        result.merge(GfexMonitoringConfig(flags))
        from TrigT1CaloMonitoring.JfexMonitorAlgorithm import JfexMonitoringConfig
        result.merge(JfexMonitoringConfig(flags))

        from TrigT1CaloMonitoring.JetEfficiencyMonitorAlgorithm import JetEfficiencyMonitoringConfig
        result.merge(JetEfficiencyMonitoringConfig(flags))


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
    h.defineTree("x,y,z,a,b;myTree","x/i,y/i,z/i,a/I,b/I",path="Developer/whatever")
    L1CaloMonitorCfgHelper.printHanConfig()