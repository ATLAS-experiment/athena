/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BCMPRIMEGEOMODELXML_BCMPRIMEDETECTORTOOL_H
#define BCMPRIMEGEOMODELXML_BCMPRIMEDETECTORTOOL_H

#include <InDetGeoModelUtils/GeoModelXmlTool.h>

namespace InDetDD
{
  class BCMPrimeDetectorManager;
}

/** @class BCMPrimeDetectorTool

  Create an Athena Tool; handle Athena services and Tools needed for
  building the BCM' geometry. Then create the geometry using the BCMPrimeDetectorFactory.

  @author Jakob Novak <jakob.novak@cern.ch>
*/

class BCMPrimeDetectorTool : public GeoModelXmlTool
{
public:
  BCMPrimeDetectorTool(const std::string &type, const std::string &name, const IInterface *parent);
  virtual ~BCMPrimeDetectorTool() = default;
  virtual StatusCode create() override final;
  virtual StatusCode clear() override final;

private:
  const InDetDD::BCMPrimeDetectorManager *m_detManager{};
};

#endif // BCMPRIMEGEOMODELXML_BCMPRIMEDETECTORTOOL_H
