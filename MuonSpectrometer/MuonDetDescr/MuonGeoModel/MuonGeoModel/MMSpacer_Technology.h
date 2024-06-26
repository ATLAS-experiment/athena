/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MMSpacer_Technology_H
#define MMSpacer_Technology_H

#include "MuonGeoModel/Technology.h"

#include <vector>
namespace MuonGM {

    // Description class to build MicroMegas spacers

    class MMSpacer_Technology : public Technology {
      public:
        // constructor
        inline MMSpacer_Technology(MYSQL& mysql, const std::string& s);
        inline double Thickness() const;

        int lowZCutOuts{0};
        double lowZCutOutWidth{0.};
        double lowZCutOutDZ{0.};

        int highZCutOuts{0};
        double highZCutOutWidth{0.};
        double highZCutOutDZ{0.};
    };

    MMSpacer_Technology::MMSpacer_Technology(MYSQL& mysql, const std::string& s)
        : Technology(mysql, s) {}

    double MMSpacer_Technology::Thickness() const { return thickness; }

} // namespace MuonGM

#endif
