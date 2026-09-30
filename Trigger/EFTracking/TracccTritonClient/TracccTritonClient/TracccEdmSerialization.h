/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  Trigger/EFTracking/TracccTritonClient/TracccTritonClient/TracccEdmSerialization.h
 * @author Miles Cochran-Branson
 * @date September 2026
 * @brief Byte (de)serialization of vecmem SoA host containers, used to ship
 *        traccc EDM collections between the Triton client and backend
 */

#ifndef TRACCCTRITONCLIENT_TRACCCEDMSERIALIZATION_H
#define TRACCCTRITONCLIENT_TRACCCEDMSERIALIZATION_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <vecmem/edm/host.hpp>

namespace TracccTriton {

/// Wire format of one serialized container, per variable in schema order:
///
///   uint64 number of elements N,
///   [jagged only: N x uint64 inner sizes],
///   raw element bytes.
///
/// Both serialization and deserialization use the same schema, so no layout information is stored.
namespace detail {

inline void putBytes(std::vector<std::uint8_t>& out, const void* data,
                     std::size_t size) {
    const auto* p = static_cast<const std::uint8_t*>(data);
    out.insert(out.end(), p, p + size);
}

inline void putSize(std::vector<std::uint8_t>& out, std::uint64_t size) {
    putBytes(out, &size, sizeof(size));
}

class Reader {
public:
    Reader(const std::uint8_t* data, std::size_t size)
        : m_pos(data), m_end(data + size) {}

    std::uint64_t size() {
        std::uint64_t value;
        bytes(&value, sizeof(value));
        return value;
    }

    void bytes(void* dst, std::size_t size) {
        if (static_cast<std::size_t>(m_end - m_pos) < size) {
            throw std::runtime_error(
                "TracccEdmSerialization: buffer too short while reading");
        }
        if (size != 0) {
            std::memcpy(dst, m_pos, size);
        }
        m_pos += size;
    }

private:
    const std::uint8_t* m_pos;
    const std::uint8_t* m_end;
};

template <typename Element, typename Allocator>
void write(std::vector<std::uint8_t>& out,
           const std::vector<Element, Allocator>& variable) {
    static_assert(std::is_trivially_copyable_v<Element>);
    putSize(out, variable.size());
    putBytes(out, variable.data(), variable.size() * sizeof(Element));
}

template <typename Element, typename InnerAllocator, typename OuterAllocator>
void write(std::vector<std::uint8_t>& out,
           const std::vector<std::vector<Element, InnerAllocator>,
                             OuterAllocator>& variable) {
    static_assert(std::is_trivially_copyable_v<Element>);
    putSize(out, variable.size());
    for (const auto& inner : variable) {
        putSize(out, inner.size());
    }
    for (const auto& inner : variable) {
        putBytes(out, inner.data(), inner.size() * sizeof(Element));
    }
}

template <typename Element, typename Allocator>
void read(Reader& in, std::vector<Element, Allocator>& variable) {
    variable.resize(in.size());
    in.bytes(variable.data(), variable.size() * sizeof(Element));
}

template <typename Element, typename InnerAllocator, typename OuterAllocator>
void read(Reader& in,
          std::vector<std::vector<Element, InnerAllocator>, OuterAllocator>&
              variable) {
    variable.resize(in.size());
    for (auto& inner : variable) {
        inner.resize(in.size());
    }
    for (auto& inner : variable) {
        in.bytes(inner.data(), inner.size() * sizeof(Element));
    }
}

}  // namespace detail

/// Append @p container to @p out in the format described above.
template <typename... VARTYPES, template <typename> class INTERFACE>
void serialize(
    const vecmem::edm::host<vecmem::edm::schema<VARTYPES...>, INTERFACE>& container,
    std::vector<std::uint8_t>& out) {
    [&]<std::size_t... Index>(std::index_sequence<Index...>) {
        (detail::write(out, container.template get<Index>()), ...);
    }(std::make_index_sequence<sizeof...(VARTYPES)>{});
}

/// Fill @p container from @p size bytes at @p data.
template <typename... VARTYPES, template <typename> class INTERFACE>
void deserialize(
    const std::uint8_t* data, std::size_t size,
    vecmem::edm::host<vecmem::edm::schema<VARTYPES...>, INTERFACE>& container) {
    detail::Reader in(data, size);
    [&]<std::size_t... Index>(std::index_sequence<Index...>) {
        (detail::read(in, container.template get<Index>()), ...);
    }(std::make_index_sequence<sizeof...(VARTYPES)>{});
}

}  // namespace TracccTriton

#endif  // TRACCCTRITONCLIENT_TRACCCEDMSERIALIZATION_H
