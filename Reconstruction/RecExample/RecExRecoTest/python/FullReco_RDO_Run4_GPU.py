#!/usr/bin/env athena.py
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Example of configuring full reconstruction from RDO for Run4, using GPUs
# for as much of the reconstruction as possible.
#

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

from AthenaConfiguration.TestDefaults import defaultTestFiles
flags.Input.Files = defaultTestFiles.RDO_RUN4

flags.Output.AODFileName = 'myAOD.pool.root'
flags.Output.ESDFileName = 'myESD.pool.root'
flags.Exec.MaxEvents = 10

from AthenaConfiguration.TestDefaults import defaultConditionsTags
from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
setupDetectorFlags(flags, None,
                   use_metadata=True,
                   toggle_geometry=True,
                   keep_beampipe=True)
flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC

# Construct the tracking geometry for the GPU.
flags.Detector.GeometryMuon = False
flags.Detector.GeometryCalo = False
flags.Acts.TrackingGeometry.UseBlueprint = True
flags.Acts.TrackingGeometry.BuildDetrayGeometry = True

# Enable full GPU based track reconstruction.
flags.Acts.Device.doClusterization = True
flags.Acts.Device.doSeeding = True
flags.Acts.Device.doTrackReconstruction = True

# Enable GPU memory caching, and monitoring.
flags.Device.Memory.Cache = True
flags.Device.Memory.Monitor = True

flags.fillFromArgs()
flags.lock()

from RecJobTransforms.RecoSteering import RecoSteering
cfg = RecoSteering(flags)

# Run the configuration.
import sys
sys.exit(cfg.run().isFailure())
