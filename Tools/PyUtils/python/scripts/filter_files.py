# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @file PyUtils.scripts.filter_files
# @purpose take a bunch of input (pool/bs) files and produce a filtered one

__doc__ = "filter multiple input (pool/bs) files"


### imports -------------------------------------------------------------------
import PyUtils.acmdlib as acmdlib

@acmdlib.command(
    name='filter-files'
    )
@acmdlib.argument(
    '-o', '--output',
    required=True,
    help="Name of the filtered output file"
    )
@acmdlib.argument(
    'files',
    nargs='+',
    help='path to the input (pool/bs) files'
    )
@acmdlib.argument(
    '-s', '--selection',
    required=True,
    help='comma separated list of tuples (run,event) numbers to select or an ascii file containg a list of such run+event numbers to select'
    )
def main(args):
    """filter multiple input (pool/bs) files"""

    import PyUtils.Logging as L
    msg = L.logging.getLogger('filter-files')
    msg.setLevel(L.logging.INFO)

    msg.info(':'*40)

    import os.path as osp
    args.files = [ osp.expandvars(osp.expanduser(fname))
                   for fname in args.files ]

    args.selection = osp.expandvars(osp.expanduser(args.selection))
    
    msg.info('input files: %s', args.files)
    msg.info('output file: %s', args.output)
    msg.info('selection:   %s', args.selection)

    import os
    if os.path.exists(args.selection):
        selection = []
        with open(args.selection, 'r') as s:
            for line in s:
                if line.strip().startswith('#'):
                    continue
                l = line.strip().split()
                if len(l)==1: # assume this is only the event number
                    runnbr, evtnbr = None, int(l[0])
                elif len(l)==2: # a pair (run,evt) number
                    runnbr, evtnbr = int(l[0]), int(l[1])
                else:
                    raise RuntimeError(
                        'file [%s] has invalid format at line:\n%r' %
                        (args.selection, line)
                        )
                selection.append((runnbr, evtnbr))
    else:
        try:
            args.selection = eval(args.selection)
        except Exception as err:
            msg.error('caught:\n%s', err)
            msg.error('.. while trying to parse selection-string')
            import traceback
            traceback.print_exc()
            return 1
        
        selection = []
        for item in args.selection:
            if not isinstance(item, (tuple, list, int)):
                raise TypeError('type: %r' % type(item))

            if isinstance(item, (tuple, list)):
                if len(item) == 1:
                    runnbr, evtnbr = None, int(item[0])
                elif len(item) == 2:
                    runnbr, evtnbr = int(item[0]), int(item[1])
                else:
                    raise RuntimeError(
                        'item [%s] has invalid arity (%s)' %
                        (item, len(item))
                        )
            else:
                runnbr, evtnbr = None, int(item)
            selection.append((runnbr, evtnbr))

    # put back the massaged selection into our workspace
    args.selection = selection[:]

    from PyUtils.MetaReader import read_metadata
    metadata = read_metadata(args.files[0], None, 'lite')[args.files[0]]
    
    if metadata['file_type'] == 'BS':
        # optimization: run directly 'AtlCopyBSEvent.exe
        import subprocess
        cmd = ' '.join([
            'AtlCopyBSEvent',
            '-e %(evt-list)s',
            '%(run-list)s',
            '--out %(output)s',
            '%(files)s',
            ])
        evt_list = [str(i) for _,i in args.selection]
        run_list = [str(i) for i,_ in args.selection if i is not None]
        cmd = cmd % {
            'evt-list': ','.join(evt_list),
            'run-list': '' if len(run_list)==0 else '-r '+','.join(run_list),
            'output': args.output,
            'files':  ' '.join(args.files),
            }
        return subprocess.call(cmd.split())
    
    # Set the configuration flags
    msg.info('== Setting ConfigFlags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = args.files

    try:
        streamToOutput = flags.Input.ProcessingTags[0].removeprefix('Stream')
    except Exception as e:
        raise RuntimeError('Could not determine the stream type') from e

    for name, value in ((f'Output.{streamToOutput}FileName', args.output),
                        (f'Output.doWrite{streamToOutput}', True)):
        if flags.hasFlag(name):
            setattr(flags, name, value)
        else:
            flags.addFlag(name, value)
    if 'DAOD' in streamToOutput:
        flags.Output.doWriteDAOD = True

    import AthenaCommon.Constants as Lvl
    flags.Exec.OutputLevel=Lvl.WARNING

    # Lock and dump the configuration flags
    flags.lock()
    msg.info('== ConfigFlags Locked')

    # Setup the main services
    msg.info('== Configuring Main Services')
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # Setup the input reading
    msg.info('== Configuring Input Reading')
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # add event filtering algorithm
    from GaudiSequencer.PyComps import PyEvtFilter
    cfg.addEventAlgo(PyEvtFilter('filter_pyalg',
                                 evt_list=args.selection,
                                 # the store-gate key
                                 evt_info='EventInfo',
                                 is_mc=flags.Input.isMC,
                                 OutputLevel=Lvl.INFO),
                     sequenceName='AthAlgSeq')

    # Configure the output stream
    msg.info(f'== Configuring Output Stream {streamToOutput!r}')
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName
    cfg.merge(OutputStreamCfg(flags, streamToOutput, takeItemsFromInput=True, extendProvenanceRecord=False))

    # Configure metadata
    msg.info('== Configuring metadata for the output stream')
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from AthenaConfiguration.Enums import MetadataCategory

    cfg.merge(SetupMetaDataForStreamCfg(flags, streamToOutput,
                                        createMetadata=[MetadataCategory.IOVMetaData]))

    # Setup the output stream algorithm
    Stream = cfg.getEventAlgo(outputStreamName(streamToOutput))
    Stream.ForceRead = True
    Stream.AcceptAlgs += ['filter_pyalg']

    msg.info(f'== Configured {streamToOutput!r} writing')

    for item in flags.Input.TypedCollections:
        ctype, cname = item.split('#')
        if ctype.startswith(('Trk', 'InDet')):
            from TrkEventCnvTools.TrkEventCnvToolsConfig import TrkEventCnvSuperToolCfg
            cfg.merge(TrkEventCnvSuperToolCfg(flags))
        if ctype.startswith(('Calo', 'LAr')):
            from LArGeoAlgsNV.LArGMConfig import LArGMCfg
            cfg.merge(LArGMCfg(flags))
        if ctype.startswith(('Calo', 'Tile')):
            from TileGeoModel.TileGMConfig import TileGMCfg
            cfg.merge(TileGMCfg(flags))
        if ctype.startswith('Muon'):
            from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
            cfg.merge(MuonGeoModelCfg(flags))

    # Needed for merging in MT
    if 'ESD' in streamToOutput:
        Stream.ExtraInputs.add(
            ( 'MuonGM::MuonDetectorManager',
                  'ConditionStore+MuonDetectorManager' ) )
        Stream.ExtraInputs.add(
            ( 'InDetDD::SiDetectorElementCollection',
                  'ConditionStore+PixelDetectorElementCollection' ) )
        Stream.ExtraInputs.add(
            ( 'InDetDD::SiDetectorElementCollection',
                  'ConditionStore+SCT_DetectorElementCollection' ) )
        Stream.ExtraInputs.add(
            ( 'InDetDD::TRT_DetElementContainer',
                  'ConditionStore+TRT_DetElementContainer' ) )

    # Now run the job
    msg.info('== Running...')
    sc = cfg.run()

    # Exit accordingly
    return sc.isFailure()
