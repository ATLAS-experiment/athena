/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/StandardComponent.h"

#include "MuonGeoModel/CHV_Technology.h"
#include "MuonGeoModel/CMI_Technology.h"
#include "MuonGeoModel/CRO_Technology.h"
#include "MuonGeoModel/LBI_Technology.h"
#include "MuonGeoModel/MYSQL.h"
#include "MuonGeoModel/Technology.h"

#include <ostream>
#include <string>

namespace MuonGM {

    std::ostream &operator<<(std::ostream &os, const StandardComponent &c) {
        os << "Component " << c.name << std::endl;
        os << c.posx << " " << c.posy << " " << c.posz << " " << c.index << " " << c.name;
        os << " " << c.iswap << " " << c.dx1 << " " << c.dx2 << " " << c.dy << " ";
        os << c.excent << " " << c.deadx << " " << c.deady << std::endl;
        return os;
    }

    double StandardComponent::GetThickness(const MYSQL& mysql) const {
        const Technology *tec = mysql.GetTechnology(name);

        if (name.compare(0, 3,"CHV") == 0) {
            const CHV *chv = dynamic_cast<const CHV*>(tec);
            return chv->height;
        } else if (name.compare(0, 3,"CRO") == 0) {
            const CRO *chv = dynamic_cast<const CRO*>(tec);
            return chv->height;
        } else if (name.compare(0, 3,"CMI") == 0) {
            const CMI *chv = dynamic_cast<const CMI*>(tec);
            return chv->height;
        } else if (name.compare(0, 3,"LBI") == 0 || name.compare(0, 2,"LB") == 0) {
            const LBI *chv = dynamic_cast<const LBI*>(tec);
            return chv->height;
        }
        return mysql.GetTechnology(name)->thickness;
    }

} // namespace MuonGM
