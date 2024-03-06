/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/MuonDetectorTool.h"

#include "AmdcDb/AmdcDb.h"
#include "AthenaKernel/ClassID_traits.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelKernel/GeoPerfUtils.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "MuonGeoModel/StationSelector.h"
#include "MuonGeoModel/MuonDetectorFactory001.h"
#include "MuonGeoModel/MuonDetectorFactoryLite.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

#include <fstream>

using namespace MuonGM;

/**
 ** Constructor(s)
 **/
MuonDetectorTool::MuonDetectorTool(const std::string &type, const std::string &name, const IInterface *parent) : 
    GeoModelTool(type, name, parent) {
    declareInterface<IGeoModelTool>(this);
}

StatusCode MuonDetectorTool::initialize() {
    ATH_MSG_INFO("Initializing ...");
    return StatusCode::SUCCESS;
}
MuonDetectorTool::~MuonDetectorTool() { 
    if (m_detector) {
        delete m_detector;
        m_detector = nullptr;
    } 
}

/**
 ** Create the Detector Node corresponding to this tool
 **/
StatusCode MuonDetectorTool::create() {

    std::ofstream geoModelStats;
    int mem{0}, umem{0};
    float cpu{0.f}, ucpu{0.f};

    MuonGM::MuonDetectorManager *mgr=nullptr;
    GeoModelExperiment *theExpt = nullptr;
    ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));
    GeoIntrusivePtr<GeoPhysVol> world{theExpt->getPhysVol()};
    ATH_CHECK(createFactory(mgr, world));

    if (!m_detector) {
        ATH_CHECK(detStore()->record(mgr,mgr->getName()));
        theExpt->addManager(mgr);

        m_manager = mgr;
    }

    if (m_dumpMemoryBreakDown) {
        umem = GeoPerfUtils::getMem();
        ucpu = float(GeoPerfUtils::getCpu() / 100.);
        geoModelStats << "At MuonDetectorTool::everything done     \t SZ= " << umem << " Kb \t Time = " << ucpu << " seconds  ---- \t DeltaM = " << umem - mem
                      << " \t Delta T =" << ucpu - cpu << std::endl;
        geoModelStats.close();
    }

    Muon::MuonSectorMapping mapping;
    /// add hash look-up for TGC sectors to the detector store here
    if (m_manager && m_manager->tgcIdHelper()) {
        unsigned int tgcHashMax = m_manager->tgcIdHelper()->detectorElement_hash_max();
        std::vector<int> *sectorHashVec = new std::vector<int>(tgcHashMax, 0);
        for (unsigned int i = 0; i < tgcHashMax; ++i) {
            const MuonGM::TgcReadoutElement *detEl = m_manager->getTgcReadoutElement(i);
            if (detEl)
                (*sectorHashVec)[i] = mapping.getSector(detEl->center().phi());
        }
        if (detStore()->record(sectorHashVec, "TGC_SectorMapping").isFailure()) {
            ATH_MSG_WARNING(" failed to record TGC sector mapping ");
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode MuonDetectorTool::createFactory(MuonGM::MuonDetectorManager * & mgr, GeoPhysVol* world) const {

    std::ofstream geoModelStats;
    int mem{0}, umem{0};
    float cpu{0.f}, ucpu{0.f};


    if (m_dumpMemoryBreakDown) {
        geoModelStats.open("MuonGeoModelStatistics_MuonDetectorTool");
        mem = GeoPerfUtils::getMem();
        cpu = float(GeoPerfUtils::getCpu() / 100.);
        geoModelStats << "At MuonDetectorTool::create entry point: \t SZ= " << mem << " Kb \t Time = " << cpu << " seconds " << std::endl;

        umem = GeoPerfUtils::getMem();
        ucpu = float(GeoPerfUtils::getCpu() / 100.);
        geoModelStats << "At MuonDetectorTool::resetting to 0:     \t SZ= " << umem - mem << " Kb \t Time = " << ucpu - cpu << " seconds " << std::endl;
        mem = umem;
        cpu = ucpu;
    }

    ATH_MSG_VERBOSE(" Show properties (user sett.s or default): LayoutName                 "
                    << m_layout << endmsg << " Show properties (user sett.s or default): IncludeCutouts             " << m_includeCutouts << endmsg
                    << " Show properties (user sett.s or default): IncludeCutoutsBog          " << m_includeCutoutsBog << endmsg
                    << " Show properties (user sett.s or default): IncludeCtbBis              " << m_includeCtbBis);

    // Get the detector configuration.
    IGeoModelSvc *geoModel;
    ATH_CHECK(service("GeoModelSvc", geoModel));

    std::string AtlasVersion = geoModel->atlasVersion();
    std::string MuonVersion = geoModel->muonVersionOverride();

    ATH_MSG_INFO("create MuonDetectorTool");
    ATH_MSG_INFO("(from GeoModelSvc)    AtlasVersion = <" << AtlasVersion << ">  MuonVersion = <" << MuonVersion << ">");

    // Unless we are using custom muons, the switch positions are going to
    // come from the database:

    std::string detectorKey = MuonVersion.empty() ? AtlasVersion : MuonVersion;
    std::string detectorNode = MuonVersion.empty() ? "ATLAS" : "MuonSpectrometer";
    ATH_MSG_INFO("Keys for Muon Switches are  (key) " << detectorKey << " (node) " << detectorNode);

    std::string tempLayout = m_layout;
    std::map<std::string, std::string> altAsciiDBMap{};

    
    // Get the detector configuration.
    ServiceHandle<IGeoDbTagSvc> geoDbTag("GeoDbTagSvc",name());
    ATH_CHECK(geoDbTag.retrieve());
    ServiceHandle<IRDBAccessSvc> accessSvc(geoDbTag->getParamSvcName(),name());
    ATH_CHECK(accessSvc.retrieve());
    GeoModelIO::ReadGeoModel* sqliteReader = geoDbTag->getSqliteReader();
    
    if (sqliteReader) {

      ATH_MSG_INFO("New DD Activated; Muon detector description input from SQLITE fie");

      MuonDetectorFactoryLite theFactory(detStore().operator->(),sqliteReader);
      theFactory.setRDBAccess(accessSvc.get());
      theFactory.create(world);  


      mgr=theFactory.getDetectorManager();
      mgr->setMinimalGeoFlag(m_minimalGeoFlag);
      mgr->setGeometryVersion(tempLayout);
      mgr->fillCache();
    
      return StatusCode::SUCCESS;
    }
    //
    // New DD:  action ends here!!
    // 
    //=====================================================
    // 
    // Old DD: action starts here:  
    //
    
    MuonDetectorFactory001 theFactory(detStore().operator->());
    theFactory.disableBEEShift(m_beeNoShiftInDefault);


    if (MuonVersion == "CUSTOM")
        ATH_MSG_WARNING("Detector Information coming from a custom configuration !!");
    else {

        ATH_MSG_DEBUG("Detector Information coming from the database (job options IGNORED)");

        IRDBRecordset_ptr switchSet = accessSvc->getRecordsetPtr("MuonSwitches", detectorKey, detectorNode);
        if ((*switchSet).size() == 0)
            return StatusCode::FAILURE;
        const IRDBRecord *switches = (*switchSet)[0];

        // m_layout                = switches->getString("LAYOUTNAME");
        tempLayout = switches->getString("LAYOUTNAME");
        //       m_includeInertMaterials = switches->getInt("BUILDINERTMATERIALS");
        //       m_minimalgeo            = switches->getInt("MINIMALGEO");
        if (MuonVersion.empty()) {
            MuonVersion = accessSvc->getChildTag("MuonSpectrometer", detectorKey, detectorNode);
            ATH_MSG_INFO("(from GeoModelSvc) in AtlasVersion = <" << AtlasVersion << ">  default MuonVersion is <" << MuonVersion << ">");
        }

        ATH_MSG_DEBUG(" m_altAsztFile: " << m_altAsztFile);
        // use ascii file to read in ASZT parameters
        if (m_altAsztFile != "")
            altAsciiDBMap.insert(std::make_pair("ASZT", m_altAsztFile));
        if (m_altCscIntAlinesFile != "")
            altAsciiDBMap.insert(std::make_pair("IACSC", m_altCscIntAlinesFile));
    }

    //
    // Locate the top level experiment node
    //

    ATH_MSG_INFO("Properties have been set as follows: " << endmsg
                                                         // <<"    LayoutName                     "<< m_layout.substr(0,1) <<endmsg
                                                         << "    LayoutName                     " << tempLayout.substr(0, 1) << endmsg 
                                                         << "    IncludeCutouts                 " << m_includeCutouts << endmsg 
                                                         << "    IncludeCutoutsBog              " << m_includeCutoutsBog << endmsg
                                                         << "    IncludeCtbBis                  " << m_includeCtbBis << endmsg 
                                                         << "    MinimalGeoFlag                 " << m_minimalGeoFlag << endmsg
                                                         << "    EnableCscIntAlignmentFromGM    " << m_useCscIntAlinesFromGM);
   
    if (m_stationSelection > 0) {
        StationSelector::SetSelectionType(m_stationSelection);
        if ((m_selectedStations.size() + m_selectedStEta.size() + m_selectedStPhi.size()) < 1) {
            ATH_MSG_ERROR(" **** Badly set Option " << endmsg << " **** SelectedStations size =" << m_selectedStations.size() << endmsg
                                                    << " **** SelectedStJzz    size =" << m_selectedStations.size() << endmsg
                                                    << " **** SelectedStJff    size =" << m_selectedStations.size() << endmsg << " **** while StationSelection = 1");
            return (StatusCode::FAILURE);
        }
        for (unsigned int i = 0; i < m_selectedStations.size(); i++) {
            ATH_MSG_INFO("          Selected stations      " << m_selectedStations[i]);
        }

        if (m_selectedStEta.size() > 0) {
            for (unsigned int i = 0; i < m_selectedStEta.size(); i++)
                ATH_MSG_INFO("          Selected Jzz locations  " << m_selectedStEta[i]);
        }
        if (m_selectedStPhi.size() > 0) {
            for (unsigned int i = 0; i < m_selectedStPhi.size(); i++)
                ATH_MSG_INFO("          Selected Jff locations  " << m_selectedStPhi[i]);
        }
    }

    if (m_dumpMemoryBreakDown) {
        umem = GeoPerfUtils::getMem();
        ucpu = float(GeoPerfUtils::getCpu() / 100.);
        geoModelStats << "At MuonDetectorTool::material mgr done   \t SZ= " << umem << " Kb \t Time = " << ucpu << " seconds  ---- \t DeltaM = " << umem - mem
                      << " \t Delta T =" << ucpu - cpu << std::endl;
        mem = umem;
        cpu = ucpu;
    }

    if (nullptr == m_detector) {
        IRDBAccessSvc *access = nullptr;
        if (m_amdcDb)
            ATH_CHECK(service("AmdcDb", access));
        else
            ATH_CHECK(service("RDBAccessSvc", access));

        bool isAmdcDb = false;
        if (dynamic_cast<AmdcDb *>(access) && m_amdcDb) {
            ATH_MSG_INFO("AmdcDb is used instead of RDBAccessSvc");
            isAmdcDb = true;
            ATH_MSG_INFO("AmdcDb->InitializedSvc() is true");
        }

        theFactory.setDBAtlasVersion(AtlasVersion);
        theFactory.setDBMuonVersion(MuonVersion);
        theFactory.setDBkey(detectorKey);
        theFactory.setDBnode(detectorNode);       
        theFactory.setAmdcDb(isAmdcDb);
        theFactory.setLayout(tempLayout);
        theFactory.setCutoutsFlag(m_includeCutouts);
        theFactory.setCutoutsBogFlag(m_includeCutoutsBog);
        theFactory.setCtbBisFlag(m_includeCtbBis);
        theFactory.setMinimalGeoFlag(m_minimalGeoFlag);
        theFactory.setDumpMemoryBreakDown(m_dumpMemoryBreakDown);
        theFactory.setFineClashFixingFlag(m_enableFineClashFixing);
        theFactory.hasCSC(m_hasCSC);
        theFactory.hasSTgc(m_hasSTgc);
        theFactory.hasMM(m_hasMM);
        if (m_stationSelection > 0)
            theFactory.setSelection(m_selectedStations, m_selectedStEta, m_selectedStPhi);

        theFactory.setRDBAccess(access);
        // theFactory.setUseRDB(1);
        theFactory.setAltAsciiDBMap(altAsciiDBMap);
        try {
            //
            // This strange way of casting is to avoid an
            // utterly brain damaged compiler warning.
            //
            theFactory.create(world);
        } catch (const std::bad_alloc &) {
            ATH_MSG_FATAL("Could not create new MuonDetectorNode!");
            return StatusCode::FAILURE;
        }

        if (m_dumpMemoryBreakDown) {
            umem = GeoPerfUtils::getMem();
            ucpu = float(GeoPerfUtils::getCpu() / 100.);
            geoModelStats << "At MuonDetectorTool::factory created     \t SZ= " << umem << " Kb \t Time = " << ucpu << " seconds  ---- \t DeltaM = " << umem - mem
                          << " \t Delta T =" << ucpu - cpu << std::endl;
            mem = umem;
            cpu = ucpu;
        }

        if (m_dumpMemoryBreakDown) {
            umem = GeoPerfUtils::getMem();
            ucpu = float(GeoPerfUtils::getCpu() / 100.);
            geoModelStats << "At MuonDetectorTool::trk cache done      \t SZ= " << umem << " Kb \t Time = " << ucpu << " seconds  ---- \t DeltaM = " << umem - mem
                          << " \t Delta T =" << ucpu - cpu << std::endl;
            mem = umem;
            cpu = ucpu;
        }
    }
    mgr=theFactory.getDetectorManager();
    mgr->fillCache();
    return StatusCode::SUCCESS;
}

StatusCode MuonDetectorTool::clear() {
    SG::DataProxy *proxy = detStore()->proxy(ClassID_traits<MuonGM::MuonDetectorManager>::ID(), m_manager->getName());
    if (proxy) {
        proxy->reset();
        m_manager = nullptr;
    }
    return StatusCode::SUCCESS;
}
