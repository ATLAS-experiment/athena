
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsMuonChamberTool.h"
///
#include <GaudiKernel/SystemOfUnits.h>
#include <algorithm>
#include <cmath>

#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>

namespace {
    constexpr double tolerance = 10 * Gaudi::Units::micrometer;
}
using namespace ActsTrk;
namespace MuonGMR4{
  using defineArgs = MuonChamber::defineArgs;
  using ReadoutSet = MuonChamber::ReadoutSet;

  ActsMuonChamberTool::ActsMuonChamberTool( const std::string& type, 
                                                            const std::string& name, 
                                                            const IInterface* parent ):
        AthAlgTool{type,name, parent} {
    declareInterface<IActsMuonChamberTool>(this);
  }
  
  StatusCode ActsMuonChamberTool::initialize() {    
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    ATH_CHECK(m_geoUtilTool.retrieve());     
    return StatusCode::SUCCESS;
  }
  ChamberSet ActsMuonChamberTool::buildChambers() const {
     ChamberSet chambers{};
     ActsGeometryContext gctx{};
     /// First access the detector manager to ask for the list of all readout elements
     std::vector<const MuonReadoutElement*> allElements = m_detMgr->getAllReadoutElements();
     /// Next loop over all volumes and group them by their mother
     std::map<PVConstLink, ReadoutSet> chamberConstituents{};
     /// Except for the Nsw because that's not having a common mother volume. 
     /// So let's plunge all readout element of a sector into a single chamber    
     std::map<int, std::vector<const MuonReadoutElement*>> nswREs{};
     for (const MuonReadoutElement* readOut: allElements) {
        if (readOut->detectorType() == ActsTrk::DetectorType::Mm ||
            readOut->detectorType() == ActsTrk::DetectorType::sTgc) {
            const int sector = m_idHelperSvc->sector(readOut->identify()) * 
                              (m_idHelperSvc->stationEta(readOut->identify()) > 0 ? 1 : -1);
            nswREs[sector].push_back(readOut);
        } else {
          chamberConstituents[readOut->getMaterialGeom()->getParent()].push_back(readOut);
        }
     }
     for (auto& [sector, readEles]:  nswREs) {
       ATH_MSG_VERBOSE("Summarize all Nsw readout elements of sector "<<sector<<" into single object");
       chamberConstituents[readEles[0]->getMaterialGeom()->getParent()] = std::move(readEles);
     }
     for (auto& [motherVol, readoutEles] : chamberConstituents) {
        /// Sort the chambers within the mother such that mdts are sorted first by distance to IP
        /// then the rpcs are sorted by distance to IP

        /// Orientation of the chamber coordinate systems
        ///   x-axis: Points towards the sky
        ///   y-axis: Points along the chamber plane
        ///   z-axis: Points along the beam axis
        /// --> Transform into the coordinate system of the chamber
        /// x-axis: Parallel to the eta channels
        /// y-axis: Along the beam axis
        /// z-axis: Towards the sky
        /// Record the thickness for the top and bottom of the chamber
        const Amg::Transform3D axisRotation{Amg::getRotateX3D(90. * Gaudi::Units::deg)*
                                            Amg::getRotateZ3D(90. * Gaudi::Units::deg)};
        
        const Amg::Transform3D motherTrans = axisRotation * motherVol->getX().inverse();        
        std::sort(readoutEles.begin(),readoutEles.end(), 
                    [&motherTrans, &gctx](const MuonReadoutElement*a, const MuonReadoutElement* b) {
                      if (a->detectorType() != b->detectorType()) {
                        return static_cast<unsigned> (a->detectorType()) < static_cast<unsigned> (b->detectorType());
                      }
                      const Amg::Vector3D aPos = motherTrans * a->center(gctx);
                      const Amg::Vector3D bPos = motherTrans * b->center(gctx);
                      if (std::abs(aPos.z() - bPos.z()) > tolerance) {
                            return aPos.z() < bPos.z();
                      }
                      return aPos.y() < bPos.y();
                });
        /// Small check that everything worked out as expected
        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream sstr{};
            for (const MuonReadoutElement* readOut: readoutEles) {
                sstr<<"*** "<<m_idHelperSvc->toStringDetEl(readOut->identify())
                    <<" "<<Amg::toString(motherTrans * readOut->center(gctx))<<std::endl;
            }
            ATH_MSG_VERBOSE(m_geoUtilTool->dumpVolume(motherVol)<<std::endl<<
                            "Associated readout elements: "<<std::endl<<sstr.str());
        }

        const MuonReadoutElement* primRE = readoutEles[0];
        /// Determine the minimal surrounding trapezoid box
        double xMinS{10.e5}, xMaxS{-10.e5}, xMinL{10.e5}, xMaxL{-10.e5}, 
               yMin{10.e5}, yMax{-10.e5}, zMin{10.e5}, zMax{-10.e5};
        defineArgs define{};
        for(const MuonReadoutElement* ele: readoutEles) {
          const Amg::Vector3D center{motherTrans* ele->center(gctx)};
          ATH_MSG_VERBOSE("Center of readout element "<<m_idHelperSvc->toStringDetEl(ele->identify())
                        <<" located at "<<Amg::toString(center, 2));
          if(ele->detectorType() == ActsTrk::DetectorType::Mdt) {
            const MdtReadoutElement* mdtReadoutEle = static_cast<const MdtReadoutElement*>(ele);
            const MdtReadoutElement::parameterBook& parameters{mdtReadoutEle->getParameters()};
            xMinS = std::min(center.x() - parameters.shortHalfX, xMinS);
            xMaxS = std::max(center.x() + parameters.shortHalfX, xMaxS);
            xMinL = std::min(center.x() - parameters.longHalfX, xMinL);
            xMaxL = std::max(center.x() + parameters.longHalfX, xMaxL);
            yMin = std::min(center.y() - 2.*parameters.halfY, yMin);
            yMax = std::max(center.y(), yMax);
            zMin = std::min(center.z(), zMin);
            zMax = std::max(center.z() + 2.*parameters.halfHeight, zMax);
          } else if(ele->detectorType() == ActsTrk::DetectorType::Rpc) {
              const RpcReadoutElement* rpcReadoutEle = static_cast<const RpcReadoutElement*>(ele);
              const RpcReadoutElement::parameterBook& parameters{rpcReadoutEle->getParameters()};
              xMinS = std::min(center.x() - parameters.halfWidth, xMinS);
              xMaxS = std::max(center.x() + parameters.halfWidth, xMaxS);
              /// Recall the Mdts are rectangles as well
              xMinL = xMinS;
              xMaxL = xMaxS;
              yMax = std::max(center.y(), yMax);
              yMin = std::min(center.y() - 2.* parameters.halfLength, yMin);
              zMin = std::min(center.z(), zMin);
              zMax = std::max(center.z() + 2.*parameters.halfThickness, zMax);
          } else if (ele->detectorType() == ActsTrk::DetectorType::Tgc) {
              const TgcReadoutElement* tgcReadoutEle{static_cast<const TgcReadoutElement*>(ele)};              
              xMinL = std::min(center.x() - 0.5*tgcReadoutEle->moduleWidthL(), xMinL);
              xMaxL = std::max(center.x() + 0.5*tgcReadoutEle->moduleWidthL(), xMaxL);
              xMinS = std::min(center.x() - 0.5*tgcReadoutEle->moduleWidthS(), xMinS);
              xMaxS = std::max(center.x() + 0.5*tgcReadoutEle->moduleWidthS(), xMaxS);
              yMin  = std::min(center.y()- tgcReadoutEle->moduleHeight(), yMin);
              yMax  = std::max(center.y(), yMax);
              zMin  = std::min(center.z(), zMin);
              zMax  = std::max(center.z() + tgcReadoutEle->moduleThickness(), zMax);
          } else if(ele->detectorType() == ActsTrk::DetectorType::sTgc) {
            const sTgcReadoutElement* stgcReadoutEle = static_cast<const sTgcReadoutElement*>(ele);
            const sTgcReadoutElement::parameterBook& parameters{stgcReadoutEle->getParameters()};
            xMinS = std::min(center.x() - parameters.sHalfChamberLength, xMinS);
            xMaxS = std::max(center.x() + parameters.sHalfChamberLength, xMaxS);
            xMinL = std::min(center.x() - parameters.lHalfChamberLength, xMinL);
            xMaxL = std::max(center.x() + parameters.lHalfChamberLength, xMaxL);
            yMin = std::min(center.y() - parameters.halfChamberHeight, yMin);
            yMax = std::max(center.y() + parameters.halfChamberHeight, yMax);
            zMin = std::min(center.z(), zMin);
            zMax = std::max(center.z() + 2.*parameters.halfChamberTck, zMax);
          } else if (ele->detectorType() == ActsTrk::DetectorType::Mm) {
            const MmReadoutElement* mmReadoutEle = static_cast<const MmReadoutElement*>(ele);
            xMinS = std::min(center.x() - 0.5*mmReadoutEle->moduleWidthS(), xMinS);
            xMaxS = std::max(center.x() + 0.5*mmReadoutEle->moduleWidthS(), xMaxS);
            xMinL = std::min(center.x() - 0.5*mmReadoutEle->moduleWidthL(), xMinS);
            xMaxL = std::max(center.x() + 0.5*mmReadoutEle->moduleWidthL(), xMaxL);
            yMin = std::min(center.y(), yMin);
            yMax = std::max(center.y() + mmReadoutEle->moduleHeight(), yMax);
            zMin = std::min(center.z(), zMin);
            zMax = std::max(center.z() + mmReadoutEle->moduleThickness(), zMax);
          } else {
              ATH_MSG_FATAL("Did the Csc made it back? Or do we have a new detector technology.");
              throw std::logic_error("Detector type not supported");
          }
          ATH_MSG_VERBOSE("Chamber dimensions: yMin: "<<yMin<<" yMax: "<<yMax
                      <<" xMinS: "<<xMinS<<" xMaxS: "<<xMaxS<<" xMinL: "<<xMinL<<" xMaxL: "<<xMaxL<< " zMin: "<<zMin<<" zMax: "<<zMax);       
        }
        
        define.halfXShort = (xMaxS - xMinS) / 2.;
        define.halfXLong = (xMaxL - xMinL) / 2.;
        define.halfY = (yMax - yMin) / 2.;
        define.halfZ = (zMax - zMin) / 2.;
 
        if (define.halfY <=0. || define.halfXShort<=0. ||define.halfXLong <=0. ||  define.halfZ<=0.) {
          ATH_MSG_WARNING("Invalid trapezoid boundaries "<<m_idHelperSvc->toStringDetEl(primRE->identify())<<
                  	      " "<<define);

        }
        const Amg::Vector3D boxCenter{xMinS + define.halfXShort, 
                                      yMin + define.halfY, 
                                      zMin + define.halfZ};
        /// Add a small tolerance to ensure that the detector elements are all inside the chamber
        define.halfY += tolerance;
        define.halfXShort += tolerance;
        define.halfXLong += tolerance;
        define.halfZ += tolerance;

        define.readoutEles = std::move(readoutEles);
        
        define.centerTrans = primRE->globalToLocalTrans(gctx) * 
                             motherTrans.inverse() *
                             Amg::Translation3D{boxCenter};
        ATH_MSG_DEBUG("Chamber "<<m_idHelperSvc->toStringChamber(primRE->identify())
                      <<" has surrounding box "<<define);

        MuonChamber muonChamb{std::move(define)};
        chambers.insert(std::move(muonChamb));
    }
    return chambers;
  }
}   
