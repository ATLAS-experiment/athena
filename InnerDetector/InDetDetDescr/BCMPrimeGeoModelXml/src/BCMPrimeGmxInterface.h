/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BCMPRIMEGEOMODELXML_BCMPRIMEGMXINTERFACE_H
#define BCMPRIMEGEOMODELXML_BCMPRIMEGMXINTERFACE_H

#include <AthenaBaseComps/AthMessaging.h>
#include <GeoModelXml/GmxInterface.h>

#include <map>

namespace InDetDD
{

class BCMPrimeGmxInterface: public GmxInterface, public AthMessaging
{
public:
  BCMPrimeGmxInterface();

  virtual int sensorId(std::map<std::string, int> &index) const override final;

private:
};

} // namespace InDetDD

#endif // BCMPRIMEGEOMODELXML_BCMPRIMEGMXINTERFACE_H
