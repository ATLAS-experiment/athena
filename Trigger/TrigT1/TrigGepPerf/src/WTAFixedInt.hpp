/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FIXED_INT_HPP
#define FIXED_INT_HPP

#include <cstdint>
#include <cassert>
#include <type_traits>

template <int N>
class FixedInt {
    static_assert(N > 0 && N <= 64, "Bit width must be between 1 and 64");

    using StorageType = uint64_t;
    StorageType m_value = 0;
    bool m_overflow_flag = false;
    bool m_underflow_flag = false;

    constexpr static StorageType mask() {
        return (N == 64) ? StorageType(-1) : ((StorageType(1) << N) - 1);
    }

    static FixedInt fromRaw(StorageType raw, bool overflow, bool underflow = false) {
        FixedInt out;
        out.m_value = raw & mask();
        out.m_overflow_flag = overflow;
        out.m_underflow_flag = underflow;
        return out;
    }

    static bool detectOverflow(StorageType a, StorageType b) {
        if constexpr (N < 64) {
            return ((a + b) & ~mask()) != 0;
        } else {
            return (a + b) < a;
        }
    }

    static bool detectOverflowMul(StorageType a, StorageType b) {
        if constexpr (N < 64) {
            return ((a * b) & ~mask()) != 0;
        } else {
            return b != 0 && (a * b) / b != a;
        }
    }

public:
    FixedInt() = default;

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    FixedInt(T v) {
        assert(v >= 0 && "Negative values are not allowed in FixedInt");
        m_value = static_cast<StorageType>(v) & mask();
    }

    FixedInt& operator=(int64_t v) {
        assert(v >= 0 && "Negative values are not allowed in FixedInt");
        m_value = static_cast<StorageType>(v) & mask();
        m_overflow_flag = false;
        m_underflow_flag = false;
        return *this;
    }

    operator int64_t() const {
        return static_cast<int64_t>(m_value & mask());
    }

    StorageType raw() const { return m_value & mask(); }

    bool overflow() const { return m_overflow_flag; }
    bool underflow() const { return m_underflow_flag; }

    // ---- Arithmetic ----
    FixedInt operator+(const FixedInt& other) const {
        StorageType result = m_value + other.m_value;
        bool overflow = detectOverflow(m_value, other.m_value);
        return fromRaw(result, overflow);
    }

    FixedInt operator-(const FixedInt& other) const {
        bool underflow = m_value < other.m_value;
        StorageType result = m_value - other.m_value;
        return fromRaw(result, false, underflow);
    }

    FixedInt operator*(const FixedInt& other) const {
        StorageType result = m_value * other.m_value;
        bool overflow = detectOverflowMul(m_value, other.m_value);
        return fromRaw(result, overflow);
    }

    FixedInt operator/(const FixedInt& other) const {
        bool overflow = (other.m_value == 0);
        StorageType result = overflow ? 0 : m_value / other.m_value;
        return fromRaw(result, overflow);
    }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    FixedInt operator+(T rhs) const {
        StorageType r = static_cast<StorageType>(rhs);
        StorageType result = m_value + r;
        bool overflow = detectOverflow(m_value, r);
        return fromRaw(result, overflow);
    }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    FixedInt operator-(T rhs) const {
        StorageType r = static_cast<StorageType>(rhs);
        bool underflow = m_value < r;
        return fromRaw(m_value - r, false, underflow);
    }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    FixedInt operator*(T rhs) const {
        StorageType r = static_cast<StorageType>(rhs);
        StorageType result = m_value * r;
        bool overflow = detectOverflowMul(m_value, r);
        return fromRaw(result, overflow);
    }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    FixedInt operator/(T rhs) const {
        bool overflow = (rhs == 0);
        StorageType result = overflow ? 0 : m_value / static_cast<StorageType>(rhs);
        return fromRaw(result, overflow);
    }

    FixedInt operator%(const FixedInt& other) const {
        bool overflow = (other.m_value == 0);
        StorageType result = overflow ? 0 : (m_value % other.m_value);
        return fromRaw(result, overflow);
    }

    // ---- Bitwise ----
    FixedInt operator&(const FixedInt& other) const { return fromRaw(m_value & other.m_value, false, false); }
    FixedInt operator|(const FixedInt& other) const { return fromRaw(m_value | other.m_value, false, false); }
    FixedInt operator^(const FixedInt& other) const { return fromRaw(m_value ^ other.m_value, false, false); }
    FixedInt operator~() const { return fromRaw((~m_value) & mask(), false, false); }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    FixedInt operator&(T rhs) const { return fromRaw(m_value & static_cast<StorageType>(rhs), false, false); }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    FixedInt operator|(T rhs) const { return fromRaw(m_value | static_cast<StorageType>(rhs), false, false); }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    FixedInt operator^(T rhs) const { return fromRaw(m_value ^ static_cast<StorageType>(rhs), false, false); }

    // FixedInt % integral type
    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    FixedInt operator%(T rhs) const {
        StorageType r = static_cast<StorageType>(rhs);
        bool overflow = (r == 0);
        StorageType result = overflow ? 0 : (m_value % r);
        return fromRaw(result, overflow);
    }

    // ---- Shift ----
    FixedInt operator<<(int s) const { return fromRaw((m_value << s) & mask(), false, false); }
    FixedInt operator>>(int s) const { return fromRaw(m_value >> s, false, false); }

    // ---- Comparison ----
    bool operator==(const FixedInt& other) const { return m_value == other.m_value; }
    bool operator!=(const FixedInt& other) const { return m_value != other.m_value; }
    bool operator<(const FixedInt& other) const { return m_value < other.m_value; }
    bool operator>(const FixedInt& other) const { return m_value > other.m_value; }
    bool operator<=(const FixedInt& other) const { return m_value <= other.m_value; }
    bool operator>=(const FixedInt& other) const { return m_value >= other.m_value; }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator==(T rhs) const { return m_value == static_cast<StorageType>(rhs); }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator!=(T rhs) const { return m_value != static_cast<StorageType>(rhs); }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator<(T rhs) const { return m_value < static_cast<StorageType>(rhs); }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator>(T rhs) const { return m_value > static_cast<StorageType>(rhs); }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator<=(T rhs) const { return m_value <= static_cast<StorageType>(rhs); }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator>=(T rhs) const { return m_value >= static_cast<StorageType>(rhs); }

    // Utility methods
    bool isZero() const { return (m_value & mask()) == 0; }
    bool isMax() const { return (m_value & mask()) == mask(); }
    bool isOne() const { return (m_value & mask()) == 1; }
    bool isEven() const { return ((m_value & 1) == 0); }
    bool isOdd() const { return ((m_value & 1) == 1); }
};

#endif // FIXED_INT_HPP