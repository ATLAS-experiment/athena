
#ifndef TRKEXTOOLS_ParametersNextVolume_H
#define TRKEXTOOLS_ParametersNextVolume_H

#include "TrkGeometry/TrackingVolume.h" //for BoundarySurfaceFace
#include "ExtrUniquePtrHolder.h"
#include "TrkParameters/TrackParameters.h"


namespace Trk{
  class TrackingGeometry;

  struct ParametersNextVolume{

    //!< the members
    const TrackingVolume* nextVolume;
    Trk::TrackParameters* nextParameters;
    Trk::TrackParameters* navParameters;
    BoundarySurfaceFace exitFace;

    ParametersNextVolume()
      : nextVolume(nullptr)
      , nextParameters(nullptr)
      , navParameters(nullptr)
      , exitFace(undefinedFace){
    }

    //!< update the boundaryInformation
    void boundaryInformation(const TrackingVolume* tvol,
                             Trk::TrackParameters* nextPars,
                             Trk::TrackParameters* navPars,
                             BoundarySurfaceFace face = undefinedFace){
      nextVolume = tvol;
      nextParameters = nextPars;
      navParameters = navPars;
      exitFace = face;
    }
    //!< reset the boundary information by invalidating it
    void resetBoundaryInformation(){
      nextVolume = nullptr;
      exitFace = undefinedFace;
      nextParameters = nullptr;
      navParameters = nullptr;
    }
  };
}
#endif
