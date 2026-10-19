# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createTrackOverlayConfigFlags():
    mlcf = AthConfigFlags()
    mlcf.addFlag("TrackOverlay.isTrackOverlaySeq", False)
    mlcf.addFlag("TrackOverlay.MLThreshold", 0.74201)

    # RoI-limited silicon overlay
    mlcf.addFlag("TrackOverlay.doRoISiOverlay", False)
    mlcf.addFlag("TrackOverlay.RoIMinClusterEt", 10000.)     # MeV, EM cluster ET to open an RoI
    mlcf.addFlag("TrackOverlay.RoIEtaHalfWidth", 0.15)
    mlcf.addFlag("TrackOverlay.RoIPhiHalfWidth", 0.3)
    mlcf.addFlag("TrackOverlay.RoIZHalfWidth", 225.)         # mm, beam-line z range of the RoI
    # Primary-pass tracks with a smaller fraction of Si clusters containing signal RDOs are pu tracks re-found
    mlcf.addFlag("TrackOverlay.RoIMinSignalClusterFraction", 0.5)

    return mlcf

if __name__ == "__main__":
    flags = createTrackOverlayConfigFlags()
    flags.dump()
