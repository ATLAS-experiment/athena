#!/bin/env python
#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# create all the histograms for each analysis - this will get called once for each 
# configured chain - we can set the HistPath either already here, or from the c++ 
# code 


# actual code to configure al;l the different algorithm instances for 
# the different slices 

import os
import sys
import glob

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TrigInDetAnalysisExample.chainString import chainString

from TrigIDR4Monitoring.TrigIDR4MonFlags  import TrigIDR4MonFlags
from TrigIDR4Monitoring.NodeUtils         import    decodeFlags
from TrigIDR4Monitoring.NodeUtils         import    updateFlags



# wrapper function for the central monitoring configuration 

def TrigIDR4MonConfig( flags, monlevels=None ):
        return TrigIDR4MonitoringCA( flags, monlevels )



# component accumulator wrapper around the overall
# monitoring functiom                

def TrigIDR4MonitoringCA( iflags, monlevels=None ):

        flags = decodeFlags( iflags )

        from AthenaMonitoring import AthMonitorCfgHelper
        monConfig = AthMonitorCfgHelper( flags, "TrigIDR4Mon" )
        
        ca = ComponentAccumulator()
        
        #        print( "monlevel: ", monlevels )

        if flags.Input.isMC:
                # this turns on the centra athena decoration algorithm
                if flags.IDTPM2.useDecorations:
                        from TrigIDR4Monitoring.TruthSelectBase import TruthPerigeeDecoratorCfg
                        ca.merge( TruthPerigeeDecoratorCfg( flags ) )

        # maybe this is too soom to specialise to IDTPM2, but hey ...
        
        algs  = TrigIDR4Monitoring( flags.IDTPM2, "Tier0", monlevel="idMon:t0:shifter", kak=ca )

        #   if flags.Input.isMC:    
        #        algs += TrigIDR4Monitoring( flags.IDTPM2, name="PhysVal", monlevel="idMon:t0" , kak=ca )
        #        algs += TrigIDR4Monitoring( flags.IDTPM2, name="PhysValShifter", monlevel="idMon:shifter", kak=ca )

        for a in algs : monConfig.addAlgorithm(a)

        ca.merge(monConfig.result())
        return ca


# As these debug ssettings will make things very slow, we don't want these
# but it is useful to leave this here to facilitate easy debugging
# ComponentAccumulator.debugMode = "trackCA trackEventAlgo trackPublicTool trackPrivateTool"

def TrigIDR4Monitoring( flags, name=None, monlevel=None, kak=None ) :  

        tools = []

        from AthenaCommon.Logging import logging
        log = logging.getLogger("TrigIDR4Monitoring")

        log.info( f"Creating monitoring: {name}" )

        # had key for a reason in the past - I wonder why it is
        # not being used now ??
        #  key     = "All"
        toolkey = "All"

        if monlevel is not None:
                log.info( "TIDA monitoring not None: monlevel: " + monlevel )        
                if "t0" in monlevel:
                        #    key     = "Expert"
                        toolkey = "Expert"
                elif "shifter" in monlevel:
                        #    key     = "Shifter"
                        toolkey = "Shifter"

        for node in flags.nodes(): log.info( f"Configuring analysis : {node.name()}" )

        for node in flags.nodes():
                TrigIDR4MonSignature( node, node.name(), toolkey, tools, monlevel, kak )

        return tools



def TrigIDR4MonSignature( flags, key, toolkey, tools, monlevel, kak ) :
        
        tida = TrigIDR4Mon_builder( flags, name = f"ID{key}_{toolkey}Tool" )
        
        tida.SliceTag = flags.SliceTag
        
        from TrigInDetMonitoring.TIDAChains import getchains

        hltchains = False

        for pattern in flags.Chains :
                if chainString(pattern).head:
                        hltchains = True
                        break
                
        if hltchains : chains = getchains( flags, flags.Chains, monlevel )
        else:          chains = flags.Chains

        if  len(chains)==0 : raise RuntimeError(f"No matched chains for Signature: {flags.name()}")
        
        if  len(chains)>0 : 

                tida.MonTools = createTATools( flags, flags.SliceTag, chains )

                tools += [ tida ]

        # just because I don't like hanging indentations at ends of functions 
        return

                

# create the actual algorithm 
def TrigIDR4Mon_builder( flags=None, name="NoName" ):
        from AthenaConfiguration.ComponentFactory import CompFactory
        alg =  CompFactory.TrigIDR4Mon( name=name )
        return alg



# create a separate specific monTool for each analysis chain
# - simplifies the overall analysis configuration

def createTATools( flags, label, chains, excludeTagChains=True, kak=None ):
        tools = []

        from TrigIDR4Monitoring.TrackAnalysisConfig import createTrackAnalysis

        for chain in chains :        
                cs = chainString(chain)
                if excludeTagChains and "tag" in cs.extra: continue
                # if it's a probe chain, find the correspondiong tag chain
                # and pass the pair to track analysis 
                tpchains = getTagnProbe( chain, chains, cs )
                tatool = createTrackAnalysis( flags, chain, tpchains, label, kak  )  
                tools += [ tatool ]
        return tools


def chomp(s, suffix):
        if s.endswith(suffix) :  return s[:-len(suffix)] 
        else : return s

        
def getTagnProbe( probe, chains, cs_probe=None ) :

        if cs_probe is None: cs_probe = chainString(probe)
        
        if "probe" not in cs_probe.extra : return [ probe ]
        
        prefix_probe = chomp( cs_probe.extra, "_probe" )
        
        # so this is a probe chain ...

        #        print( "searching for tag for ", probe )
        
        for tag in chains :        
                
                #                print( "\ttag: ", tag )
                
                if tag==probe : continue
                
                cstag = chainString(tag)

                # in principle, should be able to use any chain as a tag, and
                # any chain as a probe, but then each extra=xxx_probe would
                # need to be unique, which would prevent using wildcared
                # tag and probe chain pairs
                
                if cs_probe.head!=cstag.head : continue

                # actually, for the tag and probe analysis, you don't
                # really care about the tag trigger information, only the
                # reference selection to reconstruct the tnp selection
                # criterion

                # if cs_probe.tail!=cstag.tail : continue

                if "tag" not in cstag.extra : continue
                
                prefix_tag = chomp( cstag.extra, "_tag" )
        
                if prefix_probe != prefix_tag : continue

                #                print( "   Fount Tnp: tag:   ", tag ) 
                #                print( "   Fount Tnp: probe: ", probe ) 
                
                return [ probe, tag ]

        return [ probe ]
        
                        


def histsvc( flags ):

    from AthenaCommon.Logging import log

    if flags.Output.HISTFileName: 
            log.info( "histsvc: Create THistSvc with file name: "+flags.Output.HISTFileName )
    
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    ca = ComponentAccumulator()
    
    from AthenaConfiguration.ComponentFactory import CompFactory
    THistSvc = CompFactory.THistSvc
        
    histsvc = THistSvc()
    if flags.Output.HISTFileName:
        histsvc.Output += ["%s DATAFILE='%s' OPT='RECREATE'" % (flags.DQ.FileKey, flags.Output.HISTFileName)]
        log.info( "histsvc: "+histsvc.Output[-1] )

    ca.addService(histsvc)

    return ca



def file_exists( files ):
    if isinstance( files, str ): files = [ files ]
    for f in files:
        print( f"check file: {f}" )    
        # don't check xrootd files ...    
        if f.startswith(("root://","http://")): continue
        if not os.path.isfile(f):
            print(f"ERROR: Input file '{f}' does not exist, cwd: "+os.getcwd())
            sys.exit(1)

            
def is_mc( filename ):
    from PyUtils.MetaReader import read_metadata
    if filename.startswith(("root://","http://")): return None
    metadata = read_metadata(filename)
    return 'IS_SIMULATION' in metadata[filename].get('eventTypes', [])


def clean_args( values, args, cmdoption ):
    if not args : return
    for arg in args:
        if arg.startswith(cmdoption) :
            idx = args.index(arg)
            del args[idx]
            
    for arg in values:
        if arg in args: 
            idx = args.index(arg)
            del args[idx]


def parse_signatures( signatures, args=None ) :
    if not signatures: return None 
    sigs = []
    for sig in signatures:
        if sig.endswith(".py") or "=" in sig: continue
        sigs.append( sig )

    if args: clean_args( sigs, args, "--signatures" )    

    return sigs


def parse_files( files, args=None ) :

    if files is None : 
        print( "no input files" )
        return []
    
    if isinstance(files,str): files = [ files ]
    
    _out = []
    
    for file in files:
        if file.endswith(".py") or "=" in file: continue
        _out += file.split(",")

    out = []
        
    for f in _out :
        if f.startswith(("root://","http://")): out.append(f)
        else:  out += glob.glob(f)

    if not out :
        print( "no matching files", _out )

    if args: clean_args( _out, args, "--filesInput" )      
        
    return out
        

            
if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    log.info( "test running" )

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()

    # Input files
    # AOD file to be run w/ MT access and Mon Groups implemented
    file = 'AOD.pool.root'

    flags.Input.Files = [file]

    file_exists( flags.Input.Files )

    flags.Input.isMC  = is_mc( flags.Input.Files[0] )

    flags.Output.HISTFileName = 'duff.root'

    # in case we are looking at an express stream file ...
    flags.Common.doExpressProcessing=True

    # run using 10 threads and 5 events at a time, so 2 threads per events
    #  really these should be settable parameters
    # flags.Concurrency.NumThreads = 20
    # flags.Concurrency.NumConcurrentEvents = 10
    # flags.Concurrency.NumThreads = 10
    # flags.Concurrency.NumConcurrentEvents = 5
    # flags.Concurrency.NumThreads = 0
    # flags.Concurrency.NumConcurrentEvents = 0


    flags.Input.AODFixesDone = {"egammaAODFixesCfg"}

    tflags = decodeFlags( flags )
    
    TrigIDR4MonFlags( tflags )

    updateFlags(flags,tflags)

    tflags.lock()
    flags.lock()

    
    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(tflags)

    cfg.merge( PoolReadCfg(tflags) )

    cfg.merge( histsvc(tflags) )

    cfg.merge( TrigIDR4MonConfig( tflags ) ) 

    # If you want to turn on more detailed messages ...
    cfg.printConfig(withDetails=False) # set True for exhaustive info

    Nevents = 25000
    cfg.run(Nevents)



