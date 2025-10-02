#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

'''
@file TileInputFiles.py
@brief Python configuration of Tile input files
'''

from AthenaCommon.Logging import logging
from subprocess import check_output
from subprocess import CalledProcessError

def getInputDirectory(run, stream=None, project=None, suffix=None, year=None):
    """
    Function to find input directory with Tile Calorimeter input data files: calibrations, ...
    Arguments:
        run          -- run number
        stream       -- run stream
        project      -- data project
        suffix       -- directory suffix
        skipBadFiles -- skip known bad files
        year         -- year of data
    """

    log = logging.getLogger( 'TileInputFiles.getInputDirectory' )

    if run < 10:
        directory = '.'
    else:
        if not year:
            yr={             2023:441536, 2022:408681, 2021:387034, 2020:374260,
                2019:367983, 2018:342531, 2017:314451, 2016:288032, 2015:248505,
                2014:224307, 2013:216705, 2012:194688, 2011:171194, 2010:142682,
                2009:99717,  2008:35430,  2007:0}
            for year,beg in yr.items():
                if run>=beg:
                    break

        if stream or project or suffix:
            if not stream:
                stream = 'physics_Main'
                log.warning('%s is not set up and will be used: %s' , 'Run stream', stream)
            elif stream == 'Tile':
                stream = 'calibration_Tile'
            if not project:
                if 'calibration' in stream and 'Tile' not in stream:
                    project = f'data{year%100}_calib'
                else:
                    project = f'data{year%100}_13p6TeV'
                log.warning('%s is not set up and will be used: %s' , 'Data project', project)
            elif 'data' not in project:
                project = f'data{year%100}_{project}'
            if not suffix:
                if stream == 'physics_Main' or stream == 'physics_MinBias' or stream.startswith('calibration'):
                    suffix = 'daq.RAW'
                else:
                    suffix = 'merge.RAW'
                log.warning('%s is not set up and will be used: %s' , 'Directory suffix', suffix)

            run=str(run).zfill(8)
            directory = f'/eos/atlas/atlastier0/rucio/{project}/{stream}/{run}/{project}.{run}.{stream}.{suffix}'

        else:
            directory = f'/eos/atlas/atlascerngroupdisk/det-tile/online/{year}/daq'

    return directory


def findFiles(run, path=None, filter='.', stream=None, project=None, suffix=None, year=None, skipBadFiles=True):
    """
    Function to find Tile Calorimeter input data files: calibrations, ...
    Arguments:
        run          -- run number
        path         -- input directory
        filter       -- data file filter
        stream       -- run stream
        project      -- data project
        suffix       -- directory suffix
        year         -- year, data taken in
        skipBadFiles - skip known bad files
    """

    log = logging.getLogger( 'TileInputFiles.findFiles' )

    if not path:
        path = getInputDirectory(run, stream, project, suffix, year)

    if not path:
        log.warning('There is no input directory')
        return []

    log.info('Input directory: %s', path)

    run=str(run).zfill(7) if int(run) > 0 else str(run)
    if (path.startswith('/eos/')):
        listRunFiles = f'xrdfs eosatlas ls -l {path} | grep -e {run} | grep -v "#" '
        listRunFiles += f'| grep -v -e "         [ 0-9][ 0-9][0-9] " | grep {filter} | sed "s|^.*/||" '

    else:
        listRunFiles = f'ls {path} | grep -e {run} | grep {filter}'

    files = []
    try:
        files = check_output(listRunFiles, shell = True, text = True).splitlines()
    except CalledProcessError:
        log.warning('It seems that there are no such directory: %s', path)

    badFiles = ""
    if skipBadFiles:
        for badDataFiles in ['/afs/cern.ch/user/t/tilebeam/ARR/bad_data_files', '/afs/cern.ch/user/t/tiledaq/public/bad_data_files']:
            try:
                badFiles += open(badDataFiles).read()
            except Exception:
                log.warning('Can not read file with bad data files: %s => It is ignored', badDataFiles)

    fullNames = []
    for file_name in (files):
        good = (file_name not in badFiles)
        if good:
            if (path.startswith('/eos/')):
                fullNames.append(f'root://eosatlas.cern.ch/{path}/{file_name}')
            else:
                fullNames.append(f'{path}/{file_name}')
        else:
            log.warning('Excluding known bad data file: %s', file_name)

    return fullNames


def findFilesFromAgruments(args):
    """
    Function to find Tile Calorimeter input data files (calibrations, ...) from arguments
    Arguments:
       args   -- arguments prepared by argument parser
    """
    files = findFiles(run=args.run, path=args.inputDirectory, filter=args.filter, stream=args.stream,
                      project=args.project, suffix=args.suffix, year=args.year, skipBadFiles=args.skipBadFiles)
    return files


def getArgumentParser(**kwargs):
    """
    Function to construct and return argument parser
    """

    import argparse
    parser= argparse.ArgumentParser('Script to find Tile Calorimeter input data files: calibrations, ...', **kwargs)
    files = parser.add_argument_group('Tile find input files')
    files.add_argument("-r", "--run", type=int, default=None, help="Run number")
    files.add_argument("--inputDirectory", type=str, default=None, help="Input directory")
    files.add_argument("-s", "--stream", type=str, default=None, help="Run stream")
    files.add_argument("-p", "--project", type=str, default=None, help="Data project")
    files.add_argument("-f", "--filter", type=str, default=".", help="Data file filter")
    files.add_argument("--suffix", type=str, default=None, help="Directory suffix")
    files.add_argument("--skipBadFiles", type=bool, default=True, help="Skip bad data files?")
    files.add_argument("-y", "--year", type=int, default=None, help="Year, data taken in")

    return parser


if __name__=='__main__':

    log = logging.getLogger( 'TileInputFiles' )

    parser = getArgumentParser()
    args = parser.parse_args()

    files = findFilesFromAgruments(args)

    log.info('Input files: %s', files)
    if not files:
        log.warning("No run data files are found")
