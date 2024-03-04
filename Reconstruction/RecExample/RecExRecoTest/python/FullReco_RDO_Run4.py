#!/usr/bin/env athena.py --CA
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Example of configuring full reconstruction from RDO for Run4.

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

from AthenaConfiguration.TestDefaults import defaultTestFiles
flags.Input.Files = defaultTestFiles.RDO_RUN4

flags.Output.AODFileName = 'myAOD.pool.root'
flags.Output.ESDFileName = 'myESD.pool.root'
flags.Exec.MaxEvents = 10

from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
setupDetectorFlags(flags, None,
                   use_metadata=True,
                   toggle_geometry=True,
                   keep_beampipe=True)
flags.fillFromArgs()
flags.lock()

from RecJobTransforms.RecoSteering import RecoSteering
cfg = RecoSteering(flags)

cfg.run (flags.Exec.MaxEvents)
