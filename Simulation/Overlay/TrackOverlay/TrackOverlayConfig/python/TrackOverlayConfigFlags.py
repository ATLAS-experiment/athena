# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createTrackOverlayConfigFlags():
    mlcf = AthConfigFlags()
    mlcf.addFlag("TrackOverlay.isTrackOverlaySeq", False)
    mlcf.addFlag("TrackOverlay.MLThreshold", 0.74201)

    return mlcf

if __name__ == "__main__":
    flags = createTrackOverlayConfigFlags()
    flags.dump()
