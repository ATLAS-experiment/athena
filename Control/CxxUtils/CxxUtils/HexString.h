/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */

/**
 * @file HexString.h
 * @brief Provides a utility class to format integral types as hexadecimal strings at compile time
 *  or with VERY high performance at runtime.
 *  It should be prefered over std::format in well-understood hot pathways.
 */


#include <string_view>
#include <string>
#include <type_traits> // Required for std::is_integral_v and std::make_unsigned_t


namespace CxxUtils {

namespace detail{
/**
 * @brief Helper structure to allow string literals to be passed as non-type template parameters.
 * Not necessarily intended to be used directly.
 * * @tparam N The length of the string array, including the null terminator.
 */
template <std::size_t N>
struct FixedString {
    /// The internal character buffer.
    char buf[N]{};
    /**
     * @brief Constructs a FixedString from a string literal.
     * @param str A reference to a character array (string literal).
     */
    constexpr FixedString(const char (&str)[N]) {
        for (std::size_t i = 0; i < N; ++i) buf[i] = str[i];
    }
     /**
     * @brief Gets the length of the string excluding the null terminator.
     * @return The length of the string.
     */
    constexpr std::size_t length() const { return N - 1; }
};
}

/**
 * @brief A class that formats an integer as a hexadecimal string embedded within a format string.
 * * This class uses a compile-time format string containing a `{}` placeholder. 
 * When instantiated with an integer, it replaces the placeholder with the 
 * uppercase hexadecimal representation of that integer.
 * It should be prefered over std::format in well-understood hot pathways.
 * * @tparam FormatStr A FixedString containing exactly one "{}" placeholder.
 */
template <detail::FixedString FormatStr>
class HexString {
private:
    /// The total length of the format string.
    static constexpr std::size_t s_FormatLen = FormatStr.length();

     /**
     * @brief Finds the position of the "{}" placeholder within the format string.
     * @return The index of the '{' character, or -1 if not found.
     */
    static constexpr std::size_t get_placeholder_pos() {
        for (std::size_t i = 0; i + 1 < s_FormatLen; ++i) {
            if (FormatStr.buf[i] == '{' && FormatStr.buf[i + 1] == '}') return i;
        }
        return static_cast<std::size_t>(-1);
    }

    /// The index of the "{}" placeholder.
    static constexpr std::size_t s_PlaceholderPos = get_placeholder_pos();
    static_assert(s_PlaceholderPos != static_cast<std::size_t>(-1), "Format string must contain a '{}' placeholder.");

    /// The index of the "{}" placeholder.
    static constexpr std::size_t s_PrefixLen = s_PlaceholderPos;
    /// The length of the string postfix (after the placeholder).
    static constexpr std::size_t s_PostfixLen = s_FormatLen - s_PrefixLen - 2;
    /// The maximum number of hex digits (supports up to 64-bit integers).
    static constexpr std::size_t s_MaxHexDigits = 16; 
    
    /// The internal buffer storing the fully formatted string
    char m_text[s_PrefixLen + s_MaxHexDigits + s_PostfixLen + 1]{};
    /// The actual length of the generated string.
    std::size_t m_actual_size = 0;

public:
    /**
     * @brief Constructs the formatted string by injecting the hex value of the context into the placeholder.
     * The benefit of this is that it uses a fixed character buffer determined at compile time eliminating the
     * need for allocations in runtime.
     * * @tparam T An integral type (e.g., int, uint32_t, size_t).
     * @param context The integer value to be formatted as a hex string.
     */
    template <typename T>
    constexpr explicit HexString(T context) {
        static_assert(std::is_integral_v<T>, "HexString only accepts integer types.");

        constexpr char digits[] = "0123456789ABCDEF";
        
        // 1. Calculate hex digits needed for this specific type (e.g., sizeof(uint16_t) * 2 = 4 digits)
        constexpr std::size_t HexDigits = sizeof(T) * 2;

        m_actual_size = s_PrefixLen + HexDigits + s_PostfixLen;
        static_assert(HexDigits <= s_MaxHexDigits);
        // Cast to an unsigned equivalent of the same size to prevent sign-extension bugs during shifting
        using UnsignedT = std::make_unsigned_t<T>;
        UnsignedT value = static_cast<UnsignedT>(context);

        // 2. Write prefix
        for (std::size_t i = 0; i < s_PrefixLen; ++i) {
            m_text[i] = FormatStr.buf[i];
        }
        
        // 3. Write hex digits dynamically based on type size
        for (std::size_t i = 0; i < HexDigits; ++i) {
            std::size_t shift = (HexDigits - 1 - i) * 4;
            m_text[s_PrefixLen + i] = digits[(value >> shift) & 0xFU];
        }

        // 4. Write postfix
        for (std::size_t i = 0; i < s_PostfixLen; ++i) {
            m_text[s_PrefixLen + HexDigits + i] = FormatStr.buf[s_PrefixLen + 2 + i];
        }

        // 5. Null terminator
        m_text[m_actual_size] = '\0';
    }

    /**
     * @brief Implicit conversion operator to std::string_view.
     * @return A std::string_view representing the formatted string.
     */
    constexpr operator std::string_view() const noexcept {
        return std::string_view(m_text, m_actual_size);
    }


    //Helper friend functions to allow compilation of common use-cases.

    friend constexpr std::string operator+(std::string lhs, const HexString& rhs) {
        lhs.append(static_cast<std::string_view>(rhs));
        return lhs;
    }

    friend constexpr std::string operator+(const HexString& lhs, const std::string& rhs) {
        std::string_view lhs_view = lhs;
        std::string result;
        result.reserve(lhs_view.size() + rhs.size());
        result.append(lhs_view).append(rhs);
        return result;
    }

    friend constexpr std::string operator+(const char* lhs, const HexString& rhs) {
        return std::string(lhs) + rhs;
    }

    friend constexpr std::string operator+(const HexString& lhs, const char* rhs) {
        return lhs + std::string(rhs);
    }

    /**
     * @brief Returns the total size of the formatted string.
     * @return The number of characters in the string, excluding the null terminator.
     */
    constexpr std::size_t size() const noexcept { return m_actual_size; }

    /**
     * @brief Returns a pointer to the underlying null-terminated character array.
     * @return A const char pointer to the string data.
     */
    const char* c_str() const noexcept { return m_text; }
};
}

