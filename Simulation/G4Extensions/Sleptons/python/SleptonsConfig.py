# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
import sys, shutil, re
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.SystemOfUnits import GeV,ns # noqa: F401
from AthenaConfiguration.Enums import ProductionStep

# Example specialConfiguration {'GMSBSlepton': '100.0*GeV', 'GMSBGravitino': '1e-07*GeV', 'GMSBSleptonTime': '0.01*ns'}

"""
Defining default settings for slepton/staus. Possible options are:
G4ParticleMass (default 0.0*GeV)
G4ParticleWidth (default 0.0*GeV)
G4ParticleCharge (default +/-1.*eplus)
G4ParticlePDGCode (default sparticle pdgid)
G4ParticleStable (default True)
G4ParticleLifetime (default -1)
G4ParticleShortlived (default False)
where Particle = [STau1Minus, STau1Plus, STau2Minus, STau2Plus, SElectronRMinus, SElectronRLinus, SElectronRPlus, SElectronLPlus, SMuonRMinus, SMuonLMinus, SMuonRPlus, SMuonLPlus]
"""

# The mass of the a1(1260) meson.  The a1(1260) meson is the heaviest of the decay products when the tau is off shell.
Mass_a1Meson = 1260. # MeV

@AccumulatorCache
def get_and_fix_PDGTABLE_GMSB(replace):

    # Download generic PDGTABLE (overwrite existing one if it exists)
    from ExtraParticles.PDGHelpers import getPDGTABLE
    if getPDGTABLE('PDGTABLE.MeV'):
        shutil.move('PDGTABLE.MeV', 'PDGTABLE.MeV.org')

        # an example line to illustrate the fixed format, see PDGTABLE.MeV for more details
        # M 1000022                          0.E+00         +0.0E+00 -0.0E+00 ~chi(0,1)     0

        lines = open('PDGTABLE.MeV.org').readlines()
        for pdgid,mass,name,charge in replace:
            if not re.search(r'[MW]\s+'+str(pdgid)+r'\s+\S+', ''.join(lines)):
                lines.append('M' + str(pdgid).rjust(8) +''.ljust(26) +
                             ('%11.5E' % mass).ljust(15) +
                             '+0.0E+00'.ljust(9) + '-0.0E+00'.ljust(9) +
                             name.strip() + ''.ljust(6) + charge.strip()+''.rjust(20-len(name.strip())) + '\n')
                lines.append('W' + str(pdgid).rjust(8) +''.ljust(26) +
                             '0.E+00'.ljust(15) + '+0.0E+00'.ljust(9) + '-0.0E+00'.ljust(9) +
                             name.strip() + ''.ljust(6) + charge.strip()+''.rjust(20-len(name.strip())) + '\n')
            else:
                from past.builtins import xrange # Temporary workaround for python3 compatibility use range in CA-based config
                for i in xrange(len(lines)):
                    if re.search(r'M\s+'+str(pdgid)+r'\s+\S+', lines[i]):
                        l = lines[i]
                        lines[i] = l[0:35] + ('%11.5E' % mass).ljust(14) + l[49:]

        update = open('PDGTABLE.MeV', 'w')
        update.write(''.join(lines))
        update.close()

        print ('modified PDGTABLE\n%s\n' % ''.join(lines))
        sys.stdout.flush()


@AccumulatorCache
def load_files_for_GMSB_scenario(simdict):

    GMSBIndex = int(simdict["GMSBIndex"])
    pdgcodes = []
    if GMSBIndex == 1:
        get_and_fix_PDGTABLE_GMSB([
                              (1000022, eval(simdict["GMSBNeutralino"]), '~chi(0,1)', '0'),
                              (1000039, eval(simdict.get("GMSBGravitino",'0')), '~G', '0')
                            ])
        pdgcodes += [1000022,1000039]
    elif GMSBIndex == 2:
        m_stau    = eval(simdict["GMSBStau"])
        m_slepton = eval(simdict["GMSBSlepton"])
        get_and_fix_PDGTABLE_GMSB([
                              (1000015, m_stau, '~tau(L)', '-'),
                              (2000011, m_slepton, '~e(R)', '-'),
                              (2000013, m_slepton, '~mu(R)', '-')
                            ])
        pdgcodes += [-1000015,1000015,-2000011,2000011,-2000013,2000013]
    elif GMSBIndex == 3:
        m_stau = eval(simdict["GMSBStau"])
        m_slepton = eval(simdict["GMSBSlepton"])
        m_squark = eval(simdict["SQUARKMASS"])
        m_neutralino = eval(simdict["NEUTRALINOMASS"])
        m_gluino = eval(simdict["GLUINOMASS"])
        get_and_fix_PDGTABLE_GMSB([
                              (1000001, m_squark, '~d(L)', '-1/3'), (2000001, m_squark, '~d(R)', '-1/3'),
                              (1000002, m_squark, '~u(L)', '+2/3'), (2000002, m_squark, '~u(R)', '+2/3'),
                              (1000003, 1.00E+04, '~s(L)', '-1/3'), (2000003, 1.00E+04, '~s(R)', '-1/3'),
                              (1000004, 1.00E+04, '~c(L)', '+2/3'), (2000004, 1.00E+04, '~c(R)', '+2/3'),
                              (1000005, 1.00E+04, '~b(1)', '-1/3'), (2000005, 1.00E+04, '~b(2)', '-1/3'),
                              (1000006, 1.00E+04, '~t(1)', '+2/3'), (2000006, 1.00E+04, '~t(2)', '+2/3'),
                              (1000011, 2.50E+02, '~e(L)', '-'), (2000011, m_slepton, '~e(R)', '-'),
                              (1000012, 1.00E+04, '~nu(e,L)', '0'),
                              (1000013, 2.50E+02, '~mu(L)', '-'), (2000013, m_slepton, '~mu(R)', '-'),
                              (1000014, 1.00E+04, '~nu(e,L)', '0'),
                              (1000015, m_stau, '~tau(L)', '-'), (2000015, 2.50E+02, '~tau(R)', '-'),
                              (1000016, 1.00E+04, '~nu(tau,L)', '0'),
                              (1000021, m_gluino, '~g', '0'),
                              (1000022, m_neutralino, '~chi(0,1)', '0'),
                              (1000023, 1.00E+04, '~chi(0,2)', '0'),
                              (1000024, 1.00E+04, '~chi(+,1)', '+'),
                              (1000025, -1.00E+04, '~chi(0,3)', '0'),
                              (1000035, 1.00E+04, '~chi(0,4)', '0'),
                              (1000037, 1.00E+04, '~chi(+,2)', '+')
                            ])
        pdgcodes += [
            -1000001,1000001,-2000001,2000001,
            -1000002,1000002,-2000002,2000002,
            -1000003,1000003,-2000003,2000003,
            -1000004,1000004,-2000004,2000004,
            -1000005,1000005,-2000005,2000005,
            -1000006,1000006,-2000006,2000006,
            -1000011,1000011,-2000011,2000011,
            -1000013,1000013,-2000013,2000013,
            -1000015,1000015,-2000015,2000015,
             1000012,1000014,1000016,1000021,1000022,1000023,
            -1000024,1000024,1000025,1000035,-1000037,1000037]

    elif GMSBIndex == 4:
        get_and_fix_PDGTABLE_GMSB([
                              (1000015, m_stau, '~tau(L)', '-')
                            ])
        pdgcodes += [-1000015,1000015]

    else:
        print ('GMSBIndex %i not supported' % GMSBIndex)
        raise
    from ExtraParticles.PDGHelpers import updateExtraParticleAcceptList
    updateExtraParticleAcceptList('G4particle_acceptlist_ExtraParticles.txt', pdgcodes)


@AccumulatorCache
def get_and_fix_PDGTABLE_sleptons(flags, replace):

    # Download generic PDGTABLE (do not overwrite existing one if it exists, use existing one instead)
    from ExtraParticles.PDGHelpers import getPDGTABLE
    if getPDGTABLE('PDGTABLE.MeV'):
        shutil.move('PDGTABLE.MeV', 'PDGTABLE.MeV.org')

    # an example line to illustrate the fixed format, see PDGTABLE.MeV for more details
        # M 1000022                          0.E+00         +0.0E+00 -0.0E+00 ~chi(0,1)     0

        lines = open('PDGTABLE.MeV.org').readlines()
        for pdgid,mass,name,charge in replace:
            if not re.search(r'[MW]\s+'+str(pdgid)+r'\s+\S+', ''.join(lines)):
                lines.append('M' + str(pdgid).rjust(8) +''.ljust(26) +
                             ('%11.5E' % mass).ljust(15) +
                             '+0.0E+00'.ljust(9) + '-0.0E+00'.ljust(9) +
                             name.strip() + ''.ljust(6) + charge.strip()+''.rjust(20-len(name.strip())) + '\n')
                lines.append('W' + str(pdgid).rjust(8) +''.ljust(26) +
                             '0.E+00'.ljust(15) + '+0.0E+00'.ljust(9) + '-0.0E+00'.ljust(9) +
                             name.strip() + ''.ljust(6) + charge.strip()+''.rjust(20-len(name.strip())) + '\n')
            else:
                from past.builtins import xrange # Temporary workaround for python3 compatibility use range in CA-based config
                for i in xrange(len(lines)):
                    if re.search(r'M\s+'+str(pdgid)+r'\s+\S+', lines[i]):
                        l = lines[i]
                        lines[i] = l[0:35] + ('%11.5E' % mass).ljust(14) + l[49:]

        update = open('PDGTABLE.MeV', 'w')
        update.write(''.join(lines))
        update.close()

        print('modified PDGTABLE\n%s\n' % ''.join(lines))
        sys.stdout.flush()


def load_files_for_sleptonLLP_scenario(flags):
    simdict = flags.Input.SpecialConfiguration
    pdgcodes = []
    if "GMSBSlepton" in simdict:
        get_and_fix_PDGTABLE_sleptons(flags, [
                (2000011, eval(simdict.get("GMSBSlepton",'0')), '~e(R)', '-'),
                (2000013, eval(simdict.get("GMSBSlepton",'0')), '~mu(R)', '-'),
                (1000011, eval(simdict.get("GMSBSlepton",'0')), '~e(L)', '-'),
                (1000013, eval(simdict.get("GMSBSlepton",'0')), '~mu(L)', '-'),
                ])
        pdgcodes += [-2000011,2000011,-2000013,2000013,-1000011,1000011,-1000013,1000013]
    if "GMSBStau" in simdict:
        get_and_fix_PDGTABLE_sleptons(flags, [
                (2000015, eval(simdict.get("GMSBStau",'0')), '~tau(R)', '-'),
                (1000015, eval(simdict.get("GMSBStau",'0')), '~tau(L)', '-'),
                ])
        pdgcodes += [-2000015,2000015,-1000015,1000015]
    if "GMSBGravitino" in simdict:
        get_and_fix_PDGTABLE_sleptons(flags, [
                (1000039, eval(simdict.get("GMSBGravitino",'0')), '~G', '0'),
                ])
        pdgcodes += [1000039]
    if "coannihilationStau" in simdict:
        get_and_fix_PDGTABLE_sleptons([
                (2000015, eval(simdict.get("coannihilationStau",'0')), '~tau(R)', '-'),
                (1000015, eval(simdict.get("coannihilationStau",'0')), '~tau(L)', '-'),
                ])
        pdgcodes += [-2000015,2000015,-1000015,1000015]
    if "coannihilationSlepton" in simdict:
        get_and_fix_PDGTABLE_sleptons([
            (2000011, eval(simdict.get("coannihilationSlepton", '0')), '~e(R)', '-'),
            (2000013, eval(simdict.get("coannihilationSlepton", '0')), '~mu(R)', '-'),
            (1000011, eval(simdict.get("coannihilationSlepton", '0')), '~e(L)', '-'),
            (1000013, eval(simdict.get("coannihilationSlepton", '0')), '~mu(L)', '-'),
        ])
        pdgcodes += [-2000011, 2000011, -2000013, 2000013, -1000011, 1000011, -1000013, 1000013]
    if "coannihilationNeutralino" in simdict:
        get_and_fix_PDGTABLE_sleptons([
                (1000022, eval(simdict.get("coannihilationNeutralino", '0')), '~chi(0,1)', '0'),
                ])
        pdgcodes += [1000022]

    from ExtraParticles.PDGHelpers import updateExtraParticleAcceptList
    updateExtraParticleAcceptList('G4particle_acceptlist_ExtraParticles.txt', pdgcodes)


def SleptonsPhysicsToolCfg(flags, name="SleptonsPhysicsTool", **kwargs):
    result = ComponentAccumulator()
    if "GMSBStau" in flags.Input.SpecialConfiguration or "coannihilationStau" in flags.Input.SpecialConfiguration:
        StauMass = None
        if "GMSBStau" in flags.Input.SpecialConfiguration:
            StauMass = eval(flags.Input.SpecialConfiguration.get("GMSBStau", "None"))
            kwargs.setdefault("G4STau1MinusMass",             StauMass)
            kwargs.setdefault("G4STau1PlusMass",              StauMass)
            # TODO Check whether G4STau2(Plus/Minus)Mass should also be set here
        elif "coannihilationStau" in flags.Input.SpecialConfiguration:
            StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
            kwargs.setdefault("G4STau1MinusMass",             StauMass)
            kwargs.setdefault("G4STau1PlusMass",              StauMass)
            kwargs.setdefault("G4STau2MinusMass",             StauMass)
            kwargs.setdefault("G4STau2PlusMass",              StauMass)

    if "GMSBSlepton" in flags.Input.SpecialConfiguration:
        GMSBSlepton = eval(flags.Input.SpecialConfiguration.get("GMSBSlepton", "None"))

        kwargs.setdefault("G4SElectronRMinusMass",        GMSBSlepton)
        kwargs.setdefault("G4SElectronRPlusMass",         GMSBSlepton)
        kwargs.setdefault("G4SMuonRMinusMass",            GMSBSlepton)
        kwargs.setdefault("G4SMuonRPlusMass",             GMSBSlepton)

    result.setPrivateTools( CompFactory.SleptonsPhysicsTool(name, **kwargs) )
    return result


def AllSleptonsPhysicsToolCfg(flags, name="AllSleptonsPhysicsTool", **kwargs):
    result = ComponentAccumulator()
    if "GMSBStau" in flags.Input.SpecialConfiguration or "coannihilationStau" in flags.Input.SpecialConfiguration:
        StauMass = None
        StauLifetime = None
        if "GMSBStau" in flags.Input.SpecialConfiguration: # Check for GMSBStau key word in job options. If found set Stau values.
            StauMass = eval(flags.Input.SpecialConfiguration.get("GMSBStau", "None"))
            StauLifetime = eval(flags.Input.SpecialConfiguration.get("GMSBStauTime", "None"))
        elif "coannihilationStau" in flags.Input.SpecialConfiguration: # Check for coannihilationStau key word in evgen special configs. This is an option that is normally put in the event gen job options file.
            StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
            StauLifetime = eval(flags.Input.SpecialConfiguration.get("coannihilationStauTime", "None"))

        kwargs.setdefault("G4STau1MinusMass",             StauMass)
        kwargs.setdefault("G4STau1MinusPDGCode",          1000015)
        kwargs.setdefault("G4STau1MinusStable",           False)
        kwargs.setdefault("G4STau1MinusLifetime",         StauLifetime)

        kwargs.setdefault("G4STau1PlusMass",              StauMass)
        kwargs.setdefault("G4STau1PlusPDGCode",           -1000015)
        kwargs.setdefault("G4STau1PlusStable",            False)
        kwargs.setdefault("G4STau1PlusLifetime",          StauLifetime)

        kwargs.setdefault("G4STau2MinusMass",             StauMass)
        kwargs.setdefault("G4STau2MinusPDGCode",          2000015)
        kwargs.setdefault("G4STau2MinusStable",           False)
        kwargs.setdefault("G4STau2MinusLifetime",         StauLifetime)

        kwargs.setdefault("G4STau2PlusMass",              StauMass)
        kwargs.setdefault("G4STau2PlusPDGCode",           -2000015)
        kwargs.setdefault("G4STau2PlusStable",            False)
        kwargs.setdefault("G4STau2PlusLifetime",          StauLifetime)

    if "GMSBSlepton" in flags.Input.SpecialConfiguration:
        GMSBSlepton = eval(flags.Input.SpecialConfiguration.get("GMSBSlepton", "None"))
        GMSBSleptonTime = eval(flags.Input.SpecialConfiguration.get("GMSBSleptonTime", "None"))

        kwargs.setdefault("G4SElectronLMinusMass",        GMSBSlepton)
        kwargs.setdefault("G4SElectronLMinusPDGCode",     1000011)
        kwargs.setdefault("G4SElectronLMinusStable",      False)
        kwargs.setdefault("G4SElectronLMinusLifetime",    GMSBSleptonTime)

        kwargs.setdefault("G4SElectronLPlusMass",         GMSBSlepton)
        kwargs.setdefault("G4SElectronLPlusPDGCode",      -1000011)
        kwargs.setdefault("G4SElectronLPlusStable",       False)
        kwargs.setdefault("G4SElectronLPlusLifetime",     GMSBSleptonTime)

        kwargs.setdefault("G4SMuonLMinusMass",            GMSBSlepton)
        kwargs.setdefault("G4SMuonLMinusPDGCode",         1000013)
        kwargs.setdefault("G4SMuonLMinusStable",          False)
        kwargs.setdefault("G4SMuonLMinusLifetime",        GMSBSleptonTime)

        kwargs.setdefault("G4SMuonLPlusMass",             GMSBSlepton)
        kwargs.setdefault("G4SMuonLPlusPDGCode",          -1000013)
        kwargs.setdefault("G4SMuonLPlusStable",           False)
        kwargs.setdefault("G4SMuonLPlusLifetime",         GMSBSleptonTime)

        kwargs.setdefault("G4SElectronRMinusMass",        GMSBSlepton)
        kwargs.setdefault("G4SElectronRMinusPDGCode",     2000011)
        kwargs.setdefault("G4SElectronRMinusStable",      False)
        kwargs.setdefault("G4SElectronRMinusLifetime",    GMSBSleptonTime)

        kwargs.setdefault("G4SElectronRPlusMass",         GMSBSlepton)
        kwargs.setdefault("G4SElectronRPlusPDGCode",      -2000011)
        kwargs.setdefault("G4SElectronRPlusStable",       False)
        kwargs.setdefault("G4SElectronRPlusLifetime",     GMSBSleptonTime)

        kwargs.setdefault("G4SMuonRMinusMass",            GMSBSlepton)
        kwargs.setdefault("G4SMuonRMinusPDGCode",         2000013)
        kwargs.setdefault("G4SMuonRMinusStable",          False)
        kwargs.setdefault("G4SMuonRMinusLifetime",        GMSBSleptonTime)

        kwargs.setdefault("G4SMuonRPlusMass",             GMSBSlepton)
        kwargs.setdefault("G4SMuonRPlusPDGCode",          -2000013)
        kwargs.setdefault("G4SMuonRPlusStable",           False)
        kwargs.setdefault("G4SMuonRPlusLifetime",         GMSBSleptonTime)
    result.setPrivateTools( CompFactory.SleptonsPhysicsTool(name, **kwargs) )
    return result


## Gravitino Options
def SElectronRPlusToElectronGravitinoCfg(flags, name="SElectronRPlusToElectronGravitino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_e_plus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,e+")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SElectronRMinusToElectronGravitinoCfg(flags, name="SElectronRMinusToElectronGravitino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_e_minus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,e-")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SMuonRPlusToMuonGravitinoCfg(flags, name="SMuonRPlusToMuonGravitino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_mu_plus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,mu+")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SMuonRMinusToMuonGravitinoCfg(flags, name="SMuonRMinusToMuonGravitino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_mu_minus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,mu-")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLPlusToTauGravitinoCfg(flags, name="STauLPlusToTauGravitino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,tau+")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLMinusToTauGravitinoCfg(flags, name="STauLMinusToTauGravitino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,tau-")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SElectronLPlusToElectronGravitinoCfg(flags, name="SElectronLPlusToElectronGravitino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_e_plus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,e+")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SElectronLMinusToElectronGravitinoCfg(flags, name="SElectronLMinusToElectronGravitino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_e_minus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,e-")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SMuonLPlusToMuonGravitinoCfg(flags, name="SMuonLPlusToMuonGravitino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_mu_plus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,mu+")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SMuonLMinusToMuonGravitinoCfg(flags, name="SMuonLMinusToMuonGravitino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_mu_minus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,mu-")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauRPlusToTauGravitinoCfg(flags, name="STauRPlusToTauGravitino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,tau+")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauRMinusToTauGravitinoCfg(flags, name="STauRMinusToTauGravitino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_G,tau-")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


## Neutralino-Stau
def STauRMinusToTauNeutralinoCfg(flags, name="STauRMinusToTauNeutralino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,tau-")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauRPlusToTauNeutralinoCfg(flags, name="STauRPlusToTauNeutralino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,tau+")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLMinusToTauNeutralinoCfg(flags, name="STauLMinusToTauNeutralino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,tau-")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLPlusToTauNeutralinoCfg(flags, name="STauLPlusToTauNeutralino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,tau+")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


#########################################################################################
### Neutralino-Stau Off shell tau
## Stau-Neutralino Pion Neutrino
#########################################################################################
def STauRMinusToPionMinusNeutralinoCfg(flags, name="STauRMinusToPionMinusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = .9
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", BR)  # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,pi-,nu_tau")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauRPlusToPionPlusNeutralinoCfg(flags, name="STauRPlusToPionPlusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = .9
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,pi+,anti_nu_tau")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLMinusToPionMinusNeutralinoCfg(flags, name="STauLMinusToPionMinusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = .9
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,pi-,nu_tau")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLPlusToPionPlusNeutralinoCfg(flags, name="STauLPlusToPionPlusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = .9
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,pi+,anti_nu_tau")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


#########################################################################################
### Neutralino-Stau Off shell tau
### Stau-Neutralino Muon Neutrino
#########################################################################################
def STauRMinusToRhoMinusNeutralinoCfg(flags, name="STauRMinusToRhoMinusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = 0.0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .33
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,rho-,nu_tau")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauRPlusToRhoPlusNeutralinoCfg(flags, name="STauRPlusToRhoPlusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = 0.0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .33
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,rho+,anti_nu_tau")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLMinusToRhoMinusNeutralinoCfg(flags, name="STauLMinusToRhoMinusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = 0.0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .33
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,rho-,nu_tau")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLPlusToRhoPlusNeutralinoCfg(flags, name="STauLPlusToRhoPlusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = 0.0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .33
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,rho+,anti_nu_tau")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


#########################################################################################
### Neutralino-Stau Off shell tau
### Stau-Neutralino Electron Neutrino
#########################################################################################
def STauRMinusToEMinusNeutralinoCfg(flags, name="STauRMinusToEMinusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = .07
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .19
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e-,nu_tau,anti_nu_e")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauRPlusToEPlusNeutralinoCfg(flags, name="STauRPlusToEPlusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = .07
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .19
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e+,anti_nu_tau,nu_e")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLMinusToEMinusNeutralinoCfg(flags, name="STauLMinusToEMinusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = .07
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .19
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e-,nu_tau,anti_nu_e")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLPlusToEPlusNeutralinoCfg(flags, name="STauLPlusToEPlusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = .07
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .03
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e+,anti_nu_tau,nu_e")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


#########################################################################################
### Neutralino-Stau Off shell tau
### Stau-Neutralino Muon Neutrino
#########################################################################################
def STauRMinusToMuMinusNeutralinoCfg(flags, name="STauRMinusToMuMinusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = .03
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .18
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu-,nu_tau,anti_nu_mu")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauRPlusToMuPlusNeutralinoCfg(flags, name="STauRPlusToMuPlusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = .03
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .18
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu+,anti_nu_tau,nu_mu")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLMinusToMuMinusNeutralinoCfg(flags, name="STauLMinusToMuMinusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = .03
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .18
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu-,nu_tau,anti_nu_mu")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLPlusToMuPlusNeutralinoCfg(flags, name="STauLPlusToMuPlusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = .03
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .18
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu+,anti_nu_tau,nu_mu")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


#########################################################################################
### Neutralino-Stau Off shell tau
### Stau-Neutralino Pseuddo-Vector a1(1260) meson Neutrino
#########################################################################################
def STauRMinusToa1MinusNeutralinoCfg(flags, name="STauRMinusToa1MinusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = 0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15 ## Set the branching Ratio for if there is enough energy for.c
    kwargs.setdefault("ParticleName","s_tau_minus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,a1(1260)-,nu_tau")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauRPlusToa1PlusNeutralinoCfg(flags, name="STauRPlusToa1PlusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = 0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_plus_2")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,a1(1260)+,anti_nu_tau")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLMinusToa1MinusNeutralinoCfg(flags, name="STauLMinusToa1MinusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = 0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_minus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,a1(1260)-,nu_tau")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def STauLPlusToa1PlusNeutralinoCfg(flags, name="STauLPlusToa1PlusNeutralino", **kwargs):
    result = ComponentAccumulator()
    BR = 0
    # coannihilation Neutralino and Stau masses
    NeutralinoMass = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino", "None"))
    StauMass = eval(flags.Input.SpecialConfiguration.get("coannihilationStau", "None"))
    if NeutralinoMass is not None and StauMass is not None and StauMass - Mass_a1Meson > NeutralinoMass: ## adjust branching ratio to ensure enough energy to decay to the a1 meson.
        BR = .15
    kwargs.setdefault("ParticleName","s_tau_plus_1")
    kwargs.setdefault("BR", BR) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,a1(1260)+,anti_nu_tau")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


## Neutralino Selectron
def SElectronRPlusToElectronNeutralinoCfg(flags, name="SElectronRPlusToElectronNeutralino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_e_plus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e+")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SElectronRMinusToElectronNeutralinoCfg(flags, name="SElectronRMinusToElectronNeutralino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_e_minus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e-")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SElectronLPlusToElectronNeutralinoCfg(flags, name="SElectronLPlusToElectronNeutralino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_e_plus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e+")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SElectronLMinusToElectronNeutralinoCfg(flags, name="SElectronLMinusToElectronNeutralino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_e_minus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,e-")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


## Neutralino SMuon
def SMuonLPlusToMuonNeutralinoCfg(flags, name="SMuonLPlusToMuonNeutralino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_mu_plus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu+")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SMuonLMinusToMuonNeutralinoCfg(flags, name="SMuonLMinusToMuonNeutralino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_mu_minus_L")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu-")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SMuonRPlusToMuonNeutralinoCfg(flags, name="SMuonRPlusToMuonNeutralino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_mu_plus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu+")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SMuonRMinusToMuonNeutralinoCfg(flags, name="SMuonRMinusToMuonNeutralino", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleName","s_mu_minus_R")
    kwargs.setdefault("BR", 1.0) # Branching Ratio
    kwargs.setdefault("Daughters","s_chi_0_1,mu-")
    result.setPrivateTools( CompFactory.AddPhysicsDecayTool(name, **kwargs) )
    return result


def SleptonsLLPCfg(flags):
    result = ComponentAccumulator()
    load_files_for_sleptonLLP_scenario(flags)

    if flags.Common.ProductionStep == ProductionStep.Simulation:
        from G4AtlasServices.G4AtlasServicesConfig import PhysicsListSvcCfg
        from Gauginos.GauginosConfig import GauginosPhysicsToolCfg
        result.merge(PhysicsListSvcCfg(flags))
        physicsOptions = [ result.popToolsAndMerge(GauginosPhysicsToolCfg(flags)) ]
        physicsOptions += [ result.popToolsAndMerge(AllSleptonsPhysicsToolCfg(flags)) ]
        # Slepton decays from SleptonsConfig
        if "GMSBSlepton" in flags.Input.SpecialConfiguration:
            physicsOptions += [ result.popToolsAndMerge(SElectronRPlusToElectronGravitinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SElectronLPlusToElectronGravitinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SElectronRMinusToElectronGravitinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SElectronLMinusToElectronGravitinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SMuonRPlusToMuonGravitinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SMuonLPlusToMuonGravitinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SMuonRMinusToMuonGravitinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SMuonLMinusToMuonGravitinoCfg(flags)) ]
        if "GMSBStau" in flags.Input.SpecialConfiguration:
            physicsOptions += [ result.popToolsAndMerge(STauRPlusToTauGravitinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(STauLPlusToTauGravitinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(STauRMinusToTauGravitinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(STauLMinusToTauGravitinoCfg(flags)) ]
        if "coannihilationStau" in flags.Input.SpecialConfiguration:
            MassStau = eval(flags.Input.SpecialConfiguration.get("coannihilationStau",'0'))
            MassNeutralino = eval(flags.Input.SpecialConfiguration.get("coannihilationNeutralino",'0'))
            MassTau = 1776. # MeV
            if MassStau > MassNeutralino + MassTau: ## Check that there is energy to decay to tau.
                physicsOptions += [ result.popToolsAndMerge(STauRPlusToTauNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauLPlusToTauNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauRMinusToTauNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauLMinusToTauNeutralinoCfg(flags)) ]
            else: ## Do off shell decay for tau.
                # FIXME Dislike hard-coded numbers here
                if (abs(MassStau-MassNeutralino - 300) > .05  and abs(MassStau-MassNeutralino - 1700) > 0.05):
                    print('Warning: Branching ratios are wrong. Mass splitting of stau neutralino %s currently has no available values.' % (MassStau - MassNeutralino))
                physicsOptions += [ result.popToolsAndMerge(STauRMinusToPionMinusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauRPlusToPionPlusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauLMinusToPionMinusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauLPlusToPionPlusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauRMinusToRhoMinusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauRPlusToRhoPlusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauLMinusToRhoMinusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauLPlusToRhoPlusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauRMinusToEMinusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauRPlusToEPlusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauLMinusToEMinusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauLPlusToEPlusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauRMinusToMuMinusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauRPlusToMuPlusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauLMinusToMuMinusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauLPlusToMuPlusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauRMinusToa1MinusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauRPlusToa1PlusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauLMinusToa1MinusNeutralinoCfg(flags)) ]
                physicsOptions += [ result.popToolsAndMerge(STauLPlusToa1PlusNeutralinoCfg(flags)) ]
        if "coannihilationSlepton" in flags.Input.SpecialConfiguration:
            physicsOptions += [ result.popToolsAndMerge(SElectronRPlusToElectronNeutralinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SElectronLPlusToElectronNeutralinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SElectronRMinusToElectronNeutralinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SElectronLMinusToElectronNeutralinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SMuonRPlusToMuonNeutralinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SMuonLPlusToMuonNeutralinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SMuonRMinusToMuonNeutralinoCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(SMuonLMinusToMuonNeutralinoCfg(flags)) ]

        result.getService("PhysicsListSvc").PhysOption += physicsOptions
    return result


def GMSB_Cfg(flags):
    result = ComponentAccumulator()
    if flags.Common.ProductionStep == ProductionStep.Simulation:
        from G4AtlasServices.G4AtlasServicesConfig import PhysicsListSvcCfg
        result.merge(PhysicsListSvcCfg(flags))

    simdict = flags.Input.SpecialConfiguration
    assert "GMSBIndex" in simdict
    load_files_for_GMSB_scenario(simdict)

    if flags.Common.ProductionStep == ProductionStep.Simulation:
        from Gauginos.GauginosConfig import GauginosPhysicsToolCfg, NeutralinoToPhotonGravitinoCfg
        GMSBIndex = int(simdict["GMSBIndex"])
        physicsOptions = []
        if GMSBIndex == 1: # generic neutralino to photon scenario
            physicsOptions = [ result.popToolsAndMerge(GauginosPhysicsToolCfg(flags)) ]
            physicsOptions += [ result.popToolsAndMerge(NeutralinoToPhotonGravitinoCfg(flags)) ]
        elif GMSBIndex == 2 or GMSBIndex == 3 or GMSBIndex == 4: # generic stau scenario
            physicsOptions = [ result.popToolsAndMerge(SleptonsPhysicsToolCfg(flags)) ]
        else:
            print ('GMSBIndex %i not supported' % GMSBIndex)
            raise
        del GMSBIndex
        result.getService("PhysicsListSvc").PhysOption += physicsOptions
    return result


def GMSB_VerboseSelectorCfg(flags, name="G4UA::VerboseSelectorTool", **kwargs):
    kwargs.setdefault('TargetEvent',1)
    kwargs.setdefault('VerboseLevel',1)
    kwargs.setdefault('TargetPdgIDs',
                                    [
                                        -1000001,1000001, # ~d(L)
                                        -1000002,1000002, # ~u(L)
                                        -1000003,1000003, # ~s(L)
                                        -1000004,1000004, # ~c(L)
                                        -1000005,1000005, # ~b(1)
                                        -1000006,1000006, # ~t(1)
                                        -1000011,1000011, # ~e(L)
                                        -1000013,1000013, # ~mu(L)'
                                        -1000015,1000015, # ~tau(L)
                                        -2000001,2000001, # ~d(R)
                                        -2000002,2000002, # ~u(R)
                                        -2000003,2000003, # ~s(R)
                                        -2000004,2000004, # ~c(R)
                                        -2000005,2000005, # ~b(2)
                                        -2000006,2000006, # ~t(2)
                                        -2000011,2000011, # ~e(R)
                                        -2000013,2000013, # ~mu(R)'
                                        -2000015,2000015, # ~tau(R)
                                        1000021, # ~g
                                        1000022, # ~chi(0,1)
                                        1000023, # ~chi(0,2)
                                        -1000024,1000024, # ~chi(+,1)
                                        1000025, # ~chi(0,3)
                                        1000035, # ~chi(0,4)
                                        -1000037,1000037, # ~chi(+,2)
                                        1000039 # ~G
                                    ])
    from G4DebuggingTools.G4DebuggingToolsConfig import VerboseSelectorToolCfg
    return VerboseSelectorToolCfg(flags, name, **kwargs)


def SleptonsLLP_VerboseSelectorCfg(flags, name="G4UA::VerboseSelectorTool", **kwargs):
    kwargs.setdefault('TargetEvent',1)
    kwargs.setdefault('VerboseLevel',1)
    kwargs.setdefault('TargetPdgIDs',
                                    [
                                        -1000001,1000001, # ~d(L)
                                        -1000002,1000002, # ~u(L)
                                        -1000003,1000003, # ~s(L)
                                        -1000004,1000004, # ~c(L)
                                        -1000005,1000005, # ~b(1)
                                        -1000006,1000006, # ~t(1)
                                        -1000011,1000011, # ~e(L)
                                        -1000013,1000013, # ~mu(L)'
                                        -1000015,1000015, # ~tau(L)
                                        -2000001,2000001, # ~d(R)
                                        -2000002,2000002, # ~u(R)
                                        -2000003,2000003, # ~s(R)
                                        -2000004,2000004, # ~c(R)
                                        -2000005,2000005, # ~b(2)
                                        -2000006,2000006, # ~t(2)
                                        -2000011,2000011, # ~e(R)
                                        -2000013,2000013, # ~mu(R)'
                                        -2000015,2000015, # ~tau(R)
                                        1000021, # ~g
                                        1000022, # ~chi(0,1)
                                        1000023, # ~chi(0,2)
                                        -1000024,1000024, # ~chi(+,1)
                                        1000025, # ~chi(0,3)
                                        1000035, # ~chi(0,4)
                                        -1000037,1000037, # ~chi(+,2)
                                        1000039 # ~G
                                    ])
    from G4DebuggingTools.G4DebuggingToolsConfig import VerboseSelectorToolCfg
    return VerboseSelectorToolCfg(flags, name, **kwargs)
