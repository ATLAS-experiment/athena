// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//
//   @file    TrigRoiDescriptor.h        
//
//             RoiDescriptor class - includes constructors in preparation
//             for enforcement of correct roi sizes       
//  
//   @author sutt@cern.ch
//


/// nope - should be used for standalone also, perhaps need to 
///        protect the class def bits
/// #ifndef XAOD_ANALYSIS
/// #if !defined(TRIGSTEERINGEVENT_TRIGROIDESCRIPTOR_H)  && !defined(XAOD_STANDALONE)

#ifndef TRIGSTEERINGEVENT_TRIGROIDESCRIPTOR_H
#define TRIGSTEERINGEVENT_TRIGROIDESCRIPTOR_H

#include <stdint.h>
#include <vector>
#include <iosfwd>

#include "RoiDescriptor/RoiDescriptor.h"

#include "xAODCore/CLASS_DEF.h"


/**
 * @brief Describes the Region of Interest geometry
 *  It has basically 9 parameters
 *
 * -# zed : z position of RoI
 * -# zedPlus  : the most forward zed position of the RoI at the beamline
 * -# zedMinus : the most backward zed position of the RoI at the beamline
 * -# phi : phi position of RoI
 * -# phiPlus  : phi at the most anticlockwise position of the RoI
 * -# phiMinus : phi at the most clockwise position of the RoI
 * -# eta : pseudo-rapidity of RoI at zed
 * -# etaPlus  : pseudo-rapidity at zedPlus
 * -# etaMinus : pseudo-rapidity at zedMinus
 * \warning An attempt to construct the objects of this class with phi0 out of allowed range results in throwing exception
 */

class TrigRoiDescriptor : public RoiDescriptor {

public:

  /**
   * @brief default constructor
   * @param fullscan      if fullscan is true, this RoI will span the entire detector
   */
  TrigRoiDescriptor(bool fullscan=false);

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
  TrigRoiDescriptor(double eta, double etaMinus, double etaPlus, 
		    double phi, double phiMinus, double phiPlus, 
		    double zed=0, double zedMinus=-s_zedWidthDefault, double zedPlus=s_zedWidthDefault);
  
  /**
   * @brief constructor
   * @param l1id is event number asigned by LVL1 trigger
   * @param id RoI number within event (0,1,.....)
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
  TrigRoiDescriptor(unsigned int l1id, unsigned int id, 
		    double eta, double etaMinus, double etaPlus, 
		    double phi, double phiMinus, double phiPlus, 
		    double zed=0, double zedMinus=-s_zedWidthDefault, double zedPlus=s_zedWidthDefault);

  /**
   * @brief constructor
   * @param roiWord LVL1 trigger decision word
   * @param l1id is event number asigned by LVL1 trigger
   * @param id RoI number within event (0,1,.....)
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
  TrigRoiDescriptor(unsigned int roiWord, unsigned int l1id, unsigned int id, 
		    double eta, double etaMinus, double etaPlus, 
		    double phi, double phiMinus, double phiPlus, 
		    double zed=0, double zedMinus=-s_zedWidthDefault, double zedPlus=s_zedWidthDefault);


  TrigRoiDescriptor( const IRoiDescriptor& roi );

  TrigRoiDescriptor( const TrigRoiDescriptor& roi );

  TrigRoiDescriptor& operator=( const IRoiDescriptor& roi );

  TrigRoiDescriptor& operator=( const TrigRoiDescriptor& roi );
  
  TrigRoiDescriptor& operator=( TrigRoiDescriptor&& roi ) = default;

  // Destructor
  virtual ~TrigRoiDescriptor();

  virtual unsigned int roiId() const override final { return m_roiId; }
  virtual unsigned int l1Id() const override final { return m_l1Id; }
  virtual unsigned int roiWord() const override final { return m_roiWord; }

  virtual operator std::string() const override;

protected:

  unsigned int m_l1Id;          //!< lvl1 event number
  unsigned int m_roiId;         //!< RoI number
  unsigned int m_roiWord;       //!< lvl1 RoI word from which this RoI was initially constructed

  std::vector<uint32_t> m_serialized;

};


std::string str( const TrigRoiDescriptor& d );                           //<! printing helper
std::ostream& operator<<( std::ostream& m, const TrigRoiDescriptor& d ); //<! printing helper (wraps above)


CLASS_DEF(TrigRoiDescriptor, 6455, 1)

#endif // TRIGROIDESCRIPTOR_H
