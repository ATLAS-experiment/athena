#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import json
from ActsConfig.ActsEventCnvConfig import RunConversion
import math

if "__main__" == __name__:
    RunConversion()

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
            acts = data[event]['TrackContainers']['ConvertedTracks']
            trk = data[event]['Tracks']['CombinedITkTracks']
            if (acts != trk):
                found_difference = False
                # Okay, so simple comparison fails... let's try to find where
                print('WARNING: Acts and Trk tracks differ')
                print('We have ', len(acts), '[',
                      len(trk), '] Acts [ Trk ] tracks')
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
        print('WARNING: Found', found_ni_differences, 'tracks which have differences.')
    if not success:
        print('ERROR: the output of the conversion is not correct')
        import sys
        sys.exit(1)
    else:
        print ('SUCCESS: the output of the conversion is correct (at least, to the precision we check)')

