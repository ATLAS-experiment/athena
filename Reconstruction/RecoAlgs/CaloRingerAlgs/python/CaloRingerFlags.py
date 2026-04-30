# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def createCaloRingerConfigFlags():
    caloRingercf = AthConfigFlags()

    caloRingercf.addFlag("CaloRinger.buildJetRings", False)
    caloRingercf.addFlag("CaloRinger.buildJetAsymRings", False)
    caloRingercf.addFlag("CaloRinger.buildJetStripsRings", False)
    caloRingercf.addFlag("CaloRinger.buildJetCornerRings", False)

    caloRingercf.addFlag("CaloRinger.buildPhotonRings", False)
    caloRingercf.addFlag("CaloRinger.buildPhotonAsymRings", False)
    caloRingercf.addFlag("CaloRinger.buildPhotonStripsRings", False)
    caloRingercf.addFlag("CaloRinger.buildPhotonCornerRings", False)

    caloRingercf.addFlag("CaloRinger.buildElectronRings", True)
    caloRingercf.addFlag("CaloRinger.buildElectronAsymRings", False)
    caloRingercf.addFlag("CaloRinger.buildElectronStripsRings", False)
    caloRingercf.addFlag("CaloRinger.buildElectronCornerRings", False)

    caloRingercf.addFlag("CaloRinger.minElectronEnergy", 14)
    caloRingercf.addFlag("CaloRinger.minPhotonEnergy", 14)
    caloRingercf.addFlag("CaloRinger.minJetEnergy", 14)
    caloRingercf.addFlag("CaloRinger.useShowerShapeBarycenter", False)
    caloRingercf.addFlag("CaloRinger.doTransverseEnergy", True)
    caloRingercf.addFlag("CaloRinger.cornerShift", 3)

    return caloRingercf


if __name__ == "__main__":

    flags = createCaloRingerConfigFlags()
    flags.dump()
