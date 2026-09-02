# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


# this plugin is for mini simulation of a set of GlobalSim algorithms

# Example:
# l1global-minisim --algs JET1 --filesInput JET1.topoc_pu_type:/eos/atlas/atlascerngroupdisk/trig-gbl/online/ValidationGate/input_data/hex/mc_events/JET1/myfile_tree.hex.txt --filesOutput JET1.topoc_pu_type:loopback.txt JET1.main_output:output.txt

# example with pool output and presim:
# l1global-minisim --presim efex --filesInput /eos/atlas/atlascerngroupdisk/trig-gbl/offline/validation/valid.mc21_14TeV.537540.MGPy8EG_hh_bbbb_vbf_novhh_5fs_l1cvv1cv1.recon.AOD.e8557_s4422_r16130/AOD.41930061._000069.pool.root.1 --evtMax 10 --filesOutput eFEXDriver.eFEXSysSimTool.Key_eFexTauxTOBOutputContainer:my.pool.root eFEXDriver.eFEXSysSimTool.Key_eFexEMxTOBOutputContainer:my.pool.root


def setup(flags):
    from AthenaCommon.Logging import logging
    log = logging.getLogger('plugin_miniSim')

    # turn off everything else from the steering script by default, unless user requested through flags
    flags.DQ.doMonitoring=False
    flags.Trigger.enableL1CaloPhase1=any([flags.Trigger.L1.doeFex,flags.Trigger.L1.dojFex,flags.Trigger.L1.dogFex])

    flags.addFlag("GlobalSim.Algs",[])
    flags.addFlag("GlobalSim.txtInputs",[])
    flags.addFlag("GlobalSim.txtOutputs",[])
    flags.addFlag("GlobalSim.poolOutputs",[])

    from AthenaConfiguration.ComponentFactory import CompFactory
    excludeAlgs = ["LArCellPreparationAlg", "GlobalSimulationAlg", "PU1SuppTestBenchAlg", "LArCellMuxAlg", "Egamma1_OnlineMapNbhoodAlg"]
    availableAlgs = [f[:-3] for f in CompFactory.GlobalSim._getEntries()[-1] if f.endswith('Alg') and f not in excludeAlgs]


    if flags.hasFlag("L1CaloAthMon.UnknownArgs"):
        # declare additional parser arguments, and parse!
        import argparse
        parser = argparse.ArgumentParser(prog='l1global-minisim',formatter_class=argparse.RawTextHelpFormatter)
        parser.add_argument('--presim',nargs='+',default=[],choices=["efex","jfex","gfex","muon"],help="What to pre-simulate")
        parser.add_argument('--algs',nargs='*',default=[],help=f"algs to run. Available are: {', '.join(availableAlgs)}")
        parser.add_argument('--filesInput',nargs='*',default=[],help="inputs. If nibbler, specify as <key>:<path>. <key> can be storegate key or algorithm input property (<alg>.<input>)") # just here for help dialog
        parser.add_argument('--filesOutput',nargs='*',default=[],help="outputs to save, specify similarly to inputs")
        if "--help" in flags.L1CaloAthMon.UnknownArgs:
            # before parsing, set the epilog to list of alg parameters
            epilog = "Availble Algorithm Parameters:\n"
            from GaudiKernel.DataHandle import DataHandle
            from AthenaCommon import CfgMgr
            for algType in availableAlgs:
                epilog += f"  {algType}:\n"
                propDocs = getattr(CfgMgr,f"GlobalSim__{algType}Alg")._propertyDocDct
                for propName,propVal in getattr(CompFactory.GlobalSim,algType+"Alg").getDefaultProperties().items():
                    # skip properties that aren't part of the algorithm itself
                    description = propDocs[propName]
                    if f"[GlobalSim::{algType}Alg]" not in description: continue
                    description = description.replace(f"[GlobalSim::{algType}Alg]","")
                    epilog += f"    .{propName}: {description}\n"
            parser.epilog = epilog + "\n"
        args,unknown = parser.parse_known_args(flags.L1CaloAthMon.UnknownArgs)
        flags.L1CaloAthMon.UnknownArgs = unknown
        flags.GlobalSim.Algs = args.algs
        flags.GlobalSim.txtOutputs = args.filesOutput

        if "efex" in args.presim: flags.Trigger.L1.doeFex=True
        if "jfex" in args.presim: flags.Trigger.L1.dojFex=True
        if "gfex" in args.presim: flags.Trigger.L1.dogFex=True
        if "muon" in args.presim: flags.Trigger.L1.doMuon=True
        flags.Trigger.enableL1CaloPhase1=any([flags.Trigger.L1.doeFex,flags.Trigger.L1.dojFex,flags.Trigger.L1.dogFex])

    # determine inputs required (while checking algs exist)
    algInputs = set()
    algOutputs = set()
    inputsMap = {}
    outputsMap = {}
    knownTypes = {} # if can identify the type from the alg metadata, will collect the types here
    for alg in flags.GlobalSim.Algs:
        # check alg exists
        algType,algName = (alg.split("/") if "/" in alg else [alg,alg])
        if not hasattr(CompFactory.GlobalSim,algType+"Alg"):
            log.fatal(f"Unknown Alg: {algType}")
            log.fatal(f"Available Algs: {availableAlgs}")
            exit(-1)
        # now iterate through read and write handles
        from GaudiKernel.DataHandle import DataHandle
        from AthenaCommon import CfgMgr
        propDocs = getattr(CfgMgr,f"GlobalSim__{algType}Alg")._propertyDocDct
        for propName,propVal in getattr(CompFactory.GlobalSim,algType+"Alg").getDefaultProperties().items():
            if not isinstance(propVal,DataHandle): continue # skip non-handle properties
            sgKey = propVal.Path.split('+')[-1]
            # try to get the "type" from the property description
            for item in propDocs[propName].split(";"):
                item = item.strip()
                if item.startswith("type="):
                    knownTypes[sgKey] = item[len("type="):]
                    knownTypes[f"{algName}.{propName}"] = item[len("type="):]
                    break
            if propVal.Mode == 'W':
                algOutputs.add(sgKey)
                if sgKey not in outputsMap: outputsMap[sgKey] = []
                outputsMap[sgKey] += [f"{algName}.{propName}"]
            else:
                algInputs.add(sgKey)
                if sgKey not in inputsMap: inputsMap[sgKey] = []
                inputsMap[sgKey] += [f"{algName}.{propName}"]

    # remove from inputs the outputs of the other algs
    # what is left will be what we need provided externally
    algInputs -= algOutputs

    log.info(f"Your algorithms {flags.GlobalSim.Algs} require the following inputs: {[i for a in algInputs for i in inputsMap[a]]}")
    log.info(f"and produces the following outputs: {[i for a in algOutputs for i in outputsMap[a]]}")

    # move any txt inputs into the txtInputs flag
    cleanList = []
    for f in flags.Input.Files:
        if f.endswith(".txt"):
            # must have two ":" in it ... check
            if f.count(":") != 2:
                if f.count(":")==1:
                    if f.split(":")[0] in knownTypes:
                        f = knownTypes[f.split(":")[0]] + ":" + f
                    else:
                        log.fatal(f"{f.split(':')[0]} has no known type. Add to algorithm metadata or otherwise specify it explicitly with '<type>:' prefix")
                        exit(-1)
                else:
                    log.fatal("txt input must be specified in form <type>:<SGkey>:<filepath>")
                    exit(-1)
            flags.GlobalSim.txtInputs += [f]
        else:
            cleanList += [f]
    flags.Input.Files = cleanList

    if len(flags.GlobalSim.txtInputs):
        flags.Input.isMC = True # tell parts of job to behave as this was a simulation

        if flags.Exec.MaxEvents==-1:
            # determine max number of events of given input file types
            maxVals = {}
            for f in flags.GlobalSim.txtInputs:
                typeAndName,path = f.rsplit(":",1)
                if typeAndName not in maxVals: maxVals[typeAndName]=0
                maxVals[typeAndName] += sum(1 for _ in open(path))
            flags.Exec.MaxEvents = max(maxVals.values())
            # use the total number of lines in all the input files as the max events
            log.info(f"Processing {flags.Exec.MaxEvents} events from txt filesInput")

    txtOutputs = list(flags.GlobalSim.txtOutputs)
    flags.GlobalSim.txtOutputs = []
    filenameMap = {}
    for f in txtOutputs:
        if f.endswith(".txt"):
            # must have two ":" in it ... check
            if f.count(":") != 2:
                if f.count(":")==1:
                    if f.split(":")[0] in knownTypes:
                        f = knownTypes[f.split(":")[0]] + ":" + f
                    else:
                        log.fatal(f"{f.split(':')[0]} has no known type. Add to algorithm metadata or otherwise specify it explicitly with '<type>:' prefix")
                        exit(-1)
                else:
                    log.fatal("txt output must be specified in form <type>:<SGkey>:<filepath>")
                    exit(-1)
            flags.GlobalSim.txtOutputs += [f]
        elif f.endswith(".pool.root"):
            # need to add filenames to available outputs
            filename = f.split(":")[-1]
            if filename not in filenameMap:
                filenameMap[filename] = f"GSIM{len(filenameMap)}"
                flags.addFlag("Output."+filenameMap[filename]+"FileName",filename) # used by OutputStreamConfig
            flags.GlobalSim.poolOutputs += [f.replace(filename,filenameMap[filename])]


# I may move this subsequence creation up into the steering script if I can figure out way to control `addEventAlgo` calls
sequenceName = "GlobalSim"
cfg.addSequence(CompFactory.AthSequencer(sequenceName,StopOverride=True),parentName="AthAlgSeq")


# ---------------
# algorithms added here:

# create algos first, as will use properties to create readers
algos = []
for alg in flags.GlobalSim.Algs:
    algType,algName = (alg.split("/") if "/" in alg else [alg,alg])
    theAlg = getattr(CompFactory.GlobalSim,algType+"Alg")(algName)
    algos += [theAlg]


# add any readers for processing txt files
readerNames = []
for f in flags.GlobalSim.txtInputs:
    tobType,sgKey,filepath = f.split(":")
    if "." in sgKey:
        # is an algo property not a key
        foundProp = False
        for a in algos:
            if a.name == sgKey.split(".")[0]:
                sgKey = getattr(a,sgKey.split(".")[1]).Path.split('+')[-1]
                foundProp=True
                break
        if not foundProp:
            log.fatal(f"Unknown input property: {sgKey}")
            exit(-1)

    cfg.addEventAlgo( CompFactory.GlobalSim.TOBTextReader(
        sgKey+"Reader",BitSpec=tobType,
        InputFile=filepath,
        Output=sgKey), sequenceName=sequenceName )
    readerNames += [sgKey+"Reader"]

# now add the algs
for theAlg in algos: cfg.addEventAlgo( theAlg, sequenceName=sequenceName )

# add any writers
for f in flags.GlobalSim.txtOutputs:
    tobType,sgKey,filepath = f.split(":")
    if "." in sgKey:
        # is an algo property not a key
        foundProp = False
        for a in algos:
            if a.name == sgKey.split(".")[0]:
                sgKey = getattr(a,sgKey.split(".")[1]).Path.split('+')[-1]
                foundProp=True
                break
        if not foundProp:
            log.fatal(f"Unknown output property: {sgKey}")
            exit(-1)
    cfg.addEventAlgo( CompFactory.GlobalSim.TOBTextWriter(
        sgKey+"Writer",
        BitSpec=tobType,
        Input=sgKey,
        OutputFile=filepath), sequenceName=sequenceName )



# ----------------

# add the GraphSvc to visualize job
cfg.addService( CompFactory.GlobalSim.GraphSvc(SequenceNameFilter=sequenceName,OutputLevel=3), create=True )

if len(flags.GlobalSim.poolOutputs):
    # we have pool file outputting ... build the output streams

    allOutputs = {} # list of all outputs created during the job (i.e. writehandlekeys)

    # get possible outputs from L1Calo presim:
    from GaudiKernel.DataHandle import DataHandle
    from GaudiConfig2 import Configurable
    def getOutputs(c,prefix=""):
        out = {}
        for prop in c.getDefaultProperties().keys():
            propVal = getattr(c,prop)
            if isinstance(propVal,DataHandle) and propVal.Mode=='W':
                out[prefix+"."+prop] = propVal
            elif isinstance(propVal,Configurable):
                out.update(getOutputs(propVal,prefix+"."+prop)) # recurse into subtools etc
        return out
    for alg in cfg.getEventAlgos("L1Sim")+cfg.getEventAlgos(sequenceName):
        if alg is None: continue # if there is no L1Sim, seems getEventAlgos returns [None]
        outputs = getOutputs(alg,alg.name)
        allOutputs.update(outputs)

    # build item lists
    itemLists = {}
    import ROOT
    for output in flags.GlobalSim.poolOutputs:
        what,stream = output.split(":")
        if stream not in itemLists: itemLists[stream] = []
        if what in allOutputs:
            p = allOutputs[what]
            theType = ROOT.TClass.GetClass(p.Type).GetName()
            for a in ROOT.gROOT.GetListOfTypes():
                if a.GetFullTypeName() == theType and "_v" not in a.GetName():
                    theType = a.GetName()
                    break
            itemLists[stream] += [f"{theType}#{p.Path.split('+')[-1]}",f"{'xAOD::AuxContainerBase' if theType=='xAOD::BaseContainer' else theType.replace('Container','AuxContainer')}#{p.Path.split('+')[-1]}Aux."]

    # register streams
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    for stream,items in itemLists.items():
        print(stream,":",items)
        cfg.merge(OutputStreamCfg(flags, stream, ItemList=items,takeItemsFromInput=False,disableEventTag=True))

    def storeOutput(algName):
        # this helper method should probably be relocated to python folder for general use t some point
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        from GaudiKernel.DataHandle import DataHandle
        algo = cfg.getEventAlgo(algName)
        items = []
        for propName in algo.getDefaultProperties().keys():
            p = algo.__getattribute__(propName)
            if isinstance(p,DataHandle) and p.Mode=='W':
                items += [f"{p.Type}#{p.Path.split('+')[-1]}",f"xAOD::AuxContainerBase#{p.Path.split('+')[-1]}Aux."]
        cfg.merge(OutputStreamCfg(flags, 'AOD', ItemList=items,takeItemsFromInput=False,disableEventTag=True))

    for r in readerNames: storeOutput(r)
    for a in algos: storeOutput(a.name)
