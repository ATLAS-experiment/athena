/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// MuonTrackingGeometryBuilderImpl.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Muon
#include "MuonTrackingGeometry/MuonTrackingGeometryBuilderImpl.h"
#include "MuonTrackingGeometry/Utils.h"


#include "MuonReadoutGeometry/GlobalUtilities.h"
// Amg
#include "GeoPrimitives/GeoPrimitives.h"
// Units
#include "GaudiKernel/SystemOfUnits.h"
// Trk
#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkDetDescrUtils/BinnedArray1D.h"
#include "TrkDetDescrUtils/BinnedArray1D1D1D.h"
#include "TrkDetDescrUtils/BinnedArray2D.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkDetDescrUtils/SharedObject.h"
#include "TrkGeometry/GlueVolumesDescriptor.h"
#include "TrkGeometry/Material.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkVolumes/BevelledCylinderVolumeBounds.h"
#include "TrkVolumes/BoundarySurface.h"
#include "TrkVolumes/CombinedVolumeBounds.h"
#include "TrkVolumes/CuboidVolumeBounds.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkVolumes/DoubleTrapezoidVolumeBounds.h"
#include "TrkVolumes/PrismVolumeBounds.h"
#include "TrkVolumes/SimplePolygonBrepVolumeBounds.h"
#include "TrkVolumes/SubtractedVolumeBounds.h"
#include "TrkVolumes/TrapezoidVolumeBounds.h"

// Athena
#include "AthenaKernel/IOVInfiniteRange.h"

// STD
#include <cmath>
#include <fstream>
#include <map>

namespace Muon {

    using DetachedVolPtr        = MuonTrackingGeometryBuilderImpl::DetachedVolPtr;
    using DetachedVolVec        = MuonTrackingGeometryBuilderImpl::DetachedVolVec;
    using DetachedVolSpanPair   = MuonTrackingGeometryBuilderImpl::DetachedVolSpanPair;
    using VolumeSpanArray       = MuonTrackingGeometryBuilderImpl::VolumeSpanArray;
    using TrackingVolumePtr     = MuonTrackingGeometryBuilderImpl::TrackingVolumePtr;
// constructor
MuonTrackingGeometryBuilderImpl::MuonTrackingGeometryBuilderImpl(
    const std::string& t, const std::string& n, const IInterface* p)
    : AthAlgTool(t, n, p) {}

// Athena standard methods
// initialize

StatusCode MuonTrackingGeometryBuilderImpl::initialize() {
    // Retrieve the tracking volume helper
    // -------------------------------------------------
    ATH_CHECK(m_trackingVolumeHelper.retrieve());
    ATH_CHECK(m_trackingVolumeArrayCreator.retrieve());
    if (m_loadMSentry) {
        // retrieve envelope definition service
        // --------------------------------------------------
        ATH_CHECK(m_enclosingEnvelopeSvc.retrieve());
    }

    ATH_MSG_DEBUG( " initialize() successful");
    return StatusCode::SUCCESS;
}

std::unique_ptr<Trk::TrackingGeometry>
MuonTrackingGeometryBuilderImpl::trackingGeometryImpl(DetachedVolVec && stations,
                                                      DetachedVolVec && inertObjs,
                                                      Trk::TrackingVolume* tvol) const {
    ATH_MSG_DEBUG( " building tracking geometry");
    bool hasStations = inertObjs.size() || stations.size();

    // load local variables to container
    LocalVariablesContainer aLVC;
    aLVC.m_innerBarrelRadius = m_innerBarrelRadius;
    aLVC.m_outerBarrelRadius = m_outerBarrelRadius;
    aLVC.m_innerEndcapZ = m_innerEndcapZ;
    aLVC.m_outerEndcapZ = m_outerEndcapZ;
    aLVC.m_adjustStatic = m_adjustStatic;
    aLVC.m_static3d = m_static3d;
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // check setup
    if (m_muonInert && m_blendInertMaterial) {
        if (!aLVC.m_adjustStatic || !aLVC.m_static3d) {
            ATH_MSG_INFO( " diluted inert material hardcoded for 3D "
                                   "volume frame, adjusting setup");
            aLVC.m_adjustStatic = true;
            aLVC.m_static3d = true;
        }
    }
   // find object's span with tolerance for the alignment
    
    aLVC.m_stationSpan = findVolumesSpan(stations, 100. * m_alignTolerance,
                                         m_alignTolerance * Gaudi::Units::deg, aLVC);

    aLVC.m_inertSpan = findVolumesSpan(inertObjs, 0., 0., aLVC);

    // 0) Preparation
    // //////////////////////////////////////////////////////////////////////////////////////

    aLVC.m_muonMaterial = Trk::Material(10e10, 10e10, 0., 0., 0.);  // default material properties



    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //   Envelope definition (cutouts)
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    RZPairVector envelopeDefs;
    if (m_enclosingEnvelopeSvc) {
        // get the dimensions from the envelope service
        const RZPairVector& envelopeDefsIn = m_enclosingEnvelopeSvc->getMuonRZBoundary();

        // find the max,max pair
        unsigned int ii = 0;
        for (unsigned int i = 0; i < envelopeDefsIn.size(); i++) {
            if (envelopeDefsIn[i].second > envelopeDefsIn[ii].second)
                ii = i;
            else if (envelopeDefsIn[i].second == envelopeDefsIn[ii].second &&
                     envelopeDefsIn[i].first > envelopeDefsIn[ii].first)
                ii = i;
        }

        // find the sense of rotation
        int irot = 1;
        unsigned int inext = ii + 1;
        if (inext == envelopeDefsIn.size())
            inext = 0;
        if (envelopeDefsIn[inext].second != envelopeDefsIn[ii].second) {
            irot = -1;
            inext = ii > 0 ? ii - 1 : envelopeDefsIn.size() - 1;
        }

        // fill starting with upper low edge, end with upper high edge
        if (irot > 0) {
            for (unsigned int i = inext; i < envelopeDefsIn.size(); i++)
                envelopeDefs.push_back(envelopeDefsIn[i]);
            if (inext > 0)
                for (unsigned int i = 0; i <= inext - 1; i++)
                    envelopeDefs.push_back(envelopeDefsIn[i]);
        } else {
            int i = inext;
            while (i >= 0) {
                envelopeDefs.push_back(envelopeDefsIn[i]);
                i = i - 1;
            };
            inext = envelopeDefsIn.size() - 1;
            while (inext >= ii) {
                envelopeDefs.push_back(envelopeDefsIn[inext]);
                inext = inext - 1;
            };
        }

        // find maximal z,R extent
        double maxR = 0.;
        for (auto& envelopeDef : envelopeDefs) {
            if (envelopeDef.first > maxR)
                maxR = envelopeDef.first;
        }

        aLVC.m_outerBarrelRadius = maxR;
        aLVC.m_outerEndcapZ = envelopeDefs[0].second;

        ATH_MSG_VERBOSE("Muon envelope definition retrieved: outer R,Z:"
                        << aLVC.m_outerBarrelRadius << ","
                        << aLVC.m_outerEndcapZ);

        // construct inner and outer envelope

        for (unsigned int i = 0; i < envelopeDefs.size(); i++) {
            ATH_MSG_VERBOSE("Rz pair:" << i << ":" << envelopeDefs[i].first
                                       << "," << envelopeDefs[i].second);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (m_muonSimple) {
        auto globalBounds = std::make_unique<Trk::CylinderVolumeBounds>(aLVC.m_outerBarrelRadius, aLVC.m_outerEndcapZ);
        auto topVolume = std::make_unique<Trk::TrackingVolume>(nullptr, globalBounds.release(), aLVC.m_muonMaterial,
                                                               nullptr, nullptr, "GlobalVolume");
        return std::make_unique<Trk::TrackingGeometry>(topVolume.release());
    }

    ATH_MSG_DEBUG( "building barrel+innerEndcap+outerEndcap");

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MuonSpectrometer contains:
    //       - Barrel
    //       - Endcaps inner/outer
    std::vector<TrackingVolumePtr> volumeGarbage{};

    TrackingVolumePtr muonBarrel{}, negativeMuonOuterWheel{},
                      negativeMuonBigWheel{}, negativeMuonOuterBuffer{},
                      positiveMuonOuterWheel{}, negativeMuonSmallWheel{},
                      positiveMuonSmallWheel{}, negativeECT{}, positiveECT{},
                      positiveMuonBigWheel{}, positiveMuonOuterBuffer{};
    // volumes needed to close the geometry
    TrackingVolumePtr negBeamPipe{}, posBeamPipe{}, negDiskShield{}, posDiskShield{}, 
                      negInnerShield{}, posInnerShield{}, negOuterShield{}, posOuterShield{};

    std::unique_ptr<Trk::CylinderVolumeBounds> enclosedBounds{};

    TrackingVolumePtr barrelZPBuffer{}, barrelZMBuffer{};
    using TrackingVolumeRawPtr = Trk::TrackingVolume*;
    TrackingVolumeRawPtr barrelZP{}, centralP{}, central{},
                         negativeMuonInnerEndcap{}, positiveMuonInnerEndcap{},
                         negNavOEndcap{}, posNavOEndcap{}, negativeMuonOuterEndcap{},
                         positiveMuonOuterEndcap{}, barrel{}, negOuterEndcap{},
                         posOuterEndcap{}, negInnerEndcap{}, posInnerEndcap{}, negNavEndcap{},
                         posNavEndcap{}, negEndcap{}, posEndcap{}, negDet{}, detector{}, enclosed{};
                         

    // if input, redefine dimensions to fit expected MS entry
    if (tvol) {
        bool msEntryDefined = false;
        if (tvol->volumeName() == m_entryVolume)
            msEntryDefined = true;
        // get dimensions
        ATH_MSG_DEBUG(" msEntryDefined " << msEntryDefined);
        auto enclosedDetectorBounds =dynamic_cast<const Trk::CylinderVolumeBounds*>(&(tvol->volumeBounds()));
        if (!enclosedDetectorBounds) {
            ATH_MSG_ERROR(" dynamic cast of enclosed volume to the cylinder bounds failed, aborting MTG build-up ");
            return nullptr;
        }
        double enclosedDetectorHalfZ = enclosedDetectorBounds->halflengthZ();
        double enclosedDetectorOuterRadius =
            enclosedDetectorBounds->outerRadius();
        // get subvolumes at navigation level and check THEIR dimensions
        Trk::GlueVolumesDescriptor& enclosedDetGlueVolumes = tvol->glueVolumesDescriptor();
        std::vector<Trk::TrackingVolume*> enclosedCentralFaceVolumes = enclosedDetGlueVolumes.glueVolumes(Trk::cylinderCover);
        std::vector<Trk::TrackingVolume*> enclosedNegativeFaceVolumes = enclosedDetGlueVolumes.glueVolumes(Trk::negativeFaceXY);
        std::vector<Trk::TrackingVolume*> enclosedPositiveFaceVolumes = enclosedDetGlueVolumes.glueVolumes(Trk::positiveFaceXY);
        if (!enclosedCentralFaceVolumes.empty()) {
            auto cylR = dynamic_cast<const Trk::CylinderVolumeBounds*>(&(enclosedCentralFaceVolumes[0]->volumeBounds()));
            if (cylR && cylR->outerRadius() != enclosedDetectorOuterRadius) {
                enclosedDetectorOuterRadius = cylR->outerRadius();
                ATH_MSG_WARNING(" enclosed volume envelope outer radius does not "
                                "correspond to radius of glue volumes : adjusted ");
            }
        }
        if (!enclosedNegativeFaceVolumes.empty() &&
            !enclosedPositiveFaceVolumes.empty()) {
            double negZ = -enclosedDetectorHalfZ;
            double posZ = enclosedDetectorHalfZ;
            auto cylN = dynamic_cast<const Trk::CylinderVolumeBounds*>(&(enclosedNegativeFaceVolumes[0]->volumeBounds()));
            if (cylN){
                negZ = enclosedNegativeFaceVolumes[0]->center().z() - cylN->halflengthZ();
            }
            auto cylP =dynamic_cast<const Trk::CylinderVolumeBounds*>(&(enclosedPositiveFaceVolumes[0]->volumeBounds()));
            if (cylP) {
                posZ = enclosedPositiveFaceVolumes[0]->center().z() +
                       cylP->halflengthZ();
            }
            if (std::abs(negZ + enclosedDetectorHalfZ) > 0.001 ||
                std::abs(posZ - enclosedDetectorHalfZ) > 0.001) {
                ATH_MSG_WARNING(" enclosed volume envelope z dimension does not correspond to that of glue volumes ");
                if (std::abs(negZ + posZ) < 0.001) {
                    enclosedDetectorHalfZ = posZ;
                    ATH_MSG_WARNING( " z adjusted ");
                } else {
                    ATH_MSG_ERROR("assymetric Z dimensions - cannot recover " << negZ << "," << posZ);
                    return nullptr;
                }
            }
        }
        //

        //
        ATH_MSG_DEBUG(" dimensions of enclosed detectors (halfZ,outerR):"
                     << enclosedDetectorHalfZ << ","<< enclosedDetectorOuterRadius);
        // check if input makes sense - gives warning if cuts into muon envelope
        // adjust radius
        if (enclosedDetectorOuterRadius > aLVC.m_innerBarrelRadius) {
            ATH_MSG_WARNING( " enclosed volume too wide, cuts into "
                                      "muon envelope, abandon :R:"
                                   << enclosedDetectorOuterRadius);
            return nullptr;
        }   
        aLVC.m_innerBarrelRadius = enclosedDetectorOuterRadius;
        
        // adjust z
        if (enclosedDetectorHalfZ > m_barrelZ) {
            ATH_MSG_WARNING( " enclosed volume too long, cuts into "
                          <<"muon envelope, abandon :Z:"<< enclosedDetectorHalfZ);
            return nullptr;
        } else {
            if (enclosedDetectorHalfZ < m_barrelZ) {
                auto barrelZPBounds = std::make_unique<Trk::CylinderVolumeBounds>(aLVC.m_innerBarrelRadius,
                                                                                  0.5 * (m_barrelZ - enclosedDetectorHalfZ));
                auto barrelZMBounds = std::make_unique<Trk::CylinderVolumeBounds>(aLVC.m_innerBarrelRadius,
                                                                                  0.5 * (m_barrelZ - enclosedDetectorHalfZ));
                double zbShift = 0.5 * (m_barrelZ + enclosedDetectorHalfZ);
                
                barrelZPBuffer = std::make_unique<Trk::TrackingVolume>(makeTransform(translateZ3D(zbShift)),
                                                                       barrelZPBounds.release(), aLVC.m_muonMaterial, nullptr,
                                                                       nullptr, "BarrelRZPosBuffer");
                barrelZMBuffer = std::make_unique<Trk::TrackingVolume>(makeTransform(translateZ3D(-zbShift)),
                                                                       barrelZMBounds.release(), aLVC.m_muonMaterial, nullptr,
                                                                       nullptr, "BarrelRZNegBuffer");

                ATH_MSG_DEBUG( "glue barrel R  + barrel Z buffer");
                barrelZP = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*tvol, 
                                                                             Trk::positiveFaceXY, 
                                                                             *barrelZPBuffer.release(),
                                                                             Trk::negativeFaceXY, "All::Gaps::BarrelZP"));
                // set name
                std::string nameEncl = msEntryDefined ? "All::Gaps::Barrel" : m_entryVolume;
                ATH_MSG_DEBUG(" nameEncl " << nameEncl);
                enclosed = m_trackingVolumeHelper->glueTrackingVolumeArrays(*barrelZP, 
                                                                             Trk::negativeFaceXY, 
                                                                             *barrelZMBuffer.release(),
                                                                             Trk::positiveFaceXY, 
                                                                             nameEncl);

            } else{
                /// FIX ME
                enclosed = tvol;
            }           
        }

    } else {  // no input, create the enclosed volume
        if (m_loadMSentry && m_enclosingEnvelopeSvc) {
            const RZPairVector& envelopeDefs = m_enclosingEnvelopeSvc->getCaloRZBoundary();
            // to be implemented in detail - for the moment, take just maximal
            // extent
            ATH_MSG_DEBUG(" m_loadMSentry " << m_loadMSentry
                                            << " m_enclosingEnvelopeSvc "
                                            << m_enclosingEnvelopeSvc);
            double rmax = 0.;
            double zmax = 0.;
            for (const auto& envelopeDef : envelopeDefs) {
                rmax = std::max(envelopeDef.first, rmax);
                zmax = std::max(std::abs(envelopeDef.second), zmax);
            }
            if (!envelopeDefs.empty()) {
                if (rmax > 0. && rmax <= aLVC.m_innerBarrelRadius &&
                    zmax > 0. && zmax <= m_barrelZ) {
                    enclosedBounds = std::make_unique<Trk::CylinderVolumeBounds>(rmax, zmax);
                } else {
                    ATH_MSG_DEBUG( " input MSEntrance size (R,Z:"<< rmax << "," << zmax
                                << ") clashes with MS material, switch to default values (R,Z:"
                                << aLVC.m_innerBarrelRadius << "," << m_barrelZ << ")");
                }
            }
        }

        if (!enclosedBounds) {
            enclosedBounds = std::make_unique<Trk::CylinderVolumeBounds>(aLVC.m_innerBarrelRadius, m_barrelZ);
        }
        {
            enclosed = std::make_unique<Trk::TrackingVolume>(nullptr, enclosedBounds.release(),
                                                             aLVC.m_muonMaterial, nullptr,
                                                             nullptr, m_entryVolume).release();           
            enclosed->registerColorCode(0);
        }
        ATH_MSG_DEBUG(" register Barrel m_entryVolume " << m_entryVolume);
    }

    // construct inner and outer envelope

    for (auto& envelopeDef : envelopeDefs) {
        // ATH_MSG_VERBOSE( "Rz pair:"<< i<<":"<<
        // envelopeDefs[i].first<<","<<envelopeDefs[i].second );
        if (!aLVC.m_msCutoutsIn.empty() &&
            aLVC.m_msCutoutsIn.back().second == -aLVC.m_outerEndcapZ)
            break;
        if (aLVC.m_msCutoutsIn.empty() ||
            std::abs(aLVC.m_msCutoutsIn.back().second) > m_barrelZ ||
            std::abs(envelopeDef.second) > m_barrelZ)
            aLVC.m_msCutoutsIn.push_back(envelopeDef);
        else if (!aLVC.m_msCutoutsIn.empty() &&
                 aLVC.m_msCutoutsIn.back().second == m_barrelZ &&
                 aLVC.m_msCutoutsIn.back().first != aLVC.m_innerBarrelRadius) {
            aLVC.m_msCutoutsIn.emplace_back(aLVC.m_innerBarrelRadius,
                                            m_barrelZ);
            aLVC.m_msCutoutsIn.emplace_back(aLVC.m_innerBarrelRadius,
                                            -m_barrelZ);
            aLVC.m_msCutoutsIn.emplace_back(
                aLVC.m_msCutoutsIn[aLVC.m_msCutoutsIn.size() - 3].first,
                -m_barrelZ);
        }
    }

    unsigned int il = 1;
    while (envelopeDefs[il - 1].second != -aLVC.m_outerEndcapZ)
        il++;
    for (; il < envelopeDefs.size(); il++)
        aLVC.m_msCutoutsOut.push_back(envelopeDefs[il]);

    for (unsigned int i = 0; i < aLVC.m_msCutoutsIn.size(); i++) {
        ATH_MSG_VERBOSE("Rz pair for inner MS envelope:"
                        << i << ":" << aLVC.m_msCutoutsIn[i].first << ","
                        << aLVC.m_msCutoutsIn[i].second);
    }
    for (unsigned int i = 0; i < aLVC.m_msCutoutsOut.size(); i++) {
        ATH_MSG_VERBOSE("Rz pair for outer MS envelope:"
                        << i << ":" << aLVC.m_msCutoutsOut[i].first << ","
                        << aLVC.m_msCutoutsOut[i].second);
    }

    if (aLVC.m_msCutoutsIn[5].second != aLVC.m_innerEndcapZ) {
        aLVC.m_innerEndcapZ = aLVC.m_msCutoutsIn[5].second;
    }
    ATH_MSG_VERBOSE("inner endcap Z set to:" << aLVC.m_innerEndcapZ);

    // create central volume ("enclosed" + disk shields ) - this is to allow
    // safe gluing with 3D MS binning
    getShieldParts(aLVC);

    auto negDiskShieldBounds = std::make_unique<Trk::CylinderVolumeBounds>(aLVC.m_innerBarrelRadius, 
                                                                           0.5 * (m_diskShieldZ - m_barrelZ));
    Trk::Volume negDiskVol(makeTransform(translateZ3D(-0.5 * (m_diskShieldZ + m_barrelZ))),
                           negDiskShieldBounds.release());
    negDiskShield = processShield(negDiskVol, 2, "Muons::Detectors::NegativeDiskShield",
                                  aLVC, hasStations);

    auto posDiskShieldBounds = std::make_unique<Trk::CylinderVolumeBounds>(aLVC.m_innerBarrelRadius, 
                                                                          0.5 * (m_diskShieldZ - m_barrelZ));
    Trk::Volume posDiskVol(makeTransform(translateZ3D(0.5 * (m_diskShieldZ + m_barrelZ))),
                           posDiskShieldBounds.release());
    posDiskShield = processShield(posDiskVol, 2, "Muons::Detectors::PositiveDiskShield",
                      aLVC, hasStations);

    ATH_MSG_DEBUG( "glue enclosed  + disk shields");
    centralP = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*enclosed, 
                                                                 Trk::positiveFaceXY, 
                                                                 *posDiskShield.release(), 
                                                                 Trk::negativeFaceXY,
                                                                 "Container::CentralP"));
    central = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*centralP, 
                                                                Trk::negativeFaceXY, 
                                                                *negDiskShield.release(), 
                                                                Trk::positiveFaceXY,
                                                                "Container::Central"));
    // define basic volumes
    if (aLVC.m_adjustStatic) {
        getZParts(aLVC);
        getHParts(aLVC);
    }

    // muon barrel
    auto barrelBounds = std::make_unique<Trk::CylinderVolumeBounds>(aLVC.m_innerBarrelRadius, 
                                                                    aLVC.m_outerBarrelRadius, 
                                                                    m_diskShieldZ);
    Trk::Volume barrelVol(nullptr, barrelBounds.release());
    // process volume
    // barrel
    if (aLVC.m_adjustStatic && aLVC.m_static3d)
        muonBarrel = processVolume(barrelVol, 0, "Detectors::Barrel",
                                   aLVC, hasStations);
    else if (aLVC.m_adjustStatic)
        muonBarrel = processVolume(barrelVol, -1, "Detectors::Barrel",
                                   aLVC, hasStations);
    else
        muonBarrel = processVolume(barrelVol, m_barrelEtaPartition, m_phiPartition,
                                  "Detectors::Barrel", aLVC, hasStations);
    // inner Endcap
    // build as smallWheel+ECT
    // small wheel
    double smallWheelZHalfSize = 0.5 * (m_ectZ - m_diskShieldZ);
    auto negativeSmallWheelBounds = std::make_unique<Trk::CylinderVolumeBounds>(m_innerShieldRadius, 
                                                                                aLVC.m_outerBarrelRadius, 
                                                                                smallWheelZHalfSize);
  
    Trk::Volume negSWVol(makeTransform(translateZ3D(-m_ectZ + smallWheelZHalfSize)), 
                                       negativeSmallWheelBounds.release());
    if (aLVC.m_adjustStatic && aLVC.m_static3d) {
        negativeMuonSmallWheel = processVolume(negSWVol, 1, "Detectors::NegativeSmallWheel",
                                             aLVC, hasStations);
    } else if (aLVC.m_adjustStatic) {
        negativeMuonSmallWheel = processVolume(negSWVol, -1, "Detectors::NegativeSmallWheel",
                                               aLVC, hasStations);
    } else {
        negativeMuonSmallWheel = processVolume(negSWVol, 
                                               m_innerEndcapEtaPartition, 
                                               m_phiPartition,
                                               "Detectors::NegativeSmallWheel", 
                                               aLVC, hasStations);
    }
    //
    Trk::Volume posSWVol(negSWVol, translateZ3D(2 * (m_ectZ - smallWheelZHalfSize)));
    if (aLVC.m_adjustStatic && aLVC.m_static3d) {
        positiveMuonSmallWheel = processVolume(posSWVol, 1, "Detectors::PositiveSmallWheel",
                                               aLVC, hasStations);
    } else if (aLVC.m_adjustStatic) {
        positiveMuonSmallWheel = processVolume(posSWVol, -1, "Detectors::PositiveSmallWheel",
                                               aLVC, hasStations);
    } else {
        positiveMuonSmallWheel = processVolume(posSWVol, m_innerEndcapEtaPartition, 
                                               m_phiPartition, "Detectors::PositiveSmallWheel", 
                                               aLVC, hasStations);
    }
    // checkVolume(positiveMuonSmallWheel);
    // ECT
    double ectZHalfSize = 0.5 * (aLVC.m_innerEndcapZ - m_ectZ);
    
    auto negativeECTBounds = std::make_unique<Trk::CylinderVolumeBounds>(m_innerShieldRadius, 
                                                                         aLVC.m_outerBarrelRadius, 
                                                                         ectZHalfSize);

    Trk::Volume negECTVol(makeTransform(translateZ3D(-m_ectZ - ectZHalfSize)), 
                          negativeECTBounds.release());
    if (aLVC.m_adjustStatic && aLVC.m_static3d) {
        negativeECT = processVolume(negECTVol, 2, "Detectors::NegativeECT", 
                                    aLVC, hasStations);
    } else if (aLVC.m_adjustStatic) {
        negativeECT = processVolume(negECTVol, -1, "Detectors::NegativeECT", 
                                    aLVC, hasStations);
    } else {
        negativeECT = processVolume(negECTVol, m_innerEndcapEtaPartition, m_phiPartition,
                                     "Detectors::NegativeECT", aLVC, hasStations);
    }
    // checkVolume(negativeECT);
    //
    Trk::Volume posECTVol(negECTVol,
                          translateZ3D(2 * (m_ectZ + ectZHalfSize)));
    if (aLVC.m_adjustStatic && m_static3d) {
        positiveECT = processVolume(posECTVol, 2, "Detectors::PositiveECT", 
                                    aLVC, hasStations);
    } else if (aLVC.m_adjustStatic) {
        positiveECT = processVolume(posECTVol, -1, "Detectors::PositiveECT", 
                                    aLVC, hasStations);
    } else {
        positiveECT = processVolume(posECTVol, m_innerEndcapEtaPartition, 
                                    m_phiPartition, "Detectors::PositiveECT", 
                                    aLVC, hasStations);
    }
    // checkVolume(positiveECT);
    // glue
    negativeMuonInnerEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*negativeECT.release(), 
                                                                                Trk::positiveFaceXY, 
                                                                                *negativeMuonSmallWheel.release(),
                                                                                Trk::negativeFaceXY, 
                                                                                "Container::NegInnerEndcap"));
    positiveMuonInnerEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*positiveMuonSmallWheel.release(), 
                                                                                Trk::positiveFaceXY, 
                                                                                *positiveECT.release(),
                                                                                Trk::negativeFaceXY, 
                                                                                "Container::PosInnerEndcap"));

    // inner shields
    double innerEndcapZHalfSize = 0.5 * (aLVC.m_innerEndcapZ - m_diskShieldZ);
    auto negInnerShieldBounds = std::make_unique<Trk::CylinderVolumeBounds>(m_beamPipeRadius, 
                                                                            m_innerShieldRadius, 
                                                                            innerEndcapZHalfSize);
    Trk::Volume negisVol{makeTransform(translateZ3D(-m_diskShieldZ - innerEndcapZHalfSize)),
                         negInnerShieldBounds.release()};
    negInnerShield = processShield(negisVol, 1, "Muons::Detectors::NegativeInnerShield",
                                   aLVC, hasStations);

    auto posInnerShieldBounds = std::make_unique<Trk::CylinderVolumeBounds>(m_beamPipeRadius, 
                                                                            m_innerShieldRadius, 
                                                                            innerEndcapZHalfSize);
    Trk::Volume posisVol(makeTransform(translateZ3D(m_diskShieldZ + innerEndcapZHalfSize)),
                         posInnerShieldBounds.release());
    posInnerShield = processShield(posisVol, 1, "Muons::Detectors::PositiveInnerShield",
                                   aLVC, hasStations);

    // outer Endcap
    // build as bigWheel+buffer+outerWheel
    // outer wheel
    double outerWheelZHalfSize = 0.5 * (aLVC.m_outerEndcapZ - m_outerWheel);
    auto negativeOuterWheelBounds = std::make_unique<Trk::CylinderVolumeBounds>(m_outerShieldRadius, 
                                                                                aLVC.m_outerBarrelRadius, 
                                                                                outerWheelZHalfSize);
    Trk::Volume negOWVol(makeTransform(translateZ3D(-aLVC.m_outerEndcapZ + 
                                                    outerWheelZHalfSize)), 
                         negativeOuterWheelBounds.release());
    if (aLVC.m_adjustStatic && aLVC.m_static3d) {
        negativeMuonOuterWheel = processVolume(negOWVol, 3, "Detectors::NegativeOuterWheel",
                                               aLVC, hasStations);
    } else if (aLVC.m_adjustStatic) {
        negativeMuonOuterWheel = processVolume(negOWVol, -1, "Detectors::NegativeOuterWheel",
                                               aLVC, hasStations);
    } else {
        negativeMuonOuterWheel = processVolume(negOWVol, m_outerEndcapEtaPartition, 
                                              m_phiPartition, "Detectors::NegativeOuterWheel", 
                                              aLVC, hasStations);
    }
    //
    Trk::Volume posOWVol(negOWVol, 
                         translateZ3D(2 * (aLVC.m_outerEndcapZ - outerWheelZHalfSize)));
    
    if (aLVC.m_adjustStatic && aLVC.m_static3d) {
        positiveMuonOuterWheel = processVolume(posOWVol, 3, "Detectors::PositiveOuterWheel",
                                               aLVC, hasStations);
    } else if (aLVC.m_adjustStatic) {
        positiveMuonOuterWheel = processVolume(posOWVol, -1, "Detectors::PositiveOuterWheel",
                                               aLVC, hasStations);
    } else {
        positiveMuonOuterWheel = processVolume(posOWVol, m_outerEndcapEtaPartition, 
                                              m_phiPartition, "Detectors::PositiveOuterWheel", 
                                              aLVC, hasStations);
    }
    // outer buffer
    double outerBufferZHalfSize = 0.5 * (m_outerWheel - m_bigWheel);
    auto negativeOuterBufferBounds = std::make_unique<Trk::CylinderVolumeBounds>(m_outerShieldRadius, 
                                                                                 aLVC.m_outerBarrelRadius, 
                                                                                 outerBufferZHalfSize);
 
    Trk::Volume negBuffVol(makeTransform(translateZ3D(-m_bigWheel - 
                                                      outerBufferZHalfSize)), 
                           negativeOuterBufferBounds.release());
    if (aLVC.m_adjustStatic && aLVC.m_static3d) {
        negativeMuonOuterBuffer = processVolume(negBuffVol, 3, "Detectors::NegativeOuterBuffer", 
                                                aLVC, hasStations);
    } else if (aLVC.m_adjustStatic) {
        negativeMuonOuterBuffer = processVolume(negBuffVol, -1, "Detectors::NegativeOuterBuffer", 
                                                aLVC, hasStations);
    } else {
        negativeMuonOuterBuffer = processVolume(negBuffVol, m_outerEndcapEtaPartition, 
                                                m_phiPartition, "Detectors::NegativeOuterBuffer", 
                                                aLVC, hasStations);
    }
    //
    Trk::Volume posBuffVol(negBuffVol, translateZ3D(2 *(m_bigWheel + outerBufferZHalfSize)));
    if (aLVC.m_adjustStatic && aLVC.m_static3d) {
        positiveMuonOuterBuffer = processVolume(posBuffVol, 3, "Detectors::PositiveOuterBuffer", 
                                                aLVC, hasStations);
    } else if (aLVC.m_adjustStatic) {
        positiveMuonOuterBuffer = processVolume(posBuffVol, -1, "Detectors::PositiveOuterBuffer", 
                                                aLVC, hasStations);
    }  else {
        positiveMuonOuterBuffer = processVolume(posBuffVol, m_outerEndcapEtaPartition, 
                                                m_phiPartition, "Detectors::PositiveOuterBuffer", 
                                                aLVC, hasStations);
    }
    // big wheel
    double bigWheelZHalfSize = 0.5 * (m_bigWheel - aLVC.m_innerEndcapZ);
    auto negativeBigWheelBounds = std::make_unique<Trk::CylinderVolumeBounds>(m_outerShieldRadius, 
                                                                              aLVC.m_outerBarrelRadius, 
                                                                              bigWheelZHalfSize);
 
    Trk::Volume negBWVol(makeTransform(translateZ3D(-aLVC.m_innerEndcapZ - 
                                                     bigWheelZHalfSize)), 
                        negativeBigWheelBounds.release());
    if (aLVC.m_adjustStatic && aLVC.m_static3d) {
        negativeMuonBigWheel = processVolume(negBWVol, 3, "Detectors::NegativeBigWheel",
                                             aLVC, hasStations);
    } else if (aLVC.m_adjustStatic) {
        negativeMuonBigWheel = processVolume(negBWVol, -1, "Detectors::NegativeBigWheel",
                                             aLVC, hasStations);
    } else {
        negativeMuonBigWheel = processVolume(negBWVol, m_outerEndcapEtaPartition, 
                                             m_phiPartition, "Detectors::NegativeBigWheel", 
                                             aLVC, hasStations);
    }
    //
    Trk::Volume posBWVol(negBWVol, 
                         translateZ3D(2 * (aLVC.m_innerEndcapZ + bigWheelZHalfSize)));
    if (aLVC.m_adjustStatic && aLVC.m_static3d) {
        positiveMuonBigWheel = processVolume(posBWVol, 3, "Detectors::PositiveBigWheel",
                                             aLVC, hasStations);
    } else if (aLVC.m_adjustStatic) {
        positiveMuonBigWheel = processVolume(posBWVol, -1, "Detectors::PositiveBigWheel",
                                             aLVC, hasStations);
    } else {
        positiveMuonBigWheel = processVolume(posBWVol, m_outerEndcapEtaPartition,
                                             m_phiPartition, "Detectors::PositiveBigWheel", 
                                             aLVC, hasStations);
    }
    // glue
    negNavOEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*negativeMuonOuterWheel.release(), 
                                                                      Trk::positiveFaceXY,
                                                                      *negativeMuonOuterBuffer.release(), 
                                                                      Trk::negativeFaceXY,
                                                                      "Container::NegOEndcap"));
    
    posNavOEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*positiveMuonOuterBuffer.release(), 
                                                                      Trk::positiveFaceXY,
                                                                      *positiveMuonOuterWheel.release(), 
                                                                      Trk::negativeFaceXY,
                                                                      "Container::PosOEndcap"));
    
    
    negativeMuonOuterEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*negNavOEndcap, 
                                                                                Trk::positiveFaceXY, 
                                                                                *negativeMuonBigWheel.release(),
                                                                                Trk::negativeFaceXY, 
                                                                                "Container::NegOuterEndcap"));
    
    positiveMuonOuterEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*positiveMuonBigWheel.release(), 
                                                                                Trk::positiveFaceXY, 
                                                                                *posNavOEndcap,
                                                                                Trk::negativeFaceXY, 
                                                                                "Container::PosOuterEndcap"));

    // outer shields
    double outerEndcapZHalfSize = 0.5 * (aLVC.m_outerEndcapZ - aLVC.m_innerEndcapZ);
    double outerEndcapPosition = 0.5 * (aLVC.m_outerEndcapZ + aLVC.m_innerEndcapZ);
    auto negOuterShieldBounds = std::make_unique<Trk::CylinderVolumeBounds>(m_beamPipeRadius, 
                                                                            m_outerShieldRadius, 
                                                                            outerEndcapZHalfSize);
    Trk::Volume negosVol(makeTransform(translateZ3D(-outerEndcapPosition)),
                         negOuterShieldBounds.release());
    negOuterShield = processShield(negosVol, 0, "Muons::Detectors::NegativeOuterShield",
                                   aLVC, hasStations);

    auto posOuterShieldBounds = std::make_unique<Trk::CylinderVolumeBounds>(
        m_beamPipeRadius, m_outerShieldRadius, outerEndcapZHalfSize);
    Trk::Volume pososVol(makeTransform(translateZ3D(outerEndcapPosition)),
                         posOuterShieldBounds.release());
    posOuterShield = processShield(pososVol, 0, "Muons::Detectors::PositiveOuterShield",
                                   aLVC, hasStations);

    // beamPipe
    auto negBeamPipeBounds = std::make_unique<Trk::CylinderVolumeBounds>(m_beamPipeRadius, 
                                                                         outerEndcapZHalfSize + innerEndcapZHalfSize);
    auto posBeamPipeBounds = std::make_unique<Trk::CylinderVolumeBounds>(m_beamPipeRadius, 
                                                                         outerEndcapZHalfSize + innerEndcapZHalfSize);
    Trk::Volume negbpVol(makeTransform(translateZ3D(-aLVC.m_outerEndcapZ +  innerEndcapZHalfSize +  outerEndcapZHalfSize)),
                        negBeamPipeBounds.release());
    negBeamPipe = processVolume(negbpVol, 1, 1, "Muons::Gaps::NegativeBeamPipe", 
                                aLVC, hasStations);
    Trk::Volume posbpVol(makeTransform(translateZ3D(aLVC.m_outerEndcapZ - innerEndcapZHalfSize - outerEndcapZHalfSize)),
                        posBeamPipeBounds.release());
    posBeamPipe = processVolume(posbpVol, 1, 1, "Muons::Gaps::PositiveBeamPipe", 
                                aLVC, hasStations);

    negBeamPipe->registerColorCode(0);
    posBeamPipe->registerColorCode(0);

    ATH_MSG_DEBUG( " volumes defined ");
    //
    // glue volumes at navigation level, create enveloping volume
    // radially
    // central + barrel
    ATH_MSG_DEBUG( "glue barrel+enclosed volumes");
    barrel = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*muonBarrel.release(), 
                                                               Trk::tubeInnerCover, 
                                                               *central, 
                                                               Trk::cylinderCover,
                                                               "All::Container::Barrel"));
    // shield+outerEndcap
    ATH_MSG_DEBUG( "glue shield+outerEndcap");
    negOuterEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*negativeMuonOuterEndcap, 
                                                                       Trk::tubeInnerCover, 
                                                                       *negOuterShield.release(),
                                                                       Trk::tubeOuterCover, 
                                                                       "Container::NegativeOuterEndcap"));

    posOuterEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*positiveMuonOuterEndcap, 
                                                                       Trk::tubeInnerCover, 
                                                                       *posOuterShield.release(),
                                                                       Trk::tubeOuterCover, 
                                                                       "Container::PositiveOuterEndcap"));

    // shield+innerEndcap
    ATH_MSG_DEBUG( "glue shield+innerEndcap");
    negInnerEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*negativeMuonInnerEndcap, 
                                                                       Trk::tubeInnerCover, 
                                                                       *negInnerShield.release(),
                                                                       Trk::tubeOuterCover, 
                                                                       "Container::NegativeInnerEndcap"));
    // checkVolume(negInnerEndcap);
    posInnerEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*positiveMuonInnerEndcap, 
                                                                       Trk::tubeInnerCover, 
                                                                       *posInnerShield.release(),
                                                                       Trk::tubeOuterCover, 
                                                                       "Container::PositiveInnerEndcap"));
    // checkVolume(posInnerEndcap);
    // inner+outerEndcap
    ATH_MSG_DEBUG( "glue inner+outerEndcap");
    negNavEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*negOuterEndcap, 
                                                                     Trk::positiveFaceXY, 
                                                                     *negInnerEndcap,
                                                                     Trk::negativeFaceXY, 
                                                                     "Container::NegativeEndcap"));
   
    posNavEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*posInnerEndcap, 
                                                                     Trk::positiveFaceXY, 
                                                                     *posOuterEndcap,
                                                                     Trk::negativeFaceXY, 
                                                                     "Container::PositiveEndcap"));
    
    // beam pipe + endcaps
    ATH_MSG_DEBUG( "glue beamPipe+endcaps");
    negEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*negNavEndcap, 
                                                                  Trk::tubeInnerCover, 
                                                                  *negBeamPipe.release(),
                                                                  Trk::cylinderCover, 
                                                                  "All::Container::NegativeEndcap"));
    posEndcap = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*posNavEndcap, 
                                                                  Trk::tubeInnerCover, 
                                                                  *posBeamPipe.release(),
                                                                  Trk::cylinderCover, 
                                                                  "All::Container::PositiveEndcap"));
    // checkVolume(negEndcap);
    // checkVolume(posEndcap);
    // barrel + endcaps
    ATH_MSG_DEBUG( "glue barrel+endcaps");

    negDet = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*negEndcap, 
                                                               Trk::positiveFaceXY, 
                                                               *barrel, 
                                                               Trk::negativeFaceXY,
                                                               "All::Container::NegDet"));
    detector = (m_trackingVolumeHelper->glueTrackingVolumeArrays(*posEndcap, 
                                                                 Trk::negativeFaceXY, 
                                                                 *negDet, 
                                                                 Trk::positiveFaceXY,
                                                                 m_exitVolume));
    // blend material
    if (m_blendInertMaterial)
        blendMaterial(aLVC);

    // tracking geometry
    auto trackingGeometry = std::make_unique<Trk::TrackingGeometry>(detector, Trk::globalSearch);


    trackingGeometry->addToGarbage(std::move(stations));
    trackingGeometry->addToGarbage(std::move(inertObjs));

    volumeGarbage.push_back(std::move(muonBarrel));
    volumeGarbage.push_back(std::move(negativeMuonOuterWheel));
    volumeGarbage.push_back(std::move(negativeMuonBigWheel));
    volumeGarbage.push_back(std::move(negativeMuonOuterBuffer));
    volumeGarbage.push_back(std::move(positiveMuonOuterWheel));
    
    volumeGarbage.push_back(std::move(negativeMuonSmallWheel));
    volumeGarbage.push_back(std::move(positiveMuonSmallWheel));
    volumeGarbage.push_back(std::move(negativeECT));
    volumeGarbage.push_back(std::move(positiveECT));
    volumeGarbage.push_back(std::move(positiveMuonBigWheel));
    
    volumeGarbage.push_back(std::move(positiveMuonOuterBuffer));
    volumeGarbage.push_back(std::move(negBeamPipe));
    volumeGarbage.push_back(std::move(posBeamPipe));
    volumeGarbage.push_back(std::move(negDiskShield));
    volumeGarbage.push_back(std::move(posDiskShield));
    
    volumeGarbage.push_back(std::move(negInnerShield));
    volumeGarbage.push_back(std::move(posInnerShield));
    volumeGarbage.push_back(std::move(negOuterShield));
    volumeGarbage.push_back(std::move(posOuterShield));
    volumeGarbage.push_back(std::move(barrelZPBuffer));
    volumeGarbage.push_back(std::move(barrelZMBuffer));
    trackingGeometry->addToGarbage(std::move(volumeGarbage));
    ATH_MSG_DEBUG( " returning tracking geometry ");
    ATH_MSG_DEBUG( " with " << aLVC.m_frameNum << " subvolumes at navigation level");
    ATH_MSG_DEBUG( "( mean number of enclosed detached volumes:" << float(aLVC.m_frameStat) / aLVC.m_frameNum << ")");
    return trackingGeometry;
}

VolumeSpanArray
MuonTrackingGeometryBuilderImpl::findVolumesSpan(const DetachedVolVec& objs,
                                                 double zTol, 
                                                 double phiTol,
                                                 const LocalVariablesContainer& aLVC) const {
    VolumeSpanArray spans{};

    if (objs.empty()) {
        return spans;
    }
    
    for (const auto& obj : objs) {
        VolumeSpanPtr span{m_volumeConverter.findVolumeSpan(obj->trackingVolume()->volumeBounds(), 
                                                            obj->trackingVolume()->transform(), zTol, phiTol)};
        double x0 = obj->trackingVolume()->X0;
        double intX0 = std::abs(span->zMin - span->zMax) / (x0 + 0.000000001);
        double l0 = obj->trackingVolume()->L0;
        ATH_MSG_DEBUG("span:" << obj->name() << "," << span->zMin << ","
                              << span->zMax << "," << span->phiMin << ","
                              << span->phiMax << "," << span->rMin << ","
                              << span->rMax << " X0 " << x0 << " L0 " << l0
                              << " intX0 for span0 span1 " << intX0);

        int nspans = 0;
        // negative outer wheel
        if (span->zMin < -m_bigWheel) {
            spans[0].emplace_back(obj.get(), span);
            nspans++;
        }
        // negative big wheel
        if (span->zMin < -aLVC.m_innerEndcapZ && span->zMax > -m_bigWheel) {
            spans[1].emplace_back(obj.get(), span);
            nspans++;
        }
        // neg.ect
        if (span->zMin < -m_ectZ && span->zMax > -aLVC.m_innerEndcapZ) {
            spans[2].emplace_back(obj.get(), span);
            nspans++;
        }
        // neg.small wheel
        if (span->zMin < -m_diskShieldZ && span->zMax > -m_ectZ) {
            spans[3].emplace_back(obj.get(), span);
            nspans++;
        }
        // barrel
        if (span->zMin < m_diskShieldZ && span->zMax > -m_diskShieldZ) {
            spans[4].emplace_back(obj.get(), span);
            nspans++;
        }
        // pos.small wheel
        if (span->zMin < m_ectZ && span->zMax > m_diskShieldZ) {
            spans[5].emplace_back(obj.get(), span);
            nspans++;
        }
        // pos.ect
        if (span->zMin < aLVC.m_innerEndcapZ && span->zMax > m_ectZ) {
            spans[6].emplace_back(obj.get(), span);
            nspans++;
        }
        // positive big wheel
        if (span->zMin < m_bigWheel && span->zMax > aLVC.m_innerEndcapZ) {
            spans[7].emplace_back(obj.get(), span);
            nspans++;
        }
        // positive outer wheel
        if (span->zMax > m_bigWheel) {
            spans[8].emplace_back(obj.get(), span);
            nspans++;
        }

        if (nspans == 0)
            ATH_MSG_WARNING(" object not selected in span regions "
                            << obj->name());
        if (nspans > 1)
            ATH_MSG_VERBOSE(" object selected in " << nspans << " span regions "
                                                   << obj->name());
    }

    return spans;
}

TrackingVolumePtr MuonTrackingGeometryBuilderImpl::processVolume(const Trk::Volume& vol, 
                                                                 int etaN, int phiN, 
                                                                 const std::string& volumeName,
                                                                 LocalVariablesContainer& aLVC, 
                                                                 bool hasStations) const {
    TrackingVolumePtr tVol{};

    unsigned int colorCode = m_colorCode;

    std::vector<Trk::DetachedTrackingVolume*> blendVols;

    // partitions ? include protection against wrong setup
    if (etaN < 1 || phiN < 1) {
        ATH_MSG_ERROR( "wrong partition setup");
        etaN = 1;
        phiN = 1;
    }
    if (etaN * phiN > 1) {  // partition
        auto cyl = dynamic_cast<const Trk::CylinderVolumeBounds*>(&(vol.volumeBounds()));
        if (!cyl) {
            ATH_MSG_ERROR(" process volume: volume cylinder boundaries not retrieved, return 0 ");
            return nullptr;
        }
        double phiSect = M_PI / phiN;
        double etaSect = (cyl->halflengthZ()) / etaN;

        auto subBds = std::make_unique<Trk::CylinderVolumeBounds>(cyl->innerRadius(), cyl->outerRadius(), phiSect, etaSect);
        auto protVol = std::make_unique<Trk::Volume>(nullptr, subBds.release());

        // create subvolumes & BinnedArray
        std::vector<Trk::TrackingVolumeOrderPosition> subVolumes;
        std::vector<Trk::TrackingVolume*> sVols;     // for gluing
        std::vector<Trk::TrackingVolume*> sVolsNeg;  // for gluing
        std::vector<Trk::TrackingVolume*> sVolsPos;  // for gluing
        for (int eta = 0; eta < etaN; eta++) {
            if (colorCode > 0)
                colorCode = 26 - colorCode;
            // reference point for the check of envelope
            double posZ = vol.center().z() + etaSect * (2. * eta + 1. - etaN);
            double posR = 0.5 * (cyl->innerRadius() + cyl->outerRadius());
            int geoSignature = 4;
            // loop over inner cutouts
            for (unsigned int in = 1; in < aLVC.m_msCutoutsIn.size(); in++) {
                if (posZ >= aLVC.m_msCutoutsIn[in].second &&
                    posZ <= aLVC.m_msCutoutsIn[in - 1].second) {
                    if (posR < aLVC.m_msCutoutsIn[in].first)
                        geoSignature = 2;
                    break;
                }
            }
            if (geoSignature == 4) {
                // loop over outer cutouts
                for (unsigned int io = 1; io < aLVC.m_msCutoutsOut.size();
                     io++) {
                    if (posZ >= aLVC.m_msCutoutsOut[io - 1].second &&
                        posZ <= aLVC.m_msCutoutsOut[io].second) {
                        if (posR > aLVC.m_msCutoutsOut[io].first)
                            geoSignature = 5;
                        break;
                    }
                }
            }
            for (int phi = 0; phi < phiN; phi++) {
                if (colorCode > 0)
                    colorCode = 26 - colorCode;
                // define subvolume
                const Amg::Transform3D transf = Amg::getRotateZ3D(phiSect * (2 * phi + 1)) *
                                                translateZ3D(posZ);
                auto subVol = std::make_unique<Trk::Volume>(*protVol, transf);
                // enclosed muon objects ?
                std::string volName = volumeName + MuonGM::buildString(eta, 2) +
                                      MuonGM::buildString(phi, 2);
                blendVols.clear();
                std::vector<Trk::DetachedTrackingVolume*> detVols{};
                if (hasStations) {
                    detVols = getDetachedObjects(*subVol, blendVols, aLVC);
                }
                auto detVolVecPtr = std::make_unique<std::vector<Trk::DetachedTrackingVolume*>>(detVols);
                auto sVol = std::make_unique<Trk::TrackingVolume>(*subVol, aLVC.m_muonMaterial, 
                                                                  detVolVecPtr.release(), volName);
                // statistics
                ++aLVC.m_frameNum;
                aLVC.m_frameStat += detVols.size();
                // prepare blending
                if (m_blendInertMaterial && !blendVols.empty()) {
                    for (auto& blendVol : blendVols) {
                        aLVC.m_blendMap[blendVol].push_back(sVol.get());
                    }
                }
                //
                if (geoSignature == 2) {
                    sVol->sign(Trk::BeamPipe);
                }
                if (geoSignature == 5) {
                    sVol->sign(Trk::Cavern);
                }
                sVol->registerColorCode(colorCode);
                // reference position
                const Amg::Vector3D gp = cyl->outerRadius() * 
                                         Amg::Vector3D::UnitX();;
                // glue subVolumes
                sVols.push_back(sVol.get());
                if (eta == 0)
                    sVolsNeg.push_back(sVol.get());
                if (eta == etaN - 1)
                    sVolsPos.push_back(sVol.get());
                // in phi
                if (phiN > 1 && phi > 0) {
                    m_trackingVolumeHelper->glueTrackingVolumes(*sVol, 
                                                                Trk::tubeSectorNegativePhi,
                                                                *sVols[eta * phiN + phi - 1],
                                                                Trk::tubeSectorPositivePhi);
                    if (phi == phiN - 1)
                        m_trackingVolumeHelper->glueTrackingVolumes(*sVols[eta * phiN], 
                                                                   Trk::tubeSectorNegativePhi,
                                                                   *sVol, 
                                                                   Trk::tubeSectorPositivePhi);
                }
                // in eta
                if (etaN > 1 && eta > 0)
                    m_trackingVolumeHelper->glueTrackingVolumes(*sVol, 
                                                                Trk::negativeFaceXY,
                                                                *sVols[(eta - 1) * phiN + phi], 
                                                                Trk::positiveFaceXY);
                //
                subVolumes.emplace_back(std::move(sVol), transf * gp);
            }
        }

        Trk::BinUtility buPhi(phiN, -M_PI, M_PI, Trk::closed, Trk::binPhi);
        const Amg::Vector3D volCenter{vol.transform().translation()};
        const Trk::BinUtility buZ(etaN, 
                                  volCenter.z() - cyl->halflengthZ(),
                                  volCenter.z() + cyl->halflengthZ(), 
                                  Trk::open, Trk::binZ);
        buPhi += buZ;

        auto volBinUtil = std::make_unique<Trk::BinUtility>(buPhi); 
        auto subVols = std::make_unique<Trk::BinnedArray2D<Trk::TrackingVolume>>(std::move(subVolumes), 
                                                                                 volBinUtil.release());

        tVol = std::make_unique<Trk::TrackingVolume>(vol, aLVC.m_muonMaterial, nullptr,
                                                     subVols.release(), volumeName);
        // register glue volumes
        Trk::GlueVolumesDescriptor& volGlueVolumes = tVol->glueVolumesDescriptor();
        volGlueVolumes.registerGlueVolumes(Trk::tubeInnerCover, sVols);
        volGlueVolumes.registerGlueVolumes(Trk::tubeOuterCover, sVols);
        volGlueVolumes.registerGlueVolumes(Trk::negativeFaceXY, sVolsNeg);
        volGlueVolumes.registerGlueVolumes(Trk::positiveFaceXY, sVolsPos);

    } else {
        // enclosed muon objects ?
        blendVols.clear();
        std::vector<Trk::DetachedTrackingVolume*> muonObjs{};
        if (hasStations) {
            muonObjs = getDetachedObjects(vol, blendVols, aLVC);
        }
        auto muonObjsPtr = std::make_unique<std::vector<Trk::DetachedTrackingVolume*>>(muonObjs);

        tVol = std::make_unique<Trk::TrackingVolume>(vol, aLVC.m_muonMaterial, muonObjsPtr.release(),
                                                     volumeName);
        // statistics
        ++aLVC.m_frameNum;
        aLVC.m_frameStat += muonObjs.size();
        // prepare blending
        if (m_blendInertMaterial && !blendVols.empty()) {
            for (auto& blendVol : blendVols) {
                aLVC.m_blendMap[blendVol].push_back(tVol.get());
            }
        }
    }

    return tVol;
}

TrackingVolumePtr MuonTrackingGeometryBuilderImpl::processVolume(const Trk::Volume& vol, 
                                                                 int mode, 
                                                                 const std::string& volumeName,
                                                                 LocalVariablesContainer& aLVC, 
                                                                 bool hasStations) const {
    ATH_MSG_VERBOSE( "processing volume in mode:" << mode);

    // mode : -1 ( adjusted z/phi partition )
    //         0 ( -"- plus barrel H binning )
    //         0 ( -"- plus inner endcap H binning )
    //         0 ( -"- plus outer endcap H binning )

   TrackingVolumePtr tVol{};

    unsigned int colorCode = m_colorCode;

    std::vector<Trk::DetachedTrackingVolume*> blendVols;

    // getPartitionFromMaterial(vol);

    // retrieve cylinder
    auto cyl = dynamic_cast<const Trk::CylinderVolumeBounds*>(&(vol.volumeBounds()));
    if (!cyl) {
        ATH_MSG_ERROR(" process volume: volume cylinder boundaries not retrieved, return 0 ");
        return nullptr;
    }
    // create vector of zSteps for this volume
    std::vector<float> zSteps;
    std::vector<int> zTypes;
    double zPos = vol.center().z();
    double hz = cyl->halflengthZ();
    double z1 = zPos - hz;
    double z2 = zPos + hz;
    zSteps.push_back(z1);
    for (unsigned int iz = 0; iz < aLVC.m_zPartitions.size(); iz++) {
        if (aLVC.m_zPartitions[iz] == zSteps.front())
            zTypes.push_back(aLVC.m_zPartitionsType[iz]);
        if (aLVC.m_zPartitions[iz] > z1 && aLVC.m_zPartitions[iz] < z2) {
            zSteps.push_back(aLVC.m_zPartitions[iz]);
            if (zTypes.empty()) {
                if (iz == 0)
                    zTypes.push_back(0);
                else
                    zTypes.push_back(aLVC.m_zPartitionsType[iz - 1]);
            }
            zTypes.push_back(aLVC.m_zPartitionsType[iz]);
            z1 = aLVC.m_zPartitions[iz];
        }
    }
    zSteps.push_back(z2);

    for (unsigned int iz = 0; iz < zSteps.size(); iz++)
        ATH_MSG_DEBUG("z partition in volume:" << volumeName << ":" << iz << ":"
                                               << zSteps[iz]);

    // phi binning
    if (std::abs(zPos) > m_barrelZ &&
        cyl->outerRadius() < aLVC.m_outerBarrelRadius)
        getPhiParts(0, aLVC);
    else if (std::abs(zPos) <= m_ectZ)
        getPhiParts(2, aLVC);
    else if (std::abs(zPos) <= aLVC.m_innerEndcapZ)
        getPhiParts(3, aLVC);
    else if (std::abs(zPos) > m_outerWheel &&
             cyl->outerRadius() > m_outerShieldRadius)
        getPhiParts(1, aLVC);
    else if (std::abs(zPos) > aLVC.m_innerEndcapZ &&
             std::abs(zPos) < m_bigWheel &&
             cyl->outerRadius() > m_outerShieldRadius)
        getPhiParts(1, aLVC);
    else
        getPhiParts(0, aLVC);

    // R/H binning ?
    unsigned int etaN = zSteps.size() - 1;
    unsigned int phiN = aLVC.m_adjustedPhi.size();

    int phiTypeMax = 0;  // count different partitions

    if (mode > -1) {
        // create z,phi bin utilities
        auto zBinUtil = std::make_unique<Trk::BinUtility>(zSteps, Trk::open, Trk::binZ);
        auto pBinUtil = std::make_unique<Trk::BinUtility>(aLVC.m_adjustedPhi, Trk::closed, Trk::binPhi);
        std::vector<std::vector<std::unique_ptr<Trk::BinUtility>> > hBinUtil{};
        for (unsigned iz = 0; iz < zSteps.size() - 1; iz++) {
            std::vector<std::unique_ptr<Trk::BinUtility>> phBinUtil{};
            for (unsigned ip = 0; ip < aLVC.m_adjustedPhi.size(); ip++) {
                // retrieve reference phi
                float phiRef = 0.5 * aLVC.m_adjustedPhi[ip];
                if (ip < aLVC.m_adjustedPhi.size() - 1)
                    phiRef += 0.5 * aLVC.m_adjustedPhi[ip + 1];
                else
                    phiRef += 0.5 * aLVC.m_adjustedPhi[0] + M_PI;

                if (aLVC.m_adjustedPhiType[ip] > phiTypeMax)
                    phiTypeMax = aLVC.m_adjustedPhiType[ip];
                for (std::pair<int, float> i :
                     aLVC.m_hPartitions[mode][zTypes[iz]]
                                       [aLVC.m_adjustedPhiType[ip]]) {
                    ATH_MSG_VERBOSE(" mode " << mode << " phiRef " << phiRef
                                             << " zTypes[iz] " << zTypes[iz]
                                             << " m_adjustedPhiType[ip] "
                                             << aLVC.m_adjustedPhiType[ip]
                                             << " hPartitions " << i.second);
                }
                phBinUtil.push_back(std::make_unique<Trk::BinUtility>(phiRef, 
                                              aLVC.m_hPartitions[mode][zTypes[iz]][aLVC.m_adjustedPhiType[ip]]));
            }
            hBinUtil.push_back(std::move(phBinUtil));
        }

        // create subvolumes & BinnedArray
        std::vector<Trk::TrackingVolumeOrderPosition> subVolumesVect;
        std::vector<std::vector<std::vector<Trk::TrackingVolume*>>> subVolumes;
        std::vector<std::vector<std::shared_ptr<Trk::BinnedArray<Trk::TrackingVolume> > > >
            hBins;
        std::vector<Trk::TrackingVolume*> sVolsInn;  // for gluing
        std::vector<Trk::TrackingVolume*> sVolsOut;  // for gluing
        std::vector<Trk::TrackingVolume*> sVolsNeg;  // for gluing
        std::vector<Trk::TrackingVolume*> sVolsPos;  // for gluing
        for (unsigned int eta = 0; eta < zSteps.size() - 1; eta++) {
            if (colorCode > 0) {
                colorCode = 6 - colorCode;
            }
            double posZ = 0.5 * (zSteps[eta] + zSteps[eta + 1]);
            double hZ = 0.5 * std::abs(zSteps[eta + 1] - zSteps[eta]);
            std::vector<std::vector<Trk::TrackingVolume*> > phiSubs;
            std::vector<std::shared_ptr<Trk::BinnedArray<Trk::TrackingVolume>>> phBins;
            std::vector<int> phiType(phiTypeMax + 1, -1);  
            std::vector<std::vector<Trk::Volume*> > garbVol(phiTypeMax + 1);
            unsigned int pCode = 1;
            for (unsigned int phi = 0; phi < phiN; phi++) {
                pCode = (colorCode > 0) ? 3 - pCode : 0;
                double posPhi = 0.5 * aLVC.m_adjustedPhi[phi];
                double phiSect = 0.;
                if (phi < phiN - 1) {
                    posPhi += 0.5 * aLVC.m_adjustedPhi[phi + 1];
                    phiSect = 0.5 * std::abs(aLVC.m_adjustedPhi[phi + 1] -
                                             aLVC.m_adjustedPhi[phi]);
                } else {
                    posPhi += 0.5 * aLVC.m_adjustedPhi[0] + M_PI;
                    phiSect = 0.5 * std::abs(aLVC.m_adjustedPhi[0] + 2 * M_PI -
                                             aLVC.m_adjustedPhi[phi]);
                }
                std::vector<std::pair<int, float> > hSteps =
                    aLVC.m_hPartitions[mode][zTypes[eta]]
                                      [aLVC.m_adjustedPhiType[phi]];
                std::vector<Trk::TrackingVolume*> hSubs;
                std::vector<Trk::TrackingVolumeOrderPosition> hSubsTr;
                int phiP = phiType[aLVC.m_adjustedPhiType[phi]];

                unsigned int hCode = 1;
                for (unsigned int h = 0; h < hSteps.size() - 1; h++) {
                    hCode = colorCode > 0 ? 1 - hCode : 0;
                    // similar volume may exist already
                    std::unique_ptr<Trk::Volume> subVol{};
                    const Amg::Transform3D transf = Amg::getRotateZ3D(posPhi) * translateZ3D(posZ); 
                    //
                    int volType = 0;  // cylinder
                    if (hSteps[h].first == 1 && hSteps[h + 1].first == 0)
                        volType = 1;
                    if (hSteps[h].first == 0 && hSteps[h + 1].first == 1)
                        volType = 2;
                    if (hSteps[h].first == 1 && hSteps[h + 1].first == 1)
                        volType = 3;
                    // define subvolume
                    if (phiP > -1) {
                        subVol = std::make_unique<Trk::Volume>(*phiSubs[phiP][h],
                                                               transf *phiSubs[phiP][h]->transform().inverse());
                    } else if (phiSect < 0.5 * M_PI) {
                        auto subBds = std::make_unique<Trk::BevelledCylinderVolumeBounds>(hSteps[h].second, 
                                                                                          hSteps[h + 1].second, 
                                                                                          phiSect,
                                                                                          hZ, volType);
                        subVol = std::make_unique<Trk::Volume>(makeTransform(transf), subBds.release());
                    } else {
                        auto subBds = std::make_unique<Trk::CylinderVolumeBounds>(hSteps[h].second,
                                                                                  hSteps[h + 1].second,
                                                                                  phiSect, hZ);
                        subVol = std::make_unique<Trk::Volume>(makeTransform(transf), subBds.release());
                    }

                    // enclosed muon objects ? also adjusts material properties
                    // in case of material blend
                    std::string volName = volumeName + 
                                          MuonGM::buildString(eta, 2) +
                                          MuonGM::buildString(phi, 2) + 
                                          MuonGM::buildString(h, 2);
                    blendVols.clear();
                    std::vector<Trk::DetachedTrackingVolume*> detVols{};
                    if (hasStations) {
                        detVols = getDetachedObjects(*subVol, blendVols, aLVC);
                    }
                    auto detVolsPtr = std::make_unique<std::vector<Trk::DetachedTrackingVolume*>>(detVols);
                    auto sVol = std::make_unique<Trk::TrackingVolume>(*subVol, 
                                                                      aLVC.m_muonMaterial, 
                                                                      detVolsPtr.release(), 
                                                                      volName);

                    // statistics
                    ++aLVC.m_frameNum;
                    aLVC.m_frameStat += detVols.size();
                    // prepare blending
                    if (m_blendInertMaterial && !blendVols.empty()) {
                        for (auto& blendVol : blendVols) {
                            aLVC.m_blendMap[blendVol].push_back(sVol.get());
                        }
                    }
                    // reference point for the check of envelope
                    double posR = 0.5 * (hSteps[h].second + hSteps[h + 1].second);
                    // loop over inner cutouts
                    for (unsigned int in = 1; in < aLVC.m_msCutoutsIn.size(); ++in) {
                        if (posZ >= aLVC.m_msCutoutsIn[in].second &&
                            posZ <= aLVC.m_msCutoutsIn[in - 1].second) {
                            if (posR < aLVC.m_msCutoutsIn[in].first) {
                                sVol->sign(Trk::BeamPipe);
                            }
                            break;
                        }
                    }
                    // loop over outer cutouts
                    for (unsigned int io = 1; io < aLVC.m_msCutoutsOut.size(); ++io) {
                        if (posZ >= aLVC.m_msCutoutsOut[io - 1].second &&
                            posZ <= aLVC.m_msCutoutsOut[io].second) {
                            if (posR > aLVC.m_msCutoutsOut[io].first){
                                sVol->sign(Trk::Cavern);
                            }
                            break;
                        }
                    }
                    //
                    sVol->registerColorCode(colorCode + pCode + hCode);
                    // reference position
                    const Amg::Vector3D gp = 
                            0.5 * (hSteps[h].second + hSteps[h + 1].second) * Amg::Vector3D::UnitX();
                    hSubs.push_back(sVol.get());
  
                    // glue subVolume
                    if (h == 0)
                        sVolsInn.push_back(sVol.get());
                    if (h == hSteps.size() - 2)
                        sVolsOut.push_back(sVol.get());
                    if (eta == 0)
                        sVolsNeg.push_back(sVol.get());
                    if (eta == etaN - 1)
                        sVolsPos.push_back(sVol.get());
                    // in R/H
                    if (h > 0) {                             // glue 'manually'
                        if (volType == 1 || volType == 3) {  // plane surface
                            m_trackingVolumeHelper->setOutsideTrackingVolume(*sVol, 
                                                                             Trk::tubeSectorInnerCover, 
                                                                             hSubs[h - 1]);
                            m_trackingVolumeHelper->setOutsideTrackingVolume(*hSubs[h - 1], 
                                                                             Trk::tubeSectorOuterCover,
                                                                             sVol.get());
                        } else {  // cylinder surface
                            m_trackingVolumeHelper->setInsideTrackingVolume(*sVol, 
                                                                            Trk::tubeSectorInnerCover, 
                                                                            hSubs[h - 1]);
                            m_trackingVolumeHelper->setOutsideTrackingVolume(*hSubs[h - 1], 
                                                                             Trk::tubeSectorOuterCover,
                                                                             sVol.get());
                        }
                    }
                    // in phi
                    if (phiN > 1 && phi > 0) {
                        m_trackingVolumeHelper->setOutsideTrackingVolumeArray(*sVol, 
                                                                              Trk::tubeSectorNegativePhi, 
                                                                              phBins[phi - 1]);
                        if (phi == phiN - 1){
                            m_trackingVolumeHelper->setOutsideTrackingVolumeArray(*sVol, 
                                                                                  Trk::tubeSectorPositivePhi,
                                                                                  phBins[0]);
                        }
                    }
                    // in eta
                    if (etaN > 1 && eta > 0) {
                        m_trackingVolumeHelper->setOutsideTrackingVolumeArray(*sVol, 
                                                                              Trk::negativeFaceXY, 
                                                                              hBins[eta - 1][phi]);
                    }
                    //
                    subVolumesVect.emplace_back(std::move(sVol), transf * gp);
                    hSubsTr.emplace_back(subVolumesVect.back());
                }
                phiSubs.push_back(hSubs);
                auto volBinArray = std::make_unique<Trk::BinnedArray1D<Trk::TrackingVolume>>(hSubsTr, 
                                                                                             hBinUtil[eta][phi]->clone());
                phBins.emplace_back(std::move(volBinArray));
                // save link to current partition for cloning
                if (phiP < 0)
                    phiType[aLVC.m_adjustedPhiType[phi]] = phi;

                // finish phi gluing
                if (phiN > 1 && phi > 0) {
                    for (auto& j : phiSubs[phi - 1]) {
                        m_trackingVolumeHelper->setOutsideTrackingVolumeArray(*j, 
                                                                              Trk::tubeSectorPositivePhi,
                                                                              phBins[phi]);
                    }
                }
                if (phiN > 1 && phi == phiN - 1) {
                    for (auto& j : phiSubs[0]) {
                        m_trackingVolumeHelper->setOutsideTrackingVolumeArray(*j, 
                                                                              Trk::tubeSectorNegativePhi, 
                                                                              phBins[phi]);
                    }
                }
                // finish eta gluing
                if (etaN > 1 && eta > 0) {
                    for (auto& j: subVolumes[eta - 1][phi]) {
                        m_trackingVolumeHelper->setOutsideTrackingVolumeArray(*j, Trk::positiveFaceXY,
                                                                              phBins[phi]);
                    }
                }
            }
            subVolumes.push_back(phiSubs);
            hBins.push_back(phBins);
        }

        auto hBinVecPtr = std::make_unique<std::vector<std::vector<Trk::BinUtility*>>>(Muon::release(hBinUtil));
        auto subVols = std::make_unique<Trk::BinnedArray1D1D1D<Trk::TrackingVolume>>(subVolumesVect, 
                                                                                     zBinUtil.release(), 
                                                                                     pBinUtil.release(), 
                                                                                     hBinVecPtr.release());

        tVol = std::make_unique<Trk::TrackingVolume>(vol, aLVC.m_muonMaterial, nullptr,
                                                     subVols.release(), volumeName);
        // register glue volumes
        Trk::GlueVolumesDescriptor& volGlueVolumes = tVol->glueVolumesDescriptor();
        volGlueVolumes.registerGlueVolumes(Trk::tubeInnerCover, sVolsInn);
        volGlueVolumes.registerGlueVolumes(Trk::tubeOuterCover, sVolsOut);
        volGlueVolumes.registerGlueVolumes(Trk::negativeFaceXY, sVolsNeg);
        volGlueVolumes.registerGlueVolumes(Trk::positiveFaceXY, sVolsPos);

        return tVol;
    }

    // proceed with 2D z/phi binning
    // partitions ? include protection against wrong setup
    if (phiN < 1) {
        ATH_MSG_ERROR( "wrong partition setup");
        phiN = 1;
    } else {
        ATH_MSG_VERBOSE("partition setup:(z,phi):" << etaN << "," << phiN);
    }

    if (etaN * phiN > 1) {  // partition
        // subvolume boundaries

        // create subvolumes & BinnedArray
        std::vector<Trk::TrackingVolumeOrderPosition> subVolumes(etaN * phiN);
        std::vector<Trk::TrackingVolume*> sVols(etaN * phiN);  // for gluing
        std::vector<Trk::TrackingVolume*> sVolsNeg(phiN);      // for gluing
        std::vector<Trk::TrackingVolume*> sVolsPos(phiN);      // for gluing
        for (unsigned int eta = 0; eta < zSteps.size() - 1; ++eta) {
            double posZ = 0.5 * (zSteps[eta] + zSteps[eta + 1]);
            double hZ = 0.5 * std::abs(zSteps[eta + 1] - zSteps[eta]);
            colorCode = 26 - colorCode;
            for (unsigned int phi = 0; phi < phiN; phi++) {
                colorCode = 26 - colorCode;
                double posPhi = 0.5 * aLVC.m_adjustedPhi[phi];
                double phiSect = 0.;
                if (phi < phiN - 1) {
                    posPhi += 0.5 * aLVC.m_adjustedPhi[phi + 1];
                    phiSect = 0.5 * std::abs(aLVC.m_adjustedPhi[phi + 1] -
                                             aLVC.m_adjustedPhi[phi]);
                } else {
                    posPhi += 0.5 * aLVC.m_adjustedPhi[0] + M_PI;
                    phiSect = 0.5 * std::abs(aLVC.m_adjustedPhi[0] + 2 * M_PI -
                                             aLVC.m_adjustedPhi[phi]);
                }
                // define subvolume
                auto subBds = std::make_unique<Trk::CylinderVolumeBounds>(cyl->innerRadius(), cyl->outerRadius(), phiSect, hZ);
                const Amg::Transform3D transf = Amg::getRotateZ3D(posPhi) *
                                                translateZ3D(posZ);
                Trk::Volume subVol(makeTransform(transf), subBds.release());
                // enclosed muon objects ?
                std::string volName = volumeName + MuonGM::buildString(eta, 2) +
                                      MuonGM::buildString(phi, 2);

                Trk::Material mat = aLVC.m_muonMaterial;
                blendVols.clear();
                std::vector<Trk::DetachedTrackingVolume*> detVols{} ;
                if (hasStations) {
                    detVols = getDetachedObjects(subVol, blendVols, aLVC);
                }
                auto detVolPtr = std::make_unique<std::vector<Trk::DetachedTrackingVolume*>>(detVols);
                auto sVol = std::make_unique<Trk::TrackingVolume>(subVol, aLVC.m_muonMaterial, 
                                                                  detVolPtr.release(), 
                                                                  volName);
                // statistics
                ++aLVC.m_frameNum;
                aLVC.m_frameStat += detVols.size();
                // prepare blending
                if (m_blendInertMaterial && !blendVols.empty()) {
                    for (auto& blendVol : blendVols) {
                        aLVC.m_blendMap[blendVol].push_back(sVol.get());
                    }
                }
                // reference point for the check of envelope
                double posR = 0.5 * (cyl->innerRadius() + cyl->outerRadius());
                // loop over inner cutouts
                for (unsigned int in = 1; in < aLVC.m_msCutoutsIn.size(); ++in) {
                    if (posZ >= aLVC.m_msCutoutsIn[in].second &&
                        posZ <= aLVC.m_msCutoutsIn[in - 1].second) {
                        if (posR < aLVC.m_msCutoutsIn[in].first)
                            sVol->sign(Trk::BeamPipe);
                        break;
                    }
                }
                // loop over outer cutouts
                for (unsigned int io = 1; io < aLVC.m_msCutoutsOut.size(); ++io) {
                    if (posZ >= aLVC.m_msCutoutsOut[io - 1].second &&
                        posZ <= aLVC.m_msCutoutsOut[io].second) {
                        if (posR > aLVC.m_msCutoutsOut[io].first)
                            sVol->sign(Trk::Cavern);
                        break;
                    }
                }
                sVol->registerColorCode(colorCode);
                // reference position
                const  Amg::Vector3D gp = cyl->outerRadius() * Amg::Vector3D::UnitX();
                // glue subVolumes
                // sVols[phi*etaN+eta] = sVol;
                sVols[phiN * eta + phi] = sVol.get();
                if (eta == 0){
                    sVolsNeg[phi] = sVol.get();
                }
                if (eta == etaN - 1) {
                    sVolsPos[phi] = sVol.get();
                }
                // in phi
                if (phiN > 1 && phi > 0) {
                    m_trackingVolumeHelper->glueTrackingVolumes(*sVol, 
                                                                Trk::tubeSectorNegativePhi,
                                                                *sVols[eta * phiN + phi - 1],
                                                                Trk::tubeSectorPositivePhi);
                    if (phi == phiN - 1) {
                        m_trackingVolumeHelper->glueTrackingVolumes(*sVols[eta * phiN], 
                                                                    Trk::tubeSectorNegativePhi,
                                                                    *sVol, 
                                                                    Trk::tubeSectorPositivePhi);
                    }
                }
                // in eta
                if (etaN > 1 && eta > 0) {
                    m_trackingVolumeHelper->glueTrackingVolumes(*sVol, 
                                                                Trk::negativeFaceXY,
                                                                *sVols[(eta - 1) * phiN + phi], 
                                                                Trk::positiveFaceXY);
                }
                //
                subVolumes[phi * etaN + eta] = std::make_pair(std::move(sVol), transf * gp);

            }
        }

        // Trk::BinUtility2DZF* volBinUtil=new
        // Trk::BinUtility2DZF(zSteps,m_adjustedPhi,new
        // Amg::Transform3D(vol->transform()));
        Trk::BinUtility zBinUtil(zSteps, Trk::BinningOption::open,
                                 Trk::BinningValue::binZ);
        const Trk::BinUtility pBinUtil(aLVC.m_adjustedPhi,
                                       Trk::BinningOption::closed,
                                       Trk::BinningValue::binPhi);

        zBinUtil += pBinUtil;

        auto volBinUtil = std::make_unique<Trk::BinUtility>(zBinUtil);  // TODO verify ordering PhiZ vs. ZPhi

        auto subVols = std::make_unique<Trk::BinnedArray2D<Trk::TrackingVolume>>(subVolumes, 
                                                                                 volBinUtil.release());

        tVol = std::make_unique<Trk::TrackingVolume>(vol, aLVC.m_muonMaterial, nullptr,
                                                     subVols.release(), volumeName);
        // register glue volumes
        Trk::GlueVolumesDescriptor& volGlueVolumes = tVol->glueVolumesDescriptor();
        volGlueVolumes.registerGlueVolumes(Trk::tubeInnerCover, sVols);
        volGlueVolumes.registerGlueVolumes(Trk::tubeOuterCover, sVols);
        volGlueVolumes.registerGlueVolumes(Trk::negativeFaceXY, sVolsNeg);
        volGlueVolumes.registerGlueVolumes(Trk::positiveFaceXY, sVolsPos);

    } else {
        // enclosed muon objects ?
        blendVols.clear();
        std::vector<Trk::DetachedTrackingVolume*> muonObjs{};
        if (hasStations) {
            muonObjs = getDetachedObjects(vol, blendVols, aLVC);
        }
        auto muonObjPtr = std::make_unique<std::vector<Trk::DetachedTrackingVolume*>>(muonObjs);
        tVol = std::make_unique<Trk::TrackingVolume>(vol, aLVC.m_muonMaterial, 
                                                     muonObjPtr.release(),
                                                     volumeName);
        // statistics
        ++aLVC.m_frameNum;
        aLVC.m_frameStat += muonObjs.size();
        // prepare blending
        if (m_blendInertMaterial && !blendVols.empty()) {
            for (auto& blendVol : blendVols) {
                aLVC.m_blendMap[blendVol].push_back(tVol.get());
            }
        }
    }

    return tVol;
}

TrackingVolumePtr MuonTrackingGeometryBuilderImpl::processShield(const Trk::Volume& vol, 
                                                                 int type, 
                                                                 const std::string& volumeName,
                                                                 LocalVariablesContainer& aLVC, 
                                                                 bool hasStations) const {
    ATH_MSG_VERBOSE( "processing shield volume " << volumeName
                           << "  in mode:" << type);

    TrackingVolumePtr tVol{};

    unsigned int colorCode = m_colorCode;

    std::vector<Trk::DetachedTrackingVolume*> blendVols;

    // getPartitionFromMaterial(vol);

    // retrieve cylinder
    auto cyl = dynamic_cast<const Trk::CylinderVolumeBounds*>(&(vol.volumeBounds()));
    if (!cyl) {
        ATH_MSG_ERROR(" process volume: volume cylinder boundaries not retrieved, return 0 ");
        return nullptr;
    }
    // create vector of zSteps for this volume
    std::vector<float> zSteps;
    zSteps.clear();
    double zPos = vol.center().z();
    double hz = cyl->halflengthZ();
    double z1 = zPos - hz;
    double z2 = zPos + hz;
    zSteps.push_back(z1);
    for (double iz : aLVC.m_shieldZPart) {
        if (iz > z1 && iz < z2) {
            zSteps.push_back(iz);
            z1 = iz;
        }
    }
    zSteps.push_back(z2);

    // phi binning trivial
    aLVC.m_adjustedPhi.clear();
    aLVC.m_adjustedPhi.push_back(0.);

    unsigned int etaN = zSteps.size() - 1;

    // create z,h bin utilities
    auto zBinUtil = std::make_unique<Trk::BinUtility>(zSteps, Trk::BinningOption::open, 
                                                      Trk::BinningValue::binZ);
    auto pBinUtil = std::make_unique<Trk::BinUtility>(1, -M_PI, M_PI, Trk::BinningOption::closed, 
                                                      Trk::BinningValue::binPhi);
    std::vector<std::vector<std::unique_ptr<Trk::BinUtility>> > hBinUtil{};
    float phiRef = 0.;
    for (unsigned iz = 0; iz < zSteps.size() - 1; iz++) {
        std::vector<std::unique_ptr<Trk::BinUtility>> phBinUtil;
        phBinUtil.push_back(std::make_unique<Trk::BinUtility>(phiRef, aLVC.m_shieldHPart[type]));
        hBinUtil.push_back(std::move(phBinUtil));
    }


    // create subvolumes & BinnedArray
    std::vector<Trk::TrackingVolumeOrderPosition> subVolumesVect;
    std::vector<std::vector<std::vector<Trk::TrackingVolume*>>> subVolumes;
    std::vector<std::vector<std::shared_ptr<Trk::BinnedArray<Trk::TrackingVolume>>>> hBins;
    std::vector<Trk::TrackingVolume*> sVolsInn;  // for gluing
    std::vector<Trk::TrackingVolume*> sVolsOut;  // for gluing
    std::vector<Trk::TrackingVolume*> sVolsNeg;  // for gluing
    std::vector<Trk::TrackingVolume*> sVolsPos;  // for gluing
    for (unsigned int eta = 0; eta < zSteps.size() - 1; eta++) {
        if (colorCode > 0)
            colorCode = 26 - colorCode;
        double posZ = 0.5 * (zSteps[eta] + zSteps[eta + 1]);
        double hZ = 0.5 * std::abs(zSteps[eta + 1] - zSteps[eta]);
        std::vector<std::vector<Trk::TrackingVolume*> > phiSubs;
        std::vector<std::shared_ptr<Trk::BinnedArray<Trk::TrackingVolume>>> phBins{};
        int phi = 0;
        double posPhi = 0.;
        double phiSect = M_PI;
        std::vector<std::pair<int, float> > hSteps = aLVC.m_shieldHPart[type];
        std::vector<Trk::TrackingVolume*> hSubs;
        std::vector<Trk::TrackingVolumeOrderPosition> hSubsTr;
        unsigned int hCode = 1;
        for (unsigned int h = 0; h < hSteps.size() - 1; h++) {
            hCode = (colorCode > 0) ? 1 - hCode : 0;
            // define subvolume
            auto subBds = std::make_unique<Trk::CylinderVolumeBounds>(hSteps[h].second, hSteps[h + 1].second, phiSect, hZ);
            const double mediumRadius = subBds->mediumRadius();
            Amg::Transform3D transf = Amg::getRotateZ3D(posPhi) * translateZ3D(posZ);
            Trk::Volume subVol(makeTransform(transf), subBds.release());

            // enclosed muon objects ? also adjusts material properties in case
            // of material blend
            std::string volName = volumeName + MuonGM::buildString(eta, 2) +
                                  MuonGM::buildString(phi, 2) +
                                  MuonGM::buildString(h, 2);
            blendVols.clear();
            std::vector<Trk::DetachedTrackingVolume*> detVols{};
            if (hasStations) {
                detVols = getDetachedObjects(subVol, blendVols, aLVC);
            }
            auto detVolPtr = std::make_unique<std::vector<Trk::DetachedTrackingVolume*>>(detVols);
            auto sVol = std::make_unique<Trk::TrackingVolume>(subVol, aLVC.m_muonMaterial, 
                                                              detVolPtr.release(), volName);

            // statistics
            ++aLVC.m_frameNum;
            aLVC.m_frameStat += detVols.size();
            // prepare blending
            if (m_blendInertMaterial && !blendVols.empty()) {
                for (auto& blendVol : blendVols) {
                    aLVC.m_blendMap[blendVol].push_back(sVol.get());
                }
            }
            // reference point for the check of envelope
            double posR = 0.5 * (hSteps[h].second + hSteps[h + 1].second);
            // loop over inner cutouts
            for (unsigned int in = 1; in < aLVC.m_msCutoutsIn.size(); in++) {
                if (posZ >= aLVC.m_msCutoutsIn[in].second &&
                    posZ <= aLVC.m_msCutoutsIn[in - 1].second) {
                    if (posR < aLVC.m_msCutoutsIn[in].first)
                        sVol->sign(Trk::BeamPipe);
                    break;
                }
            }
            //
            sVol->registerColorCode(colorCode + hCode);
            // reference position
            const Amg::Vector3D gp =  mediumRadius * Amg::Vector3D::UnitX();
            hSubs.push_back(sVol.get());

            // glue subVolume
            if (h == 0)
                sVolsInn.push_back(sVol.get());
            if (h == hSteps.size() - 2)
                sVolsOut.push_back(sVol.get());
            if (eta == 0)
                sVolsNeg.push_back(sVol.get());
            if (eta == etaN - 1)
                sVolsPos.push_back(sVol.get());
            // in R/H
            if (h > 0) {  // glue 'manually'
                m_trackingVolumeHelper->setInsideTrackingVolume(*sVol, 
                                                                Trk::tubeSectorInnerCover, 
                                                                hSubs[h - 1]);
                m_trackingVolumeHelper->setOutsideTrackingVolume(*hSubs[h - 1], 
                                                                 Trk::tubeSectorOuterCover, 
                                                                 sVol.get());
            }
            // in eta
            if (etaN > 1 && eta > 0)
                m_trackingVolumeHelper->setOutsideTrackingVolumeArray(*sVol, 
                                                                      Trk::negativeFaceXY, 
                                                                      hBins[eta - 1][phi]);
   
            subVolumesVect.emplace_back(std::move(sVol), transf * gp);
            hSubsTr.emplace_back(subVolumesVect.back());
   
        }
        phiSubs.push_back(hSubs);
        /// Fix me
        auto volBinArray = std::make_unique<Trk::BinnedArray1D<Trk::TrackingVolume>>(hSubsTr, hBinUtil[eta][phi]->clone());
        phBins.push_back(std::move(volBinArray));

        // finish eta gluing
        if (etaN > 1 && eta > 0) {
            for (auto& j : subVolumes[eta - 1][phi]) {
                m_trackingVolumeHelper->setOutsideTrackingVolumeArray(*j, 
                                                                      Trk::positiveFaceXY,
                                                                      phBins[phi]);
            }
        }
        subVolumes.push_back(phiSubs);
        hBins.push_back(phBins);
    }

    auto hBinVecPtr = std::make_unique<std::vector<std::vector<Trk::BinUtility*>>>(Muon::release(hBinUtil));
    auto subVols = std::make_unique<Trk::BinnedArray1D1D1D<Trk::TrackingVolume>>(subVolumesVect, 
                                                                                 zBinUtil.release(), 
                                                                                 pBinUtil.release(), 
                                                                                 hBinVecPtr.release());

    tVol = std::make_unique<Trk::TrackingVolume>(vol, aLVC.m_muonMaterial, nullptr, 
                                                 subVols.release(), volumeName);
    // register glue volumes
    Trk::GlueVolumesDescriptor& volGlueVolumes = tVol->glueVolumesDescriptor();
    volGlueVolumes.registerGlueVolumes(Trk::tubeInnerCover, sVolsInn);
    volGlueVolumes.registerGlueVolumes(Trk::tubeOuterCover, sVolsOut);
    volGlueVolumes.registerGlueVolumes(Trk::negativeFaceXY, sVolsNeg);
    volGlueVolumes.registerGlueVolumes(Trk::positiveFaceXY, sVolsPos);

    return tVol;
}

std::vector<Trk::DetachedTrackingVolume*>
MuonTrackingGeometryBuilderImpl::getDetachedObjects(const Trk::Volume& vol,
                                                    std::vector<Trk::DetachedTrackingVolume*>& blendVols,
                                                    LocalVariablesContainer& aLVC, 
                                                    int mode) const {
    // mode : 0 all, 1 active only, 2 inert only

    std::vector<Trk::DetachedTrackingVolume*> detTVs{};

    // get min/max Z/Phi from volume (allways a cylinder/bevelled cylinder
    // volume )
    auto cyl = dynamic_cast<const Trk::CylinderVolumeBounds*>(&(vol.volumeBounds()));
    auto bcyl = dynamic_cast<const Trk::BevelledCylinderVolumeBounds*>(&(vol.volumeBounds()));

    double rmed{0.}, dphi{0.}, hz{0.}, rMin{0.}, rMax{0.}, rMaxc{0.};
    int type = 0;
    if (cyl) {
        rmed = cyl->mediumRadius();
        dphi = cyl->halfPhiSector();
        hz = cyl->halflengthZ();
        rMin = cyl->innerRadius();
        rMax = cyl->outerRadius();
        rMaxc = rMax;
    } else if (bcyl) {
        rmed = bcyl->mediumRadius();
        dphi = bcyl->halfPhiSector();
        hz = bcyl->halflengthZ();
        rMin = bcyl->innerRadius();
        rMax = bcyl->outerRadius();
        rMaxc = rMax;
        type = bcyl->type();
        if (type > 1)
            rMaxc *= 1. / cos(dphi);
    } else {
        return detTVs;
    }
    const Amg::Vector3D center = vol.transform() * (rmed *Amg::Vector3D::UnitX());

    double zMin = center[2] - hz;
    double zMax = center[2] + hz;
    double pMin = 0.;
    double pMax = +2 * M_PI;
    bool phiLim = false;
    if (dphi < M_PI) {
        pMin = center.phi() - dphi + M_PI;
        pMax = center.phi() + dphi + M_PI;
        phiLim = true;
    }

    ATH_MSG_VERBOSE(" zMin " << zMin << " zMax " << zMax << " rMin " << rMin
                             << " rMax " << rMax << " rMaxc " << rMaxc
                             << " phi limits " << pMin << " phiMax " << pMax
                             << " phiLim " << phiLim);

    // define detector region : can extend over several
    int gMin = (zMax <= -m_bigWheel) ? 0 : 1;
    if (zMin >= m_bigWheel)
        gMin = 8;
    else if (zMin >= aLVC.m_innerEndcapZ)
        gMin = 7;
    else if (zMin >= m_ectZ)
        gMin = 6;
    else if (zMin >= m_diskShieldZ)
        gMin = 5;
    else if (zMin >= -m_diskShieldZ)
        gMin = 4;
    else if (zMin >= -m_ectZ)
        gMin = 3;
    else if (zMin >= -aLVC.m_innerEndcapZ)
        gMin = 2;
    int gMax = (zMax >= m_bigWheel) ? 8 : 7;
    if (zMax <= -m_bigWheel)
        gMax = 0;
    else if (zMax <= -aLVC.m_innerEndcapZ)
        gMax = 1;
    else if (zMax <= -m_ectZ)
        gMax = 2;
    else if (zMax <= -m_diskShieldZ)
        gMax = 3;
    else if (zMax <= m_diskShieldZ)
        gMax = 4;
    else if (zMax <= m_ectZ)
        gMax = 5;
    else if (zMax <= aLVC.m_innerEndcapZ)
        gMax = 6;

    ATH_MSG_VERBOSE(" active volumes gMin " << gMin << " gMax " << gMax);

    // active, use corrected rMax
    if (mode < 2 && aLVC.m_stationSpan.size()) {
        for (int gMode = gMin; gMode <= gMax; gMode++) {
            for (const auto&[station, s] : (aLVC.m_stationSpan)[gMode]) {
                bool rLimit = !aLVC.m_static3d || (s->rMin <= rMaxc && s->rMax >= rMin);
                // Check meanZ for BME stations
                bool meanZOK = false;
                if (station->name() == "BME1_Station" ||
                    station->name() == "BME2_Station") {
                    if ((s->zMin + s->zMax) / 2. < zMax &&
                        (s->zMin + s->zMax) / 2. > zMin)
                        meanZOK = true;
                    if ((s->phiMin + s->phiMax) / 2 < pMin && phiLim)
                        meanZOK = false;
                    // if ((s->phiMin + s->phiMax) / 2 < pMin && phiLim)
                    // meanZOK = false;
                }
                if (rLimit &&
                    ((s->zMin < zMax && s->zMax > zMin) || meanZOK)) {
                    bool accepted = false;
                    if (phiLim) {
                        if (pMin >= 0 && pMax <= 2 * M_PI) {
                            if (s->phiMin <= s->phiMax &&
                                s->phiMin <= pMax && s->phiMax >= pMin)
                                accepted = true;
                            if (s->phiMin > s->phiMax &&
                                (s->phiMin <= pMax || s->phiMax >= pMin))
                                accepted = true;
                        } else if (pMin < 0) {
                            if (s->phiMin <= s->phiMax &&
                                (s->phiMin <= pMax ||
                                 s->phiMax >= pMin + 2 * M_PI))
                                accepted = true;
                            if (s->phiMin > s->phiMax)
                                accepted = true;
                        } else if (pMax > 2 * M_PI) {
                            if (s->phiMin <= s->phiMax &&
                                (s->phiMin <= pMax - 2 * M_PI ||
                                 s->phiMax >= pMin))
                                accepted = true;
                            if (s->phiMin > s->phiMax)
                                accepted = true;
                        }
                    } else
                        accepted = true;
                    if (meanZOK)
                        accepted = true;
                    if (accepted) {
                        detTVs.push_back(station);
                        ATH_MSG_VERBOSE(" active volume accepted by rLimit "
                                        << station->name() << " zMin " << zMin
                                        << " zMax " << zMax << " rMin " << rMin
                                        << " rMax " << rMax << " PhiMin "
                                        << pMin << " PhiMax " << pMax);
                    }
                } 
            }
        }
    }
    // passive
    if (mode != 1 && aLVC.m_inertSpan.size()) {
        for (int gMode = gMin; gMode <= gMax; gMode++) {
            for (const auto& [inert, s]:  (aLVC.m_inertSpan)[gMode]) {
                bool rLimit = (!aLVC.m_static3d ||
                               (s->rMin <= rMaxc && s->rMax >= rMin));
                if (rLimit && s->zMin < zMax && s->zMax > zMin) {
                    bool accepted = false;
                    if (phiLim) {
                        if (pMin >= 0 && pMax <= 2 * M_PI) {
                            if (s->phiMin <= s->phiMax &&
                                s->phiMin <= pMax && s->phiMax >= pMin)
                                accepted = true;
                            if (s->phiMin > s->phiMax &&
                                (s->phiMin <= pMax || s->phiMax >= pMin))
                                accepted = true;
                        } else if (pMin < 0) {
                            if (s->phiMin <= s->phiMax &&
                                (s->phiMin <= pMax ||
                                 s->phiMax >= pMin + 2 * M_PI))
                                accepted = true;
                            if (s->phiMin > s->phiMax)
                                accepted = true;
                        } else if (pMax > 2 * M_PI) {
                            if (s->phiMin <= s->phiMax &&
                                (s->phiMin <= pMax - 2 * M_PI ||
                                 s->phiMax >= pMin))
                                accepted = true;
                            if (s->phiMin > s->phiMax)
                                accepted = true;
                        }
                    } else
                        accepted = true;
                    if (accepted) {
                        bool perm =
                            inert->name().compare(inert->name().size() - 4, 4,
                                                  "PERM") == 0;
                        if (!m_blendInertMaterial || !m_removeBlended || perm)
                            detTVs.push_back(inert);
                        if (m_blendInertMaterial && !perm)
                            blendVols.push_back(inert);
                        ATH_MSG_VERBOSE(" Inert volume accepted by rLimit "
                                        << inert->name() << " zMin " << zMin
                                        << " zMax " << zMax << " rMin " << rMin
                                        << " rMax " << rMax << " PhiMin "
                                        << pMin << " PhiMax " << pMax);
                    }
                }
            }
        }
    }
    return detTVs;
}

bool MuonTrackingGeometryBuilderImpl::enclosed(const Trk::Volume& vol, 
                                               const Trk::VolumeSpan& span,
                                               LocalVariablesContainer& aLVC) const {
    bool encl = false;
    constexpr double tol = 1.;
    constexpr double ptol = 0.11;  // 0.08 for BT, 0.11 feet

    // get min/max Z/Phi from volume (allways a cylinder/bevelled cylinder
    // volume )
    auto cyl = dynamic_cast<const Trk::CylinderVolumeBounds*>(&(vol.volumeBounds()));
    auto bcyl =dynamic_cast<const Trk::BevelledCylinderVolumeBounds*>(&(vol.volumeBounds()));

    double rmed{0.}, dphi{0.}, hz{0.}, rMin{0.}, rMax{0.};
    if (cyl) {
        rmed = cyl->mediumRadius();
        dphi = cyl->halfPhiSector();
        hz = cyl->halflengthZ();
        rMin = cyl->innerRadius();
        rMax = cyl->outerRadius();
    } else if (bcyl) {
        rmed = bcyl->mediumRadius();
        dphi = bcyl->halfPhiSector();
        hz = bcyl->halflengthZ();
        rMin = bcyl->innerRadius();
        rMax = bcyl->outerRadius();
    } else
        return false;

    const Amg::Vector3D center = vol.transform() * (rmed* Amg::Vector3D::UnitX());

    double zMin = center[2] - hz;
    double zMax = center[2] + hz;
    double pMin = 0.;
    double pMax = +2 * M_PI;
    bool phiLim = false;
    if (dphi < M_PI) {
        pMin = center.phi() - dphi + M_PI;
        pMax = center.phi() + dphi + M_PI;
        phiLim = true;
    }
    //
    ATH_MSG_VERBOSE("enclosing volume:z:" << zMin << "," << zMax
                                          << ":r:" << rMin << "," << rMax
                                          << ":phi:" << pMin << "," << pMax);
    //
    bool rLimit = (!aLVC.m_static3d ||
                   (span.rMin < rMax - tol && span.rMax > rMin + tol));
    if (rLimit && span.zMin < zMax - tol && span.zMax > zMin + tol) {
        if (phiLim) {
            if (pMin >= 0 && pMax <= 2 * M_PI) {
                if (span.phiMin <= span.phiMax && span.phiMin < pMax + ptol &&
                    span.phiMax > pMin - ptol)
                    return true;
                if (span.phiMin > span.phiMax &&
                    (span.phiMin < pMax - ptol || span.phiMax > pMin + ptol))
                    return true;
            } else if (pMin < 0) {
                if (span.phiMin <= span.phiMax &&
                    (span.phiMin < pMax + ptol ||
                     span.phiMax > pMin - ptol + 2 * M_PI))
                    return true;
                if (span.phiMin > span.phiMax)
                    return true;
            } else if (pMax > 2 * M_PI) {
                if (span.phiMin <= span.phiMax &&
                    (span.phiMin < pMax + ptol - 2 * M_PI ||
                     span.phiMax > pMin - ptol))
                    return true;
                if (span.phiMin > span.phiMax)
                    return true;
            }
        } else {
            return true;
        }
    }
    return encl;
}
void MuonTrackingGeometryBuilderImpl::getZParts(
    LocalVariablesContainer& aLVC) const {
    // activeAdjustLevel:  1: separate MDT stations
    //                        +(inertLevel=0) barrel Z partition
    //                     2: split TGC
    //                        +(inertLevel=0) barrel R partition
    //                     3: split TGC supports
    // inertAdjustLevel:   1: BT,ECT

    // hardcode for the moment
    aLVC.m_zPartitions.clear();
    aLVC.m_zPartitionsType.clear();
    aLVC.m_zPartitions.reserve(120);
    aLVC.m_zPartitionsType.reserve(120);

    // outer endcap
    aLVC.m_zPartitions.push_back(-aLVC.m_outerEndcapZ);
    aLVC.m_zPartitionsType.push_back(1);  // EO
    aLVC.m_zPartitions.push_back(-23001.);
    aLVC.m_zPartitionsType.push_back(1);  // oute envelope change
    // if (m_activeAdjustLevel>0) { m_zPartitions.push_back(-21630.);
    // m_zPartitionsType.push_back(1); }   // EOL
    aLVC.m_zPartitions.push_back(-22030.);
    aLVC.m_zPartitionsType.push_back(1);  // EOL
    aLVC.m_zPartitions.push_back(-m_outerWheel);
    aLVC.m_zPartitionsType.push_back(0);  // Octogon
    // m_zPartitions.push_back(-17990.); m_zPartitionsType.push_back(0);   //
    // buffer
    aLVC.m_zPartitions.push_back(-18650.);
    aLVC.m_zPartitionsType.push_back(0);  // buffer
    aLVC.m_zPartitions.push_back(-m_bigWheel);
    aLVC.m_zPartitionsType.push_back(1);  // TGC3
    if (m_activeAdjustLevel > 2) {
        aLVC.m_zPartitions.push_back(-15225.);
        aLVC.m_zPartitionsType.push_back(1);
    }
    if (m_activeAdjustLevel > 1) {
        aLVC.m_zPartitions.push_back(-15172.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // end supp
    if (m_activeAdjustLevel > 2) {
        aLVC.m_zPartitions.push_back(-15128.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // supp
    if (m_activeAdjustLevel > 2) {
        aLVC.m_zPartitions.push_back(-15070.);
        aLVC.m_zPartitionsType.push_back(1);
    }
    if (m_activeAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-14940.);
        aLVC.m_zPartitionsType.push_back(1);
    }  //
    if (m_activeAdjustLevel > 2) {
        aLVC.m_zPartitions.push_back(-14805.);
        aLVC.m_zPartitionsType.push_back(1);
    }
    if (m_activeAdjustLevel > 1) {
        aLVC.m_zPartitions.push_back(-14733.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // end supp.
    if (m_activeAdjustLevel > 2) {
        aLVC.m_zPartitions.push_back(-14708.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // supp.
    if (m_activeAdjustLevel > 2) {
        aLVC.m_zPartitions.push_back(-14650.);
        aLVC.m_zPartitionsType.push_back(1);
    }  //
    if (m_activeAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-14560.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // EML
    if (m_activeAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-14080.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // EMS
    if (m_activeAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-13620.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // TGC
    if (m_activeAdjustLevel > 2) {
        aLVC.m_zPartitions.push_back(-13525.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // TGC
    if (m_activeAdjustLevel > 1) {
        aLVC.m_zPartitions.push_back(-13448.5);
        aLVC.m_zPartitionsType.push_back(1);
    }  // end supp.
    if (m_activeAdjustLevel > 2) {
        aLVC.m_zPartitions.push_back(-13421.5);
        aLVC.m_zPartitionsType.push_back(1);
    }  // supp.
    if (m_activeAdjustLevel > 2) {
        aLVC.m_zPartitions.push_back(-13346);
        aLVC.m_zPartitionsType.push_back(1);
    }  // TGC

    // inner endcap
    aLVC.m_zPartitions.push_back(-aLVC.m_innerEndcapZ);
    aLVC.m_zPartitionsType.push_back(0);  //
    if (m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-12790);
        aLVC.m_zPartitionsType.push_back(0);
    }  // ECT
    if (m_inertAdjustLevel > 1) {
        aLVC.m_zPartitions.push_back(-12100.);
        aLVC.m_zPartitionsType.push_back(0);
    }  //
    if (m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-12000.);
        aLVC.m_zPartitionsType.push_back(0);
    }  //
    if (m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-11210.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // BT
    if (m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-10480.);
        aLVC.m_zPartitionsType.push_back(0);
    }  //
    if (m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-9700.);
        aLVC.m_zPartitionsType.push_back(0);
    }  //
    if (m_inertAdjustLevel > 1) {
        aLVC.m_zPartitions.push_back(-9300.);
        aLVC.m_zPartitionsType.push_back(0);
    }  // rib
    if (m_inertAdjustLevel > 1) {
        aLVC.m_zPartitions.push_back(-8800.);
        aLVC.m_zPartitionsType.push_back(0);
    }  // ect
    if (m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-8610.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // BT
    if (m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-8000.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // BT
    aLVC.m_zPartitions.push_back(-m_ectZ);
    aLVC.m_zPartitionsType.push_back(0);  // ECT/small wheel
    if (m_activeAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-7450.);
        aLVC.m_zPartitionsType.push_back(0);
    }  // EIS
    if (m_activeAdjustLevel > 2) {
        aLVC.m_zPartitions.push_back(-7364.);
        aLVC.m_zPartitionsType.push_back(0);
    }  // EIS
    if (m_activeAdjustLevel > 0 || m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-7170.);
        aLVC.m_zPartitionsType.push_back(0);
    }  // cone assembly,TGC
    if (m_activeAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-7030.);
        aLVC.m_zPartitionsType.push_back(0);
    }  // TGC
    if (m_activeAdjustLevel > 2) {
        aLVC.m_zPartitions.push_back(-6978.);
        aLVC.m_zPartitionsType.push_back(0);
    }  // TGC

    // barrel
    aLVC.m_zPartitions.push_back(-m_diskShieldZ);
    aLVC.m_zPartitionsType.push_back(0);  // disk
    if (m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-6829.);
        aLVC.m_zPartitionsType.push_back(0);
    }  // back disk
    // if (m_inertAdjustLevel>1) { (*m_zPartitions).push_back(-6600.);
    // m_zPartitionsType.push_back(0); }   //
    aLVC.m_zPartitions.push_back(-6550.);
    aLVC.m_zPartitionsType.push_back(0);  // outer envelope change
    if (m_activeAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-6100.);
        aLVC.m_zPartitionsType.push_back(0);
    }
    if (m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-5503.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // BT
    if (m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-4772.);
        aLVC.m_zPartitionsType.push_back(0);
    }  //
    if (m_activeAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-4300.);
        aLVC.m_zPartitionsType.push_back(0);
    }  //
    aLVC.m_zPartitions.push_back(-4000.);
    aLVC.m_zPartitionsType.push_back(0);  // outer envelope change
    if (m_inertAdjustLevel > 1) {
        aLVC.m_zPartitions.push_back(-3700.);
        aLVC.m_zPartitionsType.push_back(0);
    }  //
    if (m_inertAdjustLevel > 1) {
        aLVC.m_zPartitions.push_back(-3300.);
        aLVC.m_zPartitionsType.push_back(0);
    }  //
    if (m_activeAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-2600.);
        aLVC.m_zPartitionsType.push_back(0);
    }  //
    if (m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-2078.);
        aLVC.m_zPartitionsType.push_back(1);
    }  // BT
    if (m_inertAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-1347.);
        aLVC.m_zPartitionsType.push_back(1);
    }  //  cryoring
    if (m_activeAdjustLevel > 0) {
        aLVC.m_zPartitions.push_back(-800.);
        aLVC.m_zPartitionsType.push_back(1);
    }  //  cryoring
    if (m_inertAdjustLevel > 1) {
        aLVC.m_zPartitions.push_back(-300.);
        aLVC.m_zPartitionsType.push_back(0);
    }  //
    if (static_cast<int>(m_inertAdjustLevel) +
            static_cast<int>(m_activeAdjustLevel) <
        1) {
        aLVC.m_zPartitions.push_back(-0.7 * m_diskShieldZ);
        aLVC.m_zPartitionsType.push_back(0);
    }  //

    unsigned int zSiz = aLVC.m_zPartitions.size();
    for (unsigned int i = 0; i < zSiz; i++) {
        aLVC.m_zPartitions.push_back(-aLVC.m_zPartitions[zSiz - 1 - i]);
        if (i < zSiz - 1)
            aLVC.m_zPartitionsType.push_back(
                aLVC.m_zPartitionsType[zSiz - 2 - i]);
    }
}

void MuonTrackingGeometryBuilderImpl::getPhiParts(
    int mode, LocalVariablesContainer& aLVC) const {
    if (mode == 0) {  // trivial
        aLVC.m_adjustedPhi.clear();
        aLVC.m_adjustedPhiType.clear();
        aLVC.m_adjustedPhi.push_back(0.);
        aLVC.m_adjustedPhiType.push_back(0);

    } else if (mode == 1) {
        int phiNum = 1;
        if (m_activeAdjustLevel > 0)
            phiNum = m_phiPartition;
        aLVC.m_adjustedPhi.resize(phiNum);
        aLVC.m_adjustedPhiType.resize(phiNum);
        aLVC.m_adjustedPhi[0] = 0.;
        aLVC.m_adjustedPhiType[0] = 0;
        int ic = 0;
        while (ic < phiNum - 1) {
            ic++;
            aLVC.m_adjustedPhi[ic] =
                aLVC.m_adjustedPhi[ic - 1] + 2. * M_PI / phiNum;
            aLVC.m_adjustedPhiType[ic] = 0;
        }

    } else if (mode == 2) {  // barrel(BT)
        // hardcode for the moment
        aLVC.m_adjustedPhi.resize(16);
        aLVC.m_adjustedPhiType.resize(16);

        double phiSect[2];
        phiSect[0] = (M_PI / 8 - 0.105);
        phiSect[1] = 0.105;

        aLVC.m_adjustedPhi[0] = -phiSect[0];
        aLVC.m_adjustedPhiType[0] = 0;
        int ic = 0;
        int is = 1;

        while (ic < 15) {
            ic++;
            is = 1 - is;
            aLVC.m_adjustedPhi[ic] =
                aLVC.m_adjustedPhi[ic - 1] + 2 * phiSect[is];
            aLVC.m_adjustedPhiType[ic] = 1 - is;
        }

    } else if (mode == 3) {  // ECT(+BT)
        // hardcode for the moment
        aLVC.m_adjustedPhi.resize(32);
        aLVC.m_adjustedPhiType.resize(32);

        double phiSect[3];
        phiSect[0] = 0.126;
        phiSect[2] = 0.105;
        phiSect[1] = 0.5 * (M_PI / 8. - phiSect[0] - phiSect[2]);

        aLVC.m_adjustedPhi[0] = -phiSect[0];
        aLVC.m_adjustedPhiType[0] = 0;
        aLVC.m_adjustedPhi[1] = aLVC.m_adjustedPhi[0] + 2 * phiSect[0];
        aLVC.m_adjustedPhiType[1] = 1;
        int ic = 1;
        int is = 0;

        while (ic < 31) {
            ic++;
            is = 2 - is;
            aLVC.m_adjustedPhi[ic] =
                aLVC.m_adjustedPhi[ic - 1] + 2 * phiSect[1];
            aLVC.m_adjustedPhiType[ic] = is;
            ic++;
            aLVC.m_adjustedPhi[ic] =
                aLVC.m_adjustedPhi[ic - 1] + 2 * phiSect[is];
            aLVC.m_adjustedPhiType[ic] = 1;
        }
    }
}

void MuonTrackingGeometryBuilderImpl::getHParts(
    LocalVariablesContainer& aLVC) const {
    // hardcode for the moment
    aLVC.m_hPartitions.clear();  // barrel, inner endcap, outer endcap

    // 0: barrel 2x2
    // non BT sector
    std::vector<std::pair<int, float> > barrelZ0F0;
    barrelZ0F0.emplace_back(0, aLVC.m_innerBarrelRadius);
    if (m_activeAdjustLevel > 0) {
        barrelZ0F0.emplace_back(0, 4450.);   // for DiskShieldingBackDisk
        barrelZ0F0.emplace_back(0, 6500.);   // BI/BM
        barrelZ0F0.emplace_back(0, 8900.);   // BM/BO
        barrelZ0F0.emplace_back(0, 13000.);  // outer envelope
    }
    barrelZ0F0.emplace_back(0, aLVC.m_outerBarrelRadius);

    std::vector<std::pair<int, float> > barrelZ0F1;
    barrelZ0F1.emplace_back(0, aLVC.m_innerBarrelRadius);
    if (m_inertAdjustLevel > 0) {
        barrelZ0F1.emplace_back(1, 4500.);
        barrelZ0F1.emplace_back(1, 5900.);
    } else if (m_activeAdjustLevel > 0)
        barrelZ0F1.emplace_back(0, 4450.);
    if (m_activeAdjustLevel > 0)
        barrelZ0F1.emplace_back(0, 6500.);
    if (m_inertAdjustLevel > 0)
        barrelZ0F1.emplace_back(1, 8900.);
    else if (m_activeAdjustLevel > 0)
        barrelZ0F1.emplace_back(0, 8900.);
    if (m_inertAdjustLevel > 0)
        barrelZ0F1.emplace_back(1, 10100.);
    barrelZ0F1.emplace_back(0, 13000.);  // outer envelope
    barrelZ0F1.emplace_back(0, aLVC.m_outerBarrelRadius);

    // BT sector
    std::vector<std::pair<int, float> > barrelZ1F0;
    barrelZ1F0.emplace_back(0, aLVC.m_innerBarrelRadius);
    if (static_cast<int>(m_activeAdjustLevel) +
            static_cast<int>(m_inertAdjustLevel) >
        0)
        barrelZ1F0.emplace_back(0, 4450.);
    if (m_inertAdjustLevel > 0) {
        barrelZ1F0.emplace_back(1, 5800.);
        barrelZ1F0.emplace_back(1, 6500.);
    } else if (m_activeAdjustLevel > 0)
        barrelZ1F0.emplace_back(0, 6500.);
    if (m_inertAdjustLevel > 0) {
        barrelZ1F0.emplace_back(1, 6750.);
        barrelZ1F0.emplace_back(1, 8400.);
    }
    if (m_activeAdjustLevel > 0)
        barrelZ1F0.emplace_back(0, 8770.);  // adapted for cryoring (from 8900)
    if (m_inertAdjustLevel > 0)
        barrelZ1F0.emplace_back(1, 9850.);  // adapted for cryoring (from 9600)
    barrelZ1F0.emplace_back(0, 13000.);     // outer envelope
    barrelZ1F0.emplace_back(0, aLVC.m_outerBarrelRadius);

    std::vector<std::pair<int, float> > barrelZ1F1;
    barrelZ1F1.emplace_back(0, aLVC.m_innerBarrelRadius);
    if (m_inertAdjustLevel > 0) {
        barrelZ1F1.emplace_back(1, 4500.);
        barrelZ1F1.emplace_back(1, 6000.);
    } else if (m_activeAdjustLevel > 0)
        barrelZ1F1.emplace_back(0, 4450.);
    if (m_activeAdjustLevel > 0)
        barrelZ1F1.emplace_back(0, 6500.);
    if (m_inertAdjustLevel > 0)
        barrelZ1F1.emplace_back(1, 6800.);
    if (m_inertAdjustLevel > 0) {
        barrelZ1F1.emplace_back(1, 8900.);
        barrelZ1F1.emplace_back(1, 10100.);
    } else if (m_activeAdjustLevel > 0)
        barrelZ1F1.emplace_back(0, 8900.);
    barrelZ1F1.emplace_back(0, 13000.);  // outer envelope
    barrelZ1F1.emplace_back(0, aLVC.m_outerBarrelRadius);

    std::vector<std::vector<std::vector<std::pair<int, float> > > > barrelZF(2);
    barrelZF[0].push_back(barrelZ0F0);
    barrelZF[0].push_back(barrelZ0F1);
    barrelZF[1].push_back(barrelZ1F0);
    barrelZF[1].push_back(barrelZ1F1);

    // small wheel 1x2 ( no z BT sector)
    // non BT sector
    std::vector<std::pair<int, float> > swZ0F0;
    swZ0F0.emplace_back(0, m_innerShieldRadius);
    if (m_activeAdjustLevel > 1) {
        swZ0F0.emplace_back(0, 2700.);
    }
    if (static_cast<int>(m_activeAdjustLevel) +
            static_cast<int>(m_inertAdjustLevel) >
        0)
        swZ0F0.emplace_back(0, 4450.);
    if (m_activeAdjustLevel > 0) {
        swZ0F0.emplace_back(0, 6560.);  // BI/BM
        swZ0F0.emplace_back(0, 8900.);  // BM/BO
    }
    swZ0F0.emplace_back(0, aLVC.m_outerBarrelRadius);

    // phi BT sector
    std::vector<std::pair<int, float> > swZ0F1;
    swZ0F1.emplace_back(0, m_innerShieldRadius);
    if (m_activeAdjustLevel > 1)
        swZ0F1.emplace_back(0, 2700.);
    if (static_cast<int>(m_inertAdjustLevel) +
            static_cast<int>(m_activeAdjustLevel) >
        0)
        swZ0F1.emplace_back(0, 4450.);
    if (m_inertAdjustLevel > 0)
        swZ0F1.emplace_back(1, 5900.);
    if (m_activeAdjustLevel > 0)
        swZ0F1.emplace_back(0, 6560.);
    if (m_inertAdjustLevel > 0) {
        swZ0F1.emplace_back(1, 8900.);
        swZ0F1.emplace_back(1, 10100.);
    } else if (m_activeAdjustLevel > 0)
        swZ0F1.emplace_back(0, 8900.);
    swZ0F1.emplace_back(0, aLVC.m_outerBarrelRadius);

    std::vector<std::vector<std::vector<std::pair<int, float> > > > swZF(1);
    swZF[0].push_back(swZ0F0);
    swZF[0].push_back(swZ0F1);

    // inner endcap/ECT 2x3
    // ect coil, non-BT z
    std::vector<std::pair<int, float> > innerZ0F0;
    innerZ0F0.emplace_back(0, m_innerShieldRadius);
    if (m_inertAdjustLevel > 0)
        innerZ0F0.emplace_back(0, 1100.);
    if (m_inertAdjustLevel > 1)
        innerZ0F0.emplace_back(1, 5150.);
    if (m_inertAdjustLevel > 0)
        innerZ0F0.emplace_back(1, 5300.);
    if (m_activeAdjustLevel > 0) {
        innerZ0F0.emplace_back(0, 6500.);
        innerZ0F0.emplace_back(0, 8900.);
    }
    innerZ0F0.emplace_back(0, aLVC.m_outerBarrelRadius);

    // coil gap, non-BT z
    std::vector<std::pair<int, float> > innerZ0F1;
    innerZ0F1.emplace_back(0, m_innerShieldRadius);
    if (m_inertAdjustLevel > 0)
        innerZ0F1.emplace_back(0, 1100.);
    if (m_inertAdjustLevel > 1) {
        innerZ0F1.emplace_back(1, 1400.);
        innerZ0F1.emplace_back(1, 1685.);
    }
    if (m_inertAdjustLevel > 0) {
        innerZ0F1.emplace_back(1, 4700.);
        innerZ0F1.emplace_back(1, 5900.);
    }
    if (m_activeAdjustLevel > 0) {
        innerZ0F1.emplace_back(0, 6500.);
        innerZ0F1.emplace_back(0, 8900.);
    }
    innerZ0F1.emplace_back(0, aLVC.m_outerBarrelRadius);

    // BT coil, no-BT z
    std::vector<std::pair<int, float> > innerZ0F2;
    innerZ0F2.emplace_back(0, m_innerShieldRadius);
    if (m_inertAdjustLevel > 0)
        innerZ0F2.emplace_back(0, 1100.);
    if (m_inertAdjustLevel > 1) {
        innerZ0F2.emplace_back(1, 1400.);
        innerZ0F2.emplace_back(1, 1685.);
    }
    if (m_inertAdjustLevel > 0) {
        innerZ0F2.emplace_back(1, 4450.);
        innerZ0F2.emplace_back(1, 5900.);
    }
    if (m_activeAdjustLevel > 0)
        innerZ0F2.emplace_back(0, 6500.);
    if (m_inertAdjustLevel > 0) {
        innerZ0F2.emplace_back(1, 8900.);
        innerZ0F2.emplace_back(1, 10100.);
    } else if (m_activeAdjustLevel > 0)
        innerZ0F2.emplace_back(0, 8900.);
    innerZ0F2.emplace_back(0, aLVC.m_outerBarrelRadius);

    // ect coil, z BT sector
    std::vector<std::pair<int, float> > innerZ1F0;
    innerZ1F0.emplace_back(0, m_innerShieldRadius);
    if (m_inertAdjustLevel > 0)
        innerZ1F0.emplace_back(0, 1100.);
    if (m_inertAdjustLevel > 1)
        innerZ1F0.emplace_back(1, 5150.);
    if (m_inertAdjustLevel > 0)
        innerZ1F0.emplace_back(1, 5300.);
    if (m_inertAdjustLevel > 0)
        innerZ1F0.emplace_back(1, 5800.);
    if (m_inertAdjustLevel > 0)
        innerZ1F0.emplace_back(1, 6750.);
    else if (m_activeAdjustLevel > 0)
        innerZ1F0.emplace_back(0, 6500.);
    if (m_inertAdjustLevel > 0) {
        innerZ1F0.emplace_back(1, 8400.);
        innerZ1F0.emplace_back(1, 9600.);
    } else if (m_activeAdjustLevel > 0)
        innerZ1F0.emplace_back(0, 8900.);
    innerZ1F0.emplace_back(0, aLVC.m_outerBarrelRadius);

    // coil gap, BT z sector
    std::vector<std::pair<int, float> > innerZ1F1;
    innerZ1F1.emplace_back(0, m_innerShieldRadius);
    if (m_inertAdjustLevel > 0)
        innerZ1F1.emplace_back(0, 1100.);
    if (m_inertAdjustLevel > 1) {
        innerZ1F1.emplace_back(1, 1400.);
        innerZ1F1.emplace_back(1, 1685.);
    }
    if (m_inertAdjustLevel > 0) {
        innerZ1F1.emplace_back(1, 4700.);
        innerZ1F1.emplace_back(1, 5800.);
        innerZ1F1.emplace_back(1, 6750.);
    } else if (m_activeAdjustLevel > 0)
        innerZ1F1.emplace_back(0, 6500.);
    if (m_inertAdjustLevel > 0) {
        innerZ1F1.emplace_back(1, 8400.);
        innerZ1F1.emplace_back(1, 9600.);
    } else if (m_activeAdjustLevel > 0)
        innerZ1F1.emplace_back(0, 8900.);
    innerZ1F1.emplace_back(0, aLVC.m_outerBarrelRadius);

    // BT coil, BT z sector
    std::vector<std::pair<int, float> > innerZ1F2;
    innerZ1F2.emplace_back(0, m_innerShieldRadius);
    if (m_inertAdjustLevel > 0)
        innerZ1F2.emplace_back(0, 1100.);
    if (m_inertAdjustLevel > 1) {
        innerZ1F2.emplace_back(1, 1400.);
        innerZ1F2.emplace_back(1, 1685.);
    }
    innerZ1F2.emplace_back(0, 4150.);
    if (m_inertAdjustLevel > 0) {
        innerZ1F2.emplace_back(1, 4700.);
        innerZ1F2.emplace_back(1, 5900.);
        innerZ1F2.emplace_back(1, 6800.);
    } else if (m_activeAdjustLevel > 0)
        innerZ1F2.emplace_back(0, 6500.);
    if (m_inertAdjustLevel > 0) {
        innerZ1F2.emplace_back(1, 8900.);
        innerZ1F2.emplace_back(1, 10100.);
    } else if (m_activeAdjustLevel > 0)
        innerZ1F2.emplace_back(0, 8900.);
    innerZ1F2.emplace_back(0, aLVC.m_outerBarrelRadius);

    std::vector<std::vector<std::vector<std::pair<int, float> > > > innerZF(2);
    innerZF[0].push_back(innerZ0F0);
    innerZF[0].push_back(innerZ0F1);
    innerZF[0].push_back(innerZ0F2);
    innerZF[1].push_back(innerZ1F0);
    innerZF[1].push_back(innerZ1F1);
    innerZF[1].push_back(innerZ1F2);

    // outer 1x1
    std::vector<std::pair<int, float> > outerZ0F0;
    outerZ0F0.emplace_back(0, m_outerShieldRadius);
    outerZ0F0.emplace_back(0, 2750.);   // outer envelope
    outerZ0F0.emplace_back(0, 12650.);  // outer envelope
    outerZ0F0.emplace_back(0, 13400.);  // outer envelope
    outerZ0F0.emplace_back(0, aLVC.m_outerBarrelRadius);

    std::vector<std::pair<int, float> > outerZ0F1;
    outerZ0F1.emplace_back(0, m_outerShieldRadius);
    outerZ0F1.emplace_back(0, 2750.);  // outer envelope
    if (m_activeAdjustLevel > 0) {
        outerZ0F1.emplace_back(0, 3600.);
        outerZ0F1.emplace_back(0, 5300.);
        outerZ0F1.emplace_back(0, 7000.);
        outerZ0F1.emplace_back(0, 8500.);
        outerZ0F1.emplace_back(0, 10000.);
        outerZ0F1.emplace_back(0, 12000.);
    }
    outerZ0F1.emplace_back(0, 12650.);  // outer envelope
    outerZ0F1.emplace_back(0, 13400.);  // outer envelope
    outerZ0F1.emplace_back(0, aLVC.m_outerBarrelRadius);

    std::vector<std::vector<std::vector<std::pair<int, float> > > > outerZF(2);
    outerZF[0].push_back(outerZ0F0);
    outerZF[0].push_back(outerZ0F0);
    outerZF[0].push_back(outerZ0F0);
    outerZF[1].push_back(outerZ0F1);
    outerZF[1].push_back(outerZ0F1);
    outerZF[1].push_back(outerZ0F1);

    // collect everything
    aLVC.m_hPartitions.push_back(barrelZF);
    aLVC.m_hPartitions.push_back(swZF);
    aLVC.m_hPartitions.push_back(innerZF);
    aLVC.m_hPartitions.push_back(outerZF);
}

void MuonTrackingGeometryBuilderImpl::getShieldParts(
    LocalVariablesContainer& aLVC) const {
    aLVC.m_shieldZPart.resize(18);

    aLVC.m_shieldZPart[0] = -21900.;  // elm2
    aLVC.m_shieldZPart[1] = -21500.;  // elm1
    aLVC.m_shieldZPart[2] = -21000.;  // octogon
    aLVC.m_shieldZPart[3] = -18000.;  // tube
    aLVC.m_shieldZPart[4] = -12882.;  // ect
    aLVC.m_shieldZPart[5] = -7930.;   // ect
    aLVC.m_shieldZPart[6] = -7914.;   // cone
    aLVC.m_shieldZPart[7] = -6941.;   // disk
    aLVC.m_shieldZPart[8] = -6783.;   //
    for (unsigned int i = 9; i < 18; i++)
        aLVC.m_shieldZPart[i] = -aLVC.m_shieldZPart[17 - i];

    aLVC.m_shieldHPart.clear();

    std::vector<std::pair<int, float> > outerShield;
    outerShield.emplace_back(0, m_beamPipeRadius);
    outerShield.emplace_back(0, 279.);   // outer envelope
    outerShield.emplace_back(0, 436.7);  // outer envelope
    outerShield.emplace_back(0, 1050.);  // outer envelope
    outerShield.emplace_back(0, m_outerShieldRadius);
    aLVC.m_shieldHPart.push_back(outerShield);

    std::vector<std::pair<int, float> > innerShield;
    innerShield.emplace_back(0, m_beamPipeRadius);
    innerShield.emplace_back(0, 530.);
    innerShield.emplace_back(0, m_innerShieldRadius);
    aLVC.m_shieldHPart.push_back(innerShield);

    std::vector<std::pair<int, float> > diskShield;
    diskShield.emplace_back(0, 0.);
    diskShield.emplace_back(0, 540.);
    diskShield.emplace_back(0, 750.);
    diskShield.emplace_back(0, 2700.);
    diskShield.emplace_back(0, 4255.);
    aLVC.m_shieldHPart.push_back(diskShield);
}

void MuonTrackingGeometryBuilderImpl::blendMaterial(
    LocalVariablesContainer& aLVC) const {
    // loop over map
    // std::map<const Trk::DetachedTrackingVolume*,std::vector<const
    // Trk::TrackingVolume*>* >::iterator mIter = m_blendMap.begin();
    for (const auto&[viter, vv] : aLVC.m_blendMap) {
        // find material source
        const Trk::Material* detMat = viter->trackingVolume();
        double csVol = m_volumeConverter.calculateVolume(*viter->trackingVolume());
        std::unique_ptr<const Trk::VolumeSpan> span{m_volumeConverter.findVolumeSpan(viter->trackingVolume()->volumeBounds(),
                                                                                  viter->trackingVolume()->transform(), 0., 0.)};
        if (span && csVol > 0) {
            double enVol = 0.;
            // loop over frame volumes, check if confined
            std::vector<bool> fEncl;
            // blending factors can be saved, and not recalculated for each
            // clone
            for (const auto fIter : vv) {
                fEncl.push_back(enclosed(*fIter, *span, aLVC));
                if (fEncl.back())
                    enVol += m_volumeConverter.calculateVolume(*fIter);
            }
            // diluting factor
            double dil = enVol > 0. ? csVol / enVol : 0.;
            if (dil > 0.) {
                for (auto fIter = vv.begin(); fIter != vv.end(); ++fIter) {
                    if (fEncl[fIter - vv.begin()]) {
                        Trk::TrackingVolume* vol = (*fIter);
                        vol->addMaterial(*detMat, dil);
                        if (m_colorCode == 0) {
                            vol->registerColorCode(12);
                        }
                        ATH_MSG_VERBOSE((*fIter)->volumeName()
                                        << " acquires material from "
                                        << viter->name());
                    }
                }
                ATH_MSG_VERBOSE("diluting factor:" << dil << " for "
                                                   << viter->name()
                                                   << ", blended ");

            } else {
                ATH_MSG_VERBOSE("diluting factor:" << dil << " for "
                                                   << viter->name()
                                                   << ", not blended ");
            }
        }
    }
}
}