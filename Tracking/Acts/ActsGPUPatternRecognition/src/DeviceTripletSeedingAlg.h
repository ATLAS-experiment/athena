/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUPATTERNRECOGNITION_DEVICETRIPLETSEEDINGALG_H
#define ACTSGPUPATTERNRECOGNITION_DEVICETRIPLETSEEDINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "IDeviceSeedingAlgProviderTool.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "ActsGPUEvent/TracccDetectorGeometryDescription.h"
#include "ActsGPUEvent/TracccSpacepointCollection.h"
#include "ActsGPUEvent/TracccSeedCollection.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"


template <typename scalar_t>
using unit = detray::unit<scalar_t>;

namespace ActsTrk {
/**
 * @class DeviceTripletSeedingAlg
 *
 * @brief Algorithm executing traccc triplet seeding on the GPU.
 *
 * The backend-specific seeding algorithm is provided by
 * a dedicated tool, together with the device memory resource.
 *
 * The algorithm retrieves the device resident traccc spacepoint collection from the event
 * store, passes it to the device triplet seeding algorithm, and records the
 * resulting device resident traccc seed collection back into the event store.
 *
 * The seed finder, spacepoint grid and seed filter configurations are exposed as properties.
 * The magnetic field and the beam spot position are taken from the conditions store
 * for every event.
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class DeviceTripletSeedingAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// Function initializing the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

private:

    virtual StatusCode configureTripletSeeder();

    /// @name The tool that provides backend-specific traccc seeding algorithms
    ToolHandle<IDeviceSeedingAlgProviderTool> m_seedingAlgProviderTool{
        this, "SeedingAlgProviderTool", "",
        "Tool providing the appropriate backend device seeding algorithm"};

    /// @name The name of device resident input traccc spacepoint collection
    SG::ReadHandleKey<traccc::edm::spacepoint_collection::const_view> m_inputPixelSPKey{
        this, "InputTracccPixelSpacepoints", "",
        "Input traccc spacepoint collection buffer"};

    /// @name The name of device resident output traccc seed collection
    /// {@
    SG::WriteHandleKey<traccc::edm::seed_collection::buffer> m_outputPixelSeedsKey{
        this, "OutputTracccPixelSeeds", "",
        "Output traccc pixel seed collection buffer"};
    /// @}

    /// @name The conditions objects
    /// {@
    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{
        this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};
    SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCondObjInputKey{
        this, "AtlasFieldCacheCondObj", "fieldCondObj",
        "Name of the Magnetic Field conditions object key"};
    /// @}

    /// @name Seed finder properties
    /// {@
    Gaudi::Property<float> m_zMin{this, "zMin", -3000.f, "limiting location of measurements [mm]"};
    Gaudi::Property<float> m_zMax{this, "zMax", 3000.f, "limiting location of measurements [mm]"};
    Gaudi::Property<float> m_rMin{this, "rMin", 33.f, "limiting location of measurements [mm]"};
    Gaudi::Property<float> m_rMax{this, "rMax", 320.f, "limiting location of measurements [mm]"};
    Gaudi::Property<float> m_collisionRegionMin{this, "collisionRegionMin", -114.f,
        "limiting location of collision region in z [mm]"};
    Gaudi::Property<float> m_collisionRegionMax{this, "collisionRegionMax", 114.f,
        "limiting location of collision region in z [mm]"};
    Gaudi::Property<float> m_minPt{this, "minPt", 900.f, "lower pT cutoff for seeds [MeV]"};
    Gaudi::Property<float> m_cotThetaMax{this, "cotThetaMax", 27.2899f, "cot of maximum theta angle"};
    Gaudi::Property<float> m_deltaRMin{this, "deltaRMin", 20.f,
        "minimum distance in r between two measurements within one seed [mm]"};
    Gaudi::Property<float> m_deltaRMax{this, "deltaRMax", 100.f,
        "maximum distance in r between two measurements within one seed [mm]"};
    Gaudi::Property<float> m_deltaZMax{this, "deltaZMax", 800.f,
        "maximum distance in z between two measurements within one seed [mm]"};
    Gaudi::Property<float> m_impactMax{this, "impactMax", 2.f, "maximum impact parameter [mm]"};
    Gaudi::Property<float> m_sigmaScattering{this, "sigmaScattering", 3.f,
        "how many sigmas of scattering angle should be considered"};
    Gaudi::Property<float> m_maxPtScattering{this, "maxPtScattering", 10000.f,
        "upper pt limit for scattering calculation [MeV]"};
    Gaudi::Property<float> m_radLengthPerSeed{this, "radLengthPerSeed", 0.05f,
        "average radiation lengths of material on the length of a seed"};
    Gaudi::Property<unsigned int> m_maxSeedsPerSpM{this, "maxSeedsPerSpM", 2,
        "maximum number of seeds per middle spacepoint"};
    Gaudi::Property<int> m_phiBinDeflectionCoverage{this, "phiBinDeflectionCoverage", 1,
        "sets of consecutive phi bins to cover full deflection of minimum pT particle"};
    Gaudi::Property<float> m_deltaRMinBottomSP{this, "deltaRMinBottomSP", -1.f,
        "minimum distance in r between middle and bottom SP [mm], deltaRMin if negative"};
    Gaudi::Property<float> m_deltaRMaxBottomSP{this, "deltaRMaxBottomSP", -1.f,
        "maximum distance in r between middle and bottom SP [mm], deltaRMax if negative"};
    Gaudi::Property<float> m_deltaRMinTopSP{this, "deltaRMinTopSP", -1.f,
        "minimum distance in r between middle and top SP [mm], deltaRMin if negative"};
    Gaudi::Property<float> m_deltaRMaxTopSP{this, "deltaRMaxTopSP", -1.f,
        "maximum distance in r between middle and top SP [mm], deltaRMax if negative"};
    Gaudi::Property<float> m_rMinMiddle{this, "rMinMiddle", 0.f,
        "minimum radius of the middle SP [mm]"};
    Gaudi::Property<float> m_rMaxMiddle{this, "rMaxMiddle", 1.e9f,
        "maximum radius of the middle SP [mm]"};
    Gaudi::Property<float> m_zMinMiddle{this, "zMinMiddle", -1.e9f,
        "minimum z of the middle SP [mm]"};
    Gaudi::Property<float> m_zMaxMiddle{this, "zMaxMiddle", 1.e9f,
        "maximum z of the middle SP [mm]"};
    Gaudi::Property<bool> m_interactionPointCut{this, "interactionPointCut", true,
        "enable cut on the compatibility between interaction point and doublet"};
    Gaudi::Property<bool> m_doubletDPhiCut{this, "doubletDPhiCut", false,
        "enable cut on the azimuthal separation of the doublet SPs"};
    Gaudi::Property<float> m_doubletDPhiD0Max{this, "doubletDPhiD0Max", -1.f,
        "impact parameter used in the azimuthal separation cut [mm], impactMax if negative"};
    Gaudi::Property<float> m_doubletDPhiConst{this, "doubletDPhiConst", 0.015f,
        "constant term of the azimuthal separation bound"};
    Gaudi::Property<float> m_doubletDPhiSlope{this, "doubletDPhiSlope", 2.0e-4f,
        "radial slope of the azimuthal separation bound [1/mm]"};
    Gaudi::Property<float> m_doubletDPhiCap{this, "doubletDPhiCap", 10.f,
        "upper limit of the impact parameter term of the azimuthal separation bound"};
    Gaudi::Property<float> m_cotThetaDiffMax{this, "cotThetaDiffMax", 1.e9f,
        "maximum difference of the cot(theta) of the two doublets of a triplet"};
    /// @}

    /// @name Spacepoint grid properties
    /// {@
    Gaudi::Property<float> m_gridDeltaRMax{this, "gridDeltaRMax", -1.f,
        "maximum distance in r from middle to bottom or top spacepoint used for the grid "
        "phi binning [mm], deltaRMax if negative"};
    /// @}

    /// @name Seed filter properties
    /// {@
    Gaudi::Property<float> m_deltaInvHelixDiameter{this, "deltaInvHelixDiameter", 0.00003f,
        "allowed delta between two inverted seed radii to be considered compatible [1/mm]"};
    Gaudi::Property<float> m_impactWeightFactor{this, "impactWeightFactor", 1.f,
        "impact parameter is multiplied by this factor and subtracted from the weight"};
    Gaudi::Property<float> m_compatSeedWeight{this, "compatSeedWeight", 200.f,
        "seed weight increase if a compatible seed has been found"};
    Gaudi::Property<float> m_filterDeltaRMin{this, "filterDeltaRMin", 5.f,
        "minimum distance between compatible seeds to be considered for weight boost [mm]"};
    Gaudi::Property<unsigned int> m_compatSeedLimit{this, "compatSeedLimit", 1,
        "maximum number of compatible seeds considered for the weight boost"};
    Gaudi::Property<float> m_goodSpBMinRadius{this, "goodSpBMinRadius", 150.f,
        "bottom spacepoint radius above which the weight is increased [mm]"};
    Gaudi::Property<float> m_goodSpBWeightIncrease{this, "goodSpBWeightIncrease", 400.f,
        "weight increase for a good bottom spacepoint"};
    Gaudi::Property<float> m_goodSpTMaxRadius{this, "goodSpTMaxRadius", 150.f,
        "top spacepoint radius below which the weight is increased [mm]"};
    Gaudi::Property<float> m_goodSpTWeightIncrease{this, "goodSpTWeightIncrease", 200.f,
        "weight increase for a good top spacepoint"};
    Gaudi::Property<float> m_goodSpBMinWeight{this, "goodSpBMinWeight", 380.f,
        "minimum weight of a seed with a good bottom spacepoint"};
    Gaudi::Property<float> m_seedMinWeight{this, "seedMinWeight", 200.f,
        "minimum weight of a seed with a bottom spacepoint below spBMinRadius"};
    Gaudi::Property<float> m_spBMinRadius{this, "spBMinRadius", 43.f,
        "bottom spacepoint radius above which the seed weight cut is not applied [mm]"};
    /// @}

    traccc::seedfinder_config m_seedfinder;
    traccc::seedfilter_config m_seedfilter;

};

} // namespace ActsTrk

#endif // ACTSGPUPATTERNRECOGNITION_DEVICETRIPLETSEEDINGALG_H
