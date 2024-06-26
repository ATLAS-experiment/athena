/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonSystemDescription_H
#define MuonSystemDescription_H

#include <string>

namespace MuonGM {

    class MuonSystemDescription {
      public:
        double barrelInnerRadius{0.}; // Inner radius behind the idet
        double innerRadius{0.};       // Inner radius at the beam pipe
        double outerRadius{0.};
        double endcapFrontFace{0.}; // Z at the endcap front face
        double length{0.};
        double barreLength{0.};
        double barrelInterRadius{0.};

        double extraZ{0.};
        double extraR{0.};

        std::string amdb{};

        MuonSystemDescription(std::string n);

      private:
        std::string m_name{};
    };
} // namespace MuonGM

#endif
