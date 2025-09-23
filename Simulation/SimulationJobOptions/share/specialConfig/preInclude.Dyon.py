
#########################################################################
#       preInclude.Dyon.py - Ethan Brooks, 1 Aug 2025                   #
#########################################################################

def load_files_for_dyon_scenario(MASS, CHARGE, GCHARGE):
    import os, shutil, sys

    from G4AtlasApps.SimFlags import simFlags
    from ExtraParticles.PDGHelpers import getLocalPDGTableName
    tableName = getLocalPDGTableName(simFlags.ExtraParticlesPDGTABLE.get_Value())
    f=open(tableName,'a')
    CODE=4110000+int(CHARGE)*10
    CODE2=4120000+int(CHARGE)*10

    ALINE1="M {code}                         {intmass}.E+03       +0.0E+00 -0.0E+00 DyonSS         0".format(code=CODE,intmass=int(MASS)) #Dyon magnetic and electric charges are the same sign
    ALINE2="W {code}                          0.E+00         +0.0E+00 -0.0E+00 DyonSS        0".format(code=CODE)
    ALINE3="M {code2}                         {intmass}.E+03       +0.0E+00 -0.0E+00 DyonOS         0".format(code2=CODE2,intmass=int(MASS)) #Dyon magnetic and electric charges are opposite signs
    ALINE4="W {code2}                          0.E+00         +0.0E+00 -0.0E+00 DyonOS        0".format(code2=CODE2)


    BLINE1="{code} {intmass}.00 {fcharge} {gcharge} # DyonSS".format(code=CODE, intmass=int(MASS), fcharge=float(CHARGE), gcharge=GCHARGE)
    BLINE2="-{code} {intmass}.00 -{fcharge} -{gcharge} # DyonSSBar".format(code=CODE, intmass=int(MASS), fcharge=float(CHARGE), gcharge=GCHARGE)
    BLINE3="{code2} {intmass}.00 -{fcharge} {gcharge} # DyonOS".format(code2=CODE2, intmass=int(MASS), fcharge=float(CHARGE), gcharge=GCHARGE)
    BLINE4="-{code2} {intmass}.00 {fcharge} -{gcharge} # DyonOSBar".format(code2=CODE2, intmass=int(MASS), fcharge=float(CHARGE), gcharge=GCHARGE)

    f=open('PDGTABLE.MeV','a')
    f.writelines(str(ALINE1))
    f.writelines('\n')
    f.writelines(str(ALINE2))
    f.writelines('\n')
    f.writelines(str(ALINE3))
    f.writelines('\n')
    f.writelines(str(ALINE4))
    f.writelines('\n')
    f.close()
    partmod = os.path.isfile('particles.txt')
    if partmod is True:
        os.remove('particles.txt')
    f=open('particles.txt','w')
    f.writelines(str(BLINE1))
    f.writelines('\n')
    f.writelines(str(BLINE2))
    f.writelines('\n')
    f.writelines(str(BLINE3))
    f.writelines('\n')
    f.writelines(str(BLINE4))
    f.writelines('\n')
    f.close()

    del ALINE1
    del ALINE2
    del ALINE3
    del ALINE4
    del BLINE1
    del BLINE2
    del BLINE3
    del BLINE4



doG4SimConfig = True
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
import PyUtils.AthFile as af
try:
    f = af.fopen(athenaCommonFlags.FilesInput()[0])

    if "StreamHITS" in f.infos["stream_names"]:
        from Digitization.DigitizationFlags import digitizationFlags
        simdict = digitizationFlags.specialConfiguration.get_Value()
        if simdict is None:
            # Here we are in a ReSim job, so the input is a HITS file
            raise ValueError
        doG4SimConfig = False
    else:
        from G4AtlasApps.SimFlags import simFlags
        if not "InteractingPDGCodes" in simFlags.specialConfiguration.get_Value():
            assert "CHARGE" in simdict
            CODE=4110000+int(float(simdict["CHARGE"])*10)
            CODE2=4120000+int(float(simdict["CHARGE"])*10)
            simFlags.specialConfiguration.get_Value()['InteractingPDGCodes'] = str([CODE,-1*CODE,CODE2,-1*CODE2])
        simdict = simFlags.specialConfiguration.get_Value()
except:
    from G4AtlasApps.SimFlags import simFlags
    if not "InteractingPDGCodes" in simFlags.specialConfiguration.get_Value():
        assert "CHARGE" in simdict
        CODE=4110000+int(float(simdict["CHARGE"])*10)
        CODE2=4120000+int(float(simdict["CHARGE"])*10)
        simFlags.specialConfiguration.get_Value()['InteractingPDGCodes'] = str([CODE,-1*CODE,CODE2,-1*CODE2])
    simdict = simFlags.specialConfiguration.get_Value()

assert "MASS" in simdict
assert "CHARGE" in simdict
assert "GCHARGE" in simdict
load_files_for_dyon_scenario(simdict["MASS"], simdict["CHARGE"], simdict["GCHARGE"])
pdgcodes = eval(simdict['InteractingPDGCodes']) if 'InteractingPDGCodes' in simdict else []
from ExtraParticles.PDGHelpers import updateExtraParticleWhiteList
updateExtraParticleWhiteList('G4particle_whitelist_ExtraParticles.txt', pdgcodes)

if doG4SimConfig:
    from G4AtlasApps import AtlasG4Eng
    AtlasG4Eng.G4Eng.log.info("Unlocking simFlags.EquationOfMotion to reset the value for Monopole simulation.")
    from G4AtlasApps.SimFlags import simFlags
    # FIXME ideally would include this file early enough, so that the unlocking is not required
    #simFlags.EquationOfMotion.unlock()
    simFlags.EquationOfMotion.set_On()
    simFlags.EquationOfMotion.set_Value_and_Lock("G4mplEqMagElectricField")#"MonopoleEquationOfMotion")
    simFlags.G4Stepper.set_Value_and_Lock('ClassicalRK4')
    simFlags.TightMuonStepping.set_Value_and_Lock(False)
    simFlags.PhysicsOptions += ["MonopolePhysicsTool"]
    # add monopole-specific configuration for looper killer
    simFlags.OptionalUserActionList.addAction('G4UA::MonopoleLooperKillerTool',['Step'])
    # add default HIP killer
    simFlags.OptionalUserActionList.addAction('G4UA::HIPKillerTool',['Step'])



del doG4SimConfig, simdict

