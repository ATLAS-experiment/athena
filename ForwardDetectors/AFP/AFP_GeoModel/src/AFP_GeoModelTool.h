/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFP_GeoModelTool_H
#define AFP_GeoModelTool_H

#include "GeoModelUtilities/GeoModelTool.h"
#include "AFP_GeoModelFactory.h"
#include "AFP_Geometry/AFP_Geometry.h"


class AFP_GeoModelTool final : public GeoModelTool
{

 public:
    AFP_GeoModelTool( const std::string& type, const std::string& name, const IInterface* parent );
    virtual ~AFP_GeoModelTool() override final;
    
    virtual StatusCode create() override final;
    
 private:
    AFP_CONFIGURATION m_CfgParams;
    AFP_Geometry* m_pGeometry;
    std::unique_ptr<AFP_GeoModelFactory> m_pAFPDetectorFactory;
    AFP_SIDCONFIGURATION m_defsidcfg;
    std::vector<double> m_vecAFP00XStaggering, m_vecAFP00YStaggering;
    std::vector<double> m_vecAFP01XStaggering, m_vecAFP01YStaggering;
    std::vector<double> m_vecAFP02XStaggering, m_vecAFP02YStaggering;
    std::vector<double> m_vecAFP03XStaggering, m_vecAFP03YStaggering;
    
 private:
    StatusCode checkPropertiesSettings();
};

#endif // AFP_GeoModelTool_H
