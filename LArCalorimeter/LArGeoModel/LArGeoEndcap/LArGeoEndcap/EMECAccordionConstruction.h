/*
  Copyright (C) 2024-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file EMECAccordionConstruction.h
 *
 * @brief Declaration of EMECAccordionConstruction class
 *
 */

#ifndef LARGEOENDCAP_EMECACCORDIONCONSTRUCTION_H
#define LARGEOENDCAP_EMECACCORDIONCONSTRUCTION_H

#include <string>
#include <array>

#include "GeoModelKernel/GeoDefinitions.h"


using GeoTwoVector = GeoTrf::Vector2D;
using GeoThreeVector = GeoTrf::Vector3D;

class GeoXmlInpManager;
class GeoMaterial;
class GeoPhysVol;
class GeoFullPhysVol;
class GeoShape;

namespace LArGeo {

  /** @class LArGeo::EMECAccordionConstruction
      @brief Class for construction of EMEC internal structure
   */
  class EMECAccordionConstruction
  {
  public:
    EMECAccordionConstruction() = default;
    ~EMECAccordionConstruction() = default;

    void setWheelParameters();
    void setInnerWheel(GeoFullPhysVol* innerWheel);
    void setOuterWheel(GeoFullPhysVol* outerWheel);
    void setMaterial(const std::string& name, const GeoMaterial* material);

    void constructInnerWheelStructure(bool makeSlices = true);
    void constructOuterWheelStructure(bool makeSlices = true);

  private:
    struct CutPlane { // plane equation: n.dot(p) + d = 0
      GeoThreeVector m_n;
      double m_d;
    };

    void setInnerWheelSlices();
    void setOuterWheelSlices();
    void getInnerAbsorberData(double& wmin, double& wmax,
                              double& llip1, double& ylip1,
                              double& llip2, double& ylip2) const;
    void getOuterAbsorberData(double& wmin, double& wmax,
                              double& llip1, double& ylip1,
                              double& llip2, double& ylip2) const;
    void getBladeCorners(double wmin, double wmax, double thickness,
                         double rmin, double rmax, double zdel,
                         GeoThreeVector corners[8]) const;
    CutPlane getBottomCutPlane(double zmin, double rmin,
                               double zmax, double rmax) const;
    CutPlane getTopCutPlane(double zmin, double rmin,
                            double zmax, double rmax,
                            const GeoThreeVector corners[8]) const;
    GeoThreeVector IntersectionPoint(const GeoThreeVector& p1,
                                     const GeoThreeVector& p2,
                                     const CutPlane& plane) const;
    GeoShape* constructBlade(int icase,
                             const GeoThreeVector corners[8], double xscale,
                             double pz1, double pr1min, double pr1max,
                             double pz2, double pr2min, double pr2max) const;

    // Construction of logical volumes
    void constructInnerSlices();
    void constructOuterSlices();
    void constructInnerLips(double innerLipLength1, double innerLipPosition1,
                            double innerLipLength2, double innerLipPosition2);
    void constructOuterLips(double outerLipLength1, double outerLipPosition1,
                            double outerLipLength2, double outerLipPosition2);
    void constructInnerBlades(const GeoThreeVector innerCorners[8],
                              const GeoThreeVector innerElectrodeCorners[8]);
    void constructOuterBlades(const GeoThreeVector outerCorners[8],
                              const GeoThreeVector outerElectrodeCorners[8]);

    // Construction of physical volumes
    void placeInnerGlueAndLead();
    void placeOuterGlueAndLead();
    void placeInnerSlices(bool makeSlices);
    void placeOuterSlices(bool makeSlices);
    void placeInnerAccordion(int innerNoSectors, bool makeSlices, bool makeSectors);
    void placeOuterAccordion(int outerNoSectors, bool makeSlices, bool makeSectors);

  private:
    GeoFullPhysVol* m_innerWheel = nullptr;
    GeoFullPhysVol* m_outerWheel = nullptr;

    const GeoMaterial* m_materialLiquidArgon = nullptr;
    const GeoMaterial* m_materialKapton      = nullptr;
    const GeoMaterial* m_materialLead        = nullptr;
    const GeoMaterial* m_materialSteel       = nullptr;
    const GeoMaterial* m_materialGlue        = nullptr;

    int m_innerNoElectrodes = 256; // Number of electrodes in Inner wheel
    int m_innerNoAbsorbes   = 256; // Number of absorbers in Inner wheel
    int m_innerNoWaves      = 6;   // Number of waves in Inner wheel
    static constexpr int s_innerNoBlades = 15; // Number of blades (11 half_waves + 2 quarter_waves + 2 lips)

    int m_outerNoElectrodes = 768; // Number of electrodes in Outer wheel
    int m_outerNoAbsorbes   = 768; // Number of absorbers in Outer wheel
    int m_outerNoWaves      = 9;   // Number of waves in Outer wheel
    static constexpr int s_outerNoBlades = 21; // Number of blades (17 half_waves + 2 quarter_waves + 2 lips)

    // Strings for construction of names
    std::string m_nameInnerWheel = "";
    std::string m_nameOuterWheel = "";
    std::string m_nameSlice      = "::Slice";
    std::string m_nameAbsorber   = "::Absorber";
    std::string m_nameLead       = "::Lead";
    std::string m_nameGlue       = "::Glue";
    std::string m_nameElectrode  = "::Electrode";

    std::string m_nameSuffix[s_outerNoBlades] = {
      "01","02","03","04","05","06","07","08","09","10",
      "11","12","13","14","15","16","17","18","19","20","21"
    };

    // Inner wheel accordion wave parameters
    double m_innerWheelWidth = 0;     // 514 mm
    double m_innerLipWidth = 0;       // 2 mm
    double m_innerWaveZoneWidth = 0;  // 514 - 2*2 = 510 mm
    double m_innerWaveWidth = 0;      // 510:6 = 85 mm
    double m_innerHalfWaveWidth = 0;
    double m_innerQuaterWaveWidth = 0;

    // Outer wheel accordion wave parameters
    double m_outerWheelWidth = 0;     // 514 mm
    double m_outerLipWidth = 0;       // 2 mm
    double m_outerWaveZoneWidth = 0;  // 514 - 2*2 = 510 mm
    double m_outerWaveWidth = 0;      // 510:9 = 56.67 mm
    double m_outerHalfWaveWidth = 0;
    double m_outerQuaterWaveWidth = 0;

    // Inner wheel thiknesses
    double m_innerLeadThickness = 0;
    double m_innerSteelThickness = 0;
    double m_innerGlueThickness = 0;
    double m_innerAbsorberThickness = 0;
    double m_innerElectrodeThickness = 0;
    double m_innerGlueRatio = 0;     // x-scale factor for glue in absorber
    double m_innerLeadRatio = 0;     // x-scale factor for lead in absorber

    // Outer wheel thiknesses
    double m_outerLeadThickness = 0;
    double m_outerSteelThickness = 0;
    double m_outerGlueThickness = 0;
    double m_outerAbsorberThickness = 0;
    double m_outerElectrodeThickness = 0;
    double m_outerGlueRatio = 0;     // x-scale factor for glue in absorber
    double m_outerLeadRatio = 0;     // x-scale factor for lead in absorber

    // Contraction factor
    double m_kContraction = 0;

    // Inner wheel parameters
    std::array<double, 2> m_zWheelInner = {};
    std::array<double, 2> m_rMinInner = {};
    std::array<double, 2> m_rMaxInner = {};

    // Outer wheel parameters
    std::array<double, 3>  m_zWheelOuter = {};
    std::array<double, 3>  m_rMinOuter = {};
    std::array<double, 3>  m_rMaxOuter = {};

    // Data for Inner wheel slices
    std::array<double, s_innerNoBlades + 1> m_innerWheelZ = {};
    std::array<double, s_innerNoBlades + 1> m_innerWheelRmin = {};
    std::array<double, s_innerNoBlades + 1> m_innerWheelRmax = {};
    double m_innerWheelRminIncrement = 0;
    double m_innerWheelRmaxIncrement = 0;
    double m_innerWheelZmin = 0;
    double m_innerWheelZmax = 0;

    // Data for Outer wheel slices
    std::array<double, s_outerNoBlades + 1> m_outerWheelZ = {};
    std::array<double, s_outerNoBlades + 1> m_outerWheelRmin = {};
    std::array<double, s_outerNoBlades + 1> m_outerWheelRmax = {};
    std::array<double, 2> m_outerWheelRminIncrement = {};
    std::array<double, 2> m_outerWheelRmaxIncrement = {};
    double m_outerWheelZmin = 0;
    double m_outerWheelZmax = 0;

    // Logical volumes for Inner wheel blades
    std::array<GeoPhysVol*, s_innerNoBlades> m_innerAbsorber  {{nullptr}};
    std::array<GeoPhysVol*, s_innerNoBlades> m_innerGlue      {{nullptr}};
    std::array<GeoPhysVol*, s_innerNoBlades> m_innerLead      {{nullptr}};
    std::array<GeoPhysVol*, s_innerNoBlades> m_innerElectrode {{nullptr}};
    std::array<GeoThreeVector, s_innerNoBlades> m_innerAbsorberOffset = {};
    std::array<GeoThreeVector, s_innerNoBlades> m_innerElectrodeOffset = {};

    // Logical volumes for Outer wheel blades
    std::array<GeoPhysVol*, s_outerNoBlades> m_outerAbsorber  {{nullptr}};
    std::array<GeoPhysVol*, s_outerNoBlades> m_outerGlue      {{nullptr}};
    std::array<GeoPhysVol*, s_outerNoBlades> m_outerLead      {{nullptr}};
    std::array<GeoPhysVol*, s_outerNoBlades> m_outerElectrode {{nullptr}};
    std::array<GeoThreeVector, s_outerNoBlades> m_outerAbsorberOffset = {};
    std::array<GeoThreeVector, s_outerNoBlades> m_outerElectrodeOffset = {};

    // Logical volumes for Inner wheel slices
    std::array<GeoPhysVol*, s_innerNoBlades> m_innerSlice     {{nullptr}};
    std::array<GeoThreeVector, s_innerNoBlades> m_innerSliceOffset = {};

    // Logical volumes for Outer wheel slices

    std::array<GeoPhysVol*, s_outerNoBlades> m_outerSlice     {{nullptr}};
    std::array<GeoThreeVector, s_outerNoBlades> m_outerSliceOffset = {};

    // Logical volumes for Inner wheel sectors
    std::array<GeoPhysVol*, s_innerNoBlades> m_innerSector    {{nullptr}};

    // Logical volumes for Outer wheel sectors
    std::array<GeoPhysVol*, s_outerNoBlades> m_outerSector {{nullptr}};
  };

} // namespace LArGeo

#endif

