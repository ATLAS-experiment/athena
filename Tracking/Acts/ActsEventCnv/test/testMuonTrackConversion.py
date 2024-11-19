#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import json
from ActsConfig.ActsEventCnvConfig import RunTrackConversion
from MuonGeoModelTestR4.testGeoModel import geoModelFileDefault
import math

if "__main__" == __name__:

    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    args = flags.fillFromArgs()
    
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ActsEventCnv/q447_ESD.pool.root']
    flags.GeoModel.SQLiteDB = True
    flags.GeoModel.SQLiteDBFullPath = geoModelFileDefault()

    from AthenaConfiguration.TestDefaults import defaultConditionsTags
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC

    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags

    setupDetectorFlags(
        flags,
        None,
        use_metadata=True,
        toggle_geometry=True,
        keep_beampipe=True,
    )
    flags.Detector.GeometryITkPixel=False
    flags.Detector.GeometryITkStrip=False
    flags.Detector.GeometryHGTD=False

    flags.Muon.enableAlignment = True
    flags.Muon.applyMMPassivation = False
    # If we do not set this, get OFLCOND-MC15c-SDR-14-05 and:
    # "OFLCOND-MC15c-SDR-14-05 cannot be resolved for folder /MDT/MM/PASSIVATION"
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.CheckDependencies = False
    flags.Debug.DumpCondStore = True
    flags.Concurrency.NumThreads = 1
    flags.Exec.SkipEvents = 11
    flags.Exec.MaxEvents = 1

    flags.lock()
    flags.dump(evaluate=True)

    if not flags.Muon.usePhaseIIGeoSetup:
        print ("Please make sure that the file you're testing contains the Muon R4 geometry")
        import sys
        sys.exit(1)
    
    outputfile = 'muon_dump.json'
    track_collections = ['MuonSpectrometerTracks']
    RunTrackConversion(flags, track_collections, outputfile=outputfile)

    tolerance = 0.001
    def _valuesEqual(acts, trk):
        for acts_values, trk_values in zip(acts, trk):
            if (not math.isclose(acts_values, trk_values, rel_tol=tolerance)):
                return False
        return True
    
    def _printDifferences(acts,trk):
        for acts_values, trk_values in zip(acts, trk):
            if (not math.isclose(acts_values, trk_values, rel_tol=tolerance)):
                print('Acts:', acts_values)
                print('Trk:', trk_values)

    # Now compare outputs
    import json
    success = False
    with open(outputfile) as f:
        print('--- Dumping dump.json')
        print(f.read())

    with open(outputfile) as f:

        print('--- Processing',outputfile)
        data = json.load(f)
        for event in data:
            found_ni_differences = 0
            print('Processing', event)
            try:
                acts = data[event]['TrackContainers']['ConvertedTracks']
            except KeyError:
                continue
            for converted_track_collection in track_collections:
                try:
                    trk = data[event]['Tracks'][converted_track_collection]
                except KeyError as e:
                    print('ERROR: Keyerror ', e,' for Tracks in event', event)
                    print(data[event]['Tracks'])
                else:
                    if (acts != trk):
                        found_difference = False
                        # Okay, so simple comparison fails... let's try to find where
                        if (len(trk) != len(acts)):
                            print('ERROR: Acts and Trk track lengths differ!')
                            print('We have ', len(acts), '[',
                                len(trk), '] Acts [ Trk ] tracks')
                            found_difference = True

                        for i, (acts_track, trk_track) in enumerate(zip(acts, trk)):
                            if (not _valuesEqual(acts_track['dparams'], trk_track['dparams'])):
                                print('ERROR: Acts and Trk dparams differ for track', i)
                                _printDifferences(acts_track['dparams'], trk_track['dparams'])

                                found_difference = True
                            if (not _valuesEqual(acts_track['pos'], trk_track['pos'])):
                                print('ERROR: Acts and Trk pos differ for track', i)
                                _printDifferences(acts_track['pos'], trk_track['pos'])
                                found_difference = True
                            if not found_difference:
                                # Simple comparison failed, but no numerically significant difference found (in what we compare, at least)
                                # (Of course, this does not exclude that there are important differences in quantities we are not checking!)
                                found_ni_differences += 1
                                success=True
                    else:
                        print('SUCCESS: Acts and Trk tracks agree')
                        success = True
    if found_ni_differences > 0:
        print('INFO: Found', found_ni_differences, 'tracks which have minor (possibly insignificant) differences.')
    if not success:
        print('ERROR: the output of the conversion is not correct with a positional tolerance of {tol}%'.format(tol=tolerance*100))
        import sys
        sys.exit(1)
    else:
        print ('SUCCESS: the output of the conversion is correct (at least, to the precision we check, currently {tol}%)'.format(tol=tolerance*100))

