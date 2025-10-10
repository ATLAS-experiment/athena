"""
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""

'''@file TileSimConfigFlags.py
@brief Functions to create/add extra Tile simulation configuration flags
'''


def createTileSimConfigFlags():

    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    tileSimFlags = AthConfigFlags()

    tileSimFlags.addFlag('DeltaTHit', 'NONE', help='A time granularity for G4 hits in TileHit, \
    it can be just one number, e.g. 5.0 which means that the same, granularity is used everywhere or it can be a vector which contains 3*N+1 elements, \
    e.g. 0.1,-5,5, 0.5,-75,75, 5.0 which means that for [-5,5] ns interval granularity 0.1ns will be used  for [-75,75] ns interval granularity 0.5ns will be used \
    and 5ns granularity will be used for all other hits')
    tileSimFlags.addFlag('TimeCut', 'NONE', help='Time cut for hits, all hits go to one single time bin if time is above this cut')
    tileSimFlags.addFlag('PlateToCell', 'NONE', help='Special flag for Calibration Hits. If true then Tile. Plates are the parts of the adjacent Tile cells. If false then they are Dead Materials')
    tileSimFlags.addFlag('doTileRow', 'NONE', help='Enable energy per tile row in TileHit')
    tileSimFlags.addFlag('doTOFCorrection', 'NONE', help='Apply TOF correction (subtract Time Of Flight from ATLAS center')
    tileSimFlags.addFlag('doBirk', 'NONE', help='Enable Birk\'s law')
    tileSimFlags.addFlag('OldBirk', 'NONE', help='Use expected values from NIM 80 (1970) 239-244: birk1=0.0130 g/(MeV*cm^2), birk2=9.6e-6 (g/(MeV*cm^2))^2')
    tileSimFlags.addFlag('Birk1', 'NONE', help='Parameter for Birk\'s law')
    tileSimFlags.addFlag('Birk2', 'NONE', help='Parameter for Birk\'s law')

    tileSimFlags.addFlag('Ushape', 'NONE', help='Needed for the U-shape (any value of Ushape > 0 means that tile size equal to size of master plate, for Ushape <=0 - size of the tile is like in old geometry)')
    tileSimFlags.addFlag('Steel', 'NONE', help='Select steel with 0.45 percent Manganse for absorber instead of pure Iron. Any value > 0 enables Steel')
    tileSimFlags.addFlag('PVT', 'NONE', help='Use PVT instead of PS for scintillator material. Any value > 0 enables PVT')
    tileSimFlags.addFlag('CsTube', 'NONE', help='Special option to enable Cs tubes in simulation. Any value > 0 enables them')

    return tileSimFlags
