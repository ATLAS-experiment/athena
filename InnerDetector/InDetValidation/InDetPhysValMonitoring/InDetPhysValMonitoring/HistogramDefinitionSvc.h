/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//
//  HistogramDefinitionSvc.h
//
//  Created by sroe on 07/07/2015.
//

#ifndef HistogramDefinitionSvc_h
#define HistogramDefinitionSvc_h

#include "InDetPhysValMonitoring/IHistogramDefinitionSvc.h"
#include "InDetPhysValMonitoring/SingleHistogramDefinition.h"
#include <map>
#include "AthenaBaseComps/AthService.h"
#include <memory>

class IReadHistoDef;
class ISvcLocator;
class StatusCode;
class InterfaceID;


class HistogramDefinitionSvc : public extends<AthService, IHistogramDefinitionSvc> {
public:
    HistogramDefinitionSvc(const std::string &name, ISvcLocator * svc);
    virtual ~HistogramDefinitionSvc();
    //@name Service methods, reimplemented
    //@{
    virtual StatusCode initialize();
    virtual StatusCode finalize();
    //@}
    SingleHistogramDefinition definition(std::string_view name, std::string_view  dirName="") const final;
    std::string histoType(std::string_view name, std::string_view  dirName="") const final;
    std::string title(std::string_view name, std::string_view dirName="") const final;
    unsigned int nBinsX(std::string_view name, std::string_view dirName="") const final;
    unsigned int nBinsY(std::string_view name, std::string_view dirName="") const final;
    unsigned int nBinsZ(std::string_view name, std::string_view dirName="") const final;
    IHistogramDefinitionSvc::axesLimits_t xLimits(std::string_view name, std::string_view dirName="") const final;
    IHistogramDefinitionSvc::axesLimits_t yLimits(std::string_view name, std::string_view dirName="") const final;
    IHistogramDefinitionSvc::axesLimits_t zLimits(std::string_view name, std::string_view dirName="") const final;
    std::string xTitle(std::string_view name, std::string_view dirName="") const final;
    std::string yTitle(std::string_view name, std::string_view dirName="") const final;
    std::string zTitle(std::string_view name, std::string_view dirName="") const final;
    
private:
    StringProperty m_source{this, "DefinitionSource"};
    StringProperty m_formatString{this, "DefinitionFormat", "text/plain"};
    IHistogramDefinitionSvc::Formats m_format;
    std::map<std::string, SingleHistogramDefinition> m_histoDefMap;
    std::unique_ptr<IReadHistoDef> m_reader;
    bool sourceExists();
    bool formatOk();
    
};
#endif /* defined(HistogramDefinitionSvc_h) */
