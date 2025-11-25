/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_CHAMBER_H
#define MUONREADOUTGEOMETRYR4_CHAMBER_H

#ifndef SIMULATIONBASE


#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <AthenaBaseComps/AthMessaging.h>

namespace Acts {
    class VolumeBounds;
    class Volume;
    class PlaneSurface;
}


namespace MuonGMR4 {
    class SpectrometerSector;

    class Chamber {
        public:
          /** @brief Define the list of read out elements of the chamber */
          using ReadoutSet = std::vector<const MuonReadoutElement*>;
          struct defineArgs{
              /** @brief Associated chamber surface */
              std::shared_ptr<const Acts::PlaneSurface> surface{};             
              /** @brief List of associated readout elements */
              ReadoutSet  detEles{};
              /** @brief Chamber volume bounds */
              std::shared_ptr<Acts::VolumeBounds> bounds{};
          };
          
          /** @brief Standard constructor taking the defineArgs */
          Chamber(defineArgs&& args);
          /** @brief delete the copy constructors */
          Chamber(const Chamber& other) = delete;
          const Chamber& operator=(const Chamber& other) = delete;
          /** @brief Comparison operator for set ordering */
          bool operator<(const Chamber& other) const;
          /** @brief Define a string of the chamber used for debugging */
          std::string identString() const;
          /** @brief Returns a pointer to the idHelperSvc */
          const Muon::IMuonIdHelperSvc* idHelperSvc() const;
          /** @brief Returns the chamber index */
          Muon::MuonStationIndex::ChIndex chamberIndex() const;
          /** @brief Returns the first readout elements detectorType (sorted by techIdx) */
          ActsTrk::DetectorType detectorType() const;
          /** @brief Returns the station phi of the chamber */
          int stationPhi() const;
          /** @brief Returns the station eta of the chamber */
          int stationEta() const;
          /** @brief Returns the side of the chamber */
          int8_t side() const;
          /** @brief Returns an integer representing the stationName */
          int stationName() const;
          /** @brief Returns the MS sector of the chamber */
          int sector() const;
          /** @brief Returns whether the chamber is placed in the barrel */
          bool barrel() const;
          /** @brief Returns the list of contained readout elements */
          const ReadoutSet& readoutEles() const;
          /** @brief Returns the transformation chamber frame -> global transformation
           *  @param gctx: Geometry context carrrying the alignment transformations */
          const Amg::Transform3D& localToGlobalTrans(const ActsTrk::GeometryContext& gctx) const;
          /** @brief Returns the global -> local transformation 
           *  @param gctx: Geometry context carrrying the alignment transformations */
          Amg::Transform3D globalToLocalTrans(const ActsTrk::GeometryContext& gctx) const;
          /** @brief Returns the surface associated with the chamber */
          const Acts::PlaneSurface& surface() const;
          /** @brief Long-extend of the chamber in the x-direction at positive Y */
          double halfXLong() const;
          /** @brief Short extend of the chamber in the x-direction at negative Y*/
          double halfXShort() const;
          /** @brief Extend of the chamber in the y-direction */
          double halfY() const;
          /** @brief Thickness of the chamber in the z-direction */
          double halfZ() const;          
          /** @brief Returns the reference to the defining parameters of the chamber */
          const defineArgs& parameters() const;
          /** @brief Returns the Acts::Volume representation of the chamber.
            * @param gctx: Geometry context carrrying the alignment transformations */
          std::shared_ptr<Acts::Volume> boundingVolume(const ActsTrk::GeometryContext& gctx) const;
          /** @brief Returns the volume bounds */
          std::shared_ptr<Acts::VolumeBounds> bounds() const;
          /** @brief Returns the pointer to the MS sector enclosing the chamber */
          const SpectrometerSector* parent() const;
          /** @brief Sets the connection to the MS sector enclosing the chamber */
          void setParent(const SpectrometerSector* parent);
        private:
            defineArgs m_args{};
            const SpectrometerSector* m_parent{nullptr};
    };
    std::ostream& operator<<(std::ostream& ostr,
                             const Chamber::defineArgs& args);

    std::ostream& operator<<(std::ostream& ostr,
                             const Chamber& chamber);
}
#endif
#endif