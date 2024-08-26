/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AGDDPositioner_H
#define AGDDPositioner_H

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "GeoModelKernel/GeoDefinitions.h"

#include <string>

class AGDDVolume;
class AGDDPositionerStore;
class AGDDVolumeStore;

class AGDDPositioner {
public:
    AGDDPositioner(AGDDPositionerStore& ps,
                   AGDDVolumeStore& vs,
                   const std::string& n,
                   const GeoTrf::Transform3D& t);
    virtual ~AGDDPositioner()=default;
    const std::string& Volume();
    AGDDVolume *GetVolume();
    const GeoTrf::Transform3D& Transform() const;
    bool IsSensitiveDetector() const {return m_isSensitiveDetector;}
    void SensitiveDetector(bool a) {m_isSensitiveDetector=a;}
	
private:

	bool m_isSensitiveDetector;
	std::string m_volume;
	AGDDVolume *m_theVolume;
	GeoTrf::Transform3D m_transform;
};

#endif
