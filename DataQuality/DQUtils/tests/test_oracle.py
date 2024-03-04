#! /usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def test_fetch_runs():
    from DQUtils import oracle
    from DQUtils.sugar import RunLumi, RANGEIOV_VAL

    # we just want to check this doesn't fail
    oracle.fetch_recent_runs()

    # these we check for consistency
    lastruns = set(oracle.fetch_last_n_atlas_runs(10))
    assert len(lastruns) == 10
    minrun, maxrun = min(lastruns), max(lastruns)
    selectedruns = {_[1] for _ in oracle.fetch_runs_since(minrun-1)}
    assert selectedruns == lastruns, f'{selectedruns}, {lastruns}'
    selectedruns = set(oracle.atlas_runs_between(minrun, maxrun))
    assert lastruns == selectedruns
    fakeiovs = [RANGEIOV_VAL(RunLumi(_, 1), RunLumi(_, 0xffffffff))
                for _ in range(minrun, maxrun+1)]
    assert oracle.filter_atlas_runs(fakeiovs).runs == lastruns

if __name__ == '__main__':
    test_fetch_runs()