#!/usr/bin/env athena.py --CA
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Example of configuring full reconstruction from RDO for Run4.

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-01/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4345_r15583_tid39626672_00/RDO.39626672._001121.pool.root.1']

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
