# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


def DebugAMSB(flags):
    flags.Sim.OptionalUserActionList += ['Charginos.CharginosConfig.AMSB_VerboseSelectorCfg']


def DebugGMSB(flags):
    flags.Sim.OptionalUserActionList += ['Sleptons.SleptonsConfig.GMSB_VerboseSelectorCfg']


def DebugMonopole(flags):
    flags.Sim.OptionalUserActionList += ['Monopole.MonopoleConfig.Monopole_VerboseSelectorCfg']


def DebugSleptonsLLP(flags):
    flags.Sim.OptionalUserActionList += ['Sleptons.SleptonsConfig.SleptonsLLP_VerboseSelectorCfg']


def DebugRHadrons(flags):
    flags.Sim.OptionalUserActionList += ['RHadrons.RHadronsConfig.RHadrons_VerboseSelectorCfg']