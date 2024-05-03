#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def printConfiguration(flags) -> None:
    print()
    print("************************************************************************")
    
    print("******************** Tracking reconstruction Config ********************")
    print("                     Active Config is",flags.Tracking.ActiveConfig.extension)
    flags.dump(pattern="Tracking.ActiveConfig.doAthena*", evaluate=True)
    flags.dump(pattern="Tracking.ActiveConfig.doActs*", evaluate=True)
    print("************************************************************************")
    return

def deduceConfiguration(flags,
                        key: str):
    current_flags = flags.cloneAndReplace('Tracking.ActiveConfig',
                                          f'Tracking.ITk{key}Pass')
    return current_flags

if __name__ == "__main__":
    from TrkConfig.TrkConfigFlags import TrackingComponent
    configurations = ["Main",
                      "Acts",
                      "ActsHeavyIon",
                      "ActsValidateClusters",
                      "ActsValidateSpacePoints",
                      "ActsValidateSeeds",
                      "ActsValidateTracks",
                      "ActsValidateAmbiguityResolution",
                      "ActsBenchmarkSpot"]

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Acts.doAmbiguityResolution = False

    print("************************************************************************")
    for configuration in configurations:
        current_flags = deduceConfiguration(flags, configuration)
        printConfiguration(current_flags)

    # Test workflow with ambiguity resolution
    flags.Acts.doAmbiguityResolution = True
    current_flags = deduceConfiguration(flags, "ActsValidateTracks")
    printConfiguration(current_flags)

    # Test workflow with Conversion pass
    flags.Acts.doITkConversion = True
    current_flags = deduceConfiguration(flags, "ActsConversion")
    printConfiguration(current_flags)
