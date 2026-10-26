#!/bin/env python
#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# create all the histograms for each analysis - this will get called once for each 
# configured chain - we can set the HistPath either already here, or from the c++ 
# code 
# actual code to configure al;l the different algorithm instances for 
# the different slices 

from TrigIDR4Monitoring  import TrigIDR4MonConfig 
from TrigIDR4Monitoring  import TrigIDR4MonFlags

from TrigIDR4Monitoring  import    decodeFlags
from TrigIDR4Monitoring  import    updateFlags

from TrigIDR4Monitoring.TrigIDR4MonConfig import histsvc
from TrigIDR4Monitoring.TrigIDR4MonConfig import parse_files
from TrigIDR4Monitoring.TrigIDR4MonConfig import parse_signatures


import sys

if __name__=='__main__':

    # Set the Athena configuration flags                                                                                                     
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()

    # Setup logs                                                                                                                             
    from AthenaCommon.Logging import log
    log.info( "test running" )

    parser = flags.getArgumentParser(conflict_handler='resolve')
    
    parser.add_argument( '--jsonConfig', type=str,  default="",   help='json config'  )
    parser.add_argument( '--output',     type=str,  default="",   help='output file'  )
    parser.add_argument( '--filesInput', nargs='+', default=None, help='files'        )
    parser.add_argument( '--signatures', nargs='+', default=None, help='signatures to enable (default, all signatures)')

    remaining = []

    for arg in sys.argv[1:]:
        if arg.endswith(".py") or ( not arg.startswith("-") and "=" in arg ) : continue
        remaining.append(arg)

    args, _ = parser.parse_known_args(remaining )

    # parse_files also checks if the files actually exist ...
    if args.filesInput is not None: flags.Input.Files = parse_files( args.filesInput, sys.argv )
    else:                           flags.Input.Files = parse_files( 'AOD.pool.root' )

    if not flags.Input.Files: sys.exit(0)

    signatures = parse_signatures( args.signatures, sys.argv )
    
    # default numbers of threads and slots - can be overridden from the cmd line
    flags.Concurrency.NumThreads = 20
    flags.Concurrency.NumConcurrentEvents = 10

    args = flags.fillFromArgs(parser=parser)

    if args.output: flags.Output.HISTFileName =  args.output
    else:           flags.Output.HISTFileName = 'duff.root'

    # allow command line argument to override flag
    if args.evtMax and args.evtMax != -1 : flags.Input.FileNentries = args.evtMax
        
    # in case we are looking at an express stream file ...
    flags.Common.doExpressProcessing = True

    if flags.Concurrency.NumThreads < flags.Concurrency.NumConcurrentEvents : flags.Concurrency.NumConcurrentEvents = flags.Concurrency.NumThreads

    flags.Input.AODFixesDone = { "egammaAODFixesCfg" }

    from TrigIDR4Monitoring.NodeUtils import timer
    
    tflags = decodeFlags(flags) 

    jsonConfig=None

    if args.jsonConfig :
        jsonConfig = args.jsonConfig
        log.info( f"json config: {jsonConfig}" )

    log.info( f"enable idtpm2 signatures: {signatures}" )
        
    timer( TrigIDR4MonFlags, tflags, signatures, jsonConfig )

    updateFlags(flags,tflags)

    tflags.Exec.FPE = 1

    flags.lock()
    
    tflags.lock()

    for f in flags.Input.Files: log.info( f"input file:   {f}" )
    
    log.info( f"output file:  {flags.Output.HISTFileName}" )

    log.info( f"threads:           {flags.Concurrency.NumThreads}" )
    log.info( f"concurrent events: {flags.Concurrency.NumConcurrentEvents}" )

    log.info( f"Nevents: {flags.Input.FileNentries}" )
    
    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg = MainServicesCfg(tflags)

    cfg.merge( PoolReadCfg(tflags) )

    cfg.merge( histsvc(tflags) )
    
    cfg.merge( TrigIDR4MonConfig( tflags ) ) 
    
    # If you want to turn on more detailed messages ...
    cfg.printConfig(withDetails=False) # set True for exhaustive info

    #    import ctypes
    
    #    libc = ctypes.CDLL("libm.so.6")
    #    libc.feenableexcept(0x01 | 0x04 | 0x08 )
    
    cfg.run(flags.Input.FileNentries)



