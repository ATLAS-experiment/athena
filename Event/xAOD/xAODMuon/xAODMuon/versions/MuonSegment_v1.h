/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUON_VERSIONS_MUONSEGMENT_V1_H
#define XAODMUON_VERSIONS_MUONSEGMENT_V1_H

// System include(s):

// Core include(s):
#include "AthContainers/AuxElement.h"
#include "AthLinks/ElementLink.h"
#include "MuonStationIndex/MuonStationIndex.h"

// xAOD include(s):

// Athena include(s):
#include "GeoPrimitives/GeoPrimitives.h"
#if !(defined(GENERATIONBASE) || defined(XAOD_ANALYSIS))
#include "TrkSegment/SegmentCollection.h"
#endif

// Local include(s):

namespace xAOD {

  /** @brief Class to describe a Segment reconstructed in 
   *         the MuonSpectrometer. A segment is a straight
   *         line approximation of the muon trajectory within
   *         a MS station. The segment carries a reference posiiton,
   *         and a direction. Further is provides information about the
   *         fit quality and the number of precision and trigger hits 
   *         involved in the fit */
  class MuonSegment_v1 : public SG::AuxElement {

  public:

    /// Default constructor
    MuonSegment_v1() = default;
    /// Default destructor
    virtual ~MuonSegment_v1() = default;

    /// @name Global position functions
    /// Returns the global position
    /// @{
    /// Returns the x position
    float x() const;
    /// Returns the x position
    float y() const;
    /// Returns the y position
    float z() const;
    /// Sets the global position
    void setPosition(float x, float y, float z);
    /// @}
    /// @brief Returns the position as Amg::Vector
    Amg::Vector3D position() const;
    /// @name Global direction functions
    /// Returns the global direction
    /// @{
    /// Returns the px
    float px() const;
    /// Returns the py
    float py() const;
    /// Returns the pz
    float pz() const;
    /// Sets the direction
    void setDirection(float px, float py, float pz);
    /// @}
    /// @brief Returns the direction as Amg::Vector
    Amg::Vector3D direction() const;
    /// @name Fitted time functions
    /// Returns some information about fitted time and error on the time.
    /// @{
    /// Returns the time
    float t0() const;
    /// Returns the time error
    float t0error() const;
    /// Sets the time error
    void setT0Error(float t0, float t0Error);
    /// @}

    /// @name Fit quality functions
    /// Returns some information about quality of the track fit.
    /// @{
    /// Returns the @f$ \chi^2 @f$ of the overall track fit.
    float chiSquared() const;
    /// Returns the numberDoF
    float numberDoF() const;
    /// Set the 'Fit Quality' information.
    void setFitQuality(float chiSquared, float numberDoF);
    /// @}

    /// @name Identification
	/// The general muon identification scheme is defined here: https://cds.cern.ch/record/681542/files/com-muon-2002-019.pdf
    /// @{
    /// Returns the sector number
    int sector() const;
    /// Returns the chamber index
    ::Muon::MuonStationIndex::ChIndex chamberIndex() const;
    /// Returns the eta index, which corresponds to stationEta in the offline identifiers (and the ).
    int etaIndex() const;
    /// Returns the main technology of the segment.
    ::Muon::MuonStationIndex::TechnologyIndex technology() const;
    /** @brief Set the Identifier fields of the Segment
      * @param sector: Phi sector in which the segment was constructed [1-6]
      * @param chamberIndex: Chamber index in which the constructed (e.g BIL)
      * @param etaIndex: The eta index of the asociated muon station 
      * @param technology: Technolgy of the precision hits making up the 
      *                    segment. */
    void setIdentifier(const std::uint8_t sector, 
                       const ::Muon::MuonStationIndex::ChIndex chamberIndex, 
                       const std::int8_t etaIndex, 
                       const ::Muon::MuonStationIndex::TechnologyIndex technology);
    /// @}

    /** @brief Returns the number of precision hits */
    std::uint8_t nPrecisionHits() const;
    /** @brief Returns the number of trigger phi hits */
    std::uint8_t nPhiLayers() const;
    /** @brief Returns the number of trigger eta hits */
    std::uint8_t nTrigEtaLayers() const;
    /** @brief Assign the segment hit summary
     *  @param nPrecisionHits: The number of contributin precision hits
     *  @param nPhiLayers: The number of trigger phi hits
     *  @param nTrigEtaLayers: The number of complementary eta trigger hits */
    void setNHits(const std::uint8_t nPrecisionHits, 
                  const std::uint8_t nPhiLayers,
                  const std::uint8_t nTrigEtaLayers);
    /** @brief Assign the number of hits with a large pull per hit category
     *  @param nPrecOutliers: Number of precision outliers
     *  @param nTrigPhiOutliers: Number of trigger phi outliers
     *  @param nTrigEtaOutliers: Number of trigger eta outliers */
    void setNOutliers(const std::uint8_t nPrecOutliers,
                      const std::uint8_t nTrigPhiOutliers,
                      const std::uint8_t nTrigEtaOutliers);
    /** @brief Returns the number of precision outliers */
    std::uint8_t nPrecisionOutliers() const;
    /** @brief Returns the number of trigger phi outliers */
    std::uint8_t nTriggerPhiOutliers() const;
    /** @brief Returns the number of trigger eta outliers */
    std::uint8_t nTriggerEtaOutliers() const;

    /** @brief Assign the number of expected but missing hits
     *  @param nPrecHoles: Number of precision holes
     *  @param nTrigPhiHoles: Number of trigger phi holes
     *  @param nTrigEtaHoles: Number of trigger eta holes */
    void setNHoles(const std::uint8_t nPrecHoles,
                      const std::uint8_t nTrigPhiHoles,
                      const std::uint8_t nTrigEtaHoles);
    /** @brief Returns the number of precision holes */
    std::uint8_t nPrecisionHoles() const;
    /** @brief Returns the number of trigger phi holes */
    std::uint8_t nTriggerPhiHoles() const;
    /** @brief Returns the number of trigger eta holes */
    std::uint8_t nTriggerEtaHoles() const;

#if !(defined(GENERATIONBASE) || defined(XAOD_ANALYSIS))
    const ElementLink< ::Trk::SegmentCollection >& muonSegment() const;
    void setMuonSegment(const ElementLink< ::Trk::SegmentCollection >& segment);
#endif

  }; // end of the MuonSegment_v1 class definitions

} // end of the xAOD namespace

#endif // XAODMUON_VERSIONS_MUONSEGMENT_V1_H
