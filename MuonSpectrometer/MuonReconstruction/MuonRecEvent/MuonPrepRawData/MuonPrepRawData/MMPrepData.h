/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONPREPRAWDATA_MMPREPDATA_H
#define MUONPREPRAWDATA_MMPREPDATA_H

// Base classes
#include "MuonPrepRawData/MuonCluster.h"
#include "TrkSurfaces/Surface.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonPrepRawData/NswClusteringUtils.h"

#include <vector>


namespace Muon {



  /** @brief Class to represent MM measurements. */
  class MMPrepData final: public MuonCluster {

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:

    MMPrepData() = default;
    MMPrepData(const MMPrepData &) = delete;
    MMPrepData &operator=(const MMPrepData &) = delete;
    MMPrepData(MMPrepData &&) noexcept = default;
    MMPrepData &operator=(MMPrepData &&) noexcept = default;


    /** @brief Constructor.
	@param RDOId The identifier of the central strip of the cluster
	@param locpos The local coords of the measurement (this object will now own the LocalPostion)
	@param rdoList Vector of all the Identifiers of the strips used in this cluster
	@param locErrMat The error of the measurement (this object will now own the ErrorMatrix)
	@param detEl The pointer to the Detector Element on which this measurement was made (must NOT be zero). Ownership is NOT taken
	(the pointer is assumed to belong to GeoModel and will not be deleted)
    */

   /** @brief full constructor including time, charge and strip vectors */
    MMPrepData( const Identifier& RDOId,
		const IdentifierHash &idDE,
		Amg::Vector2D&& locpos,
		std::vector<Identifier>&& rdoList,
		Amg::MatrixX&& locErrMat,
		const MuonGM::MMReadoutElement* detEl,
		const short int time,
		const int charge,
		const float driftDist,
		std::vector<uint16_t>&& stripNumbers,
		std::vector<short int>&& stripTimes,
		std::vector<int>&& stripCharges );

    /** @brief constructor including time and charge and drift distance */
    MMPrepData( const Identifier& RDOId,
		const IdentifierHash &idDE,
		Amg::Vector2D&& locpos,
		std::vector<Identifier>&& rdoList,
		Amg::MatrixX&& locErrMat,
		const MuonGM::MMReadoutElement* detEl,
		const short int time,
		const int charge,
		const float driftDist );

    /** @brief constructor including time and charge */
    MMPrepData( const Identifier& RDOId,
		const IdentifierHash &idDE,
		Amg::Vector2D&& locpos,
		std::vector<Identifier>&& rdoList,
		Amg::MatrixX&& locErrMat,
		const MuonGM::MMReadoutElement* detEl,
		const short int time,
		const int charge );

    MMPrepData( const Identifier& RDOId,
		const IdentifierHash &idDE,
		Amg::Vector2D&& locpos,
		std::vector<Identifier>&& rdoList,
		Amg::MatrixX&& locErrMat,
		const MuonGM::MMReadoutElement* detEl);

    /** @brief Destructor: */
    virtual ~MMPrepData() = default;

    /** @brief set microTPC parameters */
    void setMicroTPC(float angle, float chisqProb);

    /** @brief set drift distances and uncertainties */
    void setDriftDist(std::vector<float>&& driftDist,
                      std::vector<AmgVector(2)>&& driftDistErrors);

    // setter functions for the EventTPConverters
    void setDriftDist(std::vector<float>&& driftDist,
                      std::vector<float>&& stripDriftErrors_0_0,
                      std::vector<float>&& stripDriftErrors_1_1);

    /** @brief Returns the global position*/
    virtual const Amg::Vector3D& globalPosition() const override final;

    /** @brief Returns the detector element corresponding to this PRD.
	The pointer will be zero if the det el is not defined (i.e. it was not passed in by the ctor)*/
    virtual const MuonGM::MMReadoutElement* detectorElement() const override final;

    /** Interface method checking the type*/
    virtual bool type(Trk::PrepRawDataType type) const override final
    {
      return type == Trk::PrepRawDataType::MMPrepData;
    }

    /** @brief Returns the time (in ns) */
    short int time() const;

    /** @brief Returns the AD */
    int charge() const;

    /** @brief Returns the Drift Distance */
    float driftDist() const;

    /** @brief Returns the microTPC angle */
    float angle() const;

    /** @brief Returns the microTPC chisq Prob. */
    float chisqProb() const;

    /** @brief returns the list of strip numbers */
    const std::vector<uint16_t>& stripNumbers() const;

    /** @brief returns the list of times */
    const std::vector<short int>& stripTimes() const;

    /** @brief returns the list of charges */
    const std::vector<int>& stripCharges() const;

    /** @brief returns the list of drift distances */
    const std::vector<float>& stripDriftDist() const;

    /** @brief returns the list of drift distances */
    const std::vector<AmgVector(2)>& stripDriftErrors() const;

    // getter functions for the EventTPConverters
    std::vector<float> stripDriftErrors_0_0() const;
    std::vector<float> stripDriftErrors_1_1() const;

    /** @brief Dumps information about the PRD*/
    virtual MsgStream&    dump( MsgStream&    stream) const override final;

    /** @brief Dumps information about the PRD*/
    virtual std::ostream& dump( std::ostream& stream) const override final;


    enum class Author: short{
      RDOTOPRDConverter = -1,
      SimpleClusterBuilder,
      ClusterTimeProjectionClusterBuilder,
      uTPCClusterBuilder,
      nAuthorsPRD
    };

    enum class Quality: uint8_t{
       unKnown = 0,
    };

    Quality quality() const { return m_quality; }
    void setQuality(const Quality q)  { m_quality = q; }


    Author author() const;
    void setAuthor(Author author);

  private:

    /** @brief Cached pointer to the detector element - should never be zero.*/
    const MuonGM::MMReadoutElement* m_detEl{nullptr};

    /** @brief measured time */
    /** @brief the time is calibrated, i.e. it is in units of ns, after t0 subtraction **/
    short int m_time{0};

    /** @brief measured charge */
    /** @brief the charge is calibrated, i.e. it is in units of electrons, after pedestal subtraction **/
    int m_charge{0};

    /** @brief drift distance */
    float m_driftDist{0.f};

    /** @angle and chisquare from micro-TPC fit */
    float m_angle{0.f};
    float m_chisqProb{0.f};

    /** @list of strip numbers, time and charge, of the strips associated to the PRD */
    std::vector<uint16_t> m_stripNumbers{};
    std::vector<short int> m_stripTimes{};
    std::vector<int> m_stripCharges{};
    std::vector<float> m_stripDriftDist{};
    std::vector<AmgVector(2)>  m_stripDriftErrors{};
    Author m_author{Author::nAuthorsPRD};
    Quality m_quality{Quality::unKnown};

  };

  inline const MuonGM::MMReadoutElement* MMPrepData::detectorElement() const
  {
    return m_detEl;
  }
  // return globalPosition:
  inline const Amg::Vector3D& MMPrepData::globalPosition() const
  {
    if (!m_globalPosition) {
      m_globalPosition.set(std::make_unique<Amg::Vector3D>(
        m_detEl->surface(identify())
          .Trk::Surface::localToGlobal(localPosition())));
    }
    if (not m_globalPosition) throw Trk::PrepRawDataUndefinedVariable();
    return *m_globalPosition;
  }

  inline short int MMPrepData::time() const
  {
    return m_time;
  }

  inline int MMPrepData::charge() const
  {
    return m_charge;
  }

  inline float MMPrepData::driftDist() const
  {
    return m_driftDist;
  }

  inline float MMPrepData::angle() const
  {
    return m_angle;
  }

  inline float MMPrepData::chisqProb() const
  {
    return m_chisqProb;
  }

  inline const std::vector<uint16_t>& MMPrepData::stripNumbers() const
  {
    return m_stripNumbers;
  }

  inline const std::vector<short int>& MMPrepData::stripTimes() const
  {
    return m_stripTimes;
  }

  inline const std::vector<int>& MMPrepData::stripCharges() const
  {
    return m_stripCharges;
  }

  inline const std::vector<float>& MMPrepData::stripDriftDist() const
  {
    return m_stripDriftDist;
  }

  inline const std::vector<AmgVector(2)>& MMPrepData::stripDriftErrors() const
  {
    return m_stripDriftErrors;
  }

  inline MMPrepData::Author MMPrepData::author() const {
    return m_author;
  }

}

#endif // MUONPREPRAWDATA_MMREPDATA_H

