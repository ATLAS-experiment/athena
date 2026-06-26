/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
#undef NDEBUG
#include <iostream>
#include <cstdint>
#include <cassert>
#include <format>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include "CxxUtils/HexString.h"


// Helper function to test a specific value against std::format
template <typename T>
void run_test_case(T value) {
    // 1. Generate Actual output using HexString
    CxxUtils::HexString<"Memory Address: 0x{} !"> hex_str(value);
    std::string actual(hex_str.c_str());

    // 2. Generate Expected output using std::format
    // HexString forces unsigned hex representation and pads to the exact type size.
    using UnsignedT = std::make_unsigned_t<T>;
    UnsignedT u_val = static_cast<UnsignedT>(value);
    
    // {:0{}X} -> Zero-padded, dynamic width (second param), Uppercase Hex
    std::size_t expected_width = sizeof(T) * 2;
    std::string expected = std::format("Memory Address: 0x{:0{}X} !", u_val, expected_width);

    // 3. Verify
    if (actual != expected) {
        std::cerr << "[FAIL] Type Size: " << sizeof(T) << " bytes\n"
                  << "Value (as int64): " << static_cast<int64_t>(value) << "\n"
                  << "Expected: " << expected << "\n"
                  << "Actual:   " << actual << "\n";
        assert(false && "Output mismatch between HexString and std::format");
    }
}

// Helper to run boundaries and common values for a specific type
template <typename T>
void test_integer_type() {
    run_test_case<T>(0);
    run_test_case<T>(1);
    run_test_case<T>(42);
    run_test_case<T>(-1); // Tests 2's complement unsigned casting
    run_test_case<T>(std::numeric_limits<T>::max());
    run_test_case<T>(std::numeric_limits<T>::min());
    
    if constexpr (sizeof(T) >= 2) {
        run_test_case<T>(static_cast<T>(0xABCD));
    }
    if constexpr (sizeof(T) >= 4) {
        run_test_case<T>(static_cast<T>(0xDEADBEEF));
    }
}

// Tests the operator overloads (+ string concatenation)
void test_operators() {
    CxxUtils::HexString<"0x{}"> op_test(static_cast<uint16_t>(0x1337));
    
    // Test: std::string + HexString
    std::string concat1 = std::string("Value is ") + op_test;
    assert(concat1 == "Value is 0x1337");

    // Test: HexString + std::string
    std::string concat2 = op_test + std::string(" bits");
    assert(concat2 == "0x1337 bits");

    // Test: const char* + HexString
    std::string concat3 = "Result: " + op_test;
    assert(concat3 == "Result: 0x1337");

    // Test: HexString + const char*
    std::string concat4 = op_test + " [OK]";
    assert(concat4 == "0x1337 [OK]");
}

void test_constexpr() {
    // 1. Force instantiation and execution entirely inside a compile-time context
    static constexpr CxxUtils::HexString<"Base: 0x{}"> const_expr_test(54u); // 54 = 0x00000036
    
    // 2. Validate properties using static_assert
    static_assert(const_expr_test.size() == 16, "Compile-time size calculation failed!");
    
    // 3. Convert to string_view at compile time to check literal equality
    constexpr std::string_view view = static_cast<std::string_view>(const_expr_test);
    static_assert(view == "Base: 0x00000036", "Compile-time string format mismatch!");
}

int main() {
    std::cout << "Testing 8-bit integers...\n";
    test_integer_type<int8_t>();
    test_integer_type<uint8_t>();

    std::cout << "Testing 16-bit integers...\n";
    test_integer_type<int16_t>();
    test_integer_type<uint16_t>();

    std::cout << "Testing 32-bit integers...\n";
    test_integer_type<int32_t>();
    test_integer_type<uint32_t>();

    std::cout << "Testing 64-bit integers...\n";
    test_integer_type<int64_t>();
    test_integer_type<uint64_t>();

    std::cout << "Testing concatenation operators...\n";
    test_operators();

    std::cout << "Testing constexpr execution...\n";
    test_constexpr();

    std::cout << "All CxxUtils::HexString tests passed perfectly!\n";
    
    return 0;
}

