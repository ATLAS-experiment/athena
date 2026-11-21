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
    excludeAlgs = ["GlobalSimulationAlg", "PU1SuppTestBenchAlg", "LArCellMuxAlg", "Egamma1_OnlineMapNbhoodAlg"]
    availableAlgs = [f[:-3] for f in CompFactory.GlobalSim._getEntries()[-1] if f.endswith('Alg') and f not in excludeAlgs]


    if flags.hasFlag("L1CaloAthMon.UnknownArgs"):
        # declare additional parser arguments, and parse!
        import argparse
        parser = argparse.ArgumentParser(prog='l1global-minisim',formatter_class=argparse.RawTextHelpFormatter)
        parser.add_argument('--presim',nargs='+',default=[],choices=["efex","jfex","gfex","muon","lasp"],help="What to pre-simulate")
        parser.add_argument('--algs',nargs='*',default=[],help=f"algs to run. Available are: {', '.join(availableAlgs)}")
        parser.add_argument('--filesInput',nargs='*',default=[],help="If nibbler, specify as <key>:<path>. <key> can be storegate key or algorithm input property (<alg>.<input>)") # just here for help dialog
        parser.add_argument('--filesOutput',nargs='*',default=[],help="outputs to save, specify similarly to inputs, but <key> can also be comma separated list when piping to a pool file")
        if "--help" in flags.L1CaloAthMon.UnknownArgs:
            # before parsing, set the epilog to list of alg parameters
            epilog = "Available Algorithm Parameters (specify in '--postConfig' using \"<algName>.<propName>=<val>\" syntax)  :\n"
            from GaudiKernel.DataHandle import DataHandle
            for algType in availableAlgs:
                epilog += f"  {algType}:\n"
                configurable = getattr(CompFactory.GlobalSim,algType+"Alg")
                for propName,propVal in configurable.getDefaultProperties().items():
                    # skip properties that aren't part of the algorithm itself
                    description = configurable._descriptors[propName].__doc__
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
        if "lasp" in args.presim: flags.GlobalSim.Algs += ["LArCellPreparation"]
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
        configurable = getattr(CompFactory.GlobalSim,algType+"Alg")
        for propName,propVal in configurable.getDefaultProperties().items():
            if not isinstance(propVal,DataHandle): continue # skip non-handle properties
            sgKey = propVal.Path.split('+')[-1]
            # try to get the "type" from the property description
            for item in configurable._descriptors[propName].__doc__.split(";"):
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
    if algType=="LArCellPreparation" and algType==algName:
        # default LArCellPreparation ... use config method, don't add to algos
        from GlobalSimulation.LArCellPreparationAlgConfig import LArCellPreparationAlgCfg
        cfg.merge( LArCellPreparationAlgCfg(flags,name=algName), sequenceName=sequenceName )
    else:
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

# at this point need to apply any postConfig to set properties on algs, in case they affect output stream setup
applyPostConfig(skipUnapplied=True)

# add the GraphSvc to visualize job
cfg.addService( CompFactory.GlobalSim.GraphSvc(SequenceNameFilter=sequenceName,OutputLevel=3), create=True )

# get all inputs and outputs

allInputs = {}
allOutputs = {} # list of all outputs created during the job (i.e. writehandlekeys)

# get possible outputs from L1Calo presim:
from GaudiKernel.DataHandle import DataHandle
from GaudiConfig2 import Configurable

def getInOutputs(c,prefix=""):
    ins,outs = {},{}
    if prefix=="": prefix=c.name
    for prop,defaultVal in c.getDefaultProperties().items():
        propVal = getattr(c,prop)
        if isinstance(propVal,DataHandle) and propVal.Mode=='W':
            outs[prefix+"."+prop] = (propVal,c._descriptors[prop].__doc__,"+" in defaultVal.Path) # returns tuple of property and doc string
        elif isinstance(propVal,DataHandle) and propVal.Mode=='R':
            ins[prefix+"."+prop] = (propVal,c._descriptors[prop].__doc__,"+" in defaultVal.Path)
        elif isinstance(propVal,Configurable):
            sins,souts = getInOutputs(propVal,prefix+"."+prop) # recurse into subtools etc
            ins.update(sins)
            outs.update(souts)
    return ins,outs
for alg in cfg.getEventAlgos("L1Sim")+cfg.getEventAlgos(sequenceName):
    if alg is None: continue # if there is no L1Sim, seems getEventAlgos returns [None]
    inputs,outputs = getInOutputs(alg)
    allInputs.update(inputs)
    allOutputs.update(outputs)


if len(flags.GlobalSim.poolOutputs):
    # we have pool file outputting ... build the output streams
    from AthenaCommon.Logging import logging
    log = logging.getLogger('plugin_miniSim')

    # build item lists
    itemLists = {}
    import ROOT
    for output in flags.GlobalSim.poolOutputs:
        whats,stream = output.split(":")
        if stream not in itemLists: itemLists[stream] = []
        for what in whats.split(","): # supports comma-separated list of outputs into same stream
            if what in allOutputs:
                p = allOutputs[what][0]
                theType = ROOT.TClass.GetClass(p.Type).GetName()
                for a in ROOT.gROOT.GetListOfTypes():
                    if a.GetFullTypeName() == theType and "_v" not in a.GetName():
                        theType = a.GetName()
                        break
                itemLists[stream] += [f"{theType}#{p.Path.split('+')[-1]}",f"{'xAOD::AuxContainerBase' if theType=='xAOD::BaseContainer' else theType.replace('Container','AuxContainer')}#{p.Path.split('+')[-1]}Aux."]
            else:
                log.fatal(f"Unrecognised output: {what}")
                exit(-1)

    # register streams
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    for stream,items in itemLists.items():
        log.info(stream+" : "+" ".join(items))
        cfg.merge(OutputStreamCfg(flags, stream, ItemList=items,
                                  MetadataItemList=["IOVMetaDataContainer#*"],
                                  takeItemsFromInput=False,disableEventTag=True))
        cfg.getEventAlgo(f"Stream{stream}").WritingTool.SubLevelBranchName = "<key>"

    # create TagInfo metadata to store all properties of all algorithms relevant
    allProps = {}
    def fillProps(c,prefix=""):
        if prefix=="": prefix=c.name
        for prop in c.getDefaultProperties().keys():
            if prop in ["Cardinality","Enable","Asynchronous","FilterCircularDependencies","ExtraInputs","ExtraOutputs","MonitorService","NeededResources","ErrorMax","OutputLevel","RegisterForContextService","Timeline","EvtStore","DetStore","AuditFinalize","AuditExecute","AuditInitialize","AuditReinitialize","AuditRestart","AuditStart","AuditStop"]: continue
            propVal = getattr(c,prop)
            if isinstance(propVal,Configurable):
                fillProps(propVal,prefix+"."+prop) # recurse into subtools etc
            else:
                allProps["l1global-minisim/"+prefix+"."+prop] = str(propVal).split("+",1)[-1] if isinstance(propVal,DataHandle) else str(propVal)
    for alg in cfg.getEventAlgos("L1Sim")+cfg.getEventAlgos(sequenceName)+cfg.getEventAlgos("AthOutSeq")+cfg.getCondAlgos():
        if alg is None: continue # if there is no algo, seems getEventAlgos returns [None]
        fillProps(alg)

    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    cfg.merge(TagInfoMgrCfg(flags,allProps))

# use inputs and outputs to build a graph

# create a node for every algorithm in our sequence
with open("newgraph.dot", "w") as f:
    createdObjs = set()
    f.write("digraph datadeps {\n  rankdir=\"LR\";\n")

    for alg in cfg.getEventAlgos(sequenceName):
        if alg is None: continue
        label = alg.getType() + (('/'+alg.name) if alg.name != alg.getType else '')
        color = None
        if alg.getType() == "GlobalSim::TOBTextReader":
            label = str(alg.InputFile).split("/")[-1]
            color = 'blue'
        elif alg.getType() == "GlobalSim::TOBTextWriter":
            label = str(alg.OutputFile).split("/")[-1]
            color = 'blue'
        f.write(f"  \"Alg_{alg.name}\" [label=\"{label}\", shape=box{', color='+color if color is not None else ''}]\n")
        for propName,(propHandle,propDoc,notDecor) in allInputs.items():
            if propName.split(".")[0] != alg.name: continue
            # got an input
            p = propHandle.Path.split('+')[-1]
            if p == "": continue # skip blank keys
            if not notDecor: print("Decor = ",propName,propHandle.Path)
            if p not in createdObjs:
                # create a node for this object
                f.write(f"  \"{p}\" [label=\"{p}\", shape=plaintext]\n")
                createdObjs.add(p)
            # join input to alg node
            f.write(f" \"{p}\" -> \"Alg_{alg.name}\" [label=\"{propName.split('.',1)[-1]}\"];\n")
        for propName,(propHandle,propDoc,notDecor) in allOutputs.items():
            if propName.split(".")[0] != alg.name: continue
            # got an output
            p = propHandle.Path.split('+')[-1]
            if p == "": continue # skip blank keys
            if not notDecor: print("Decor = ",propName,propHandle.Path)
            if p not in createdObjs:
                # create a node for this object
                f.write(f"  \"{p}\" [label=\"{p}\", shape=plaintext]\n")
                createdObjs.add(p)
            # join alg output node
            f.write(f" \"Alg_{alg.name}\" ->  \"{p}\"  [label=\"{propName.split('.',1)[-1]}\"];\n")

    # add any presim outputs
    presimOutputs = {}
    for alg in cfg.getEventAlgos("L1Sim"):
        if alg is None: continue
        for propName,(propHandle,propDoc,notDecor) in allOutputs.items():
            if propName.split(".")[0] != alg.name: continue
            p = propHandle.Path.split('+')[-1]
            if p == "": continue # skip blank keys
            if not notDecor: continue # skip decoration outputs
            if p not in createdObjs:
                # create a node for this object
                f.write(f"  \"{p}\" [label=\"{p}\", shape=plaintext]\n")
                createdObjs.add(p)
            presimOutputs[propName] = p
    if len(presimOutputs):
        f.write(f"  \"presim\" [label=\"presim\", shape=box, color=purple]\n")
        if len(flags.Input.Files)>0:
            f.write(f"  \"presim_input\" [label=\"{flags.Input.Files[0].split('/')[-1]}\", shape=box, color=blue]\n")
            f.write(f" \"presim_input\" ->  \"presim\";\n")
        for l,v in presimOutputs.items():
            f.write(f" \"presim\" ->  \"{v}\"  [label=\"{l}\"];\n")

    # add output streams
    for alg in cfg.getEventAlgos("AthOutSeq"):
        if alg is None: continue
        # go through objects in ItemList ... if find any objects we created, add a node and join
        myItems = set()
        for item in alg.ItemList:
            if item.split('#')[-1] in createdObjs:
                myItems.add(item.split('#')[-1])
        if len(myItems):
            f.write(f"  \"Stream_{alg.name}\" [label=\"{alg.OutputFile}\", shape=box, color=red]\n")
            for item in myItems:
                f.write(f" \"{item}\" -> \"Stream_{alg.name}\";\n")
    f.write("}\n")

