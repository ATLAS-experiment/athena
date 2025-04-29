/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#ifndef CALIBRATORREGISTRY_H
#define CALIBRATORREGISTRY_H

#include "src/detail/MakeDerivedVariant.h"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"

// Helper class to store and retrieve calibrators for a certain measurement type and measurement dimension
// this class only provides calibrators for types and associated dimension which are defined by the measurement container
// and the associated dimension. The template parameter is assumed to be:
// std::variant< ContainerRefWithDim<ContainerType1,N1>, ContainerRefWithDim<ContainerType2,N2>, .... >
// where ContainerType1 is the type of the container e.g. xAOD::PixelClusterContainer, and N1 the associated
// dimension e.g. 2.
template <typename MeasurementTypeTraits, typename bound_track_parameters_t, typename measurement_container_variant_t>
struct CalibratorRegistry {
   template <std::size_t DIM>
   using Measurement = typename MeasurementTypeTraits::template Measurement<DIM>;
   template <std::size_t DIM>
   using MeasurementCovariance = typename MeasurementTypeTraits::template MeasurementCovariance<DIM>;

   template <std::size_t DIM, typename measurement_t>
   using Calibrator = Acts::Delegate<
      std::pair<Measurement<DIM>,MeasurementCovariance<DIM> >
                (const Acts::GeometryContext&,
                 const Acts::CalibrationContext&,
                 const measurement_t &,
                 const bound_track_parameters_t &)>;

   // Get the calibrator for the given measurement type T_ValueType, and the measurement dimension
   template <std::size_t DIM, typename T_ValueType>
   const Calibrator<DIM, T_ValueType> &calibrator() const;

   // register a calibrator for the given measurement type and dimension.
   // the allowed measurement types and their allowed dimensions are defined by the measurement container variant
   template <std::size_t DIM, typename T_ValueType>
   void setCalibrator(const Calibrator<DIM, T_ValueType> &calibrator);

private:
   // helper for type specific calibrator storage
   struct CalibratorType {
      // the MeasurementContainer is a variant containing pointer to containers
      template <typename T_ContainerPtr>
      using container_type = std::remove_cv_t<std::remove_pointer_t<T_ContainerPtr> >;

      // this is supposed to  be the value type of the measurement container
      // T_Container is ContainerWithDimension<T>, needed is T::const_value_type where T should
      // be xAOD::PixelClusterContainer which should be DataVector<xAOD::PixelCluster>
      template <typename T_Container>
      using value_type = typename MeasurementTypeTraits::template MeassurementContainerValueType<typename T_Container::container_type >;

      // the calibrators are associated to the element types of the container without pointer and const qualifiers
      template <typename T_ContainerWithDimension>
      using type = Calibrator< T_ContainerWithDimension::dimension() , std::remove_cv_t<std::remove_pointer_t< value_type<T_ContainerWithDimension> > > >;
   };

  using CalibratorVariant = ActsTrk::detail::MakeDerivedVariant::MakeVariant< CalibratorType, measurement_container_variant_t >::variant_type;

   std::array< CalibratorVariant, std::variant_size_v<measurement_container_variant_t> > m_calibrators;

   template <std::size_t DIM, typename T_ValueType, std::size_t N>
   static
   const CalibratorVariant &
   getCalibratorGeneric(const std::array< CalibratorVariant, std::variant_size_v<measurement_container_variant_t> > &calibrator_arr);
};

// helper to find the variant index for the given measurement type
template <typename MeasurementTypeTraits, typename bound_track_parameters_t, typename measurement_container_variant_t>
template <std::size_t DIM, typename T_ValueType, std::size_t N>
inline
const typename CalibratorRegistry<MeasurementTypeTraits, bound_track_parameters_t, measurement_container_variant_t>::CalibratorVariant &
CalibratorRegistry<MeasurementTypeTraits, bound_track_parameters_t, measurement_container_variant_t>
   ::getCalibratorGeneric(const std::array< CalibratorVariant, std::variant_size_v<measurement_container_variant_t> > &calibrator_arr){
   if constexpr(N==1) {
      return calibrator_arr[0];
   }
   else {
     using Container = std::remove_cv_t<std::remove_pointer_t<decltype( ActsTrk::detail::MakeDerivedVariant::lvalue(std::get<N-1>(measurement_container_variant_t{})))> >;
      // Container should be ContainerWithDimension<T>, needed is ContainerWithDimension<T>::contaienr_type
      using ElementType = typename MeasurementTypeTraits::template MeassurementContainerValueType<typename Container::container_type>;
      using BaseElementType = typename std::remove_cv_t<std::remove_pointer_t< ElementType > >;
      if constexpr(std::is_same<T_ValueType,BaseElementType>::value && Container::dimension() == DIM) {
         return calibrator_arr[N-1];
      }
      else {
         return getCalibratorGeneric<DIM, T_ValueType,N-1>(calibrator_arr);
      }
   }
}

template <typename MeasurementTypeTraits, typename bound_track_parameters_t, typename measurement_container_variant_t>
template <std::size_t DiM, typename T_ValueType>
inline
void CalibratorRegistry<MeasurementTypeTraits, bound_track_parameters_t, measurement_container_variant_t>
::setCalibrator(const Calibrator<DiM, T_ValueType> &calibrator) {
   CalibratorVariant universial { calibrator };
   assert( universial.index() < m_calibrators.size() );
   m_calibrators.at(universial.index()) = std::move( universial);
}

template <typename MeasurementTypeTraits, typename bound_track_parameters_t, typename measurement_container_variant_t>
template <std::size_t DIM, typename T_ValueType>
inline
const typename CalibratorRegistry<MeasurementTypeTraits, bound_track_parameters_t, measurement_container_variant_t>::template Calibrator<DIM, T_ValueType> &
CalibratorRegistry<MeasurementTypeTraits, bound_track_parameters_t, measurement_container_variant_t>
::calibrator() const {
   const CalibratorVariant &universal = getCalibratorGeneric< DIM, T_ValueType,std::variant_size_v<measurement_container_variant_t> >(m_calibrators);
   return std::get<Calibrator<DIM, T_ValueType> >(universal);
}
#endif
