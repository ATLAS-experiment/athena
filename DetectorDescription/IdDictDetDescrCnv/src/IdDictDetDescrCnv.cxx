/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 IdDictDetDescrCnv package
 -----------------------------------------
 ***************************************************************************/

//<doc><file>   $Id: IdDictDetDescrCnv.cxx,v 1.21 2009-02-15 13:08:19 schaffer
//Exp $ <version>     $Name: not supported by cvs2svn $

#include "IdDictDetDescrCnv.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaKernel/StorableConversions.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "GeoModelUtilities/DecodeVersionKey.h"
#include "IdDictDetDescr/IdDictManager.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

// To write text file
#include <fstream>
#include <iostream>

//--------------------------------------------------------------------

long int IdDictDetDescrCnv::repSvcType() const {
    return (storageType());
}

//--------------------------------------------------------------------

StatusCode IdDictDetDescrCnv::initialize() {
    // First call parent init
    ATH_CHECK(DetDescrConverter::initialize());
    ATH_MSG_INFO("in initialize");
    // Must set indet tag to EMPTY
    m_inDetIDTag = "EMPTY";
    return StatusCode::SUCCESS;
}
StatusCode IdDictDetDescrCnv::finalize() {
    ATH_MSG_INFO("in finalize");
    return StatusCode::SUCCESS;
}
//--------------------------------------------------------------------

StatusCode IdDictDetDescrCnv::createObj(IOpaqueAddress *pAddr,
                                        DataObject *&pObj) {
    //
    // Here we create an IdDictManager and provide it with an
    // IdDictMgr which has been filled by an IdDictParser. This mgr
    // is used by each IdHelper to initialize itself.
    //
    //   Lifetime management:
    //
    //     IdDictDetDescrCnv holds onto ONE IdDictParser, which in
    //     turn holds the same IdDictMgr.
    //
    //     Multiple initializations are possible. parseXMLDescription
    //     will look for a new set of xml files, clear any
    //     pre-existing IdDictMgr help by the parser and then parse
    //     the new xml files, filling the IdDictMgr.
    //
    //     Since the parser "refills" the same IdDictMgr, one has the
    //     option to delete and recreate the IdDictManager, or just
    //     keep the same one which will be refreshed with the new
    //     description.
    //
    ATH_MSG_INFO(
        "in createObj: creating a IdDictManager object in the detector store");
    DetDescrAddress *ddAddr = dynamic_cast<DetDescrAddress *>(pAddr);
    if (!ddAddr) {
        ATH_MSG_ERROR("Could not cast to DetDescrAddress.");
        return StatusCode::FAILURE;
    }

    // Get the StoreGate key of this container.
    std::string mgrKey = *(ddAddr->par());
    if (mgrKey.empty())
        ATH_MSG_DEBUG("No Manager key ");
    else
        ATH_MSG_DEBUG("Manager key is " << mgrKey);

    ATH_CHECK(parseXMLDescription());

    // Create the manager - only once
    IdDictManager *dictMgr = new IdDictManager(m_parser->m_idd);

    ATH_MSG_DEBUG("Created IdDictManager ");

    // Print out the dictionary names
    printDicts(dictMgr);

    // Pass a pointer to the container to the Persistency service
    // by reference.
    pObj = SG::asStorable(dictMgr);

    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------

StatusCode IdDictDetDescrCnv::parseXMLDescription() {
    ATH_MSG_DEBUG("in parseXMLDescription()");

    // Parse the xml files to obtain the iddict dictionaries
    //
    //   Parsing of the xml files may not be needed. So we check. The
    //   conditions to reparse are:
    //
    //     - first pass, i.e. creating a new IdDictParser
    //     - a change in an input xml file
    //     - a change in one of the "options" such as doIdChecks,
    //       doInitNeighbors, etc.
    //

    m_doParsing = false;  // Preset to no parsing

    if (!m_parser) {
        // Create parser
        m_parser = std::make_unique<IdDictParser>();
        m_doParsing = true;
    }

    // We access the properties on each pass

    // Set flag for doing checks of ids
    ATH_CHECK(loadPropertyWithParse("DoIdChecks", m_doChecks));

    // Set flag for initializing neighbours
    ATH_CHECK(loadPropertyWithParse("DoInitNeighbours", m_doNeighbours));

    // Name of IdDict file
    ATH_CHECK(loadPropertyWithParse("IdDictName", m_idDictName));
    ATH_MSG_INFO("IdDictName:  " << m_idDictName);

    // Get the file names: two options - either from jobOpt
    // properties of the DetDescrCnvSvc or from RDB tags

    // Determine if Dict filename comes from DD database or
    // if properties from JobOptions should be used.
    ATH_CHECK(loadProperty("IdDictFromRDB", m_idDictFromRDB));

    // Determine if the dictionary content comes from DD database or
    // if properties from JobOptions should be used.
    ATH_CHECK(loadProperty("useGeomDB_InDet", m_useGeomDB_InDet));

    if (m_idDictFromRDB)
        ATH_MSG_DEBUG("Dictonary file name from DD database");
    else
        ATH_MSG_DEBUG(
            "Dictonary file name from job options or using defaults.");

    // Get the file name to parse:
    //
    //   1) From Relational DB
    //   2) Via jobOptions
    //   3) default name in the xml files
    //
    //  Currently the logic is 1) or 2) and 3) covers the case where
    //  no file name is found.
    //
    if (m_idDictFromRDB) {
        // Get file names from RDB
        ATH_CHECK(getFileNamesFromTags());
        ATH_MSG_DEBUG("Looked for ID file name from RDB ");
    } else {
        // Get file names from properties
        ATH_CHECK(getFileNamesFromProperties());
        ATH_MSG_DEBUG("Looked for ID file name from properties ");
    }

    // Only parse if necessary
    if (m_doParsing) {
        // Register the requested files with the xml parser
        ATH_CHECK(registerFilesWithParser());
        ATH_MSG_DEBUG("Registered file names ");

        // Check whether a tag is needed for dictionary initialization

        // NOTE: the internal tag for IdDict is global, but is only
        // used for InDet and thus is defined by InDet
        std::string tag{};
        if (m_inDetIDTag == "EMPTY")
            ATH_CHECK(loadProperty("IdDictGlobalTag", tag));
        else
            tag = m_inDetIDTag;

        // Parse the dictionaries
        m_parser->parse(m_idDictName, tag);
        if (tag.empty())
            tag = "default";
        ATH_MSG_DEBUG("Read dict:  " << m_idDictName << " with tag " << tag);

        // Set flag to check ids
        IdDictMgr &mgr = m_parser->m_idd;

        mgr.set_do_checks(m_doChecks);
        ATH_MSG_DEBUG("Set IdDictManager doChecks flag to"
                      << (m_doChecks ? "true" : "false"));

        // Set flag to initialize neighbours
        mgr.set_do_neighbours(m_doNeighbours);
        ATH_MSG_DEBUG("Set IdDictManager doNeighbours flag to "
                      << (m_doNeighbours ? "true" : "false"));

        // Do some checks
        const IdDictMgr::dictionary_map &dm = mgr.get_dictionary_map();
        if (dm.empty()) {
            ATH_MSG_ERROR("No dictionaries found!");
            return StatusCode::FAILURE;
        }
        ATH_MSG_DEBUG("Found " << dm.size() << " dictionaries.");

        // Register the requested files and tags with the id dicts
        ATH_CHECK(registerInfoWithDicts());
        ATH_MSG_DEBUG("Registered info with id dicts ");
    } else {
        ATH_MSG_WARNING(
            "NOTE:  ** parseXMLDescription called, but parsing was deemed "
            "unnecessary ** ");
    }
    ATH_MSG_DEBUG("parseXMLDescription: Finished parsing and setting options ");
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------

long int IdDictDetDescrCnv::storageType() {
    return DetDescr_StorageType;
}

//--------------------------------------------------------------------
const CLID &IdDictDetDescrCnv::classID() {
    return ClassID_traits<IdDictManager>::ID();
}

//--------------------------------------------------------------------
IdDictDetDescrCnv::IdDictDetDescrCnv(ISvcLocator *svcloc)
    : DetDescrConverter(ClassID_traits<IdDictManager>::ID(), svcloc),
      AthMessaging{"IdDictDetDescrCnv"} {}
//--------------------------------------------------------------------
void IdDictDetDescrCnv::printDicts(const IdDictManager *dictMgr) {
   
    ATH_MSG_INFO("Found id dicts:");
    if (!dictMgr)
        return;

    std::string tag = dictMgr->manager()->tag();
    ATH_MSG_INFO("Using dictionary tag: " << (tag.empty() ? "<no tag>" : tag));

    const IdDictMgr::dictionary_map &dm =
        dictMgr->manager()->get_dictionary_map();
    IdDictMgr::dictionary_map::const_iterator it;

    int n = 0;

    for (it = dm.begin(); it != dm.end(); ++it, ++n) {
        const IdDictDictionary &dictionary = *((*it).second);
        std::string version =
            ("" != dictionary.m_version) ? dictionary.m_version : "default";
        msg(MSG::INFO) << "Dictionary " << dictionary.m_name;
        if (dictionary.m_name.size() < 20) {
            std::string space(20 - dictionary.m_name.size(), ' ');
            msg(MSG::INFO) << space;
        }
        msg(MSG::INFO) << " version " << version;
        if (version.size() < 20) {
            std::string space(20 - version.size(), ' ');
            msg(MSG::INFO) << space;
        }
        if (dictionary.dict_tag().size()) {
            msg(MSG::INFO) << " DetDescr tag " << dictionary.dict_tag();
            if (dictionary.dict_tag().size() < 20) {
                std::string space(25 - dictionary.dict_tag().size(), ' ');
                msg(MSG::INFO) << space;
            }
        } else {
            msg(MSG::INFO) << " DetDescr tag (using default)";
        }
        ATH_MSG_INFO(" file " << dictionary.file_name());
    }
}

//--------------------------------------------------------------------
StatusCode IdDictDetDescrCnv::getFileNamesFromProperties() {
    // Check whether non-default names have been specified for the
    // IdDict files of the subsystems

    // Atlas IDs
    ATH_CHECK(loadProperty("AtlasIDFileName", m_atlasIDFileName));
    // InDet Ids
    ATH_CHECK(loadProperty("InDetIDFileName", m_inDetIDFileName));
    // LAr ids
    ATH_CHECK(loadProperty("LArIDFileName", m_larIDFileName));
    // Tile ids
    ATH_CHECK(loadProperty("TileIDFileName", m_tileIDFileName));
    // Calo ids
    ATH_CHECK(loadProperty("CaloIDFileName", m_caloIDFileName));
    // Calo neighbor files
    ATH_CHECK(loadProperty("FullAtlasNeighborsFileName", m_fullAtlasNeighborsName));
    ATH_CHECK(loadProperty("FCAL2DNeighborsFileName", m_fcal2dNeighborsName));
    ATH_CHECK(loadProperty("FCAL3DNeighborsNextFileName", m_fcal3dNeighborsNextName));
    ATH_CHECK(loadProperty("FCAL3DNeighborsPrevFileName", m_fcal3dNeighborsPrevName));

    ATH_CHECK(loadProperty("TileNeighborsFileName", m_tileNeighborsName));
    // Muon ids
    ATH_CHECK(loadProperty("MuonIDFileName", m_muonIDFileName));
    // ForwardDetectors ids
    ATH_CHECK(loadProperty("ForwardIDFileName", m_forwardIDFileName));
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------
StatusCode IdDictDetDescrCnv::getFileNamesFromTags() {
    // Fetch file names and tags from the RDB
    ATH_CHECK(m_geoDbTagSvc.retrieve());
    ATH_MSG_DEBUG("Accessed " << m_geoDbTagSvc->getParamSvcName());
    m_rdbAccessSvc.setName(m_geoDbTagSvc->getParamSvcName());
    ATH_CHECK(m_rdbAccessSvc.retrieve());

    auto assignTagAndName = [this](const IRDBRecordset_ptr &idDictSet,
                                   std::string &fileName,
                                   std::string &dictTag) {
        if (idDictSet->size()) {
            const IRDBRecord *idDictTable = (*idDictSet)[0];
            const std::string dictName = idDictTable->getString("DICT_NAME");
            fileName = idDictTable->getString("DICT_FILENAME");
            dictTag = idDictSet->tagName();
            // NOTE: the internal tag for IdDict is global, but is
            // only used for InDet and thus is defined by InDet
            if (!idDictTable->isFieldNull("DICT_TAG")) {
                m_inDetIDTag = idDictTable->getString("DICT_TAG");
            }
            ATH_MSG_DEBUG(" using dictionary:  "
                          << dictName << ", file: " << fileName
                          << ", with internal tag: " << m_inDetIDTag
                          << ", dictionary tag: " << dictTag);

        } else {
            ATH_MSG_WARNING(" no record set found for dictionary"<<idDictSet->nodeName()<< " - using default dictionary ");
        }
    };

    // Function for reading ID dictionary as a BLOB from SQLite
    // and writing it on the disk in the run directory
    auto getEmbeddedDict = [this](const std::string& dictName,
                                  std::string &fileName,
                                  std::string &dictTag) -> bool
    {
      ATH_MSG_DEBUG("Try to access "<<dictName);
      IRDBRecordset_ptr rec = m_rdbAccessSvc->getRecordsetPtr(dictName,"","");
      if(rec->size()>0) {
        const IRDBRecord *dictRecord = (*rec)[0];
        std::string dictString = dictRecord->getString("CONTENTS");
        //  write to the temporary file
        std::string dictFileName = dictName+"-fromSQLite.xml";
        std::ofstream dictFile;
        dictFile.open(dictFileName);
        dictFile << dictString;
        dictFile.close();

        fileName = dictFileName;
        dictTag.clear();   // This may change in the future if we also write dict tags into SQLite

        ATH_MSG_DEBUG(dictName << " read from the SQLite database as a BLOB");
        return true;
      }
      return false;
    };

    bool useGeomDB = (m_geoDbTagSvc->getSqliteReader() == nullptr);

    std::string detTag{""}, detNode{""}, dictName{""};
    DecodeVersionKey detectorKey("ATLAS");
    IRDBRecordset_ptr idDictSet{};

    bool embeddedDict{false};

    // Get InDet
    if (m_useGeomDB_InDet) {
        // Get Innner Detector xml and write to the temporary file
        // InDetIdDict.xml
        detectorKey = DecodeVersionKey(m_geoDbTagSvc.get(), "InnerDetector");
        ATH_MSG_DEBUG("From Version Tag: " << detectorKey.tag() << " at Node: "
                                           << detectorKey.node());
        detTag = detectorKey.tag();
        detNode = detectorKey.node();
        idDictSet = m_rdbAccessSvc->getRecordsetPtr("DICTXDD", detTag, detNode);

        // Size == 0 if not found
        if (idDictSet->size()) {
            const IRDBRecord *recordInDet = (*idDictSet)[0];
            std::string InDetString = recordInDet->getString("XMLCLOB");

            //  write to the temporary file
            std::ofstream blobFile;
            blobFile.open("InDetIdDict.xml");
            blobFile << InDetString << std::endl;
            blobFile.close();
        } else {
            ATH_MSG_WARNING(" no record set found for InDetIdentifier - using default dictionary ");
        }
    } else {
      // Attempt to read the embedded disctionary from SQLite
      embeddedDict = !useGeomDB && getEmbeddedDict("IdDictInnerDetector",m_inDetIDFileName,m_inDetIdDictTag);
      if(!embeddedDict) {
        // Fall back on getting file names from the database
        if (useGeomDB) {
            detectorKey = DecodeVersionKey(m_geoDbTagSvc.get(), "InnerDetector");
            ATH_MSG_DEBUG("From Version Tag: " << detectorKey.tag() << " at Node: " << detectorKey.node());
            detTag = detectorKey.tag();
            detNode = detectorKey.node();
        }
        idDictSet = m_rdbAccessSvc->getRecordsetPtr("InDetIdentifier", detTag, detNode);
        assignTagAndName(idDictSet, m_inDetIDFileName, m_inDetIdDictTag);
      }
    }

    // Get LAr
    // Attempt to read the embedded disctionary from SQLite
    embeddedDict = !useGeomDB && getEmbeddedDict("IdDictLArCalorimeter",m_larIDFileName,m_larIdDictTag);
    if(!embeddedDict) {
        // Fall back on getting file names from the database
        if (useGeomDB) {
            detectorKey = DecodeVersionKey(m_geoDbTagSvc.get(), "LAr");
            ATH_MSG_DEBUG( "From Version Tag: " << detectorKey.tag() << " at Node: " << detectorKey.node() );
            detTag = detectorKey.tag();
            detNode = detectorKey.node();
        }
        idDictSet = m_rdbAccessSvc->getRecordsetPtr("LArIdentifier", detTag, detNode);
        assignTagAndName(idDictSet, m_larIDFileName, m_larIdDictTag);
    }

    // Get Tile
    // Attempt to read the embedded disctionary from SQLite
    embeddedDict = !useGeomDB && getEmbeddedDict("IdDictTileCalorimeter",m_tileIDFileName,m_tileIdDictTag);
    if(!embeddedDict) {
        // Fall back on getting file names from the database
        if (useGeomDB) {
            detectorKey = DecodeVersionKey(m_geoDbTagSvc.get(), "TileCal");
            ATH_MSG_DEBUG( "From Version Tag: " << detectorKey.tag() << " at Node: " << detectorKey.node() );
            detTag = detectorKey.tag();
            detNode = detectorKey.node();
        }
        idDictSet = m_rdbAccessSvc->getRecordsetPtr("TileIdentifier", detTag, detNode);
        assignTagAndName(idDictSet, m_tileIDFileName, m_tileIdDictTag);
    }

    // Get Calo
    // Attempt to read the embedded disctionary from SQLite
    embeddedDict = !useGeomDB && getEmbeddedDict("IdDictCalorimeter",m_caloIDFileName,m_caloIdDictTag);
    if(!embeddedDict) {
      // Fall back on getting file names from the database
      if (useGeomDB) {
        detectorKey = DecodeVersionKey(m_geoDbTagSvc.get(), "Calorimeter");
        ATH_MSG_DEBUG( "From Version Tag: " << detectorKey.tag()<< " at Node: " << detectorKey.node() );
        detTag = detectorKey.tag();
        detNode = detectorKey.node();
      }
      idDictSet = m_rdbAccessSvc->getRecordsetPtr("CaloIdentifier", detTag, detNode);
      assignTagAndName(idDictSet, m_caloIDFileName, m_caloIdDictTag);
    }
    // Calo neighbor files:
    IRDBRecordset_ptr caloNeighborTable = m_rdbAccessSvc->getRecordsetPtr("CaloNeighborTable", detTag, detNode);

    if (caloNeighborTable->size() == 0 && useGeomDB) {
        caloNeighborTable = m_rdbAccessSvc->getRecordsetPtr("CaloNeighborTable", "CaloNeighborTable-00");
    }
    // Size == 0 if not found
    if (caloNeighborTable->size()) {
        const IRDBRecord *neighborTable = (*caloNeighborTable)[0];
        m_fullAtlasNeighborsName = neighborTable->getString("FULLATLASNEIGHBORS");
        m_fcal2dNeighborsName = neighborTable->getString("FCAL2DNEIGHBORS");
        m_fcal3dNeighborsNextName = neighborTable->getString("FCAL3DNEIGHBORSNEXT");
        m_fcal3dNeighborsPrevName = neighborTable->getString("FCAL3DNEIGHBORSPREV");
        m_tileNeighborsName = neighborTable->getString("TILENEIGHBORS");
        ATH_MSG_DEBUG(" using neighbor files:  ");
        ATH_MSG_DEBUG("   FullAtlasNeighborsFileName:  " << m_fullAtlasNeighborsName);
        ATH_MSG_DEBUG("   FCAL2DNeighborsFileName:     " << m_fcal2dNeighborsName);
        ATH_MSG_DEBUG("   FCAL3DNeighborsNextFileName: " << m_fcal3dNeighborsNextName);
        ATH_MSG_DEBUG("   FCAL3DNeighborsPrevFileName: " << m_fcal3dNeighborsPrevName);
        ATH_MSG_DEBUG("   TileNeighborsFileName:       " << m_tileNeighborsName);
    }

    // Get Muon
    // Attempt to read the embedded disctionary from SQLite
    embeddedDict = !useGeomDB && getEmbeddedDict("IdDictMuonSpectrometer", m_muonIDFileName, m_muonIdDictTag);
    if(!embeddedDict) {
      // Fall back on getting file names from the database
      if (useGeomDB) {
        detectorKey = DecodeVersionKey(m_geoDbTagSvc.get(), "MuonSpectrometer");
        ATH_MSG_DEBUG( "From Version Tag: " << detectorKey.tag()<< " at Node: " << detectorKey.node() );
        detTag = detectorKey.tag();
        detNode = detectorKey.node();
      }
      idDictSet = m_rdbAccessSvc->getRecordsetPtr("MuonIdentifier", detTag, detNode);
      assignTagAndName(idDictSet, m_muonIDFileName, m_muonIdDictTag);
    }

    // Get Forward
    // Attempt to read the embedded disctionary from SQLite
    embeddedDict = !useGeomDB && getEmbeddedDict("IdDictForwardDetectors",m_forwardIDFileName,m_forwardIdDictTag);
    if(!embeddedDict) {
      // Fall back on getting file names from the database
      if (useGeomDB) {
        detectorKey = DecodeVersionKey(m_geoDbTagSvc.get(), "ForwardDetectors");
        ATH_MSG_DEBUG( "From Version Tag: " << detectorKey.tag() << " at Node: " << detectorKey.node() );
        detTag = detectorKey.tag();
        detNode = detectorKey.node();
      }
      idDictSet = m_rdbAccessSvc->getRecordsetPtr("ForDetIdentifier", detTag, detNode);

      // For older datasets use ForDetIdentifier-00 as fallback
      if (idDictSet->size() == 0 && useGeomDB) {
        idDictSet = m_rdbAccessSvc->getRecordsetPtr("ForDetIdentifier",
                                                  "ForDetIdentifier-00");
        ATH_MSG_DEBUG(" explicitly requesting ForDetIdentifier-00 tag for pre-forward "
                "detector data  ");
      }
      // Size == 0 if not found
      assignTagAndName(idDictSet, m_forwardIDFileName, m_forwardIdDictTag);
    }
    ATH_MSG_DEBUG("End access to RDB for id dictionary info ");
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------
StatusCode IdDictDetDescrCnv::registerFilesWithParser() {
    // If InDetIdDict.xml exists set InDetFileName set to it's name
    if (m_useGeomDB_InDet) {
        std::ifstream ifile;
        ifile.open("InDetIdDict.xml");
        if (ifile)
            m_inDetIDFileName = "InDetIdDict.xml";
        else
            ATH_MSG_WARNING(" no temp. file InDetIdDict.xml found - using file "
                            << m_inDetIDFileName);
    }

    if (!m_atlasIDFileName.empty()) {
        m_parser->register_external_entity("ATLAS", m_atlasIDFileName);
        ATH_MSG_INFO("Reading ATLAS            IdDict file "
                     << m_atlasIDFileName);
    }
    if (!m_inDetIDFileName.empty()) {
        m_parser->register_external_entity("InnerDetector", m_inDetIDFileName);
        ATH_MSG_INFO("Reading InnerDetector    IdDict file "
                     << m_inDetIDFileName);
    }
    if (!m_larIDFileName.empty()) {
        m_parser->register_external_entity("LArCalorimeter", m_larIDFileName);
        ATH_MSG_INFO("Reading LArCalorimeter   IdDict file "
                     << m_larIDFileName);
    }
    if (!m_tileIDFileName.empty()) {
        m_parser->register_external_entity("TileCalorimeter", m_tileIDFileName);
        ATH_MSG_INFO("Reading TileCalorimeter  IdDict file "
                     << m_tileIDFileName);
    }
    if (!m_caloIDFileName.empty()) {
        m_parser->register_external_entity("Calorimeter", m_caloIDFileName);
        ATH_MSG_INFO("Reading Calorimeter      IdDict file "
                     << m_caloIDFileName);
    }
    if (!m_muonIDFileName.empty()) {
        m_parser->register_external_entity("MuonSpectrometer",
                                           m_muonIDFileName);
        ATH_MSG_INFO("Reading MuonSpectrometer IdDict file "
                     << m_muonIDFileName);
    }
    if (!m_forwardIDFileName.empty()) {
        m_parser->register_external_entity("ForwardDetectors",
                                           m_forwardIDFileName);
        ATH_MSG_INFO("Reading ForwardDetectors IdDict file "
                     << m_forwardIDFileName);
    }
    return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------
StatusCode IdDictDetDescrCnv::registerInfoWithDicts() {
    // Save the file name and tag in each of the dictionaries
    IdDictMgr &mgr = m_parser->m_idd;

    auto setDictPaths = [this, &mgr](const std::string &dict_name,
                                     const std::string &file_name,
                                     const std::string &dict_tag) {
        if (file_name.empty()) {
            ATH_MSG_DEBUG("No idDict will be loaded for " << dict_name);
            return StatusCode::SUCCESS;
        }
        IdDictDictionary *dict = mgr.find_dictionary(dict_name);
        if (!dict) {
            ATH_MSG_ERROR("unable to find idDict for " << dict_name);
            return StatusCode::FAILURE;
        }
        dict->set_file_name(file_name);
        dict->set_dict_tag(dict_tag);
        ATH_MSG_DEBUG("For " << dict_name << " idDict, setting file/tag: "
                             << file_name << " " << dict_tag);
        return StatusCode::SUCCESS;
    };
    ATH_CHECK(setDictPaths("ATLAS", m_atlasIDFileName, m_atlasIdDictTag));
    ATH_CHECK(setDictPaths("InnerDetector", m_inDetIDFileName, m_inDetIdDictTag));
    ATH_CHECK(setDictPaths("LArCalorimeter", m_larIDFileName, m_larIdDictTag));
    ATH_CHECK(setDictPaths("TileCalorimeter", m_tileIDFileName, m_tileIdDictTag));
    ATH_CHECK(setDictPaths("Calorimeter", m_caloIDFileName, m_caloIdDictTag));
    ATH_CHECK(setDictPaths("MuonSpectrometer", m_muonIDFileName, m_muonIdDictTag));
    ATH_CHECK(setDictPaths("ForwardDetectors", m_forwardIDFileName, m_forwardIdDictTag));

    auto addMetaData = [&mgr, this](const std::string &key,
                                    const std::string &value) {
        if (value.empty()) {
            ATH_MSG_DEBUG("No value given for key " << key);
        } else {
            mgr.add_metadata(key, value);
            ATH_MSG_DEBUG("Added to dict mgr meta data: <" << key << ","
                                                           << value << ">");
        }
    };
    addMetaData("FULLATLASNEIGHBORS", m_fullAtlasNeighborsName);
    addMetaData("FCAL2DNEIGHBORS", m_fcal2dNeighborsName);
    addMetaData("FCAL3DNEIGHBORSNEXT", m_fcal3dNeighborsNextName);
    addMetaData("FCAL3DNEIGHBORSPREV", m_fcal3dNeighborsPrevName);
    addMetaData("TILENEIGHBORS", m_tileNeighborsName);

    return StatusCode::SUCCESS;
}

template <class dType>
StatusCode IdDictDetDescrCnv::loadProperty(const std::string &propertyName,
                                           dType &pipeTo) {
    if (!m_detDescrProxy)
        ATH_CHECK(serviceLocator()->service("DetDescrCnvSvc", m_detDescrProxy));
    // cppcheck-suppress nullPointerRedundantCheck; false positive
    if (!m_detDescrProxy->hasProperty(propertyName)) {
        ATH_MSG_FATAL("DetDescrSvc does not have the property "
                      << propertyName);
        return StatusCode::FAILURE;
    }
    const Gaudi::Details::PropertyBase &prop =
        m_detDescrProxy->getProperty(propertyName);
    const Gaudi::Property<dType> *propPtr{
        dynamic_cast<const Gaudi::Property<dType> *>(&prop)};
    if (!propPtr) {
        ATH_MSG_ERROR("Property " << propertyName << " is not of type"
                                  << typeid(dType).name() << " but of "
                                  << typeid(prop).name());
        return StatusCode::FAILURE;
    }
    pipeTo = propPtr->value();
    ATH_MSG_DEBUG("Flag " << propertyName << " is: " << pipeTo);
    return StatusCode::SUCCESS;
}

/// Same as loadProperty but additionally m_doParsing is set to true if the
/// input value does not match the set property value
template <class dType>
StatusCode IdDictDetDescrCnv::loadPropertyWithParse(
    const std::string &propertyName, dType &pipeTo) {
    dType cache{};
    ATH_CHECK(loadProperty(propertyName, cache));
    m_doParsing |= cache != pipeTo;
    pipeTo = std::move(cache);
    return StatusCode::SUCCESS;
}
