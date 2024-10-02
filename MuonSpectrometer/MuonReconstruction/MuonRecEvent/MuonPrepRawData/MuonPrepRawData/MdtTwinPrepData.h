/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MdtTwinPrepData.h
//   Header file for class MdtTwinPrepData
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Class to implement DriftCircle for Muons in Twin Tube layers
///////////////////////////////////////////////////////////////////
// Version 1.0 02/02/2009 Alex Koutsman
///////////////////////////////////////////////////////////////////

#ifndef MUONPREPDATA_MDTTWINPREPDATA_H
#define MUONPREPDATA_MDTTWINPREPDATA_H

// Base classes
#include "MuonPrepRawData/MdtPrepData.h"

class MdtPrepDataContainerCnv;

namespace MuonGM {
  class MdtReadoutElement;
}

namespace Muon
{
  
  /** @brief Class to represent measurements from the Monitored Drift Tubes*/    
  class MdtTwinPrepData :   public MdtPrepData 
  {

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:

    friend class ::MdtPrepDataContainerCnv;


    MdtTwinPrepData() = default;
    MdtTwinPrepData(const MdtTwinPrepData &)  = default;
    MdtTwinPrepData &operator=(const MdtTwinPrepData &) = default;


    /** Constructor with parameters: this class owns the pointers passed (except the MuonDetectorElement)
        @param id Identifier of the tube which generated DriftCircle, 
        @param collectionHash IdentifierHash of the PRD collection in which the MdtTwinPrepData is stored
        @param driftRadius this local position object should contain the drift radius and xTwin(local position along tube) calculated from twin hit
        @param errDriftRadius the error on the driftRadius and xTwin (i.e. 2d error matix)
        @param detEl pointer to the associated detectr element (will not be owned by MdtTwinPrepData - i.e. will not be deleted)
        @param tdc TDC count
        @param adc ADC count
        @param tdc TDC twin count
        @param adc ADC twin count
        @param status enum to indicate whether the measurement is masked, in time, etc (see MdtTwinPrepDataStatus for details).
    */
    MdtTwinPrepData(const Identifier &id,
                    const Amg::Vector2D& driftRadiusXTwin,
                    const Amg::MatrixX& errDriftRadiusXTwin,
                    const MuonGM::MdtReadoutElement* detEl,
                    const int tdc, const int adc,
                    const int tdcTwin, const int adcTwin,
                    const MdtDriftCircleStatus status);

    /** @brief Destructor*/
    virtual ~MdtTwinPrepData() = default;
    
    /** @brief Returns the global position of the hit using twin hit information */
    const Amg::Vector3D& globalPosition() const;

    /** @brief Returns the TDC of twin tube(typically range is 0 to 2500). */
    int tdcTwin() const;

    /** @brief Returns the ADC of twin tube(typically range is 0 to 250)*/
    int adcTwin() const;

    /** @brief Dumps information about the PRD*/
    virtual MsgStream&    dump( MsgStream&    stream) const;
    
    /** @brief Dumps information about the PRD*/
    virtual std::ostream& dump( std::ostream& stream) const;
    
  private:
      /** @brief TDC value of twin tube - typical TDC spectra can go from 0 up to 2500.*/
      int m_tdcTwin{0};

      /** @brief ADC value of twin tube - typical ADC spectra can go from 0 up to 250.*/
      int m_adcTwin{0};
  };
  
  //////////////////////////////////////////////////////////////////
  // Inline methods:
  ///////////////////////////////////////////////////////////////////
  
  
    inline int MdtTwinPrepData::tdcTwin() const { return m_tdcTwin; }
    inline int MdtTwinPrepData::adcTwin() const { return m_adcTwin; }

    // return globalPosition:
    inline const Amg::Vector3D& MdtTwinPrepData::globalPosition() const {
      if (!m_globalPosition){
        Amg::Vector3D gPos{detectorElement()->surface(identify())
                          .Trk::Surface::localToGlobal(localPosition())};
        m_globalPosition.set(std::make_unique<Amg::Vector3D>(std::move(gPos)));
      }
      return *m_globalPosition;
    }
}

#endif // MUONPREPDATA_MDTTWINPREPDATA_H


