/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELSVC_GEOMODELSVC_H
#define GEOMODELSVC_GEOMODELSVC_H

#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "GeoModelInterfaces/IGeoModelTool.h"
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelRead/ReadGeoModel.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"
#include "AthenaBaseComps/AthService.h"
#include "CxxUtils/checker_macros.h"
#include "StoreGate/StoreGateSvc.h"
#include "EventInfoMgt/ITagInfoMgr.h"
#include <memory>

class ISvcLocator;
class GMDBManager;
namespace GeoModelIO {
  class ReadGeoModel;
}

template <class TYPE> class SvcFactory;

class GeoModelSvc : public AthService, virtual public IGeoModelSvc,
                    virtual public ITagInfoMgr::Listener
{
public:
    friend class SvcFactory<GeoModelSvc>;

    // Standard Constructor
    GeoModelSvc(const std::string& name, ISvcLocator* svc);

    virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE() override;
    virtual StatusCode finalize() override;

    virtual StatusCode queryInterface( const InterfaceID& riid, void** ppvInterface ) override;

    /// Callback from TagInfoMgr on TagInfo change
    virtual void       tagInfoUpdated() override final;

    StatusCode compareTags();

    virtual const IGeoModelTool* getTool(std::string toolName) const override;

    virtual StatusCode clear() override;

private:
    ToolHandleArray<IGeoModelTool> m_detectorTools{this,"DetectorTools",{}};

    ISvcLocator*        m_pSvcLocator{nullptr};

    ServiceHandle<StoreGateSvc> m_detStore{this,"DetectorStore","DetectorStore",""};
    ServiceHandle<ITagInfoMgr>  m_tagInfoMgr{this,"TagInfoMgr","TagInfoMgr",""};
    ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{this,"GeoDbTagSvc","GeoDbTagSvc",""};

    Gaudi::Property<std::string> m_atlasVersion{this,"AtlasVersion","","ATLAS Geometry Tag"};

    Gaudi::Property<std::string> m_inDetVersionOverride{this,"InDetVersionOverride","","Overrider for InDet version"};
    Gaudi::Property<std::string> m_pixelVersionOverride{this,"PixelVersionOverride","","Overrider for Pixel version"};
    Gaudi::Property<std::string> m_sctVersionOverride{this,"SCT_VersionOverride","","Overrider for SCT version"};
    Gaudi::Property<std::string> m_trtVersionOverride{this,"TRT_VersionOverride","","Overrider for TRT version"};
    Gaudi::Property<std::string> m_larVersionOverride{this,"LAr_VersionOverride","","Overrider for LAr version"};
    Gaudi::Property<std::string> m_tileVersionOverride{this,"TileVersionOverride","","Overrider for Tile version"};
    Gaudi::Property<std::string> m_muonVersionOverride{this,"MuonVersionOverride","","Overrider for Muon version"};
    Gaudi::Property<std::string> m_caloVersionOverride{this,"CaloVersionOverride","","Overrider for Calo version"};
    Gaudi::Property<std::string> m_bFieldVersionOverride{this,"MagFieldVersionOverride","","Overrider for MagField version"};
    Gaudi::Property<std::string> m_cavInfraVersionOverride{this,"CavernInfraVersionOverride","","Overrider for CavernInfra version"};
    Gaudi::Property<std::string> m_forDetVersionOverride{this,"ForwardDetectorsVersionOverride","","Overrider for Forward Detectors version"};

    Gaudi::Property<bool> m_callBackON{this,"AlignCallbacks",true,"Read alignment in callbacks"};
    Gaudi::Property<bool> m_ignoreTagDifference{this,"IgnoreTagDifference",false,"Ignore TagInfo and configuration tag diffs"};

    Gaudi::Property<bool> m_useTagInfo{this,"UseTagInfo",true,"Use TagInfo"};
    Gaudi::Property<bool> m_statisticsToFile{this,"StatisticsToFile",false,"Generate GeoModelStatistics file in the run directory"};

    Gaudi::Property<int>  m_supportedGeometry{this,"SupportedGeometry",0,"Supported geometry flag is set in jobOpt and is equal to major release version"};
    Gaudi::Property<bool> m_ignoreTagSupport{this,"IgnoreTagSupport",false,"Skip checking if the geometry tag is supported/obsolete"};

    Gaudi::Property<bool> m_sqliteDb{this,"SQLiteDB",false,"Activate GeoModel initialization from SQLite"};
    Gaudi::Property<std::string> m_sqliteDbFullPath{this,"SQLiteDBFullPath","","Explicit setting of full path to SQLiteDB. For testing purposes only"};

    std::unique_ptr<GeoModelIO::ReadGeoModel> m_sqliteReader{};
    std::unique_ptr<GMDBManager>              m_sqliteDbManager{};

    virtual const std::string & atlasVersion()                     const override {return m_atlasVersion;}
    virtual const std::string & inDetVersionOverride()             const override {return m_inDetVersionOverride;}
    virtual const std::string & pixelVersionOverride()             const override {return m_pixelVersionOverride;}
    virtual const std::string & SCT_VersionOverride()              const override {return m_sctVersionOverride;}
    virtual const std::string & TRT_VersionOverride()              const override {return m_trtVersionOverride;}
    virtual const std::string & LAr_VersionOverride()              const override {return m_larVersionOverride;}
    virtual const std::string & tileVersionOverride()              const override {return m_tileVersionOverride;}
    virtual const std::string & muonVersionOverride()              const override {return m_muonVersionOverride;}
    virtual const std::string & caloVersionOverride()              const override {return m_caloVersionOverride;}
    virtual const std::string & magFieldVersionOverride()          const override {return m_bFieldVersionOverride;}
    virtual const std::string & cavernInfraVersionOverride()       const override {return m_cavInfraVersionOverride;}
    virtual const std::string & forwardDetectorsVersionOverride()  const override {return m_forDetVersionOverride;}

    virtual const std::string & inDetVersion()              const override {return m_geoDbTagSvc->inDetVersion();}
    virtual const std::string & pixelVersion()              const override {return m_geoDbTagSvc->pixelVersion();}
    virtual const std::string & SCT_Version()               const override {return m_geoDbTagSvc->SCT_Version();}
    virtual const std::string & TRT_Version()               const override {return m_geoDbTagSvc->TRT_Version();}
    virtual const std::string & LAr_Version()               const override {return m_geoDbTagSvc->LAr_Version();}
    virtual const std::string & tileVersion()               const override {return m_geoDbTagSvc->tileVersion();}
    virtual const std::string & muonVersion()               const override {return m_geoDbTagSvc->muonVersion();}
    virtual const std::string & caloVersion()               const override {return m_geoDbTagSvc->caloVersion();}
    virtual const std::string & magFieldVersion()           const override {return m_geoDbTagSvc->magFieldVersion();}
    virtual const std::string & cavernInfraVersion()        const override {return m_geoDbTagSvc->cavernInfraVersion();}
    virtual const std::string & forwardDetectorsVersion()   const override {return m_geoDbTagSvc->forwardDetectorsVersion();}

    virtual GeoModel::GeoConfig geoConfig() const override {return m_geoDbTagSvc->geoConfig();}

    StatusCode geoInit();
    StatusCode fillTagInfo() const;
};

#endif // GEOMODELSVC_GEOMODELSVC_H

