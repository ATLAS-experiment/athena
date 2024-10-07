/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_SPACEPOINT_COLLECTOR_H
#define ACTSTRK_SPACEPOINT_COLLECTOR_H

#include "TrkSurfaces/Surface.h"

// EDMs
#include "xAODInDetMeasurement/SpacePointContainer.h"

#include <math.h>
#include <cmath>
#include "Acts/EventData/SpacePointContainer.hpp"
#include "Acts/Definitions/Algebra.hpp"

namespace ActsTrk {
  
  class SpacePointCollector {
  public:  
    friend Acts::SpacePointContainer<ActsTrk::SpacePointCollector, Acts::detail::RefHolder>;
    
    using ValueType = xAOD::SpacePoint;
    
    SpacePointCollector(std::vector<const ValueType*>& externalStorage);
    
    SpacePointCollector(const SpacePointCollector&) = delete;
    SpacePointCollector(SpacePointCollector&&) noexcept = default;
    SpacePointCollector& operator=(const SpacePointCollector&) = delete;
    SpacePointCollector& operator=(SpacePointCollector&&) noexcept = default;
    virtual ~SpacePointCollector() = default;
    
    std::size_t size_impl() const;
    
    float x_impl(std::size_t idx) const;
    float y_impl(std::size_t idx) const;
    float z_impl(std::size_t idx) const;

    float varianceR_impl(std::size_t idx) const;
    float varianceZ_impl(std::size_t idx) const;

    const ValueType& get_impl(std::size_t idx) const;
    
    std::any component_impl(Acts::HashedString key, std::size_t n) const {
      using namespace Acts::HashedStringLiteral;

      const ValueType& sp = get_impl(n);      
      static const SG::ConstAccessor<xAOD::ArrayFloat3> topStripDirAcc("topStripDirection");
      if (not topStripDirAcc.isAvailable(sp)) {
	throw std::runtime_error("no such component " + std::to_string(key) + " for this space point");
      }      
      
      switch (key) {
      case "TopStripVector"_hash:
	return Acts::Vector3( sp.topHalfStripLength() * sp.topStripDirection().cast<double>() );
      case "BottomStripVector"_hash:
	return Acts::Vector3( sp.bottomHalfStripLength() * sp.bottomStripDirection().cast<double>() );
      case "StripCenterDistance"_hash:
	return Acts::Vector3( sp.stripCenterDistance().cast<double>() );
      case "TopStripCenterPosition"_hash:
	return Acts::Vector3( sp.topStripCenter().cast<double>() );
      default:
	throw std::runtime_error("no such component " + std::to_string(key));	  
      }
    }
    
  private:
    const std::vector<const ValueType*>& storage() const;
    std::vector<const ValueType*>& storage();
    
  private:
    //    xAOD::SpacePointContainer
    std::vector<const ValueType*>* m_storage {nullptr};
  };
  
  inline std::size_t
    SpacePointCollector::size_impl() const 
  { return storage().size(); }
  
  inline float 
    SpacePointCollector::x_impl(std::size_t idx) const
  { return storage()[idx]->x(); }

  inline float 
    SpacePointCollector::y_impl(std::size_t idx) const 
  { return storage()[idx]->y(); }

  inline float 
    SpacePointCollector::z_impl(std::size_t idx) const 
  { return storage()[idx]->z(); }

  inline float 
    SpacePointCollector::varianceR_impl(std::size_t idx) const
  { return storage()[idx]->varianceR(); }

  inline float 
    SpacePointCollector::varianceZ_impl(std::size_t idx) const
  { return storage()[idx]->varianceZ(); }

  inline const typename SpacePointCollector::ValueType&
    SpacePointCollector::get_impl(std::size_t idx) const
  { 
    return *storage()[idx];
  }

  inline const std::vector<const typename SpacePointCollector::ValueType*>&
    SpacePointCollector::storage() const 
  { return *m_storage; }

  inline std::vector<const typename SpacePointCollector::ValueType*>&
    SpacePointCollector::storage()
  { return *m_storage; }
  
} // namespace ActsTrk

#endif
