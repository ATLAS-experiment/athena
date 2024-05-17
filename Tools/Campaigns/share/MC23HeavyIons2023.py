# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.BeamFlags import jobproperties as bf
bf.Beam.numberOfCollisions = 0

from Digitization.DigitizationFlags import digitizationFlags
from SimulationConfig.SimEnums import PixelRadiationDamageSimulationType
digitizationFlags.pixelPlanarRadiationDamageSimulationType.set_Value_and_Lock(PixelRadiationDamageSimulationType.RamoPotential.value)
digitizationFlags.dataRunNumber.set_Value_and_Lock(460000)

from AthenaCommon.Resilience import protectedInclude
protectedInclude('LArConfiguration/LArConfigRun3Old_NoPileup.py') # TO CHECK is this actually what we want c.f. LArConfigRun3Old.py
from AthenaCommon.BeamFlags import jobproperties as bf
bf.Beam.numberOfCollisions.set_Value_and_Lock(1.0)

protectedInclude('PyJobTransforms/HepMcParticleLinkVerbosity.py')
