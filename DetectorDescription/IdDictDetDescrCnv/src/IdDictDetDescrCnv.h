/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICTDETDESCRCNV_IDDICTDETDESCRCNV_H
#define IDDICTDETDESCRCNV_IDDICTDETDESCRCNV_H
/**
 * @file IdDictDetDescrCnv.h
 *
 * @brief Converter for the DetDescrCnvSvc which parses the identifier
 * xml dictionaries and creates an IdDictManager in the
 * DetectorStore. This is used by the IdHelpers to initialize
 * themselves.
 *
 * @author RD Schaffer <R.D.Schaffer@cern.ch>
 *
 */

//<<<<<< INCLUDES                                                       >>>>>>

#include "DetDescrCnvSvc/DetDescrConverter.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "IdDictParser/IdDictParser.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"
//<<<<<< PUBLIC TYPES                                                   >>>>>>

class IdDictManager;

//<<<<<< CLASS DECLARATIONS                                             >>>>>>

/**
 * @class IdDictDetDescrCnv
 *
 * @brief Converter for the DetDescrCnvSvc which parses the identifier
 * xml dictionaries and creates an IdDictManager in the
 * DetectorStore. This is used by the IdHelpers to initialize
 * themselves.
 *
 */

class IdDictDetDescrCnv : public DetDescrConverter {
   public:
    virtual long int repSvcType() const override;
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode createObj(IOpaqueAddress *pAddr,
                                 DataObject *&pObj) override;

    /// Storage type and class ID (used by CnvFactory)
    static long int storageType();
    static const CLID &classID();

    IdDictDetDescrCnv(ISvcLocator *svcloc);

   private:
    
    ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{"GeoDbTagSvc/GeoDbTagSvc", "IdDictDetDescrCnv"};
    ServiceHandle<IRDBAccessSvc> m_rdbAccessSvc{"RDBAccessSvc/RDBAccessSvc", "IdDictDetDescrCnv"};

    /// Propxy to the DetDescrCnvSvc
    SmartIF<IProperty> m_detDescrProxy;

    /// Loads the property from the DetDecrCnvSvc and pipes its value
    /// Returns failure if either the service, the property don't exist or
    /// the data type is wrong
    template <class dType>
    StatusCode loadProperty(const std::string &propertyName, dType &pipeTo);

    /// Same as loadProperty but additionally m_doParsing is set to true if the
    /// input value does not match the set property value
    template <class dType>
    StatusCode loadPropertyWithParse(const std::string &propertyName,
                                     dType &pipeTo);

    /// Get file names from properties
    StatusCode getFileNamesFromProperties();

    /// Get file names from properties
    StatusCode getFileNamesFromTags();

    /// Register the requested files with the xml parser
    StatusCode registerFilesWithParser();

    /// Register the requested files and tag with the created id dicts
    StatusCode registerInfoWithDicts();

    /// Print out the contained dictionaries and version
    void printDicts(const IdDictManager *dictMgr);

    /// Create and (re)initialize the IdDictManager - only create the
    /// first time
    StatusCode parseXMLDescription();

    /// A helper function for collecting calo neighbor file names from the database
    void collectCaloNeighbors(IRDBRecordset_ptr recordset);

    /// The xml parser for the dictionary descriptions
    std::unique_ptr<IdDictParser> m_parser{};

    /// Flag to tell helpers to do Checks
    bool m_doChecks{false};

    /// Flag to generate neighbor information - for calos
    bool m_doNeighbours{true};

    /// Name of top-level xml dict file
    std::string m_idDictName{};

    /// Flag to get dict parameters from Relational DetDescr DB
    bool m_idDictFromRDB{false};

    /// Flag to which determines whether the xml files are parsed or
    /// not
    bool m_doParsing{true};

    /// File to be read for top-level subsystem ids values
    std::string m_atlasIDFileName{};

    /// File to be read for InDet ids
    std::string m_inDetIDFileName{};

    /// File to be read for LAr ids
    std::string m_larIDFileName{};

    /// File to be read for Tile ids
    std::string m_tileIDFileName{};

    /// File to be read for Calo ids
    std::string m_caloIDFileName{};

    /// Files for Calo Neighbors
    std::string m_fullAtlasNeighborsName{};
    std::string m_fcal2dNeighborsName{};
    std::string m_fcal3dNeighborsNextName{};
    std::string m_fcal3dNeighborsPrevName{};
    std::string m_tileNeighborsName{};

    /// File to be read for Muon ids
    std::string m_muonIDFileName{};

    /// File to be read for Forward det ids
    std::string m_forwardIDFileName{};

    /// Tag of RDB record for Atlas top-level ids
    std::string m_atlasIdDictTag{};

    /// Tag of RDB record for InDet ids
    std::string m_inDetIdDictTag{};

    /// Tag of RDB record for LAr ids
    std::string m_larIdDictTag{};

    /// Tag of RDB record for Tile ids
    std::string m_tileIdDictTag{};

    /// Tag of RDB record for Calo ids
    std::string m_caloIdDictTag{};

    /// Tag of RDB record for Muon ids
    std::string m_muonIdDictTag{};

    /// Tag of RDB record for forwards det ids
    std::string m_forwardIdDictTag{};

    /// Internal InDet id tag
    std::string m_inDetIDTag{};

    // Flag to read InDet geometry from the Geom DB
    bool m_useGeomDB_InDet{false};
};

#endif  // IDDICTDETDESCRCNV_IDDICTDETDESCRCNV_H
