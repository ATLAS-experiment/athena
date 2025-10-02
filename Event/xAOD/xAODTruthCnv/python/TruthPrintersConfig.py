# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Core import(s).
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

# I/O import(s).
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

# Local import(s).
from xAODTruthCnv.TruthFixersConfig import TruthParticleFixerAlgCfg, \
    TruthVertexFixerAlgCfg

# System import(s).
import os
import sys


def TruthParticlePrinterAlgCfg(flags, name="TruthParticlePrinter", **kwargs):
    '''Configure the TruthParticlePrinterAlg algorithm.
    '''
    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()
    # Create the algorithm.
    alg = CompFactory.xAODReader.TruthParticlePrinterAlg(name, **kwargs)
    result.addEventAlgo(alg)
    # Return the result to the caller.
    return result


def TruthVertexPrinterAlgCfg(flags, name="TruthVertexPrinter", **kwargs):
    '''Configure the TruthVertexPrinterAlg algorithm.
    '''
    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()
    # Create the algorithm.
    alg = CompFactory.xAODReader.TruthVertexPrinterAlg(name, **kwargs)
    result.addEventAlgo(alg)
    # Return the result to the caller.
    return result


if __name__ == '__main__':

    # Set up the job's flags.
    flags = initConfigFlags()
    flags.Exec.MaxEvents = 10
    flags.Input.Files = [
        "%s/ASG/DAOD_PHYS/p6697/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.deriv.DAOD_PHYS.e8514_e8528_s4162_s4114_r15540_r15516_p6697/DAOD_PHYS.43700597._000577.pool.root.1" %
        os.environ.get('ATLAS_REFERENCE_DATA',
                       '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art')]
    flags.fillFromArgs()
    flags.lock()

    # Set up the main services.
    acc = MainServicesCfg(flags)

    # Set up the input file reading.
    acc.merge(PoolReadCfg(flags))

    # Fix the reading of the truth collections that we are going to print.
    acc.merge(TruthParticleFixerAlgCfg(flags, name='TruthElectronsFixer',
                                       container='TruthElectrons',
                                       ParticleLinks=['parentLinks', 'childLinks']))
    acc.merge(TruthParticleFixerAlgCfg(flags, name='TruthMuonsFixer',
                                       container='TruthMuons',
                                       ParticleLinks=['parentLinks', 'childLinks']))
    acc.merge(TruthVertexFixerAlgCfg(flags, name='PrimaryVertexFixer',
                                     container='TruthPrimaryVertices'))

    # Set up the truth printing algorithm(s).
    acc.merge(TruthParticlePrinterAlgCfg(
        flags, name='TruthElectronsPrinter', Container='TruthElectrons'))
    acc.merge(TruthParticlePrinterAlgCfg(
        flags, name='TruthMuonsPrinter', Container='TruthMuons'))
    acc.merge(TruthVertexPrinterAlgCfg(
        flags, name='PrimaryVertexPrinter', Container='TruthPrimaryVertices'))

    # Run the configuration.
    sys.exit(acc.run().isFailure())
