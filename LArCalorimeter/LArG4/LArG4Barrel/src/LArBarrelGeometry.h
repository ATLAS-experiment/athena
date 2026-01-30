/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// LArBarrelGeometry.hh

#ifndef LARG4BARREL_LARBARRELGEOMETRY_H
#define LARG4BARREL_LARBARRELGEOMETRY_H

#include "ILArBarrelGeometry.h"
#include "AthenaBaseComps/AthService.h"

#include "LArG4Code/LArG4Identifier.h"
#include "LArG4Code/LArVG4DetectorParameters.h"
#include "G4ThreeVector.hh"
#include "G4StepPoint.hh"
#include "G4Step.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4TouchableHistory.hh"

#include <string>

// Forward declarations.
class LArG4Identifier;
class G4Step;
class LArCoudeElectrodes;
class LArCoudeAbsorbers;
class LArStraightAbsorbers;
class LArStraightElectrodes;

namespace LArG4 {

  namespace Barrel {

    class Geometry: public extends<AthService, ILArBarrelGeometry> {

    public:

      //constructor
      Geometry(const std::string& name, ISvcLocator * pSvcLocator);

      virtual ~Geometry() = default;

      virtual StatusCode initialize() override final;
      virtual StatusCode finalize() override final;

      virtual void initializeForSDCreation() override final;

      // Full identifier computation from a G4 step
      virtual LArG4Identifier CalculateIdentifier( const G4Step* ) const override final;

      // Super resolution identifier computation from a G4 step
      virtual LArG4Identifier CalculateSuperResolutionIdentifier(const G4Step* a_step) const override final;


      // Given a point compute all quantities (cell number, distance to electrode, etc...)
      virtual void findCell( CalcData & currentCellData, const double & x, const double & y, const double & z,
                             const double & r, const double & eta, const double & phi, const bool detail) const override final;

    private:

      LArG4Identifier CalculateECAMIdentifier( const G4Step* , const G4int indEcam, const bool inSTAC=true,int zside=1) const;
      bool CheckLArIdentifier(int sampling,int region, int eta,int phi) const;
      bool CheckDMIdentifier(int type, int sampling, int region, int eta, int phi) const;

      // detector name, for translated geometry
      Gaudi::Property<std::string> m_detectorName{this, "DetectorName", "LArMgr"};
      // to handle small difference (mostly phi wrapping and +-z symmetry)
      // between atlas and test beam
      Gaudi::Property<bool> m_testbeam{this, "TestBeam", false};

      G4String m_ecamName;

      // global EMBarrel dimensions
      double m_rMinAccordion{0.};
      double m_rMaxAccordion{0.};
      double m_zMinBarrel{0.};
      double m_zMaxBarrel{0.};
      double m_zMaxBarrelDMMargin{0.};
      double m_etaMaxBarrel{0.};

      // GU 11/06/2003  total number of cells in phi
      int m_NCellTot{0};    // either 64 or 1024 for TestBeam or Atlas
      int m_NCellMax{0};    // 1024

      // Accordion parameters
      int m_Nbrt{0};         //   number of straight sections (=14)
      int m_Nbrt1{0};        //   number of folds (=15)

      // Accordion parameters, refering to the neutral fibre
      double m_gam0{0.};         //phi position for the first absorber  neutral fiber
      double m_rint_eleFib{0.}; //2.78

      double *m_rc{nullptr};
      double *m_phic{nullptr};
      double *m_xc{nullptr};
      double *m_yc{nullptr};
      double *m_delta{nullptr};
      int m_parity{0};

      // to access G4 geometry
      const LArCoudeElectrodes* m_coudeelec{nullptr};
      const LArCoudeAbsorbers* m_coudeabs{nullptr};
      const LArStraightElectrodes* m_electrode{nullptr};
      const LArStraightAbsorbers* m_absorber{nullptr};

      bool m_iflSAG{false};

      // intermediate values for phi cell computation
      G4int m_NRphi{0};
      G4double m_Rmin{0.};
      G4double m_Rmax{0.};
      G4double m_Rphi[5000] = {0};
      G4double m_dR{0.};

      // function to compute distance to electrode
      double Distance_Ele(const double &x, const double &y,
                          const int &PhiC, int &Num_Straight, const int &Num_Coude,
                          double &xl) const;
      // function to compute distance to absorber
      double Distance_Abs(const double &x, const double &y,
                          const int &nabs, const int &Num_Straight, const int &Num_Coude) const;

      // longitudinal and eta segmentation of electrodes
      G4int SampSeg(G4double,G4double,G4double,G4int&,G4int&,G4int&,G4int&,G4int&) const;

      /// phi vs r of first absorber in nominal geometry

      /// Initialize r-phi reference map (called from constructor)
      void GetRphi();
      ///
      G4double Phi0(G4double) const;
      ///
      G4int PhiGap(const double &, const double &, const double &) const;

    protected:

      Geometry();


    } ;

  } //end of Barrel namespace

} // end of LArG4 namespace

#endif // LARG4BARREL_LARBARRELGEOMETRY_H
