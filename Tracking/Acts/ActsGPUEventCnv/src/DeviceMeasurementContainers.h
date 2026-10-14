/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_DEVICEMEASUREMENTCOLLECTION_H
#define ACTSGPUEVENT_DEVICEMEASUREMENTCOLLECTION_H


#include "traccc/definitions/qualifiers.hpp"
#include <vecmem/edm/container.hpp>

#include <array>
#include <compare>
#include <ostream>

namespace ActsTrk {

/// Interface for the @c DeviceMeasurementContainer type.
///
/// It provides the API that users would interact with, while using the
/// columns/arrays of the SoA containers, or the variables of the AoS proxies
/// created on top of the SoA containers.
///
template <typename BASE>
class DevicePixelMeasurement : public BASE {

    public:
    /// @name Functions inherited from the base class
    /// @{

    /// Inherit the base class's constructor(s)
    using BASE::BASE;
    /// Inherit the base class's assignment operator(s).
    using BASE::operator=;

    /// @}

    /// @name Measurement Information
    /// @{

    /// @return A non-const long unsigned int
    TRACCC_HOST_DEVICE
    auto& identifier() { return BASE::template get<0>(); }
    /// @return A const long unsigned int
    TRACCC_HOST_DEVICE
    const auto& identifier() const { return BASE::template get<0>(); }

    /// @return A non-const array of 2 floats
    TRACCC_HOST_DEVICE
    auto& local_position() { return BASE::template get<1>(); }
    /// @return A const array of 2 floats
    TRACCC_HOST_DEVICE
    const auto& local_position() const { return BASE::template get<1>(); }

    /// @return A non-const array of 4 floats
    TRACCC_HOST_DEVICE
    auto& local_variance() { return BASE::template get<2>(); }
    /// @return A const array of 4 floats
    TRACCC_HOST_DEVICE
    const auto& local_variance() const { return BASE::template get<2>(); }

    /// @return A non-const array of 3 floats
    TRACCC_HOST_DEVICE
    auto& global_position() { return BASE::template get<3>(); }
    /// @return A const array of 3 floats
    TRACCC_HOST_DEVICE
    const auto& global_position() const { return BASE::template get<3>(); }

    /// Athena module identifier hash where the measurement belongs (non-const)
    /// @return A non-const unsigned integer
    TRACCC_HOST_DEVICE
    auto& module_id_hash() { return BASE::template get<4>(); }
    /// Athena module identifier hash where the measurement belongs (const)
    /// @return A const unsigned integer
    TRACCC_HOST_DEVICE
    const auto& module_id_hash() const { return BASE::template get<4>(); }

    /// @}

    /// @name Utility functions
    /// @{

    /// Equality operator
    ///
    /// @note This function must only be used on proxy objects, not on
    ///       containers!
    ///
    /// @param[in] other The object to compare with
    /// @return @c true if the objects are equal, @c false otherwise
    ///
    template <typename T>
    TRACCC_HOST_DEVICE bool operator==(const DevicePixelMeasurement<T>& other) const;

    /// Comparison operator
    ///
    /// @note This function must only be used on proxy objects, not on
    ///       containers!
    ///
    /// @param[in] other The object to compare with
    /// @return A weak ordering object, describing the relation between the
    ///         two objects
    ///
    template <typename T>
    TRACCC_HOST_DEVICE std::partial_ordering operator<=>(
        const DevicePixelMeasurement<T>& other) const;

    /// @}

    private:
    /// @returns a string stream that prints the measurement details
    TRACCC_HOST
    friend std::ostream& operator<<(std::ostream& os, const DevicePixelMeasurement& m) {
        os << "id = " << m.identifier()
            << ", ";
        os << "module_hash = " << m.module_id_hash()
            << ", ";
        os << "local_pos = [" << m.local_position()[0]
            << ", " << m.local_position()[1]
            << "], ";
        os << "local_var = [" << m.local_variance()[0]
            << ", " << m.local_variance()[1]
            << "], ";
        os << "global_pos = [" << m.global_position()[0]
            << ", " << m.global_position()[1]
            << ", " << m.global_position()[2]
            << "]";
        return os;
    }
};  // class DevicePixelmeasurement

/// Interface for the @c DeviceMeasurementContainer type.
///
/// It provides the API that users would interact with, while using the
/// columns/arrays of the SoA containers, or the variables of the AoS proxies
/// created on top of the SoA containers.
///
template <typename BASE>
class DeviceStripMeasurement : public BASE {

    public:
    /// @name Functions inherited from the base class
    /// @{

    /// Inherit the base class's constructor(s)
    using BASE::BASE;
    /// Inherit the base class's assignment operator(s).
    using BASE::operator=;

    /// @}

    /// @name Measurement Information
    /// @{

    /// @return A non-const long unsigned int
    TRACCC_HOST_DEVICE
    auto& identifier() { return BASE::template get<0>(); }
    /// @return A const long unsigned int
    TRACCC_HOST_DEVICE
    const auto& identifier() const { return BASE::template get<0>(); }

    /// @return A non-const float
    TRACCC_HOST_DEVICE
    auto& local_position() { return BASE::template get<1>(); }
    /// @return A const float
    TRACCC_HOST_DEVICE
    const auto& local_position() const { return BASE::template get<1>(); }

    /// @return A non-const float
    TRACCC_HOST_DEVICE
    auto& local_variance() { return BASE::template get<2>(); }
    /// @return A const float
    TRACCC_HOST_DEVICE
    const auto& local_variance() const { return BASE::template get<2>(); }

    /// Athena module identifier hash where the measurement belongs (non-const)
    /// @return A non-const unsigned integer
    TRACCC_HOST_DEVICE
    auto& module_id_hash() { return BASE::template get<3>(); }
    /// Athena module identifier hash where the measurement belongs (const)
    /// @return A const unsigned integer
    TRACCC_HOST_DEVICE
    const auto& module_id_hash() const { return BASE::template get<3>(); }

    /// @}

    /// @name Utility functions
    /// @{

    /// Equality operator
    ///
    /// @note This function must only be used on proxy objects, not on
    ///       containers!
    ///
    /// @param[in] other The object to compare with
    /// @return @c true if the objects are equal, @c false otherwise
    ///
    template <typename T>
    TRACCC_HOST_DEVICE bool operator==(const DeviceStripMeasurement<T>& other) const;

    /// Comparison operator
    ///
    /// @note This function must only be used on proxy objects, not on
    ///       containers!
    ///
    /// @param[in] other The object to compare with
    /// @return A weak ordering object, describing the relation between the
    ///         two objects
    ///
    template <typename T>
    TRACCC_HOST_DEVICE std::partial_ordering operator<=>(
        const DeviceStripMeasurement<T>& other) const;

    /// @}

    private:
    /// @returns a string stream that prints the measurement details
    TRACCC_HOST
    friend std::ostream& operator<<(std::ostream& os, const DeviceStripMeasurement& m) {
        os << "id = " << m.identifier()
            << ", ";
        os << "module_hash = " << m.module_id_hash()
            << ", ";
        os << "local_pos = " << m.local_position()[0]
            << ", ";
        os << "local_var = " << m.local_variance()[0]
            << ", ";
        return os;
    }

};  // class DeviceStripmeasurement

/// SoA container of Pixel measurements
using DevicePixelMeasurementContainer = vecmem::edm::container<
    DevicePixelMeasurement
    // identifier
    , vecmem::edm::type::vector<long unsigned int>
    // local_position
    , vecmem::edm::type::vector<std::array<float, 2u>>
    // local_variance
    , vecmem::edm::type::vector<std::array<float, 4u>>
    // global_position
    , vecmem::edm::type::vector<std::array<float, 3u>>
    // module hash
    , vecmem::edm::type::vector<unsigned int>
    >;

/// SoA container of Strip measurements
using DeviceStripMeasurementContainer = vecmem::edm::container<
    DeviceStripMeasurement
    // identifier
    , vecmem::edm::type::vector<long unsigned int>
    // local_position
    , vecmem::edm::type::vector<float>
    // local_variance
    , vecmem::edm::type::vector<float>
    // module hash
    , vecmem::edm::type::vector<unsigned int>
    >;

}  // namespace ActsTrk

#endif // ACTSGPUEVENT_DEVICEMEASUREMENTCOLLECTION_H