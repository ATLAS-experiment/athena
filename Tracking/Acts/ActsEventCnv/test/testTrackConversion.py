#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import json
from ActsConfig.ActsEventCnvConfig import RunTrackConversion
import math

if "__main__" == __name__:

    track_collections = ['CombinedITkTracks']

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from TrkConfig.TrackCollectionReadConfig import TrackCollectionReadCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    flags = initConfigFlags()
    args = flags.fillFromArgs()

    flags.Input.Files = [
        '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/ESD/ATLAS-P2-RUN4-03-00-00/ESD.ttbar_mu0.pool.root']
    flags.IOVDb.GlobalTag = "OFLCOND-MC15c-SDR-14-05"
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.CheckDependencies = True
    
    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, None, use_metadata=True,
                       toggle_geometry=True, keep_beampipe=True)

    flags.lock()
    flags.dump()

    RunTrackConversion(flags, track_collections)

    def _valuesEqual(acts, trk):
        for acts_values, trk_values in zip(acts, trk):
            if (not math.isclose(acts_values, trk_values)):
                return False
        return True

    # Now compare outputs
    import json
    success = False
    with open('dump.json') as f:
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
                except KeyError:
                    import pdb ; pdb.set_trace()
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
                            print('Acts dparams:', acts_track['dparams'])
                            print('Trk dparams:', trk_track['dparams'])
                            found_difference = True
                        if (not _valuesEqual(acts_track['pos'], trk_track['pos'])):
                            print('ERROR: Acts and Trk pos differ for track', i)
                            print('Acts pos:', acts_track['pos'])
                            print('Trk pos:', trk_track['pos'])
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
        print('ERROR: the output of the conversion is not correct')
        import sys
        sys.exit(1)
    else:
        print ('SUCCESS: the output of the conversion is correct (at least, to the precision we check)')

