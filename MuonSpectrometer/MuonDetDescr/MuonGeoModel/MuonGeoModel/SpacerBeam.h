/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SpacerBeam_H
#define SpacerBeam_H

#include "MuonGeoModel/DetectorElement.h"
#include "MuonGeoModel/StandardComponent.h"

class GeoVPhysVol;

namespace MuonGM {
    class MYSQL;

    class SpacerBeam : public DetectorElement {
      public:
        SpacerBeam(const MYSQL& mysql, Component *s);
        GeoVPhysVol *build(StoredMaterialManager& matManager,
                           bool);
        GeoVPhysVol *build(StoredMaterialManager& matManager,
                           int cutoutson, bool);
        virtual void print() const override;

        double width{0.};
        double length{0.};
        double thickness{0.};
        double lowerThickness{0.};
        double longWidth{0.}; // for trapezoidal layers
        double height{0.};    // web of I-beam (or height of box)
        double largeness{0.}; // flange width of I-beam (CHV, CRO, CMI)
        double excent{0.};    // angle of beams in trapezoidal chambers

      private:
        StandardComponent m_component{};
        double m_cy{0.};           // y coordinate (parallel to tube length) of component
        double m_hole_pos1{0.};    // Location of 1st LB hole in cross beam
        double m_hole_pos2{0.};    // Location of 2nd LB hole in cross beam
        double m_lb_height{0.};    // Height of LB hole
        double m_lb_width{0.};     // Width of LB hole
        double m_cross_excent{0.}; // angle of cross beams needed by LB
    };

} // namespace MuonGM

#endif
