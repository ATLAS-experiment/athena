# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import ProductionStep


@AccumulatorCache
def load_files_for_quirks_scenario(flags):
    from ExtraParticles.PDGHelpers import getPDGTABLE
    if getPDGTABLE('PDGTABLE.MeV'):
        quirk_firststring=1e-6 #mm
        quirk_maxboost=1e-2
        quirk_maxmerge=1e-6 #mm
        quirk_debugenabled=False
        quirk_debugdist=1000 #mm
        quirk_debugsteps=1000
        simdict = flags.Input.SpecialConfiguration
        quirk_mass = float(simdict["MASS"])
        quirk_charge = float(simdict["CHARGE"])
        quirk_pdgid = int(simdict["PDGID"])
        quirk_stringforce = float(simdict["STRINGFORCE"])

        f = open('PDGTABLE.MeV', 'a')
        f.write("M%8d                          %.8E +0.0E+00 -0.0E+00 Quirk               +\n" % (quirk_pdgid, quirk_mass))
        f.write("W%8d                          0.E+00         +0.0E+00 -0.0E+00 Quirk               +\n" % quirk_pdgid)
        f.close()

        if flags.Common.ProductionStep == ProductionStep.Simulation:
            f = open('quirks_setup.txt', 'w')
            for x in [quirk_mass, quirk_charge, quirk_pdgid, quirk_stringforce, quirk_firststring, quirk_maxboost, quirk_maxmerge, quirk_maxmerge]:
                f.write(repr(x) + "\n")
            if quirk_debugenabled:
                f.write("1\n")
                f.write(repr(quirk_debugdist) + "\n")
                f.write(repr(quirk_debugsteps) + "\n")
            else:
                f.write("0\n")
            f.close()


def QuirksPhysicsToolCfg(flags, name="QuirksPhysicsTool", **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools( CompFactory.QuirksPhysicsTool(name, **kwargs) )
    return result


def DebugSteppingActionToolCfg(flags, name="DebugSteppingActionTool", **kwargs):
    result = ComponentAccumulator()
    # TODO UserActionConfig flag not yet migrated
    # example custom configuration
    # if name in flags.Sim.UserActionConfig.keys():
    #     for prop,value in flags.Sim.UserActionConfig[name].iteritems():
    #         kwargs.setdefault(prop,value)
    result.setPrivateTools( CompFactory.G4UA.DebugSteppingActionTool(name, **kwargs) )
    return result


def QuirksPreInclude(flags):
    """Declare the quirk to the EVNT->G4 input filter, before the flags lock."""
    simdict = flags.Input.SpecialConfiguration
    if "InteractingPDGCodes" not in simdict:
        assert "PDGID" in simdict
        CODE = int(simdict["PDGID"])
        simdict['InteractingPDGCodes'] = str([CODE, -CODE])
        flags.Input.SpecialConfiguration = simdict

    killer = 'G4UserActions.G4UserActionsConfig.MonopoleLooperKillerToolCfg'
    if killer not in flags.Sim.OptionalUserActionList:
        flags.Sim.OptionalUserActionList += [killer]


def QuirksCfg(flags):
    result = ComponentAccumulator()
    if flags.Common.ProductionStep == ProductionStep.Simulation:
        from G4AtlasServices.G4AtlasServicesConfig import PhysicsListSvcCfg
        result.merge(PhysicsListSvcCfg(flags))

    simdict = flags.Input.SpecialConfiguration
    assert "MASS" in simdict
    assert "CHARGE" in simdict
    assert "PDGID" in simdict
    assert "STRINGFORCE" in simdict
    load_files_for_quirks_scenario(flags)
    pdgcodes = eval(simdict['InteractingPDGCodes']) if 'InteractingPDGCodes' in simdict else []
    from ExtraParticles.PDGHelpers import updateExtraParticleAcceptList
    updateExtraParticleAcceptList('G4particle_acceptlist_ExtraParticles.txt', pdgcodes)

    if flags.Common.ProductionStep == ProductionStep.Simulation:
        from GaudiKernel.GaudiHandles import PrivateToolHandleArray
        physicsOptions = PrivateToolHandleArray([ result.popToolsAndMerge(QuirksPhysicsToolCfg(flags)) ])
        result.getService("PhysicsListSvc").PhysOption = physicsOptions + result.getService("PhysicsListSvc").PhysOption
    return result
