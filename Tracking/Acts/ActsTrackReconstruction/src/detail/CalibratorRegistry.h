/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef CALIBRATORREGISTRY_H
#define CALIBRATORREGISTRY_H

#include "src/detail/MakeDerivedVariant.h"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"

template <typename calibrator_type_helper_t, typename measurement_container_variant_t>
struct CalibratorRegistryBase {
   using CalibratorType =calibrator_type_helper_t;

   template <std::size_t DIM, typename T_ValueType>
   using Calibrator = typename CalibratorType::Calibrator<DIM,T_ValueType>;

   // Get the calibrator for the given measurement type T_ValueType, and the measurement dimension
   template <std::size_t DIM, typename T_ValueType>
   const typename calibrator_type_helper_t::Calibrator<DIM, T_ValueType> &calibrator() const;

   // register a calibrator for the given measurement type and dimension.
   // the allowed measurement types and their allowed dimensions are defined by the measurement container variant
   template <std::size_t DIM, typename T_ValueType>
   void setCalibrator(const typename calibrator_type_helper_t::Calibrator<DIM, T_ValueType> &calibrator);

   using CalibratorVariant = ActsTrk::detail::MakeDerivedVariant::MakeVariant< calibrator_type_helper_t,
                                                                               measurement_container_variant_t >::variant_type;

protected:
   std::array< CalibratorVariant, std::variant_size_v<measurement_container_variant_t> > m_calibrators;

   template <std::size_t DIM, typename T_ValueType, std::size_t N>
   static
   const CalibratorVariant &
   getCalibratorGeneric(const std::array< CalibratorVariant, std::variant_size_v<measurement_container_variant_t> > &calibrator_arr);
};

template <typename MeasurementTypeTraits, typename measurement_container_variant_t, typename bound_track_parameters_t>
struct PreCalibratorTypeTraits {
   template <std::size_t DIM>
   using Measurement = typename MeasurementTypeTraits::template Measurement<DIM>;
   template <std::size_t DIM>
   using MeasurementCovariance = typename MeasurementTypeTraits::template MeasurementCovariance<DIM>;

   template <std::size_t DIM, typename measurement_t>
   using Calibrator = Acts::Delegate<
      std::tuple<Measurement<DIM>,MeasurementCovariance<DIM>, unsigned int>
                (const Acts::GeometryContext&,
                 const Acts::CalibrationContext&,
                 const Acts::Surface &,
                 const measurement_t &,
                 const bound_track_parameters_t &)>;

   // the MeasurementContainer is a variant containing pointer to containers
   template <typename T_ContainerPtr>
   using container_type = std::remove_cv_t<std::remove_pointer_t<T_ContainerPtr> >;

   // helper to determine the measurement value type from the container type
   template <typename T_Container>
   using container_value_type = typename T_Container::value_type;

   // this is supposed to  be the value type of the measurement container
   // T_Container is ContainerWithDimension<T>, needed is T::const_value_type where T should
   // be xAOD::PixelClusterContainer which should be DataVector<xAOD::PixelCluster>
   template <typename T_Container>
   using value_type = typename MeasurementTypeTraits::template MeassurementContainerValueType<typename T_Container::container_type >;

   // the calibrators are associated to the element types of the container without pointer and const qualifiers
   template <typename T_ContainerWithDimension>
   using type = Calibrator< T_ContainerWithDimension::dimension() , std::remove_cv_t<std::remove_pointer_t< value_type<T_ContainerWithDimension> > > >;
};

template <typename MeasurementTypeTraits, typename measurement_container_variant_t, typename track_state_proxy_t >
struct CalibratorTypeTraits {
   template <std::size_t DIM>
   using Measurement = typename MeasurementTypeTraits::template Measurement<DIM>;
   template <std::size_t DIM>
   using MeasurementCovariance = typename MeasurementTypeTraits::template MeasurementCovariance<DIM>;

   template <std::size_t DIM, typename measurement_t>
   using Calibrator = Acts::Delegate<
      void (const Acts::GeometryContext&,
            const Acts::CalibrationContext&,
            const measurement_t &,
            track_state_proxy_t &)>;

   // the MeasurementContainer is a variant containing pointer to containers
   template <typename T_ContainerPtr>
   using container_type = std::remove_cv_t<std::remove_pointer_t<T_ContainerPtr> >;

   // helper to determine the measurement value type from the container type
   template <typename T_Container>
   using container_value_type = typename T_Container::value_type;
   
   // this is supposed to  be the value type of the measurement container
   // T_Container is ContainerWithDimension<T>, needed is T::const_value_type where T should
   // be xAOD::PixelClusterContainer which should be DataVector<xAOD::PixelCluster>
   template <typename T_Container>
   using value_type = typename MeasurementTypeTraits::template MeassurementContainerValueType<typename T_Container::container_type >;

   // the calibrators are associated to the element types of the container without pointer and const qualifiers
   template <typename T_ContainerWithDimension>
   using type = Calibrator< T_ContainerWithDimension::dimension() , std::remove_cv_t<std::remove_pointer_t< value_type<T_ContainerWithDimension> > > >;
};

// Helper class to store and retrieve calibrators for a certain measurement type and measurement dimension
// this class only provides calibrators for types and associated dimension which are defined by the measurement container
// and the associated dimension. The template parameter is assumed to be:
// std::variant< ContainerRefWithDim<ContainerType1,N1>, ContainerRefWithDim<ContainerType2,N2>, .... >
// where ContainerType1 is the type of the container e.g. xAOD::PixelClusterContainer, and N1 the associated
// dimension e.g. 2.
template <typename MeasurementTypeTraits, typename measurement_container_variant_t, typename track_state_proxy_t>
using CalibratorRegistry = CalibratorRegistryBase<CalibratorTypeTraits<MeasurementTypeTraits,
                                                                       measurement_container_variant_t,
                                                                       track_state_proxy_t>,
                                                  measurement_container_variant_t>;
template <typename MeasurementTypeTraits, typename measurement_container_variant_t, typename bound_track_parameters_t>
using PreCalibratorRegistry = CalibratorRegistryBase<PreCalibratorTypeTraits<MeasurementTypeTraits,
                                                                             measurement_container_variant_t,
                                                                             bound_track_parameters_t>,
                                                     measurement_container_variant_t>;

// helper to find the variant index for the given measurement type
template <typename calibrator_type_helper_t, typename measurement_container_variant_t>
template <std::size_t DIM, typename T_ValueType, std::size_t N>
inline
const typename CalibratorRegistryBase<calibrator_type_helper_t,measurement_container_variant_t>::CalibratorVariant &
CalibratorRegistryBase<calibrator_type_helper_t, measurement_container_variant_t>
   ::getCalibratorGeneric(const std::array< CalibratorVariant, std::variant_size_v<measurement_container_variant_t> > &calibrator_arr){
   if constexpr(N==1) {
      return calibrator_arr[0];
   }
   else {
     using Container = std::remove_cv_t<std::remove_pointer_t<decltype( ActsTrk::detail::MakeDerivedVariant::lvalue(std::get<N-1>(measurement_container_variant_t{})))> >;
      // Container should be ContainerWithDimension<T>, needed is ContainerWithDimension<T>::contaienr_type
      using ElementType = typename calibrator_type_helper_t::template container_value_type<typename Container::container_type>;
      using BaseElementType = typename std::remove_cv_t<std::remove_pointer_t< ElementType > >;
      if constexpr(std::is_same<T_ValueType,BaseElementType>::value && Container::dimension() == DIM) {
         return calibrator_arr[N-1];
      }
      else {
         return getCalibratorGeneric<DIM, T_ValueType,N-1>(calibrator_arr);
      }
   }
}

template <typename calibrator_type_helper_t, typename measurement_container_variant_t>
template <std::size_t DiM, typename T_ValueType>
inline
void CalibratorRegistryBase<calibrator_type_helper_t, measurement_container_variant_t>
::setCalibrator(const typename calibrator_type_helper_t::Calibrator<DiM, T_ValueType> &calibrator) {
   CalibratorVariant universial { calibrator };
   assert( universial.index() < m_calibrators.size() );
   m_calibrators.at(universial.index()) = std::move( universial);
}

template <typename calibrator_type_helper_t, typename measurement_container_variant_t>
template <std::size_t DIM, typename T_ValueType>
inline
const typename CalibratorRegistryBase<calibrator_type_helper_t, measurement_container_variant_t>::template Calibrator<DIM, T_ValueType> &
CalibratorRegistryBase<calibrator_type_helper_t, measurement_container_variant_t>
::calibrator() const {
   const CalibratorVariant &universal = getCalibratorGeneric< DIM, T_ValueType,std::variant_size_v<measurement_container_variant_t> >(m_calibrators);
   return std::get<Calibrator<DIM, T_ValueType> >(universal);
}
#endif
