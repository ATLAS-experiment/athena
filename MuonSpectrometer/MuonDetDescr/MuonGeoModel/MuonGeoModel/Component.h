/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Component_H
#define Component_H
#include <string>

namespace MuonGM {

    class Component {
      public:
        Component() = default;
        Component(const Component &c) = default;
        Component &operator=(const Component &c) = default;
        virtual ~Component() = default;
        double GetThickness() const;
        std::string name{};
        double dx1{0.};
        double dx2{0.};
        double dy{0.};
    };
} // namespace MuonGM

#endif
