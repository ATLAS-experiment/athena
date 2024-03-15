/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONPREPRAWDATA_STGCPREPDATA_H
#define MUONPREPRAWDATA_STGCPREPDATA_H

// Base classes
#include "MuonPrepRawData/MuonCluster.h"
#include "TrkSurfaces/Surface.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "MuonPrepRawData/NswClusteringUtils.h"


namespace Muon {



  /** @brief Class to represent sTgc measurements. */
  class sTgcPrepData final: public MuonCluster {


    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:
    sTgcPrepData() = default;
    sTgcPrepData(const sTgcPrepData &) = delete;
    sTgcPrepData(sTgcPrepData &&) noexcept = default;
    sTgcPrepData &operator=(const sTgcPrepData &) = delete;
    sTgcPrepData &operator=(sTgcPrepData &&) noexcept = default;

    /** @brief Constructor.
    @param RDOId The identifier of the central strip of the cluster
    @param locpos The local coords of the measurement (this object will now own the LocalPostion)
    @param rdoList Vector of all the Identifiers of the strips used in this cluster
    @param locErrMat The error of the measurement (this object will now own the ErrorMatrix)
    @param detEl The pointer to the Detector Element on which this measurement was made (must NOT be zero). Ownership is NOT taken
    (the pointer is assumed to belong to GeoModel and will not be deleted)
    @param bcBitMap bitmap storing in which bunches the channel fired
    */
    sTgcPrepData( const Identifier& RDOId,
                  const IdentifierHash &idDE,
                  Amg::Vector2D&& locpos,
                  std::vector<Identifier>&& rdoList,
                  Amg::MatrixX&& locErrMat,
                  const MuonGM::sTgcReadoutElement* detEl,
                  const int charge,
                  const short int time,
                  std::vector<uint16_t>&& stripNumbers,
                  std::vector<short int>&& stripTimes,
                  std::vector<int>&& stripCharges );


    sTgcPrepData( const Identifier& RDOId,
                  const IdentifierHash &idDE,
                  Amg::Vector2D&& locpos,
                  std::vector<Identifier>&& rdoList,
                  Amg::MatrixX&& locErrMat,
                  const MuonGM::sTgcReadoutElement* detEl,
                  const int charge = 0,
                  const short int time = 0);

    /** @brief Destructor: */
    virtual ~sTgcPrepData() = default;

    /** @brief Returns the global position*/
    virtual const Amg::Vector3D& globalPosition() const override final;

    /** @brief Returns the detector element corresponding to this PRD.
    The pointer will be zero if the det el is not defined (i.e. it was not passed in by the ctor)*/
    virtual const MuonGM::sTgcReadoutElement* detectorElement() const override final;

    /** Interface method checking the type*/
    virtual bool type(Trk::PrepRawDataType type) const override final
    {
      return type == Trk::PrepRawDataType::sTgcPrepData;
    }

    /** @brief Dumps information about the PRD*/
    virtual MsgStream&    dump( MsgStream&    stream) const override final;

    /** @brief Dumps information about the PRD*/
    virtual std::ostream& dump( std::ostream& stream) const override final;

    /** @brief Returns the bcBitMap of this PRD
    bit2 for Previous BC, bit1 for Current BC, bit0 for Next BC */
    int charge() const;
    short int time() const;
    
    /** @brief returns the list of strip numbers */
    const std::vector<uint16_t>& stripNumbers() const;

    /** @brief returns the list of times */
    const std::vector<short int>& stripTimes() const;

     /** @brief returns the list of charges */
    const std::vector<int>& stripCharges() const;

    enum class Author: uint8_t{
       unKnown = 0,
       RdoToPrdConverter,
       TrigProcessor,
       SimpleClusterBuilder,
       Caruana
    };

    enum class Quality: uint8_t{
       unKnown = 0,
    };

    Quality quality() const { return m_quality; }
    void setQuality(const Quality q) { m_quality = q; }

    Author author() const { return m_author; }
    void setAuthor(const Author a) { m_author = a; }
  private:

    /** Cached pointer to the detector element - should never be zero.*/
    const MuonGM::sTgcReadoutElement* m_detEl{nullptr};
    int m_charge{0};
    short int m_time{0};
    /** @list of strip numbers, time and charge, of the strips associated to the PRD */
    std::vector<uint16_t> m_stripNumbers{};
     /**
     strip times for the sTGCs will be available for the commissioning of the NSW.
     Later the tdo (time) will not be in the datastream due to high occupancy.
     Then this an empty vector which does not take much space
     Patrick Scholer 3.12.2019
         */
    std::vector<short int> m_stripTimes{};
    std::vector<int> m_stripCharges{};

    Quality m_quality{Quality::unKnown};
    Author m_author{Author::unKnown};

  };

  inline const MuonGM::sTgcReadoutElement* sTgcPrepData::detectorElement() const {
    return m_detEl;
  }
  // return globalPosition:
  inline const Amg::Vector3D& sTgcPrepData::globalPosition() const {
    if (not m_globalPosition) {
      m_globalPosition.set(std::make_unique<Amg::Vector3D>(
        m_detEl->surface(identify())
          .Trk::Surface::localToGlobal(localPosition())));
    }

    if (not m_globalPosition) throw Trk::PrepRawDataUndefinedVariable();
    return *m_globalPosition;
  }

  inline int sTgcPrepData::charge() const { return m_charge; }

  inline short int sTgcPrepData::time() const { return m_time; }

  inline const std::vector<uint16_t>& sTgcPrepData::stripNumbers() const { return m_stripNumbers; }
  inline const std::vector<short int>& sTgcPrepData::stripTimes() const { return m_stripTimes; }
  inline const std::vector<int>& sTgcPrepData::stripCharges() const { return m_stripCharges; }

}

#endif // MUONPREPRAWDATA_STGCPREPDATA_H

