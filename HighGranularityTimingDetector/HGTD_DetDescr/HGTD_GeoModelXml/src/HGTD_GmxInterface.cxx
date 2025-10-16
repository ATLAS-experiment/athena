/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "HGTD_GmxInterface.h"
#include <HGTD_ReadoutGeometry/HGTD_DetectorManager.h>
#include <HGTD_Identifier/HGTD_ID.h>
#include <HGTD_ReadoutGeometry/HGTD_DetectorElement.h>
#include <HGTD_ReadoutGeometry/HGTD_ModuleDesign.h>

#include <InDetSimEvent/SiHitIdHelper.h>
#include <ReadoutGeometryBase/SiCommonItems.h>

#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecord.h>
#include <RDBAccessSvc/IRDBRecordset.h>
#include <GeoModelRead/ReadGeoModel.h>
#include <GeoModelKernel/GeoFullPhysVol.h>

namespace
{
constexpr int HGTD_HitIndex{2};
}

HGTD_GmxInterface::HGTD_GmxInterface(HGTD_DetectorManager *detectorManager,
                                     InDetDD::SiCommonItems *commonItems)
    : AthMessaging("HGTD_GmxInterface"),
      m_detectorManager(detectorManager),
      m_commonItems(commonItems)
{
}


int HGTD_GmxInterface::sensorId(std::map<std::string, int> &index) const
{
    const HGTD_ID* hgtdIdHelper = dynamic_cast<const HGTD_ID *> (m_commonItems->getIdHelper());
    bool newIdenSche = hgtdIdHelper->get_useNewIdentifierScheme(); // to choise which identification scheme will be used
    
    // Return the Simulation HitID (nothing to do with "ATLAS Identifiers" aka "Offline Identifiers"
    int hitIdOfWafer;

    if(newIdenSche){
        hitIdOfWafer = SiHitIdHelper::GetHelper()->buildHitId(HGTD_HitIndex,
                                                                  index["endcap"],
                                                                  index["layer"],
                                                                  0,
                                                                  index["moduleInLayer"],
                                                                  0); // side is just 0 for HGTD

    ATH_MSG_DEBUG("Index list: " << index["endcap"] << " " << index["layer"] << " "
                                 << index["moduleInLayer"] );
    ATH_MSG_DEBUG("hitIdOfWafer = " << std::hex << hitIdOfWafer << std::dec);
    ATH_MSG_DEBUG(" endcap = " << SiHitIdHelper::GetHelper()->getBarrelEndcap(hitIdOfWafer)
                  << " layer = " << SiHitIdHelper::GetHelper()->getLayerDisk(hitIdOfWafer)
                  << " moduleInLayer = " << SiHitIdHelper::GetHelper()->getPhiModule(hitIdOfWafer));
    } else {
        hitIdOfWafer = SiHitIdHelper::GetHelper()->buildHitId(HGTD_HitIndex,
                                                                  index["endcap"],
                                                                  index["layer"],
                                                                  index["moduleNumberInRow"],
                                                                  index["rowNumber"],
                                                                  0); // side is just 0 for HGTD

    ATH_MSG_DEBUG("Index list: " << index["endcap"] << " " << index["layer"] << " "
                                 << index["moduleNumberInRow"] << " " << index["rowNumber"]);
    ATH_MSG_DEBUG("hitIdOfWafer = " << std::hex << hitIdOfWafer << std::dec);
    ATH_MSG_DEBUG(" endcap = " << SiHitIdHelper::GetHelper()->getBarrelEndcap(hitIdOfWafer)
                  << " layer = " << SiHitIdHelper::GetHelper()->getLayerDisk(hitIdOfWafer)
                  << " moduleNumberInRow = " << SiHitIdHelper::GetHelper()->getEtaModule(hitIdOfWafer)
                  << " rowNumber = " << SiHitIdHelper::GetHelper()->getPhiModule(hitIdOfWafer));

    }
    return hitIdOfWafer;

}


void HGTD_GmxInterface::addSensorType(const std::string &clas,
                                      const std::string &typeName,
                                      const std::map<std::string, std::string> &parameters)
{
    ATH_MSG_DEBUG("addSensorType called for class " << clas << ", typeName " << typeName);

    if (clas == "LGAD_module") {
        // TODO: implement method to actually add the sensor type (also to the detector manager)
        makeLgadModule(typeName, parameters);
    } else {
        ATH_MSG_ERROR("addSensorType: unrecognised sensor class: " << clas);
        ATH_MSG_ERROR("No sensor design created");
    }
}


void HGTD_GmxInterface::makeLgadModule(const std::string &typeName,
                                       const std::map<std::string, std::string> &parameters)
{
    double thickness{};
    double xPitch{};
    double yPitch{};
    int circuitsPerColumn{};
    int circuitsPerRow{};
    int padColumns{};
    int padRows{};

    // read parameters
    // TO DO : checking for unlogical values
    getParameter(typeName, parameters, "thickness", thickness);
    getParameter(typeName, parameters, "xPitch", xPitch);
    getParameter(typeName, parameters, "yPitch", yPitch);
    getParameter(typeName, parameters, "circuitsPerColumn", circuitsPerColumn);
    getParameter(typeName, parameters, "circuitsPerRow", circuitsPerRow);
    getParameter(typeName, parameters, "padColumns", padColumns);
    getParameter(typeName, parameters, "padRows", padRows);

    std::shared_ptr<const InDetDD::PixelDiodeMatrix> normalCell = InDetDD::PixelDiodeMatrix::construct(xPitch, yPitch);
    std::shared_ptr<const InDetDD::PixelDiodeMatrix> singleRow  = InDetDD::PixelDiodeMatrix::construct(InDetDD::PixelDiodeMatrix::phiDir, 0,
                                                                          std::move(normalCell), padColumns, 0);
    std::shared_ptr<const InDetDD::PixelDiodeMatrix> fullMatrix = InDetDD::PixelDiodeMatrix::construct(InDetDD::PixelDiodeMatrix::etaDir, 0,
                                                                          std::move(singleRow), padRows, 0);


    InDetDD::DetectorDesign::Axis yDirection = InDetDD::DetectorDesign::yAxis;

    InDetDD::HGTD_ModuleDesign* design = new InDetDD::HGTD_ModuleDesign(thickness,
                                                                        circuitsPerColumn, circuitsPerRow,
                                                                        padColumns, padRows/2,
                                                                        padColumns, padRows/2,
                                                                        std::move(fullMatrix),
                                                                        InDetDD::CarrierType::electrons, 1, yDirection );


   m_geometryMap[typeName] = design;
}


void HGTD_GmxInterface::addSensor(const std::string &typeName,
                                  std::map<std::string, int> &index,
                                  int /* sensitiveId */,
                                  GeoVFullPhysVol *fpv)
{
    //
    // Get the ATLAS "Offline" wafer identifier
    //
    const HGTD_ID* hgtdIdHelper = static_cast<const HGTD_ID *> (m_commonItems->getIdHelper());

    Identifier id;

    bool useNewIdentifierScheme = hgtdIdHelper->get_useNewIdentifierScheme(); // to find which identification scheme is configured
    if(useNewIdentifierScheme){
        id = hgtdIdHelper->wafer_id(index["endcap"],
                                    index["layer"],
                                    index["moduleInLayer"],
                                    0);
        ATH_MSG_DEBUG("HGTD New ID scheme");
    } else {
        id = hgtdIdHelper->wafer_id(index["endcap"],
                                    index["layer"],
                                    index["rowNumber"],
                                    index["moduleNumberInRow"]);
        ATH_MSG_DEBUG("HGTD Old ID scheme");
    }               
    IdentifierHash hashId = hgtdIdHelper->wafer_hash(id);

    //
    // Now do our best to check if this is a valid id. If either the gmx file is wrong, or the xml file
    // defining the allowed id's is wrong, you can get disallowed id's. These cause a crash later
    // if allowed through. To do the check, we ask for the hash-id of this id. Invalid ids give a
    // special invalid hash-id (0xFFFFFFFF). But we don't exit the run, to help debug things quicker.
    // //
    if (hashId.is_valid()) {
        ATH_MSG_DEBUG("valid id");
        for (const auto& [key, value] : index) {
            ATH_MSG_DEBUG(key << " = " << value << "; ");
        }
    } else {
        ATH_MSG_ERROR("Invalid id for sensitive module " << typeName << " volume with indices");
        for (const auto& [key, value] : index) {
            ATH_MSG_ERROR(key << " = " << value << "; ");
        }
        ATH_MSG_ERROR("Refusing to make it into a sensitive element. Incompatible gmx and identifier-xml files.");
        return;
    }

    //
    // Create the detector element and add to the DetectorManager
    //
    const InDetDD::HGTD_ModuleDesign* design = m_geometryMap[typeName];
    if (design == nullptr) {
        ATH_MSG_ERROR("addSensor: Error: Readout sensor type " << typeName << " not found.");
        throw std::runtime_error("readout sensor type " + typeName + " not found.");
    }
    m_detectorManager->addDetectorElement(new InDetDD::HGTD_DetectorElement(id, design, fpv, m_commonItems));

    return;
}

void HGTD_GmxInterface::buildReadoutGeometryFromSqlite(IRDBAccessSvc * rdbAccessSvc,GeoModelIO::ReadGeoModel* sqlreader)
{
    IRDBRecordset_ptr LGAD_module = rdbAccessSvc->getRecordsetPtr("LGAD_module","");
    const std::array<std::string,7> LGAD_moduleParamNames({"thickness","xPitch","yPitch","circuitsPerColumn","circuitsPerRow","padColumns","padRows"});

    if(LGAD_module->size() !=0){
       for (const IRDBRecord_ptr& typeParams:*LGAD_module){
            std::map<std::string,std::string> LGAD_moduleMap;
            for(const std::string& paramName:LGAD_moduleParamNames){
                LGAD_moduleMap[paramName] = typeParams->getString(paramName);
            }
            std::string LGAD_moduleName = typeParams->getString("SensorType");
            makeLgadModule(LGAD_moduleName,LGAD_moduleMap);
        }
    }
    else ATH_MSG_WARNING("Could not retrieve LGAD_module table");

    //Now, loop over the FullPhysVols and create the HGTD_DetectorElements (including splitting where needed)
    //lots of string parsing...
    std::vector<std::string> fields({"endcap","layer","moduleInLayer"});
    //The map below is a map of string keys which contains all the Identifier/DetElement relevant info, and the associated FullPhysVol
    std::map<std::string, GeoFullPhysVol*> mapFPV = sqlreader->getPublishedNodes<std::string, GeoFullPhysVol*>("GeoModelXML");
    for (const auto&[fullPhysVolInfoString, fullPhysVolPointer] : mapFPV){
        //find the name of the corresponding detector design type
        size_t startLGAD = fullPhysVolInfoString.find("lgad");
        if(startLGAD==std::string::npos){
            ATH_MSG_DEBUG("GeoFullPhysVol "<<fullPhysVolInfoString<<" does not have the expected format. Skipping");
            continue;
        }
        std::string typeName = fullPhysVolInfoString.substr(startLGAD);
        std::map<std::string, int> index;
        for (const std::string& field:fields){
            size_t first = fullPhysVolInfoString.find(field+"_");
            size_t last = fullPhysVolInfoString.find('_',first+field.size()+1);//start looking only after end of first delimiter (plus 1 for the "_" appended) ends
            if(first==std::string::npos || last==std::string::npos){
                ATH_MSG_DEBUG("Could not extract "<<field<<" from "<<fullPhysVolInfoString<<". Skipping");
                continue;
            }
            std::string strNew = fullPhysVolInfoString.substr(first+field.size()+1,last-(first+field.size()+1));
            index[field] = std::stoi(strNew);
        }

        addSensor(typeName,index,0,fullPhysVolPointer);
    }
}
