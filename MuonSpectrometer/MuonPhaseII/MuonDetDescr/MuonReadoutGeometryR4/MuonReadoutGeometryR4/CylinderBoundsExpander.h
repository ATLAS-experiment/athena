/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSMUONDETECTOR_CYLINDERBOUNDSEXPANDER_H
#define ACTSMUONDETECTOR_CYLINDERBOUNDSEXPANDER_H
#ifndef SIMULATIONBASE
///
#include "MuonReadoutGeometryR4/BoundsExpander.h"

namespace MuonGMR4 {
    /** @brief Auxiliary class to construct cylinder volume bounds from subvolumes
     *         the ranges of the bounds are expanded using the vertices of the subvolume */
    class CylinderBoundsExpander final : public BoundsExpander{
      public:
          explicit CylinderBoundsExpander(std::unique_ptr<const Acts::Logger> loggerObj = nullptr);
          /** @brief Use the expand overloads w.r.t. chamber, sector */
          using BoundsExpander::expand;
          /** @brief Use the makeBounds overload for the readout element */
          using BoundsExpander::makeBounds;
          /** @copydoc BoundsExpander::expand */
          virtual void expand(const Acts::GeometryContext& tgContext,
                              const Acts::Volume& volume) override;
          /** @copydoc BoundsExpander::makeBounds */
          virtual std::shared_ptr<Acts::VolumeBounds> 
                  makeBounds(Acts::VolumeBoundFactory& factory) override final;
          /** @brief Minimum seen radius */
          double rMin() const;
          /** @brief Minimum seen radius (non-const) */
           double& rMin();
          /** @brief Maximum seen radius */
          double rMax() const;
          /** @brief  Maximum seen radius (non-const)*/
          double& rMax();
          /** @brief Minimum seen longitudinal displacement */
          double zMin() const;
          /** @brief Minimum seen longitudinal displacement (non-const) */
           double& zMin();
          /** @brief Maximum seen longitudinal displacement  */
          double zMax() const;
          /** @brief  Maximum seen longitudinal displacement (non-const)*/
          double& zMax();
      private:
        /** @brief Minimum r of any shown vertex */
        double m_rMin{2.*Acts::UnitConstants::km};
        /** @brief Maximum r of any shown vertex */
        double m_rMax{-2.*Acts::UnitConstants::km};
       /** @brief Minimum z of any shown vertex */
       double m_zMin{m_rMin};
       /** @brief Maximum z of any shown vertex */
       double m_zMax{m_rMax};
       /** @brief Extra margin to be applied in R  */
        double m_extraR{0.1};
        /** @brief Extra margin to be applied in z */
        double m_extraZ{0.1};
    };

}
#endif
#endif