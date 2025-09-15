/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#ifndef MEASUREMENTCONTAINERWITHDIMENSION_H
#define MEASUREMENTCONTAINERWITHDIMENSION_H
#include <variant>
#include <vector>
#include <sstream>
#include <algorithm>

// Helper type to associate the measurement dimension to the pointer to a measurement container
template <typename container_t, std::size_t DIM>
struct ContainerRefWithDim {
public:
   using container_type = container_t;
   ContainerRefWithDim() : m_container(nullptr) {}
   ContainerRefWithDim(const container_t &container) : m_container(&container) {}
   static constexpr std::size_t s_dimension = DIM;
   // the dimension of the measurements stored in this container
   static constexpr std::size_t dimension() { return s_dimension; }

   // pointer to the container
   const container_t *containerPtr() const {return m_container; }
   const container_t &container() const    {return *m_container; }
private:
   const container_t *m_container;
};

#ifdef ENABLE_DEBUG_TRACE
#define DEBUG_TRACE(a) do { a; } while (0)
#else
#define DEBUG_TRACE(a) do {  } while (0)
#endif

// Variant which allows to store pointer to different measurement containers with associated dimension
// T_Container should be of type ContainerRefWithDim< ContainerType, N>
// T_Container should:
// - implement the static constexpr T_Container::dimension()
// - define the alias T_Container::container_type
template <typename derived_t, typename ...T_Container>
class MeasurementContainerWithDimension {
public:
   using measurement_container_variant_t = std::variant< T_Container...>;

protected:

   const derived_t &derived() const { return *static_cast<const derived_t *>(this); }

   template <typename T>
   static constexpr T lvalue(T &&a) { return a;}

   template <std::size_t N=std::variant_size_v< measurement_container_variant_t > >
   static constexpr std::size_t dimMax(std::size_t max_dim=0ul) {
      if constexpr(N>0) {
         using this_variant_type  = decltype( lvalue(std::get<N-1>(measurement_container_variant_t{}) ) );
         return dimMax<N-1>(std::max( this_variant_type::dimension(), max_dim ) );
      }
      return max_dim;
   }

   template <typename container_t, std::size_t N>
   static constexpr bool isSameContainer() {
      using this_variant_type  = decltype( lvalue(std::get<N>(measurement_container_variant_t{}) ) );
      return std::is_same<container_t, typename this_variant_type::container_type>::value;
   }

   // helper to check whether the measurement variant contains a certain container
   template <typename container_t, std::size_t N= std::variant_size_v< measurement_container_variant_t > >
   static constexpr unsigned int  countVariants() {
      if constexpr(N==0) {
         return 0;
      }
      else {
         return countVariants<container_t, N-1>()+ isSameContainer<container_t, N-1>();
      }
   }
   template <typename container_t, std::size_t N= std::variant_size_v< measurement_container_variant_t > >
   static unsigned int  countDerivedVariants(const container_t &a) {
      if constexpr(N==0) {
         return 0;
      }
      else {
         if constexpr(isSameContainer<container_t, N-1>()) {
            return countDerivedVariants<container_t, N-1>(a)+1;
         }
         else {
            using this_variant_type  = decltype( lvalue(std::get<N-1>(measurement_container_variant_t{}) ) );
            using this_container_type = typename this_variant_type::container_type;
            if constexpr(std::is_base_of_v<container_t, this_container_type>
                         && std::has_virtual_destructor_v<this_container_type> && std::has_virtual_destructor_v<container_t>) {
               return countDerivedVariants<container_t, N-1>(a)+ ( dynamic_cast<const this_container_type *>(&a) != nullptr );
            }
            else {
               return countDerivedVariants<container_t, N-1>(a);
            }
         }
      }
   }
public:
   template <std::size_t N= std::variant_size_v< measurement_container_variant_t > >
   static void dumpVariantTypes(std::ostream &out) {
      if constexpr(N>0) {
         using a_type = decltype( lvalue(std::get<N-1>(measurement_container_variant_t{}) ) );
         out << N-1 << ": " << typeid(a_type).name() << std::endl;
         dumpVariantTypes<N-1>(out);
      }
   }
protected:
   static void throwContainerNotInVariant(const char *a) {
      std::stringstream msg;
      msg << "Container "  << a << " not in variant:" << std::endl;
      dumpVariantTypes(msg);
      throw std::runtime_error(msg.str());
   }

   template <typename container_t, bool check_dimension, std::size_t N >
   static measurement_container_variant_t getContainerWithDimensionNoAmbiguities(const container_t &a, unsigned int dimension=0) {
      if constexpr(N==0) {
         // this measns that the container_t has not been found in the list of possible options of the variant
         // reasons: the type is not in the list, a derived type was provided and compaibility was not understood
         throwContainerNotInVariant(typeid(a).name());
         using this_variant_type  = decltype( lvalue(std::get<0>(measurement_container_variant_t{}) ) );
         return this_variant_type();
      }
      else {
         using this_variant_type  = decltype( lvalue(std::get<N-1>(measurement_container_variant_t{}) ) );
         using this_container_type = typename this_variant_type::container_type ;
         DEBUG_TRACE( std::cout << "DEBUG search option for " << typeid(container_t).name() << " (aka " <<  typeid(a).name() << ")"
                                << " option: " << N << " " << typeid(this_container_type).name() << " dim " << this_variant_type::dimension()
                                << " check dimension ? " << check_dimension << " desired dimension " << dimension
                                << std:: endl );
         if constexpr(std::is_same_v<this_container_type, container_t>) {
            // this option of the variant has perfect type match.
            if constexpr(!check_dimension && countVariants<container_t>()==1) {
               // if dimensions are not to be checked there must not be other options which also match the type
               // the caller should have ensureed that this branch is impossible,
               // but this can only be excluded at runtime, so a static_assert
               // is impossible here
               assert( (countVariants<this_container_type,N-1>()) == 0 );
               DEBUG_TRACE( std::cout << "DEBUG --> for " << typeid(container_t).name() << " (aka " <<  typeid(a).name() << ")"
                                      << " foumd unambiguous option: " << N << " " << typeid(this_container_type).name() << " dim " << this_variant_type::dimension()
                                      << std:: endl );
               return ContainerRefWithDim<container_t, this_variant_type::dimension()>(a);
            }
            else {
               // otherwise check if the dimension of this option is identical to the
               // expected dimension
               if (this_variant_type::dimension() == dimension) {
                  DEBUG_TRACE( std::cout << "DEBUG --> for " << typeid(container_t).name() << " (aka " <<  typeid(a).name() << ")"
                                         << " foumd dimension option: " << N << " " << typeid(this_container_type).name() << " dim " << this_variant_type::dimension()
                                         << std:: endl );
                  return ContainerRefWithDim<container_t, this_variant_type::dimension()>(a);
               }
            }
         }
         else if constexpr(std::is_base_of_v<container_t, this_container_type>) {
            // it is also possible that the container is a base of an option of the variant.
            if constexpr(std::has_virtual_destructor_v<this_container_type> && std::has_virtual_destructor_v<container_t>) {
               // casting can only be done safely if dynamic casts are possible, which requires
               // a virtuaal destructor
               const this_container_type *derived_container = dynamic_cast<const this_container_type *>(&a);
               if (derived_container) {
                  // if the provided container can be casted to the type of this variant option
                  // then check the dimensions
                  if constexpr(!check_dimension) {
                     // Branch may be incorrectly chose and/or assert may fail
                     // if A is base of B and B is Base of C and both B and C are options of the variant
                     // the auto-detection will treat B
                     if constexpr(countVariants<container_t,N-1>() != 0) {
                        return getContainerWithDimensionNoAmbiguities<container_t, check_dimension, N-1>(a, dimension);
                     }
                     else {
                     DEBUG_TRACE( std::cout << "DEBUG --> for " << typeid(this_container_type).name() << " (aka " <<  typeid(a).name() << ")"
                                            << " foumd derived option: " << N << " " << typeid(this_container_type).name() << " dim " << this_variant_type::dimension()
                                            << std:: endl );
                     return ContainerRefWithDim<this_container_type, this_variant_type::dimension()>(*derived_container);
                     }
                  }
                  else {
                     if (this_variant_type::dimension() == dimension) {
                        DEBUG_TRACE( std::cout << "DEBUG --> for " << typeid(this_container_type).name() << " (aka " <<  typeid(a).name() << ")"
                                               << " derived with dimension option: " << N << " " << typeid(this_container_type).name()
                                               << " dim " << this_variant_type::dimension()
                                               << std:: endl );
                        return ContainerRefWithDim<this_container_type, this_variant_type::dimension()>(*derived_container);
                     }
                  }
               }
            }
         }
         return getContainerWithDimensionNoAmbiguities<container_t, check_dimension, N-1>(a, dimension);
      }
   }

   template <typename container_t, std::size_t N>
   static unsigned int getDimensionOfDerivedVariantOption(const container_t &a) {
      if constexpr(N==0) {
         // failure
         return 0;
      }
      else {
         using this_variant_type  = decltype( lvalue(std::get<N-1>(measurement_container_variant_t{}) ) );
         using this_container_type = typename this_variant_type::container_type;

         DEBUG_TRACE( std::cout << "DEBUG determin dimension for " << typeid(container_t).name() << " (aka " <<  typeid(a).name() << ")"
                                << " option: " << N << " " << typeid(this_container_type).name() << " dim " << this_variant_type::dimension()
                                << std:: endl );

         if constexpr(std::is_same<container_t, this_container_type>::value) {
            DEBUG_TRACE( std::cout << "DEBUG get dimension for " << typeid(container_t).name() << " (aka " <<  typeid(a).name() << ")"
                                   << " option: " << N << " " << typeid(this_container_type).name() << " dim " << this_variant_type::dimension()
                                   << std:: endl );
            return derived_t::getDimension(a);
         }
         else if constexpr(std::is_base_of_v<container_t, this_container_type>) {
            DEBUG_TRACE( std::cout << "DEBUG try get dimension for derived  " << typeid(container_t).name() << " (aka " <<  typeid(a).name() << ")"
                                   << " option: " << N << " " << typeid(this_container_type).name() << " dim " << this_variant_type::dimension()
                                   << std:: endl );
            // it is also possible that the container is a base of an option of the variant.
            if constexpr(std::has_virtual_destructor_v<this_container_type> && std::has_virtual_destructor_v<container_t>) {
               // if there is no ambiguity there is no point in determining the dimension
               // @TODO but this does not guarantee that this option matches if the provided container
               //       can have multiple dimensions.
               if constexpr(countVariants<this_container_type>() > 1) {
                  // casting can only be done safely if dynamic casts are possible, which requires
                  // a virtual destructor
                  const this_container_type *derived_container = dynamic_cast<const this_container_type *>(&a);
                  if (derived_container) {
                     return derived_t::getDimension(*derived_container);
                  }
               }
            }
         }
         return getDimensionOfDerivedVariantOption<container_t, N-1>(a);
      }
   }

   template <typename container_t, std::size_t = std::variant_size_v< measurement_container_variant_t > >
   static measurement_container_variant_t getContainerWithDimension(const container_t &a) {
      constexpr unsigned int variant_multiplicity = countVariants<container_t>();
      // the container must by among the options of the  measurement_container_variant_t
      static constexpr std::size_t NVariants = std::variant_size_v< measurement_container_variant_t >;
      if constexpr( variant_multiplicity == 1) {
         return getContainerWithDimensionNoAmbiguities<container_t, false, NVariants>(a);
      }
      else {
         if (variant_multiplicity==0) {
            if (countDerivedVariants<container_t>(a) == 1) {
               return getContainerWithDimensionNoAmbiguities<container_t, false, NVariants>(a);
            }
         }
         DEBUG_TRACE( std::cout << "DEBUG need to search with dimension check for " << typeid(container_t).name() << " (aka " <<  typeid(a).name() << ")"
                      << " multiplicity : " << variant_multiplicity << " derived " << countDerivedVariants<container_t>(a)
                      << std:: endl );
         return getContainerWithDimensionNoAmbiguities<container_t, true, NVariants>(a, getDimensionOfDerivedVariantOption<container_t,NVariants>(a));
      }
   }
};

template <typename derived_t, typename... T_Container>
class MeasurementContainerListWithDimension : protected MeasurementContainerWithDimension<derived_t, T_Container...> {
public:
   using Base = MeasurementContainerWithDimension<derived_t,T_Container...>;
   using measurement_container_variant_t = Base::measurement_container_variant_t;
private:
   std::vector< measurement_container_variant_t > m_containerList;

public:
   static constexpr std::size_t getMeasurementDimMax() {
      return Base::dimMax();
   }
   static void dumpVariantTypes() {
      Base::dumpVariantTypes();
   }

   std::size_t size() const {
      return m_containerList.size();
   }

   const measurement_container_variant_t &at(std::size_t container_index) const {
      return m_containerList.at(container_index);
   }
   const std::vector< measurement_container_variant_t > &containerList() const { return m_containerList;}

   template <typename container_t>
   void setContainer(std::size_t container_index, const container_t &container) {
      if (container_index>=m_containerList.size()) {
         m_containerList.resize(container_index+1);
      }
      m_containerList[container_index] = this->getContainerWithDimension(container);
      if ( std::holds_alternative<  decltype(this->lvalue(get<0>(measurement_container_variant_t{}))) >( m_containerList[container_index])
           && std::get<0>(m_containerList[container_index]).containerPtr() == nullptr) {
         // @TODO should never happen
         std::runtime_error("Unhandled measurement type");
      }

   }

};
#endif
