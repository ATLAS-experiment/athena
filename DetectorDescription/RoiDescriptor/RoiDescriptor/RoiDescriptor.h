// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//
//   @file    RoiDescriptor.h        
//
//             RoiDescriptor class - includes constructors in preparation
//             for enforcement of correct roi sizes       
//  
//   @author sutt@cern.ch
//


#ifndef ROIDESCRIPTOR_ROIDESCRIPTOR_H
#define ROIDESCRIPTOR_ROIDESCRIPTOR_H

#include <iostream>
#include <atomic>

#include "IRegionSelector/IRoiDescriptor.h"
#include "IRegionSelector/RoiUtil.h"


/**
 * @brief Describes the Region of Ineterest geometry
 *  It has basically 9 parameters
 *
 * -# zed          : central z position of RoI
 * -# zedMinus     : most negative z position of the roi
 * -# zedPlus      : most positive z position of the roi
 * -# phi          : azimuthal angle (radians) of "centre" of RoI at origin in range from [-pi, pi]
 * -# phiMinus     : the most      clockwise (negative phi direction) phi value for the RoI
 * -# phiPlus      : the most anti-clockwise (positive phi direction) phi value for the RoI
 * -# eta          : pseudo-rapidity of "centre" of RoI 
 * -# etaMinus     : pseudo-rapidity at zedMinus
 * -# etaPlus      : pseudo-rapidity at zedPlus
 */

//coverity[MISSING_MOVE_ASSIGNMENT]
class RoiDescriptor : public IRoiDescriptor {

public:

  /// convenient
  static constexpr bool FULLSCAN = true;
  static constexpr bool ROI = false;

public:

  /**
   * @brief default constructor
   * @param fullscan      if fullscan is true, this RoI will span the entire detector
   */
  RoiDescriptor(bool fullscan=ROI);

  /**
   * @brief constructor
   * @param eta      eta of RoI
   * @param etaMinus eta at rear  of RoI
   * @param etaPlus  eta at front of RoI
   * @param phi      phi of RoI
   * @param phiMinus minimum phi of RoI
   * @param phiPlus  maximum phi of RoI
   * @param zed      zed of RoI
   * @param zedMinus zed at rear  of RoI
   * @param zedPlus  zed at front of RoI
   */
  RoiDescriptor(double eta_,   double etaMinus_,   double etaPlus_, 
		double phi_,   double phiMinus_,   double phiPlus_, 
		double zed_=0, double zedMinus_=-s_zedWidthDefault, double zedPlus_=s_zedWidthDefault   );
  /**
   * @brief constructor
   * @param etaMinus eta at rear  of RoI
   * @param etaPlus  eta at front of RoI
   * @param phiMinus minimum phi of RoI
   * @param phiPlus  maximum phi of RoI
   */
  RoiDescriptor(double etaMinus_, double etaPlus_, 
		double phiMinus_, double phiPlus_ ); 

  /**
   * @brief copy constructors
   */
  RoiDescriptor( const IRoiDescriptor& roi );
  RoiDescriptor& operator=( const IRoiDescriptor& r );  

  /*
   *  need an explicit class copy constructor
   */
  RoiDescriptor( const RoiDescriptor& roi );
  RoiDescriptor& operator=( const RoiDescriptor& r );  

  
  // Destructor
  virtual ~RoiDescriptor();


  // Methods to retrieve data members

  virtual double phi() const override final { return m_phi; }
  virtual double eta() const override final { return m_eta; }
  virtual double zed() const override final { return m_zed; }

  /// these quantities probably don't need to be used any more
  /// - they are implemented here only because we had them in 
  ///   the original legacy interface
 
  virtual unsigned int roiId()   const override { return 0; }
  virtual unsigned int l1Id()    const override { return 0; }
  virtual unsigned int roiWord() const override { return 0; }

  virtual double zedPlus()  const override final { return m_zedPlus; } //!< z at the most forward end of the RoI
  virtual double zedMinus() const override final { return m_zedMinus; } //!< z at the most backward end of the RoI

  virtual double etaPlus()  const override final { return m_etaPlus; }   //!< gets eta at zedPlus
  virtual double etaMinus() const override final { return m_etaMinus; }   //!< gets eta at zMinus

  virtual double phiPlus()  const override final { return m_phiPlus; }    //!< gets phiPlus
  virtual double phiMinus() const override final { return m_phiMinus; }   //!< gets phiMinus


  /// versioning 
  virtual int version() const override final { return m_version; }
  void version(int v)                  { m_version = v; }


  /// output
  virtual operator std::string() const override;


  /// is this a full scan RoI?
  virtual bool  isFullscan() const override final { return m_fullscan; }
 
  /// SuperRoI compatability methods

  /// am I a SuperRoi?
  virtual bool composite() const override final { return m_composite; }
  void setComposite(bool b=true)          { m_composite=b; }

  /// always manage constituents ???
  bool manageConstituents() const { return m_manageConstituents; }
  void manageConstituents(bool b) { m_manageConstituents=b; }

  /// number of constituents
  virtual unsigned size() const override final { return m_roiDescriptors.size(); }

  /// find an RoiDescriptor constituent
  virtual const IRoiDescriptor* at(int i) const override final { return m_roiDescriptors.at(i); }

  /// clear the vector
  void clear()  { m_roiDescriptors.clear(); }  // setComposite(false); }

  /// reserve elements in vector
  void reserve(size_t s) { m_roiDescriptors.reserve(s); }

  /// add a RoiDescriptor
  void push_back(const IRoiDescriptor* roi) { m_roiDescriptors.push_back(roi); setComposite(true); }

  /// iterators
  virtual roi_iterator  begin() const override final { return m_roiDescriptors.begin(); }
  virtual roi_iterator  end()   const override final { return m_roiDescriptors.end(); }

  /// return the gradients
  virtual double dzdrMinus() const override final { return m_dzdrMinus; }       //!<  dz/dr at the rear of the RoI
  virtual double dzdrPlus()  const override final { return m_dzdrPlus; }        //!<  dz/dr at the front of the RoI

  virtual double drdzMinus() const override final { return m_drdzMinus; }       //!<  dr/dz at the rear of the RoI
  virtual double drdzPlus()  const override final { return m_drdzPlus; }        //!<  dr/dz at the front of the RoI

  /// methods to calculate z position at the RoI boundary 
  /// at a given radius
  virtual double zedMin(double r) const override final;
  virtual double zedMax(double r) const override final;

  virtual double zedOuterPlus()  const override final { return m_zedOuterPlus; } //!< z at the most forward end of the RoI
  virtual double zedOuterMinus() const override final { return m_zedOuterMinus; } //!< z at the most backward end of the RoI

  virtual double rhoMin(double z) const override final;
  virtual double rhoMax(double z) const override final;

public:

  static double zedWidthDefault() { return s_zedWidthDefault; }

  /// set default z-width (but only before any RoiDescriptor has been created)
  static void zedWidthDefault( double d );

protected:

  friend class TrigRoiDescriptorCnv_p2;
  friend class TrigRoiDescriptorCnv_p3;

  /**
   * @brief construct RoiDescriptor internals -  similar to constructors
   * @see  RoiDescriptor( .... )
   */
  void construct(double eta, double etaMinus, double etaPlus, 
		 double phi, double phiMinus, double phiPlus, 
		 double zed, double zedMinus, double zedPlus); 

  void construct( const IRoiDescriptor& _roi );

protected:

  /// default parameters - there may be better ways, but this will do
  static std::atomic<double> s_zedWidthDefault;
  /// to ensure default width is only set once at job startup
  static std::atomic<bool> s_firstInstanceCreated;

protected:

  float m_phi;                 //!< phi of RoI center
  float m_eta;                 //!< eta of RoI center
  float m_zed;                 //!< zed of RoI center

  float m_phiMinus;            //!< most negative RoI in azimuthal
  float m_phiPlus;             //!< most positive RoI in azimuthal 
  float m_etaMinus;             //!< eta of RoI at zedMinus
  float m_etaPlus;              //!< eta of RoI at zedPlus
  float m_zedMinus;             //!< z position at most negative position along the beamline
  float m_zedPlus;              //!< z position at most positive position along the beamline

  float m_dzdrMinus;       //!<  dz/dr at the rear of the RoI
  float m_dzdrPlus;        //!<  dz/dr at the front of the RoI

  float m_drdzMinus;    //!<  dr/dz at the rear of the RoI
  float m_drdzPlus;     //!<  dr/dz at the front of the RoI

  float m_zedOuterMinus;  //!< z at rear of RoI at the outer radius ( = 1100 mm) 
  float m_zedOuterPlus;   //!< z at front of RoI at the outer radius ( = 1100 mm) 

  bool m_fullscan;             //!< flag this as a full detector RoI
  bool m_composite;            //!< flag this as a composite RoI
  bool m_manageConstituents;   //!< flag to determine whether consituents should be managed

  int  m_version;              //!< transient version identifier

  std::vector<const IRoiDescriptor*>  m_roiDescriptors;  //!< roi constituents
  
};


std::string str( const RoiDescriptor& d );                           //<! printing helper
std::ostream& operator<<( std::ostream& m, const RoiDescriptor& d ); //<! printing helper (wraps above)

#endif // ROIDESCRIPTOR_H

