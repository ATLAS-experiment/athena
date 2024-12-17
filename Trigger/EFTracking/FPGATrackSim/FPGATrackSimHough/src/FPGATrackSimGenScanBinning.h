// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimGenScanBinning_H
#define FPGATrackSimGenScanBinning_H

/**
 * @file FPGATrackSimGenScanBinning.h
 * @author Elliot Lipeles
 * @date Sept 10th, 2024
 * @brief Binning Classes for GenScanTool
 *
 * Declarations in this file (there are a series of small classes):
 *      class FPGATrackSimGenScanBinningBase
 *            base class that defines interface to FPGATrackSimGenScanTool
 * 
 *      class FPGATrackSimGenScanGeomHelpers
 *            very small class to implement common track parameter calculations
 *            that are shared by clases
 * 
 *      class GenScanStdBinning : FPGATrackSimGenScanBinningBase
 *            implements standard pT, eta, phi, d0, z0 binning
 * 
 *      class GenScanKeyLyrTool
 *            implements key layer math for binning using where in phi and z a track 
 *            crosses two "keylayers" plus the distance the track has bent from a 
 *            straight line midway between the layers (called xm in the code, but is 
 *            also known as sagitta). This common helper tool avoid code duplication.
 * 
 *      class GenScanKeyLyrBinning : FPGATrackSimGenScanBinningBase
 *            implements keylyr binning with slice=z values at two radii, scan=phi 
 *            values at two radii, and row=xm (sagitta)
 * 
 *      class GenScanKeyLyrPhiSlicedBinning : FPGATrackSimGenScanBinningBase
 *            implements keylyr binning with slice=phi values at two radii and xm, 
 *            scan =z values at two radii, and row=xm. The track rates will be the 
 *            same as GenScanKeyLyrBinning, but the internal throughput rates are 
 *            different and are needed to understand firmware requirements
 *      
 * 
 * Overview of stucture:
 *    -- This is to be used by the FPGATrackSimGenScanTool to define the binning 
 *       (read that header first)
 *    -- A binning is defined by inheriting from FPGATrackSimGenScanBinningBase
 *    -- There are 3 implementations currently in this file, see descriptions in
 *       "Declarations" section above
 *    -- In addition, there are two helper tools for the math for standard track 
 *       parameters and keylayer track parameters
 * 

 * 
 * References:
 *
 */
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"

#include "FourMomUtils/xAODP4Helpers.h"
#include "GaudiKernel/StatusCode.h"

#include <initializer_list>
#include <string>
#include <vector>

//-------------------------------------------------------------------------------------------------------
// Binning base class
//       Nomenclature:
//             slice = first larger region binning (z0-eta binning in typical Hough)
//             scan, row are the independent and dependent variables in a standard Hough ( e.g. pT  and phi_track)
//             slicePar = variables used to slice into overlapping regions (e.g. z0,eta)
//             sliceVar = variable that slicePars specify a valid range of (e.g. hit_z)
//             scanPar = scan variables of Hough-like scan (e.g. pT,d0)
//             rowPar = variable that scanPars specify a valid range of for a given hit (e.g. phi_track)
//-------------------------------------------------------------------------------------------------------
class FPGATrackSimGenScanBinningBase : virtual public IAlgTool {
public:
   DeclareInterfaceID(FPGATrackSimGenScanBinningBase, 2, 0);

  //--------------------------------------------------------------------------------------------------
  //
  // These just makes ParSet and IdxSet types that have 5 double or unsigned int
  // for the 5-track parameters. The point of the structs is that they are fixed
  // to the right length and can be converted back and forth to std::vector
  // through cast operators and constructors
  //
  //--------------------------------------------------------------------------------------------------

    struct ParSet : public std::array<double,5> {
      using array<double,5>::array;
      ParSet(const std::vector<double>& val) {
        if (val.size() != 5) {
          throw std::invalid_argument("Not enough parameters in ParSet initialization");  
        } 
        std::copy(val.begin(), val.end(), this->begin());
      }
      operator const std::vector<double>() const { return std::vector<double>(this->begin(),this->end());}
    };
    struct IdxSet : public std::array<unsigned, 5> {
      using array<unsigned,5>::array;
      IdxSet(const std::vector<unsigned>& val) {
        if (val.size() != 5) {
          throw std::invalid_argument("Not enough parameters in IdxSet initialization");  
        } 
        std::copy(val.begin(), val.end(), this->begin());
      }
      operator const std::vector<unsigned>() const { return std::vector<unsigned>(this->begin(),this->end());}
    };
  
    //--------------------------------------------------------------------------------------------------
    //
    // Virtual methods that are overloaded to define the binning
    //
    //--------------------------------------------------------------------------------------------------

    // Specification of parameters
    virtual const std::string &
    parNames(unsigned i) const = 0;

    // overloaded to define which parameters are slice, scan, and row
    // they return vectors which are the numbers of the parameters in the slice or scan
    // and just an unsigned for row since there can be only one row parmeter
    virtual std::vector<unsigned> slicePars() const = 0;
    virtual std::vector<unsigned> scanPars() const = 0;
    virtual unsigned rowParIdx() const = 0;

    // convert back and forth from pT, eta, phi, d0, z0 and internal paramater set
    virtual const ParSet trackParsToParSet(const FPGATrackSimTrackPars &pars) const = 0;
    virtual const FPGATrackSimTrackPars parSetToTrackPars(const ParSet &parset) const = 0;

    // calculate the distance in phi or eta from a track defined by parset to a hit
    // these can be implemented as any variable in the r-phi or r-eta plane (not necessarily eta and phi).
    virtual double phiResidual(const ParSet &parset, FPGATrackSimHit const *hit, [[maybe_unused]] bool debug=false) const = 0;
    virtual double etaResidual(const ParSet &parset,  FPGATrackSimHit const *hit, [[maybe_unused]] bool debug=false) const = 0;

    // Used to scale the histograms binning appropriately (very approximate, but at least the default ranges come up reasonable)
    virtual double phiHistScale() const = 0;
    virtual double etaHistScale() const = 0;

    // To determine if a hit is in a slice or row there are two options
    //  1) Directly implement hitInSlice and idxsetToRowParBinRange. 
    //  2) Use the default implementaions of those in which case sliceVar, sliceVarExpected, and rowPar should be implemented
    // Tells if a hit is in a slice
    virtual bool hitInSlice(const IdxSet &idx, FPGATrackSimHit const *hit) const;

    // Tells the range of row parameters a hit is conistent with assuming the other parameters (slice and scan)
    // are set by the given bin idx
    virtual std::pair<unsigned, unsigned> idxsetToRowParBinRange(const IdxSet &idx, FPGATrackSimHit const *hit) const;
    
    private:
    // Hit variable used in slicing (e.g. for eta-z0 slicing it would be the z of the hit)
    virtual double sliceVar([[maybe_unused]] FPGATrackSimHit const *hit) const;

    // Find expected sliceVar for a track with the given parameters (e.g. or eta-z0 slicing this is the expect z for a given hit radius)
    virtual double sliceVarExpected([[maybe_unused]] const ParSet &pars, [[maybe_unused]] FPGATrackSimHit const *hit) const;
    
    // Find the parameter for a given hit and parameters (e.g. given pT and d0 what is track phi)
    virtual double rowPar([[maybe_unused]] const ParSet &pars, [[maybe_unused]] FPGATrackSimHit const *hit) const;
    

    //--------------------------------------------------------------------------------------------------
    //
    // Functional Implementation
    //
    //--------------------------------------------------------------------------------------------------

    public:
    // merge of both slice and scan par lists
    std::vector<unsigned> sliceAndScanPars() const {
        std::vector<unsigned> retv = slicePars();
        std::vector<unsigned> scan = scanPars();
        retv.insert(retv.end(), scan.begin(), scan.end());
        return retv;
    };
       
    // Find distance from bin center to hit in either slice direction or the row direction
    double phiShift(const IdxSet &idx, FPGATrackSimHit const *hit, bool debug=false) const { return phiResidual(binCenter(idx),hit,debug);}
    double etaShift(const IdxSet &idx, FPGATrackSimHit const *hit, bool debug=false) const { return etaResidual(binCenter(idx),hit,debug);}

    // accessors to get the numbers of bins in just the slice, scan, sliceAndScan, or row (subset of the 5-d space)
    virtual std::vector<unsigned> sliceBins() const { return subVec(slicePars(), m_parBins); }
    virtual std::vector<unsigned> scanBins() const { return subVec(scanPars(), m_parBins); }
    virtual std::vector<unsigned> sliceAndScanBins() const { return subVec(sliceAndScanPars(), m_parBins); }
    virtual unsigned rowBin() const { return m_parBins[rowParIdx()]; }

    //  extract just the slice, scan, sliceAndScan, or row part of a 5-d index
    virtual std::vector<unsigned> sliceIdx(const IdxSet &idx) const { return subVec(slicePars(), idx); }
    virtual std::vector<unsigned> scanIdx(const IdxSet &idx) const { return subVec(scanPars(), idx); }
    virtual std::vector<unsigned> sliceAndScanIdx(const IdxSet &idx) const { return subVec(sliceAndScanPars(), idx); }
    virtual unsigned rowIdx(const IdxSet &idx) const { return idx[rowParIdx()]; }

    // Bin boundary utilities
    double binCenter(unsigned par, unsigned bin) const { return m_parMin[par] + m_parStep[par] * (double(bin) + 0.5); }
    double binLowEdge(unsigned par, unsigned bin) const { return m_parMin[par] + m_parStep[par] * (double(bin)); }
    double binHighEdge(unsigned par, unsigned bin) const { return m_parMin[par] + m_parStep[par] * (double(bin) + 1.0); }
    ParSet binLowEdge(const IdxSet &idx) const;
    ParSet binCenter(const IdxSet &idx) const;

    // center of whole region
    ParSet center() const;

    // get bin value for a specific parameter value
    unsigned binIdx(unsigned par, double val) const { return (val > m_parMin[par]) ? unsigned(floor((val - m_parMin[par]) / m_parStep[par])) : 0; }
    unsigned rowParBinIdx(double val) const { return binIdx(rowParIdx(), val); }
    
    // check if 1-d or 5-d parameter is within the range of the binning
    bool inRange(unsigned par, double val) const { return ((val < m_parMax[par]) && (val > m_parMin[par])); }
    bool inRange(const ParSet &pars) const;
    
    // convert parset (the binning parameters) to a 5-d bin
    IdxSet binIdx(const ParSet &pars) const;

    // convert FPGATrackSimTrackPars to 5-d bin
    const IdxSet parsToBin(FPGATrackSimTrackPars &pars) const;

    // Generic Utility for splitting in vector (e.g. idx or #bins 5-d vectors)
    // into subvectors (e.g. idx for just the scan parameters). Technically, for 
    // a list of parameter indices (elems) gives the subvector of the invec with just those indices
    std::vector<unsigned> subVec(const std::vector<unsigned>& elems, const IdxSet& invec) const;

    // Opposite of above subVec, this sets the subvector
    StatusCode setIdxSubVec(IdxSet &idx, const std::vector<unsigned>& subvecelems, const std::vector<unsigned>& subvecidx) const;

    // Makes are set of parameters corresponding to the corners specified by scanpars of the bin specified by idx
    // e.g. if scan pars is (pT,d0) then the set is (low pT,low d0), (low pT, high d0), (high pT,low d0), (high pT, high d0)
    std::vector<ParSet> makeVariationSet(const std::vector<unsigned> &scanpars, const IdxSet &idx) const;
    
    //
    // Internal data
    //
    static constexpr unsigned NPars = 5;
    ParSet m_parMin;
    ParSet m_parMax;
    ParSet m_parStep;
    IdxSet m_parBins;

    // invalid bin value, there is no way a true bin could be there
    const IdxSet m_invalidBin{std::initializer_list<unsigned>({std::numeric_limits<unsigned>::max(),std::numeric_limits<unsigned>::max(),
         std::numeric_limits<unsigned>::max(),std::numeric_limits<unsigned>::max(),std::numeric_limits<unsigned>::max()})};
};



//-------------------------------------------------------------------------------------------------------
//
// Geometry Helpers -- does basic helix calculations
//
//-------------------------------------------------------------------------------------------------------
class FPGATrackSimGenScanGeomHelpers
{
public:
    // This is the constant needed to relate hit phi to track phi due to curvature
    static constexpr double CurvatureConstant = fpgatracksim::A;

    // standard eta to theta calculation
    static double ThetaFromEta(double eta);

    // standard theta to eta calculation
    static double EtaFromTheta(double theta);

    // find the expected z position from the radius and track parameters
    static double zFromPars(double r, const FPGATrackSimTrackPars &pars);

    // find the expected z position from the radius and track parameters
    static double phiFromPars(double r, const FPGATrackSimTrackPars &pars);

    // find the track phi that would be consistent with the other track parameters and the hit (r,phi)
    static double parsToTrkPhi(const FPGATrackSimTrackPars &pars, FPGATrackSimHit const *hit);
};

//-------------------------------------------------------------------------------------------------------
//
// Standard pT, d0, phi, eta, z0 implmentation of FPGATrackSimGenScanStdTrkBinning
//   -- this is then effectively a "standard" Hough transform except that it slices
//      in (z0,eta) and then the accumulator scans is in both pT and d0 with phi_track 
//      as the row of the accumulator
//
//-------------------------------------------------------------------------------------------------------
class FPGATrackSimGenScanStdTrkBinning : public extends<AthAlgTool, FPGATrackSimGenScanBinningBase> 
{
public:
  FPGATrackSimGenScanStdTrkBinning(const std::string& algname, const std::string &name, const IInterface *ifc) :
  base_class(algname, name, ifc), m_parNames({"z0", "eta", "qOverPt", "d0", "phi"})
  {
    declareInterface<FPGATrackSimGenScanBinningBase>(this);
  }
  
  virtual StatusCode initialize() override { return StatusCode::SUCCESS; }

  virtual const std::string &parNames(unsigned i) const override { return m_parNames[i]; }
  virtual unsigned rowParIdx() const override { return 4;}
  
  virtual std::vector<unsigned> slicePars() const override { return std::vector<unsigned>({0,1}); }
  virtual std::vector<unsigned> scanPars() const override { return std::vector<unsigned>({2,3}); }

  virtual double etaHistScale() const override {return 200.0;}
  virtual double phiHistScale() const override {return 0.1;}

  virtual const ParSet trackParsToParSet(const FPGATrackSimTrackPars &pars) const override
  {
    return ParSet({pars[FPGATrackSimTrackPars::IZ0], pars[FPGATrackSimTrackPars::IETA],
                                pars[FPGATrackSimTrackPars::IHIP], pars[FPGATrackSimTrackPars::ID0], pars[FPGATrackSimTrackPars::IPHI]});
  }
  virtual const FPGATrackSimTrackPars parSetToTrackPars(const ParSet &parset) const override
  {
    FPGATrackSimTrackPars pars;
    pars[FPGATrackSimTrackPars::IZ0] = parset[0];
    pars[FPGATrackSimTrackPars::IETA] = parset[1];
    pars[FPGATrackSimTrackPars::IHIP] = parset[2];
    pars[FPGATrackSimTrackPars::ID0] = parset[3];
    pars[FPGATrackSimTrackPars::IPHI] = parset[4];
    return pars;
  }

  virtual double sliceVarExpected(const ParSet &pars, FPGATrackSimHit const *hit) const override
  {
    return FPGATrackSimGenScanGeomHelpers::zFromPars(hit->getR(),parSetToTrackPars(pars));
  }
  virtual double sliceVar(FPGATrackSimHit const *hit) const override
  {
    return hit->getZ();
  }
  virtual double rowPar(const ParSet &pars, FPGATrackSimHit const *hit) const override
  {
    return FPGATrackSimGenScanGeomHelpers::parsToTrkPhi(parSetToTrackPars(pars),hit);
  }
  
  virtual double etaResidual(const ParSet &parset ,FPGATrackSimHit const * hit, [[maybe_unused]] bool debug=false) const override {
    // this uses a shift in "eta*radius" instead of "z"
    double theta = FPGATrackSimGenScanGeomHelpers::ThetaFromEta(parset[1]);  // 1 = eta
    return (hit->getZ() - parset[0]) * sin(theta) - hit->getR() * cos(theta); // 0=z0
  }
  virtual double phiResidual(const ParSet &parset,FPGATrackSimHit const *hit, [[maybe_unused]] bool debug=false) const override {
    return xAOD::P4Helpers::deltaPhi(hit->getGPhi(), FPGATrackSimGenScanGeomHelpers::phiFromPars(hit->getR(),parSetToTrackPars(parset)));    
  }

private:
  const std::vector<std::string> m_parNames;
};

//-------------------------------------------------------------------------------------------------------
//
// Tool for doing Key Layer Math
//   -- that is converting be the track parameters set by the standard (pT,eta,phi,d0,z0) and 
//      the "KeyLayer" parametes set by the phi and z at to radii (R1,R2) and a deviation from
//      straight line between the two points (xm)
//   -- All the math  for the r-phi plane is based finding the rotated coordinate system where
//      the points (R1,phiR1) and (R2,phiR2) both lie on the x=0 axis. Then the x of a track halfway
//      between the points (called xm) is the sagitta.
// 
//-------------------------------------------------------------------------------------------------------
class FPGATrackSimGenScanKeyLyrHelper
{
public:
  FPGATrackSimGenScanKeyLyrHelper(double r1, double r2) : m_R1(r1), m_R2(r2) {}
  FPGATrackSimGenScanKeyLyrHelper() {} // NOTE r1 and r2 must be set before using class

  struct KeyLyrPars {
    KeyLyrPars() {}
    KeyLyrPars(const FPGATrackSimGenScanBinningBase::ParSet &parset)
        : z1(parset[0]), z2(parset[1]), phi1(parset[2]), phi2(parset[3]), xm(parset[4]) {}
    double z1;
    double z2;
    double phi1;
    double phi2;
    double xm;
  };

  // convert (r,phi) to (x,y)
  std::pair<double, double> getXY(double r, double phi) const;

  // Get rotation angles need to rotate the two given points to be only seperated in the y direction (both have same x)
  // results is the sin and cos of rotation
  std::pair<double, double> getRotationAngles(const std::pair<double, double>& xy1, const std::pair<double, double>& xy2) const;

  // Apply a rotation angles (sin and cos) to a point xy
  std::pair<double, double> rotateXY(const std::pair<double, double>& xy, const std::pair<double, double>& ang) const;

  // Simple struct and function to get the rotation angle and full rotated information in one call
  struct rotatedConfig
  {
    std::pair<double, double> xy1p;   // point 1 after rotation
    std::pair<double, double> xy2p;   // point 2 after rotation
    std::pair<double, double> rotang; // rotation angle
    double y;                         // y seperation after rotation
  };
  rotatedConfig getRotatedConfig(const KeyLyrPars &keypars) const;

  // get the x,y coordinates of a hit in the rotated coordinate system specified by rotang
  std::pair<double, double> getRotatedHit(const std::pair<double, double>& rotang, const FPGATrackSimHit *hit) const;

  // Convert back and forth to standard track parameters
  KeyLyrPars trackParsToKeyPars(const FPGATrackSimTrackPars &pars) const;
  FPGATrackSimTrackPars keyParsToTrackPars(const KeyLyrPars &keypars) const;

  // find expected z hit position given a radius
  double zExpected(const KeyLyrPars& keypars, double r) const;

  // find expected x position of a hit at given a radius in the rotated coordinate system
  double xExpected(const KeyLyrPars& keypars, const FPGATrackSimHit *hit) const;

  // takes hit position and calculated what xm would be for track going through that hit
  double xmForHit(const KeyLyrPars& keypars, const FPGATrackSimHit *hit) const;

  // Find shift from nominal track to hit in the "x" direction 
  double deltaX(const KeyLyrPars& keypars, const FPGATrackSimHit *hit) const;

  // accessors
  double R1() const {return m_R1;}
  double R2() const {return m_R2;}
  void setR1(const double r1) {m_R1=r1;}
  void setR2(const double r2) {m_R2=r2;}

  private:
    double m_R1;
    double m_R2;
};

//-------------------------------------------------------------------------------------------------------
//
// Key Layer Binning implementation (z slice then phi scan)
//  -- Using the FPGATrackSimGenScanKeyLyrHelper to implement an instance of 
//     FPGATrackSimGenScanBinningBase which slices in z and then scans in (phiR1,phiR2),
//     and then uses xm as the final row variable
//
//-------------------------------------------------------------------------------------------------------
class FPGATrackSimGenScanKeyLyrBinning :  public extends<AthAlgTool, FPGATrackSimGenScanBinningBase>
{
public:
  FPGATrackSimGenScanKeyLyrBinning(const std::string& algname, const std::string &name, const IInterface *ifc) :
  base_class(algname, name, ifc), m_keylyrtool(m_rin,m_rout), m_parNames({"zR1", "zR2", "phiR1", "phiR2", "xm"})
  {
    declareInterface<FPGATrackSimGenScanBinningBase>(this);
  }

  virtual StatusCode initialize() override {
    m_keylyrtool.setR1(m_rin);
    m_keylyrtool.setR2(m_rout);
    return StatusCode::SUCCESS; 
  }
  
  Gaudi::Property<double> m_rin{this, "rin", {-1.0}, "Radius of inner layer for keylayer definition"};
  Gaudi::Property<double> m_rout{this, "rout", {-1.0}, "Radius of outer layer for keylayer definition"};

  virtual const std::string &parNames(unsigned i) const override { return m_parNames[i]; }
  virtual unsigned rowParIdx() const override { return 4;}
  virtual std::vector<unsigned> slicePars() const override { return std::vector<unsigned>({0,1}); }
  virtual std::vector<unsigned> scanPars() const override { return std::vector<unsigned>({2,3}); }
  
  virtual double etaHistScale() const override {return 300.0;}
  virtual double phiHistScale() const override {return 60.0;}

  ParSet keyparsToParSet(const FPGATrackSimGenScanKeyLyrHelper::KeyLyrPars& keypars) const {
    return ParSet({keypars.z1,keypars.z2,keypars.phi1,keypars.phi2,keypars.xm});
  }

  FPGATrackSimGenScanKeyLyrHelper::KeyLyrPars parSetToKeyPars(const ParSet &parset) const {
    return FPGATrackSimGenScanKeyLyrHelper::KeyLyrPars(parset);
  }

  virtual const ParSet trackParsToParSet(const FPGATrackSimTrackPars &pars) const override {  
    return keyparsToParSet(m_keylyrtool.trackParsToKeyPars(pars));
  }
  virtual const FPGATrackSimTrackPars parSetToTrackPars(const ParSet &parset) const override {      
    return m_keylyrtool.keyParsToTrackPars(parSetToKeyPars(parset));
  }
  virtual double sliceVarExpected(const ParSet &pars, FPGATrackSimHit const *hit) const override 
  {
    return m_keylyrtool.zExpected(parSetToKeyPars(pars),hit->getR());
  }
  virtual double sliceVar(FPGATrackSimHit const *hit) const override {
    return hit->getZ();
  }
  virtual double rowPar(const ParSet &parset, FPGATrackSimHit const *hit) const override
  {
    return m_keylyrtool.xmForHit(parSetToKeyPars(parset),hit);
  }
  virtual double etaResidual(const ParSet &parset,FPGATrackSimHit const * hit, [[maybe_unused]] bool debug=false) const override {
    return hit->getZ()- m_keylyrtool.zExpected(parSetToKeyPars(parset),hit->getR());    
  }

  virtual double phiResidual(const ParSet &parset,  FPGATrackSimHit const *hit, [[maybe_unused]] bool debug=false) const override {
    return m_keylyrtool.deltaX(parSetToKeyPars(parset),hit);
  }

private:
    FPGATrackSimGenScanKeyLyrHelper m_keylyrtool;
    const std::vector<std::string> m_parNames;

 }; 

//-------------------------------------------------------------------------------------------------------
//
// Key Layer Binning Phi Sliced (then z scanned)
//    -- This scans slices in all phi parameters ("phiR1", "phiR2", "xm")
//       and then scans in in ("zR1", "zR2")
//    -- This was used to explore the throughput of inverting these opperations
//    -- Also the hitInSlice and idxsetToRowParBinRange functions were switched
//       to be simple windows so that its more like real firmware
//
//-------------------------------------------------------------------------------------------------------
class FPGATrackSimGenScanPhiSlicedKeyLyrBinning : public extends<AthAlgTool, FPGATrackSimGenScanBinningBase>
{
public:
  FPGATrackSimGenScanPhiSlicedKeyLyrBinning(const std::string& algname, const std::string &name, const IInterface *ifc) :
  base_class(algname, name, ifc), m_parNames({"zR1", "zR2", "phiR1", "phiR2", "xm"})
  {
    declareInterface<FPGATrackSimGenScanBinningBase>(this);
  }

  virtual StatusCode initialize() override {
    m_keylyrtool.setR1(m_rin);
    m_keylyrtool.setR2(m_rout);
    return StatusCode::SUCCESS; 
  }

  Gaudi::Property<double> m_rin{this, "rin", {-1.0}, "Radius of inner layer for keylayer definition"};
  Gaudi::Property<double> m_rout{this, "rout", {-1.0}, "Radius of outer layer for keylayer definition"};

  virtual const std::string &parNames(unsigned i) const override { return m_parNames[i]; }
  virtual unsigned rowParIdx() const override { return 4;}
  virtual std::vector<unsigned> slicePars() const override { return std::vector<unsigned>({2,3,4}); }
  virtual std::vector<unsigned> scanPars() const override { return std::vector<unsigned>({0,1}); }
  
  virtual double etaHistScale() const override {return 60.0;}
  virtual double phiHistScale() const override {return 30.0;}

  ParSet keyparsToParSet(const FPGATrackSimGenScanKeyLyrHelper::KeyLyrPars& keypars) const {
    return ParSet({keypars.z1,keypars.z2,keypars.phi1,keypars.phi2,keypars.xm});
  }

  FPGATrackSimGenScanKeyLyrHelper::KeyLyrPars parSetToKeyPars(const ParSet &parset) const {
    return FPGATrackSimGenScanKeyLyrHelper::KeyLyrPars(parset);
  }

  virtual const ParSet
  trackParsToParSet(const FPGATrackSimTrackPars &pars) const override {
    return keyparsToParSet(m_keylyrtool.trackParsToKeyPars(pars));
  }
  virtual const FPGATrackSimTrackPars
  parSetToTrackPars(const ParSet &parset) const override {
    return m_keylyrtool.keyParsToTrackPars(parSetToKeyPars(parset));
  }
  
  virtual double phiResidual(const ParSet &parset, FPGATrackSimHit const *hit, [[maybe_unused]] bool debug) const override;
  
  virtual double etaResidual(const ParSet &parset, FPGATrackSimHit const *hit, [[maybe_unused]] bool debug) const override
  {
    return hit->getZ()- m_keylyrtool.zExpected(parSetToKeyPars(parset),hit->getR());
  }

  virtual bool hitInSlice(const IdxSet &idx, FPGATrackSimHit const *hit) const override {
    double r1 = m_keylyrtool.R1();
    double r2 = m_keylyrtool.R2();
    auto keypars = parSetToKeyPars(binCenter(idx));
    auto tmppars = keypars;
    tmppars.xm = m_parStep[4] / 2.0;
    double xrange = m_keylyrtool.xExpected(tmppars, hit) + (r1*m_parStep[2] + (r2*m_parStep[3] - r1*m_parStep[2]) / (r2 - r1) * (hit->getR() - r1))/2.0;
    return (std::abs(phiShift(idx, hit)) < xrange);
  }

  virtual std::pair<unsigned, unsigned> idxsetToRowParBinRange(const IdxSet &idx, [[maybe_unused]] FPGATrackSimHit const *hit) const override
  {
    double r1 = m_keylyrtool.R1();
    double r2 = m_keylyrtool.R2();
    double lowz_in = binLowEdge(0,idx[0]);
    double highz_in = binHighEdge(0,idx[0]);
    double lowz_out = binLowEdge(1,idx[1]);
    double highz_out = binHighEdge(1,idx[1]);


    // simple box cut
    if (hit->getZ() < lowz_in + (lowz_out-lowz_in) * (hit->getR()-r1)/(r2-r1))
      return std::pair<unsigned, unsigned>(0, 0); // empty range

    if (hit->getZ() > highz_in + (highz_out-highz_in) * (hit->getR()-r1)/(r2-r1))
      return std::pair<unsigned, unsigned>(0, 0); // empty range

    return std::pair<unsigned, unsigned>(rowIdx(idx), rowIdx(idx) + 1); // range covers just 1 row bin
  }

private:
    FPGATrackSimGenScanKeyLyrHelper m_keylyrtool;
    const std::vector<std::string> m_parNames;

 }; 

#endif // FPGATrackSimGenScanBinning_H
