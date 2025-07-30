/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PLRGmxInterface.h"

#include <InDetIdentifier/PLR_ID.h>
#include <InDetReadoutGeometry/SiDetectorDesign.h>
#include <InDetReadoutGeometry/SiDetectorElement.h>
#include <InDetSimEvent/SiHitIdHelper.h>
#include <PixelReadoutGeometry/PixelDetectorManager.h>
#include <PixelReadoutGeometry/PixelModuleDesign.h>
#include "ReadoutGeometryBase/PixelDiodeTree.h"
#include "ReadoutGeometryBase/PixelDiodeTreeBuilder.h"
#include <ReadoutGeometryBase/SiCommonItems.h>
#include <InDetGeoModelUtils/WaferTree.h>


namespace InDetDD
{

PLRGmxInterface::PLRGmxInterface(PixelDetectorManager *detectorManager,
                                 SiCommonItems *commonItems,
                                 WaferTree *moduleTree)
  : PixelGmxInterface(detectorManager, commonItems, moduleTree)
{}

int PLRGmxInterface::sensorId(std::map<std::string, int> &index) const
{
  // Return the Simulation HitID (nothing to do with "ATLAS Identifiers" aka "Offline Identifiers")

  // Check if identifier is valid
  // TODO: drop this check in the future
  const PLR_ID *pixelIdHelper = dynamic_cast<const PLR_ID *>(m_commonItems->getIdHelper());
  if (not pixelIdHelper){
    ATH_MSG_ERROR("Failed dynamic_cast to PLR_ID in PLRGmxInterface::sensorId");
    return -1;
  }
  Identifier id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                          index["layer_wheel"],
                                          index["phi_module"],
                                          index["eta_module"]);
  IdentifierHash hashId = pixelIdHelper->wafer_hash(id);
  if (!hashId.is_valid()) {
    ATH_MSG_WARNING("PLR Invalid hash for Index list: " << index["barrel_endcap"] << " " << index["layer_wheel"] << " "
                    << index["eta_module"] << " " << index["phi_module"] << " " << index["side"]);
    return -1;
  }
  // Compute the actuall SiHitId, first number is the part number: lumi=2
  int hitIdOfModule = SiHitIdHelper::GetHelper()->buildHitId(2,
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


void PLRGmxInterface::addSensorType(const std::string& clas,
                                    const std::string& typeName,
                                    const std::map<std::string, std::string>& parameters)
{
  ATH_MSG_DEBUG("addSensorType called for class " << clas << ", typeName " << typeName);
  // only load the sensor type that the PLR will use
  if (clas == "SingleChip_RD53" && (typeName == "RD53_20x19_Single_25x100" || typeName == "PLR_20x19_Single_25x100")) {
    makePLRModule(typeName, parameters);
  }
}


void PLRGmxInterface::addSensor(const std::string& typeName,
                                  std::map<std::string, int> &index,
                                  int /*sensitiveId*/,
                                  GeoVFullPhysVol *fpv)
{
  //
  // Get the ATLAS "Offline" wafer identifier
  //
  const PLR_ID *pixelIdHelper = dynamic_cast<const PLR_ID *>(m_commonItems->getIdHelper());
  if (not pixelIdHelper){
    ATH_MSG_ERROR("Failed dynamic_cast to PLR_ID in PLRGmxInterface::addSensor");
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

  //
  // Create the detector element and add to the DetectorManager
  //
  auto it = m_geometryMap.find(typeName);
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


void PLRGmxInterface::makePLRModule(const std::string &typeName,
                                    const std::map<std::string, std::string> &parameters)
{
  int circuitsPerEta{1}; // row
  int circuitsPerPhi{1}; // column
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
  bool is3D{true};

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

  constexpr InDetDD::PixelReadoutTechnology readoutTechnology = InDetDD::PixelReadoutTechnology::RD53;

  // helper function to associate attributes to sub-matrices and diodes.
  auto computeAttribute = [readoutTechnology,
                           pitchPhi,
                           pitchEta,
                           circuitsPerPhi,
                           circuitsPerEta,
                           rowsPerCircuit,
                           columnsPerCircuit
                           ](const std::array<PixelDiodeTree::IndexType,2> &split_idx,
                             const PixelDiodeTree::Vector2D &diode_width,
                             [[maybe_unused]] const std::array<bool,4> &ganged,
                             [[maybe_unused]] unsigned int split_i,
                             PixelDiodeTree::AttributeType current_matrix_attribute,
                             PixelDiodeTree::AttributeType current_diode_attribute)
     -> std::tuple<PixelDiodeTree::AttributeType,PixelDiodeTree::AttributeType>
     {
        assert (readoutTechnology==InDetDD::PixelReadoutTechnology::RD53);
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

        current_matrix_attribute = InDetDD::detail::makeAttributeType(chip_idx[1] + chip_idx[0]*2);
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
                               std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{0u,0u},   // @TODO add dead zone for run1-3 pixels
                                                                          std::array<unsigned int,2>{0u,0u}    // @TODO add dead zone for run1-3 pixels
                               },
                               computeAttribute,
                               nullptr);

  // Setting module identifier to InDetDD::PLR
  // (so far) primarily useful to avoid orientation warnings
  InDetDD::DetectorType detectorType{InDetDD::PLR};

  auto design = std::make_unique<PixelModuleDesign>(thickness,
                                                    circuitsPerPhi, circuitsPerEta,
                                                    columnsPerCircuit, rowsPerCircuit,
                                                    columnsPerCircuit, rowsPerCircuit,
                                                    std::move(diode_tree), carrier,
                                                    readoutSide, is3D, detectorType,
                                                    readoutTechnology);

  ATH_MSG_DEBUG("readout geo - design " << typeName << " " << design->width() << "x" << design->length() << "x" << design->thickness()
                << " " << design->rows() << "x" << design->columns()
                << ", " << circuitsPerPhi << "x" << circuitsPerEta << " "
                << rowsPerCircuit << "x" << columnsPerCircuit
                << " carrier " << carrier << " readout side " << readoutSide << ":\n"
                << diode_tree.debugStringRepr());

  [[maybe_unused]] auto observedPtr = m_detectorManager->addDesign(std::move(design));

  // Add to map for addModule routine
  m_geometryMap[typeName] = m_detectorManager->numDesigns() - 1;
}

} // namespace InDetDD
