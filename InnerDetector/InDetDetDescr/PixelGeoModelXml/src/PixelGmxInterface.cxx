/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "PixelGeoModelXml/PixelGmxInterface.h"

#include <InDetGeoModelUtils/WaferTree.h>
#include <InDetReadoutGeometry/SiDetectorDesign.h>
#include <InDetReadoutGeometry/SiDetectorElement.h>
#include <InDetSimEvent/SiHitIdHelper.h>
#include <PixelReadoutGeometry/PixelDetectorManager.h>
#include <PixelReadoutGeometry/PixelModuleDesign.h>
#include <ReadoutGeometryBase/PixelDiodeMatrix.h>
#include "ReadoutGeometryBase/PixelDiodeTree.h"
#include "ReadoutGeometryBase/PixelDiodeTreeBuilder.h"
#include <ReadoutGeometryBase/SiCommonItems.h>

#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecord.h>
#include <RDBAccessSvc/IRDBRecordset.h>
#include <GeoModelRead/ReadGeoModel.h>
#include <GeoModelKernel/GeoFullPhysVol.h>

namespace
{
constexpr int PixelHitIndex{0};

InDetDD::PixelReadoutTechnology getPixelReadoutTechnology(InDetDD::DetectorType detectorType, int rowsPerCircuit, int columnsPerCircuit) {
   if (detectorType == InDetDD::DetectorType::PixelBarrel
       || detectorType == InDetDD::DetectorType::PixelEndcap
       || detectorType == InDetDD::DetectorType::PixelInclined
       || detectorType == InDetDD::DetectorType::PLR) {
      // if ITk
      return InDetDD::PixelReadoutTechnology::RD53;
   }
   else {
      // if not ITk
      if (rowsPerCircuit*columnsPerCircuit>26000) { return InDetDD::PixelReadoutTechnology::FEI4; }
      else                                        { return InDetDD::PixelReadoutTechnology::FEI3; }
   }
}
}


namespace InDetDD
{

namespace ITk
{

PixelGmxInterface::PixelGmxInterface(PixelDetectorManager *detectorManager,
                                     SiCommonItems *commonItems,
                                     WaferTree *moduleTree)
  : AthMessaging("PixelGmxInterface"),
    m_detectorManager(detectorManager),
    m_commonItems(commonItems),
    m_moduleTree(moduleTree)
{}


int PixelGmxInterface::sensorId(std::map<std::string, int> &index) const
{
  // Return the Simulation HitID (nothing to do with "ATLAS Identifiers" aka "Offline Identifiers")

  // Check if identifier is valid
  // TODO: drop this check in the future
  const PixelID *pixelIdHelper = dynamic_cast<const PixelID *>(m_commonItems->getIdHelper());
  if (not pixelIdHelper){
    ATH_MSG_ERROR("Failed dynamic_cast to PixelID in PixelGmxInterface::sensorId");
    return -1;
  }
  Identifier id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                          index["layer_wheel"],
                                          index["phi_module"],
                                          index["eta_module"]);
  IdentifierHash hashId = pixelIdHelper->wafer_hash(id);
  if (!hashId.is_valid()) {
    ATH_MSG_WARNING("Invalid hash for Index list: " << index["barrel_endcap"] << " " << index["layer_wheel"] << " "
                    << index["eta_module"] << " " << index["phi_module"] << " " << index["side"]);
    return -1;
  }

  // Compute the actuall SiHitId
  int hitIdOfModule = SiHitIdHelper::GetHelper()->buildHitId(PixelHitIndex,
                                                             index["barrel_endcap"],
                                                             index["layer_wheel"],
                                                             index["eta_module"],
                                                             index["phi_module"],
                                                             index["side"]);

  ATH_MSG_DEBUG("Index list: " << index["barrel_endcap"] << " " << index["layer_wheel"] << " "
                               << index["eta_module"] << " " << index["phi_module"] << " " << index["side"]);
  ATH_MSG_DEBUG("hitIdOfModule = " << std::hex << hitIdOfModule << std::dec);
  ATH_MSG_DEBUG(" bec = " << SiHitIdHelper::GetHelper()->getBarrelEndcap(hitIdOfModule)
                << " lay = " << SiHitIdHelper::GetHelper()->getLayerDisk(hitIdOfModule)
                << " eta = " << SiHitIdHelper::GetHelper()->getEtaModule(hitIdOfModule)
                << " phi = " << SiHitIdHelper::GetHelper()->getPhiModule(hitIdOfModule)
                << " side = " << SiHitIdHelper::GetHelper()->getSide(hitIdOfModule));

  return hitIdOfModule;
}


void PixelGmxInterface::addSensorType(const std::string& clas,
                                      const std::string& typeName,
                                      const std::map<std::string, std::string>& parameters)
{
  ATH_MSG_DEBUG("addSensorType called for class " << clas << ", typeName " << typeName);

  if (clas == "SingleChip_RD53" || clas == "QuadChip_RD53") {
    makePixelModule(typeName, parameters);
    // @TODO remove once all endcap modules are oriented consistently i.e. there is
    // one relation between local "hardware" coordinates and local offline coordinates.
    // Currently all endcap modules have a surface normal (defined by the transform),
    // which points outwards. This is the case for endcap modules for which the sensor
    // facing side points towards the IP (even phi index), and those for which the sensor
    // facing side points outwards (odd phi index). By introducing separate module design
    // objects for endcap modules with even or odd phi index, the module design can
    // provide extra information to indicate the translation scheme between hardware
    // coordinates and offline coordinates.
    if (   typeName.find("Quad")!= std::string::npos
        && (   typeName.find("endcap")!= std::string::npos
            || typeName.find("inclined")!= std::string::npos)) {
       makePixelModule(typeName+"_even",parameters);
    }
  } else {
    ATH_MSG_ERROR("addSensorType: unrecognised module class: " << clas);
    ATH_MSG_ERROR("No module design created");
  }
}


void PixelGmxInterface::makePixelModule(const std::string &typeName,
                                        const std::map<std::string, std::string> &parameters)
{
  int circuitsPerEta{2}; // row
  int circuitsPerPhi{2}; // column
  double thickness{0.150};
  double pitchEta{};
  double pitchPhi{};
  double pitchEtaLong{};
  double pitchPhiLong{};
  double pitchEtaEnd{};
  double pitchPhiEnd{};
  int nEtaLongPerSide{};
  int nPhiLongPerSide{};
  int nEtaEndPerSide{};
  int nPhiEndPerSide{};
  int rowsPerCircuit{};
  int columnsPerCircuit{};

  // unused
  InDetDD::CarrierType carrier{InDetDD::electrons};
  int readoutSide{1};
  bool is3D{false};

  bool phiSymmetric{true};
  bool etaSymmetric{true};
  bool depthSymmetric{true};

  // read parameters
  getParameter(typeName, parameters, "circuitsPerEta", circuitsPerEta);
  getParameter(typeName, parameters, "circuitsPerPhi", circuitsPerPhi);
  getParameter(typeName, parameters, "thickness", thickness);
  getParameter(typeName, parameters, "is3D", is3D);
  getParameter(typeName, parameters, "rows", rowsPerCircuit);
  getParameter(typeName, parameters, "columns", columnsPerCircuit);
  getParameter(typeName, parameters, "pitchEta", pitchEta);
  getParameter(typeName, parameters, "pitchPhi", pitchPhi);
  getParameter(typeName, parameters, "pitchEtaLong", pitchEtaLong);
  getParameter(typeName, parameters, "pitchPhiLong", pitchPhiLong);
  getParameter(typeName, parameters, "pitchEtaEnd", pitchEtaEnd);
  getParameter(typeName, parameters, "pitchPhiEnd", pitchPhiEnd);
  getParameter(typeName, parameters, "nPhiLongPerSide", nPhiLongPerSide);
  getParameter(typeName, parameters, "nEtaLongPerSide", nEtaLongPerSide);
  getParameter(typeName, parameters, "nPhiEndPerSide", nPhiEndPerSide);
  getParameter(typeName, parameters, "nEtaEndPerSide", nEtaEndPerSide);

  checkParameter(typeName, parameters, "phiSymmetric", phiSymmetric);
  checkParameter(typeName, parameters, "etaSymmetric", etaSymmetric);
  checkParameter(typeName, parameters, "depthSymmetric", depthSymmetric);

  //
  // Make Module Design and add to DetectorManager
  //
  ATH_MSG_DEBUG("readout geo - design thickness " << thickness << " "
                << " circuits " << circuitsPerPhi << " " << circuitsPerEta << " "
                << " rows/columns " << rowsPerCircuit << " " << columnsPerCircuit << " "
                << " pitch regular/long/end " << pitchPhi << " " << pitchEta
                << " " << pitchPhiLong << " " << pitchEtaLong
                << " " << pitchPhiEnd << " "  << pitchEtaEnd
                << " n-long " << nPhiLongPerSide << " " << nEtaLongPerSide
                << " n-end "  << nPhiEndPerSide  << " " << nEtaEndPerSide
                << carrier << " " << readoutSide);

  //For optionally setting PixelBarrel,PixelEndcap,PixelInclined
  //(so far) primarily useful for the latter to avoid orientation warnings
  InDetDD::DetectorType detectorType{InDetDD::PixelBarrel};  // TODO: we should probably fail and not default to barrel here.
  int detectorTypeEnum = 0;
  if (checkParameter(typeName, parameters, "detectorType", detectorTypeEnum)) {
    if (detectorTypeEnum == 1) detectorType = InDetDD::PixelBarrel;
    else if (detectorTypeEnum == 2) detectorType = InDetDD::PixelEndcap;
    else if (detectorTypeEnum == 3) detectorType = InDetDD::PixelInclined;
  }

  InDetDD::PixelReadoutTechnology readoutTechnology = getPixelReadoutTechnology(detectorType, rowsPerCircuit, columnsPerCircuit );

  if (   circuitsPerPhi*rowsPerCircuit<0    || circuitsPerPhi*rowsPerCircuit    >= std::numeric_limits<PixelDiodeTree::CellIndexType>::max()
      || circuitsPerEta*columnsPerCircuit<0 || circuitsPerEta*columnsPerCircuit >= std::numeric_limits<PixelDiodeTree::CellIndexType>::max()) {
     std::stringstream amsg;
     amsg << "Index overflows index type of PixelDiodeTree. Parameters "
          << "( " <<  circuitsPerPhi << " * " << rowsPerCircuit << " ), ( "
          << "( " <<  circuitsPerEta << " * " << columnsPerCircuit << " ) !<"
          << std::numeric_limits<PixelDiodeTree::CellIndexType>::max() << " each.";
     throw std::runtime_error(amsg.str());
  }

  // @TODO remove once all endcap modules are oriented consistently i.e. there is
  // one relation between local "hardware" coordinates and local offline coordinates
  bool flipFE=(typeName.find("_even") !=std::string::npos);

  // helper function to associate correct  diode type and front-end number to sub-matrices and diodes
  // in the diode tree as attributes.
  auto computeAttribute = [readoutTechnology,
                           pitchPhi,
                           pitchEta,
                           circuitsPerPhi,
                           circuitsPerEta,
                           rowsPerCircuit,
                           columnsPerCircuit,
                           flipFE
                           ](const std::array<PixelDiodeTree::IndexType,2> &split_idx,
                             const PixelDiodeTree::Vector2D &diode_width,
                             [[maybe_unused]] const std::array<bool,4> &ganged,
                             [[maybe_unused]] unsigned int split_i,
                             PixelDiodeTree::AttributeType current_matrix_attribute,
                             PixelDiodeTree::AttributeType current_diode_attribute)
     -> std::tuple<PixelDiodeTree::AttributeType,PixelDiodeTree::AttributeType>
     {
        // split_idx the absolute index at which this sub-matrix is split into 4 sub-sub-matrices
        // diode_width the diode pitch in both directions
        // ganged ganged[0],ganged[1] whether the pixel diode is ganged in the corresponding direction
        //        ganged[2],ganged[3] whether the diode is inside (true) or outside the dead zone
        //        where ganged[2] denotes the flag in local-x and ganged[3] in local-y direction
        //
        // split_i   defines which of the 4 areas the diode belongs to :  2 | 3        ^
        //                                                                -----        |  local-y (chip-columns)
        //                                                                0 | 1        |
        //                                                                ---> local-x (chip-rows)
        //
        // current_matrix_attribute the default attribute for the unsplit sub-matrix assigned by the builder
        // current_diode_attribute the default attribute assigned to the current diode associated to the split
        //                         area specified by split_i
        // return new matrix attribute, new diode attribute

        // if the pixel is significantly wider in one direction consider the pixel to be long
        // or if wider in both directions large
        assert(split_idx[0]>=0 && split_idx[1]>=0);
        std::array<int,2> chip_idx{split_idx[0]/rowsPerCircuit, split_idx[1]/columnsPerCircuit};

        unsigned int n_large_dimensions = (  (std::abs(diode_width[0]-pitchPhi)>pitchPhi*.25)
                                            +(std::abs(diode_width[1]-pitchEta)>pitchEta*.25));
        std::cout << "DEBUG compute diode-type for " << split_idx[0] << " " << split_idx[1] << " | " << split_i
                  << " width " << diode_width[0] << " " << diode_width[1]  << " normal pitch " << pitchPhi << " " << pitchEta
                  << " large dim " << n_large_dimensions
                  << std::endl;

        switch (n_large_dimensions) {
        case 1:
           current_diode_attribute=InDetDD::detail::makeAttributeType(InDetDD::PixelDiodeType::LONG);
           break;
        case 2:
           current_diode_attribute=InDetDD::detail::makeAttributeType(InDetDD::PixelDiodeType::LARGE);
           break;
        default:
           current_diode_attribute=InDetDD::detail::makeAttributeType(InDetDD::PixelDiodeType::NORMAL);
        }

        if (readoutTechnology==InDetDD::PixelReadoutTechnology::RD53) {
           // The matrix attribute is used to store the front-end number, this works because
           // the matrices are first split by circuit and then by inner edge.

           // @TODO Is the numbering-scheme something that should be specified by the DB ?
           //
           //  The front-ends are numbered like ^   0 | 1       2 | 3
           //                                   |   -----      ------
           //                           local-x |   2 | 3       0 \ 1
           //                           row/phi |   (even)      (odd)
           //                                   + ---> local-y (chip-column/eta)
           //
           // (the sensor facing side of even modules points towards the IP)

           // Numbering scheme taken from the ITkPixelReadoutManager:
           if (flipFE) {
              current_matrix_attribute = InDetDD::detail::makeAttributeType(chip_idx[1] + (circuitsPerPhi-chip_idx[0]-1)*2);
           }
           else {
              current_matrix_attribute = InDetDD::detail::makeAttributeType(chip_idx[1] + chip_idx[0]*2);
           }

        }
        else {
           // @TODO compute front-end number correctly
           // just do something simple:
           // if there is a single row just  the chip-column (local-y, eta)
           // if there are two rows: top row chip-column starting from the opposite end; bottom row: chip column + chips per top row
           //         ^    0   |..  |n/2-1
           // local-x |    ---------------         [swapped axis direction to fit into fewer lines]
           // /eta    |    n-1 |... |n/2
           //         --> local-y (chip-rows, phi)
           current_matrix_attribute = InDetDD::detail::makeAttributeType( chip_idx[0] > 0
                                                                          ? circuitsPerEta - chip_idx[1] - 1
                                                                          : (circuitsPerPhi-1) * circuitsPerEta + chip_idx[1]);
        }
        return std::make_tuple(current_matrix_attribute, current_diode_attribute);
     };

  PixelDiodeTree diode_tree
        = createPixelDiodeTree(std::array<unsigned int,2>{static_cast<unsigned int>(circuitsPerPhi),static_cast<unsigned int>(circuitsPerEta)},
                               std::array<unsigned int,2>{static_cast<unsigned int>(rowsPerCircuit),static_cast<unsigned int>(columnsPerCircuit)},
                               PixelDiodeTree::Vector2D{pitchPhi,pitchEta},  // regular ptich
                               std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{static_cast<unsigned int>(nPhiEndPerSide),
                                                                                                     static_cast<unsigned int>(nEtaEndPerSide)},   // outer edge in pixels
                                                                          std::array<unsigned int,2>{static_cast<unsigned int>(nPhiLongPerSide),
                                                                                                     static_cast<unsigned int>(nEtaLongPerSide)}}, // inner edge in pixels
                               std::array<PixelDiodeTree::Vector2D,2>{PixelDiodeTree::Vector2D{pitchPhiEnd,  pitchEtaEnd},      // outer edge pitch (correct?)
                                                                      PixelDiodeTree::Vector2D{pitchPhiLong,pitchEtaLong}       // inner edge pitch
                               },
                               std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{0u,0u},   // @TODO add dead zone for run1-3 pixels?
                                                                          std::array<unsigned int,2>{0u,0u}    // @TODO add dead zone for run1-3 pixels?
                               },
                               computeAttribute,
                               nullptr);

  auto design = std::make_unique<PixelModuleDesign>(thickness,
                                                    phiSymmetric, etaSymmetric, depthSymmetric,
                                                    circuitsPerPhi, circuitsPerEta,
                                                    columnsPerCircuit, rowsPerCircuit,
                                                    columnsPerCircuit, rowsPerCircuit,
                                                    std::move(diode_tree), carrier,
                                                    readoutSide, is3D, detectorType, readoutTechnology);

  ATH_MSG_DEBUG("readout geo - design : " << typeName
                << " " << design->width() << "x" << design->length() << "x" << design->thickness()
                << " " << design->rows() << "x" << design->columns()
                << ", " << circuitsPerPhi << "x" << circuitsPerEta << " "
                << rowsPerCircuit << " " << columnsPerCircuit << ":\n"
                << diode_tree.debugStringRepr());

  [[maybe_unused]] auto observePtr = m_detectorManager->addDesign(std::move(design));

  // Add to map for addModule routine
  m_geometryMap[typeName] = m_detectorManager->numDesigns() - 1;
}


void PixelGmxInterface::addSensor(const std::string& typeName,
                                  std::map<std::string, int> &index,
                                  int /*sensitiveId*/,
                                  GeoVFullPhysVol *fpv)
{
  //
  // Get the ATLAS "Offline" wafer identifier
  //
  const PixelID *pixelIdHelper = dynamic_cast<const PixelID *>(m_commonItems->getIdHelper());
  if (not pixelIdHelper){
    ATH_MSG_ERROR("Failed dynamic_cast to PixelID in PixelGmxInterface::addSensor");
    return;
  }
  Identifier id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                          index["layer_wheel"],
                                          index["phi_module"],
                                          index["eta_module"]);
  IdentifierHash hashId = pixelIdHelper->wafer_hash(id);
  //
  //    Now do our best to check if this is a valid id. If either the gmx file is wrong, or the xml file
  //    defining the allowed id's is wrong, you can get disallowed id's. These cause a crash later
  //    if allowed through. To do the check, we ask for the hash-id of this id. Invalid ids give a
  //    special invalid hash-id (0xFFFFFFFF). But we don't exit the run, to help debug things quicker.
  //
  if (!hashId.is_valid()) {
    ATH_MSG_ERROR("Invalid id for sensitive module " << typeName << " volume with indices");
    for (const auto& [key, value] : index) {
      msg() << MSG::ERROR << key << " = " << value << "; ";
    }
    msg() << MSG::ERROR << endmsg;
    ATH_MSG_ERROR("Refusing to make it into a sensitive element. Incompatible gmx and identifier-xml files.");
    return;
  }

  // @TODO remove once all endcap modules are oriented consistently i.e. there is
  // one relation between local "hardware" coordinates and local offline coordinates
  // Currently all endcap modules have a surface normal (defined by the transform),
  // which points outwards. This is the case for endcap modules for which the sensor
  // facing side points towards the IP (even phi index), and those for which the sensor
  // facing side points outwards (odd phi index). By introducing separate module design
  // objects for endcap modules with even or odd phi index, the module design can
  // provide extra information to indicate the translation scheme between hardware
  // coordinates and offline coordinates.
  bool flipFE=index["barrel_endcap"]!=0 && index["phi_module"]%2==0 && typeName.find("Quad") != std::string::npos;
  //
  // Create the detector element and add to the DetectorManager
  //
  auto it = m_geometryMap.find( (flipFE ? typeName+"_even" : typeName));
  if(it == m_geometryMap.end()) {
    ATH_MSG_ERROR("addSensor: Error: Readout sensor type " << typeName << " not found.");
    throw std::runtime_error("readout sensor type " + typeName + " not found.");
  }
  const SiDetectorDesign *design = m_detectorManager->getDesign(it->second);
  ATH_MSG_VERBOSE("Adding sensor with design: " << typeName << " " << design);
  if (design == nullptr) {
    ATH_MSG_ERROR("addSensor: Error: Readout sensor type " << typeName << " not found.");
    throw std::runtime_error("readout sensor type " + typeName + " not found.");
  }

  m_detectorManager->addDetectorElement(new SiDetectorElement(id, design, fpv, m_commonItems));

  //
  // Build up a map-structure for numerology
  //
  Wafer module((unsigned int) hashId);
  std::string errorMessage("");
  if (!m_moduleTree->add(index["barrel_endcap"],
                         index["layer_wheel"],
                         index["eta_module"],
                         index["phi_module"],
                         module,
                         errorMessage)) {
    ATH_MSG_ERROR(errorMessage);
  }

  return;
}

void PixelGmxInterface::buildReadoutGeometryFromSqlite(IRDBAccessSvc * rdbAccessSvc,GeoModelIO::ReadGeoModel* sqlreader){

    const std::array<std::string,2> sensorTypes{"QuadChip_RD53","SingleChip_RD53"};
    const std::array<std::string,17> rd53_ParamNames{"circuitsPerEta","circuitsPerPhi","columns","detectorType","is3D","nEtaEndPerSide","nEtaLongPerSide","nPhiEndPerSide","nPhiLongPerSide","pitchEta","pitchEtaEnd","pitchEtaLong","pitchPhi","pitchPhiEnd","pitchPhiLong","rows","thickness"};
    
    for(const std::string & sType:sensorTypes){
       IRDBRecordset_ptr rd53 = rdbAccessSvc->getRecordsetPtr(sType,"");
       if(rd53->size() !=0){
          for (unsigned int iR =0;iR<rd53->size();iR++){
            std::map<std::string,std::string> rd53_Map;
            for(const std::string & paramName:rd53_ParamNames){
            std::string paramValue = (*rd53)[iR]->getString(paramName);
            rd53_Map[paramName] = std::move(paramValue);
        }
           std::string rd35_Name = (*rd53)[iR]->getString("SensorType");
           makePixelModule(rd35_Name,rd53_Map);
           // @TODO remove once all endcap modules are oriented consistently i.e. there is
           // one relation between local "hardware" coordinates and local offline coordinates
           if (   rd35_Name.find("Quad")!= std::string::npos
               && (   rd35_Name.find("endcap")!= std::string::npos
                   || rd35_Name.find("inclined")!= std::string::npos)) {
              makePixelModule(rd35_Name+"_even",rd53_Map);
           }
          } 
       }
    else ATH_MSG_WARNING("Could not retrieve "<<sType<<" table");
    }

    //Now, loop over the FullPhysVols and create the SiDetectorElements
    //lots of string parsing...
    const std::array<std::string,5> fields{"barrel_endcap","layer_wheel","phi_module","eta_module","side"}; 
    //First, find which name the tables are in the file under (depends upon the plugin used to create the input file)
    //sort these in order of precedence - ITkPlugin, then ITkPixelPlugin, then GeoModelXMLPlugin
    const std::array<std::string,3> publishers({"ITk","ITkPixel","GeoModelXML"});
    //The below is a map of string keys which will contain all the Identifier/DetElement relevant info, and the associated FullPhysVol
    // (once filled from the published table in the SQLite)
    std::map<std::string, GeoFullPhysVol*> mapFPV;
    for (auto & iPub : publishers){
        //setting the "checkTable" option to true, so that an empty map will be returned if not found and we can try the next one
         mapFPV = sqlreader->getPublishedNodes<std::string, GeoFullPhysVol*>(iPub,true);
         if (!mapFPV.empty()) {
            ATH_MSG_DEBUG("Using FPV tables from publisher "<<iPub);
            break;
         }
    }
    if (mapFPV.empty()) ATH_MSG_ERROR("Could not find any FPV tables under the expected names: "<<publishers);

    for (const auto&[fullPhysVolInfoString, fullPhysVolPointer] : mapFPV){
        //find the name of the corresponding detector design type
        size_t startRG = fullPhysVolInfoString.find("RD53_");
        if(startRG==std::string::npos){
            ATH_MSG_DEBUG("GeoFullPhysVol "<<fullPhysVolInfoString<<" does not have the expected format. Skipping");
            continue;
            } 
        std::string typeName = fullPhysVolInfoString.substr(startRG);
        std::map<std::string, int> index;
        for (const std::string & field:fields){
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

void PixelGmxInterface::addAlignable(int level,
                                     std::map<std::string, int> &index,
                                     GeoVFullPhysVol *fpv,
                                     GeoAlignableTransform *transform)
{
  ATH_MSG_DEBUG("addAlignable called");
  const PixelID *pixelIdHelper = dynamic_cast<const PixelID *> (m_commonItems->getIdHelper());
  if (not pixelIdHelper){
    ATH_MSG_ERROR("Dynamic cast to PixelID failed in PixelGmxInterface::addAlignable");
    return;
  }
  Identifier id;
  switch (level) {
    case 0:
      id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                 index["layer_wheel"],
                                 index["phi_module"],
                                 index["eta_module"],
                                 index["side"]);
      break;
    case 1:
      id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                 index["layer_wheel"],
                                 index["phi_module"],
                                 index["eta_module"],
                                 0);
      break;
    case 2:
      id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                 index["layer_wheel"],
                                 0,
                                 0,
                                 0);
      break;
    case 3:
      id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                 0,
                                 0,
                                 0,
                                 0);
      break;
    default:
      throw GaudiException("Unknown level " + std::to_string(level) + " for alignment in addAlignable",
                           "PixelGmxInterface::addAlignable", StatusCode::FAILURE);
      break;
  }
  m_detectorManager->addAlignableTransform(level, id, transform, fpv);
}


} // namespace ITk
} // namespace InDetDD
