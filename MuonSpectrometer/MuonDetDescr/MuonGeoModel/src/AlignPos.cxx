/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/AlignPos.h"

namespace MuonGM {
    std::ostream &operator<<(std::ostream &os, const AlignPos &p) {
        os << "phi ind=" << p.phiindex << " z ind=" << p.zindex << " tra_s=" << p.tras << " tra_z=" << p.traz << " tra_t=" << p.trat << " (in mm)"
           << " rot_s=" << p.rots * 1000. << " rot_z=" << p.rotz * 1000. << " rot_t=" << p.rott * 1000. << " (angles in milli-radians) "
           << " is barrel? " << p.isBarrel << " station type= " << p.tectype;

        return os;
    }

} // namespace MuonGM
