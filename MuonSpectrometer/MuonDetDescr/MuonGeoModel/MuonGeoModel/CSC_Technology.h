/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CSC_H
#define CSC_H

#include "MuonGeoModel/Technology.h"

namespace MuonGM {
    class MYSQL;

    class CSC : public Technology {
      public:
        inline CSC(MYSQL& sql, const std::string& s);
        int numOfLayers{0};
        double innerRadius{0.};
        double totalThickness{0.};
        double nonsisa{0.};
        double honeycombthick{0.};
        double g10thick{0.}; 
        double wirespacing{0.};
        double anocathodist{0.};
        double gapbetwcathstrips{0.};
        double readoutstripswidth{0.};
        double phistripwidth{0.};
        double floatingstripswidth{0.};
        double rectwasherthick{0.};
        double roxacellwith{0.};
        double roxwirebargap{0.}; 
        double fullgasgapwirewidth{0.}; 
        double fullwirefixbarwidth{0.};
        double wirebarposx{0.}; 
        double wirebarposy{0.}; 
        double wirebarposz{0.};

        double cathreadoutpitch{0.};
        double phireadoutpitch{0.};
        int nEtastrips{0}; 
        int nPhistrips{0};
    };

    CSC::CSC(MYSQL& mysql, const std::string& s)
        : Technology(mysql, s){}
} // namespace MuonGM

#endif
