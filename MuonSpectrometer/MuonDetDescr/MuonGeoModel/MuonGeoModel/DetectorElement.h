/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DetectorElement_H
#define DetectorElement_H

#include "GeoPrimitives/GeoPrimitives.h"

#include "GeoModelInterfaces/StoredMaterialManager.h"
#include <string>

namespace MuonGM {

    class DetectorElement {
      public:
        std::string name{};
        std::string logVolName{};

        DetectorElement(const std::string& n) : name(n) {  }

        void setLogVolName(const std::string& str) { logVolName = str; }

        virtual void print() const = 0;

        virtual ~DetectorElement() = default;

    }; // class DetectorElement

} // namespace MuonGM

#endif
