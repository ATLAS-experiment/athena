#!/usr/bin/env python
"""
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""

import sys
import subprocess


def runCommand(cmd, log, title, logFile):
    log.info(f'Running {title}:')
    log.info(' '.join([f'\'{o}\'' if ' ' in o else o for o in cmd]))

    try:
        subprocess.run(cmd, stdout=logFile, stderr=subprocess.STDOUT, text=True, check=True)
        log.info(f'Finished {title} successfully!')
    except subprocess.CalledProcessError:
        log.error(f'Failed to run {title}!')
        sys.exit(-1)


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Constants import INFO

    import logging
    log = logging.getLogger('TileTB')
    log.setLevel(INFO)
    flags = initConfigFlags()

    parser = flags.getArgumentParser(description='Run Tile TB simulation and digitization.')

    simargs = parser.add_argument_group('Tile TB simulation')
    simargs.add_argument('--runNumber', default=410000, help='Run number to simulate')
    simargs.add_argument('--physics-list', default=None, help='Override G4 physics list')
    simargs.add_argument('--conditions-tag', default='OFLCOND-MC12-SDR-27', help='Conditions tag to use')
    simargs.add_argument('--pid', default=11, help='Beam PID')
    simargs.add_argument('--energy', default=100000, help='Beam energy in MeV')
    simargs.add_argument('--eta', default=None, help='Set eta only if you want eta-projective scan')
    simargs.add_argument('--theta', default=None, help='For 90 degrees scans put theta=+/-90. Positive theta - beam enters from positive eta side (as defined in CTB setup!)')
    simargs.add_argument('--z', default=None, help='Z coordinate is calculated along front face of module at R=2290, \
    this is the distance from center of the modules to the desired impact point, positive value - impact point at positive eta side. \
    For 90 degrees scans sensitive Z is from 2300 to 2300+3*100+3*130+3*150+2*190=3820')
    simargs.add_argument('--y', default=None, help='Put non-zero value here if you want beam above (>0) or below (<0) center')
    simargs.add_argument('--phi', default=None, help='Put 5.625 or -5.625 if you want to rotate table to up/bottom module')
    simargs.add_argument('--layout', type=str, default='2B1EB', choices=['2B1EB', '2B2EB', '3B', '5B'], help='TileCal standalone setup with 2 barrels and 1 extended barrel on top (2B1EB), \
    2 barrels and 2 extended barrels on top (2B2EB), 3 barrels (3B), 5  barrels (5B) - use for sampling fraction calculation')
    simargs.add_argument('--ushape', default=None, help='Needed for the U-shape (any value of Ushape > 0 means that tile size equal to size of master plate, for Ushape <=0 - size of the tile is like in old geometry )')
    simargs.add_argument('--steel', default=None, help='Select steel with 0.45 percent Manganse for absorber instead of pure Iron. Any value > 0 enables Steel')
    simargs.add_argument('--pvt', default=None, help='Use PVT instead of PS for scintillator material. Any value > 0 enables PVT')
    simargs.add_argument('--cstube', default=None, help='Special option to enable Cs tubes in simulation. Any value > 0 enables them')
    simargs.add_argument('--verbose-tracking', action='store_true', help='Use verbose G4 tracking')
    simargs.add_argument('--calibration-run', action='store_true', help='Simulation with calibration hits')
    simargs.add_argument('--delta-thit', default=None, help='A time granularity for G4 hits in TileHit, \
    it can be just one number, e.g. 5.0 which means that the same, granularity is used everywhere or it can be a vector which contains 3*N+1 elements, \
    e.g. 0.1,-5,5, 0.5,-75,75, 5.0 which means that for [-5,5] ns interval granularity 0.1ns will be used  for [-75,75] ns interval granularity 0.5ns will be used \
    and 5ns granularity will be used for all other hits')
    simargs.add_argument('--time-cut', default=None, help='Time cut for hits, all hits go to one single time bin if time is above this cut ')
    simargs.add_argument('--plate-to-cell', default=None, help='Special flag for Calibration Hits. If true then Tile. Plates are the parts of the adjacent Tile cells. If false then they are Dead Materials')
    simargs.add_argument('--do-tile-raw', default=None, help='Enable energy per tile row in TileHit')
    simargs.add_argument('--do-tof-correction', default=None, help='Apply TOF correction (subtract Time Of Flight from ATLAS center')
    simargs.add_argument('--do-birk', default=None, help='Enable Birk\'s law')
    simargs.add_argument('--old-birk', action='store_true', help='Use expected values from NIM 80 (1970) 239-244: birk1=0.0130 g/(MeV*cm^2), birk2=9.6e-6 (g/(MeV*cm^2))^2')
    simargs.add_argument('--birk1', default=None, help='Parameter for Birk\'s law')
    simargs.add_argument('--birk2', default=None, help='Parameter for Birk\'s law')
    simargs.add_argument('--range-cut', default=None, help='Set non-standard range cut')
    simargs.add_argument('--vp1', action='store_true', help='Run VP1')

    digargs = parser.add_argument_group('Tile TB digitization')
    digargs.add_argument('--sfr-tag', default=None, help='Override sampling fraction, fixed: TileOfl02CalibSfr-SIM-06, eta dependent: TileOfl02CalibSfr-SIM-07')

    parser.set_defaults(evtMax=10)
    args, _ = parser.parse_known_args()

    layout = f'tb_Tile2000_2003_{args.layout}'
    filePrefix = f'tiletb_pid{args.pid}_ene{args.energy}'

    # =======>>> Set up the Tile TB simulation flags
    simulationFlags = [f'flags.TestBeam.BeamPID={args.pid}',
                       f'flags.TestBeam.BeamEnergy={args.energy}',
                       ' flags.TestBeam.Zbeam=[-20,20]',
                       ' flags.TestBeam.Ybeam=[-20,20]',
                       ' flags.TestBeam.Tbeam=[-31250,-23750]',
                       f'flags.TestBeam.Layout="{layout}"']

    if args.eta:
        filePrefix += f'_eta{args.eta}'
        simulationFlags += [f'flags.TestBeam.Eta={args.eta}']
    elif args.theta and args.z:
        filePrefix += f'_z{args.z}_{args.theta}deg'
        simulationFlags += [f'flags.TestBeam.Theta={args.theta}', f'flags.TestBeam.Z={args.z}']
    else:
        log.error('The following arguments should be provided: --eta or (--theta and --z)!')
        sys.exit(-1)

    if args.y:
        simulationFlags += [f'flags.TestBeam.Y={args.y}']
    if args.phi:
        simulationFlags += [f'flags.TestBeam.Phi={args.phi}']

    if args.verbose_tracking:
        simulationFlags += ['flags.Sim.G4Commands+=["/tracking/verbose 1"]']
    if args.calibration_run:
        simulationFlags += ['from SimulationConfig.SimEnums import CalibrationRun; flags.Sim.CalibrationRun=CalibrationRun.Tile']

    # Check if post configuration of Tile simulation is needed
    postConfig = False
    if any([args.ushape, args.steel, args.pvt, args.cstube, args.delta_thit, args.time_cut, args.plate_to_cell,
            args.do_tile_raw, args.do_tof_correction, args.do_birk, args.old_birk, args.birk1, args.birk2]):
        postConfig = True
        if args.ushape:
            simulationFlags += [f'flags.Tile.Sim.Ushape={args.ushape}']
        if args.steel:
            simulationFlags += [f'flags.Tile.Sim.Steel={args.steel}']
        if args.pvt:
            simulationFlags += [f'flags.Tile.Sim.PVT={args.pvt}']
        if args.cstube:
            simulationFlags += [f'flags.Tile.Sim.CsTube={args.cstube}']
        if args.delta_thit:
            simulationFlags += [f'flags.Tile.Sim.DeltaTHit={args.delta_thit}']
        if args.time_cut:
            simulationFlags += [f'flags.Tile.Sim.TimeCut={args.time_cut}']
        if args.plate_to_cell:
            simulationFlags += [f'flags.Tile.Sim.PlateToCell={args.plate_to_cell}']
        if args.do_tile_raw:
            simulationFlags += [f'flags.Tile.Sim.DoTileRow={args.do_tile_raw}']
        if args.do_tof_correction:
            simulationFlags += [f'flags.Tile.Sim.DoTOFCorrection={args.DoTOFCorrection}']
        if args.do_birk:
            simulationFlags += [f'flags.Tile.Sim.DoBirk={args.do_birk}']
        if args.old_birk:
            simulationFlags += [f'flags.Tile.Sim.OldBirk={args.old_birk}']
        if args.birk1:
            simulationFlags += [f'flags.Tile.Sim.birk1={args.birk1}']
        if args.birk2:
            simulationFlags += [f'flags.Tile.Sim.birk2={args.birk2}']

    # =======>>> Set up the Tile TB simulation preExec
    simulationPreExec = '; '.join(simulationFlags)

    # =======>>> Set up the Tile TB simulation postExec
    simulationPostExec = 'flags.dump(); '
    if args.range_cut:
        simulationPostExec += f' cfg.getService("PhysicsListSvc").GeneralCut={args.range_cut};'
    if args.vp1:
        simulationPostExec += ' from VP1Algs.VP1AlgConfig import VP1AlgCfg; cfg.merge(VP1AlgCfg(flags));'

    hitsFile = f'{filePrefix}.HITS.pool.root'

    # =======>>> Set up the Tile TB simulation command
    simulationCmd = ['TestBeam_tf.py', '--CA',
                     '--maxEvents', f'{args.evtMax}',
                     '--DataRunNumber', f'{args.runNumber}',
                     '--outputHITSFile', f'{hitsFile}',
                     '--conditionsTag', f'{args.conditions_tag}',

                     '--ignorePatterns', 'ISF_MC12Truth.+ERROR']
    if args.physics_list:
        simulationCmd += ['--physicsList', f'{args.physics_list}']
    if simulationPreExec:
        simulationCmd += ['--preExec', f'{simulationPreExec}']
    if simulationPostExec:
        simulationCmd += ['--postExec', f'{simulationPostExec}']

    # =======>>> Set up the Tile TB digitization flags
    digitizationFlags = '; '.join([f'flags.TestBeam.Layout="{layout}"'])

    # =======>>> Set up the Tile TB digitization command
    digitizationCmd = ['athena', '--CA', 'TileSimEx/TileDigiRec.py',
                       '--filesInput', f'{hitsFile}',
                       '--preExec', f'{digitizationFlags}',
                       '--file-prefix', f'{filePrefix}',
                       '--aant-ntuple',
                       '--hits-ntuple',
                       '--testbeam']

    # =======>>> Run Tile TB simulation and digitization commands
    with open(f'{filePrefix}.log', 'w') as logFile:
        runCommand(simulationCmd, log, 'Tile TB simulation', logFile)
        runCommand(digitizationCmd, log, 'Tile TB digitization', logFile)
