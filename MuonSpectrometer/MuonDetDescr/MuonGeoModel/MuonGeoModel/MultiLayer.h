/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MultiLayer_H
#define MultiLayer_H

#include "MuonGeoModel/DetectorElement.h"

#include <string>
#include <utility>
#include <vector>

class GeoFullPhysVol;

namespace MuonGM {
    class MYSQL;

    class MultiLayer : public DetectorElement {
      public:
        int nrOfLayers{0};
        int nrOfTubes{0};    // tubes in a layer
        double tubePitch{0.}; // tube pitch in a layer
        double width{0.};
        double length{0.};
        double thickness{0.};
        double mdtthickness{0.};
        double longWidth{0.}; // for trapezoidal layers
        int nrOfSteps{0};    // for trapezoidal layers,
                       // nr of steps in the staircase
        MultiLayer(const MYSQL& mysql, const std::string& n);
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              const MYSQL& mysql);
        std::array<double, 4> yy{};
        std::array<double, 4> xx{};
        virtual void print() const override;

        int cutoutNsteps{0};           // how many sub-multilayers there are along y-amdb
        std::array<int, 5> cutoutNtubes{};        // how many tubes in the sub-multilayer [i]
        std::array<double, 5> cutoutXtubes{};     // where is the centre of the tubes in sub-ml[i] along x-amdb
        std::array<double, 5> cutoutYmax{};       // max Y(amdb) of this sub-multilayer
        std::array<double, 5> cutoutTubeLength{}; // tube length
        std::array<bool, 5> cutoutFullLength{};   // true if this region is outside the cutout
        bool cutoutAtAngle{false};         // true if this station has cutouts at an angle; //EMS1,3 and BOS6

        // the same but for several cutouts along the amdb x (GeoModel y)
        // gives the (x1,x2) and (y1,y2) tuples of rectangles which are NOT cutout
        std::vector<std::pair<double, double>> m_nonCutoutXSteps{};
        std::vector<std::pair<double, double>> m_nonCutoutYSteps{};
    };
} // namespace MuonGM

#endif
