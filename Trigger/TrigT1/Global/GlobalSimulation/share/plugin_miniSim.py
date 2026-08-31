# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


# this plugin is for mini simulation of a set of GlobalSim algorithms

# Example:
# l1global-minisim --algs JET1 --filesInput JET1.topoc_pu_type:/eos/atlas/atlascerngroupdisk/det-gbl/online/ValidationGate/input_data/hex/mc_events/JET1/myfile_tree.hex.txt --filesOutput JET1.topoc_pu_type:loopback.txt JET1.main_output:output.txt


def setup(flags):
    from AthenaCommon.Logging import logging
    log = logging.getLogger('plugin_miniSim')

    # turn off everything else from the steering script
    flags.DQ.doMonitoring=False
    flags.Trigger.enableL1CaloPhase1=False

    flags.addFlag("GlobalSim.Algs",[])
    flags.addFlag("GlobalSim.txtInputs",[])
    flags.addFlag("GlobalSim.txtOutputs",[])

    from AthenaConfiguration.ComponentFactory import CompFactory
    availableAlgs = [f[:-3] for f in CompFactory.GlobalSim._getEntries()[-1] if f.endswith('Alg')]

    if flags.hasFlag("L1CaloAthMon.UnknownArgs"):
        # declare additional parser arguments, and parse!
        import argparse
        parser = argparse.ArgumentParser()
        parser.add_argument('--algs',nargs='*',default=[],help=f"algs to run. Available are: {', '.join(availableAlgs)}")
        parser.add_argument('--filesInput',nargs='*',default=[],help="inputs. If nibbler, specify as <key>:<path>. <key> can be storegate key or algorithm input property (<alg>.<input>)") # just here for help dialog
        parser.add_argument('--filesOutput',nargs='*',default=[],help="outputs to save, specify similarly to inputs")
        args,unknown = parser.parse_known_args(flags.L1CaloAthMon.UnknownArgs)
        flags.L1CaloAthMon.UnknownArgs = unknown
        flags.GlobalSim.Algs = args.algs
        flags.GlobalSim.txtOutputs = args.filesOutput

    if len(flags.GlobalSim.Algs) == 0:
        log.fatal("You must specify what algorithms to run. Use the --algs option")
        log.fatal(f"Available algs: {availableAlgs}")
        exit(-1)

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
        else:
            pass # todo: should check if root file and if so, add to aod list


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

if flags.Output.AODFileName != "":

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
