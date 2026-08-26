/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_BITSPEC_H
#define GLOBALSIM_BITSPEC_H

#include <bit>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <nlohmann/json.hpp>

#include "AthContainers/AuxElement.h"

namespace GlobalSim {

// ============================================================================
// BitField
//
// Lo and Hi are inclusive bit positions.
//
//   BitField<0, 10, int>
//
// represents an 11-bit field occupying bits [10:0].
//
// The bit representation is:
//
//   std::bitset<11>
//
// ============================================================================

    struct AuxSpec {
        std::string_view name;
        std::uint64_t mask;
        std::size_t shift;
        bool has_mask;

        static constexpr std::size_t parseNumber(std::string_view str,std::size_t& pos,std::size_t end) {
            std::size_t value{};
            const auto [ptr, ec] = std::from_chars(
                    str.data() + pos,
                    str.data() + end,
                    value
            );

            if (ec != std::errc{})
                throw std::invalid_argument("Expected number");

            pos = ptr - str.data();
            return value;
        }

        static constexpr AuxSpec parse(std::string_view spec) {
            const auto open = spec.find('[');

            // No mask specification.
            if (open == std::string_view::npos) {
                return {
                        spec,
                        std::numeric_limits<std::uint64_t>::max(),
                        0,
                        false
                };
            }

            // Expected: name[lo:hi] or name[bit]
            const auto colon = spec.find(':', open + 1);
            const auto close = spec.find(']', open + 1);

            if (close == std::string_view::npos ||close != spec.size() - 1) {
                throw std::invalid_argument("Invalid AuxSpec");
            }

            std::size_t pos = open + 1;
            const std::size_t lo = parseNumber(spec, pos, colon == std::string_view::npos ? close : colon);

            std::size_t hi = lo;

            if (colon != std::string_view::npos) {
                // Range: [lo:hi]
                if (colon == open + 1 || colon + 1 == close) {
                    throw std::invalid_argument("Invalid AuxSpec range");
                }
                pos = colon + 1;
                hi = parseNumber(spec, pos, close);
            }

            if (pos != close || lo > hi || hi >= 64)
                throw std::invalid_argument("Invalid bit range");

            const std::size_t width = hi - lo + 1;

            const std::uint64_t mask =
                    width == 64
                    ? std::numeric_limits<std::uint64_t>::max()
                    : ((std::uint64_t{1} << width) - 1) << lo;

            return {
                    spec.substr(0, open),
                    mask,
                    lo,
                    true
            };
        }

    };




    //struct NoMask {};

    template<unsigned Lo, unsigned Hi, typename AuxValue, typename Value=AuxValue/*, auto Mask = NoMask{}*/>
    class BitField {
    public:
        static_assert(Hi >= Lo,"BitField: Hi must be >= Lo");
        // next line makes it required that AuxValue type is big enough for this bitfield
        static_assert(sizeof(AuxValue) * CHAR_BIT >= Hi - Lo + 1,"AuxType is too small for BitField");
    public:
        static constexpr unsigned lo = Lo;
        static constexpr unsigned hi = Hi;
        static constexpr unsigned width = Hi - Lo + 1;
        // leaving this commented as the alternative way to do constexpr masking
        /*static constexpr auto mask = Mask;
        static constexpr bool has_mask = !std::is_same_v<decltype(Mask), NoMask>;
        static constexpr std::size_t shift = [] {
            if constexpr (has_mask)
            return std::countr_zero(static_cast<uint64_t>(Mask));
            else
            return std::size_t{0};
        }();*/

        using value_type = Value;
        using bits_type = std::bitset<width>;

        using encoder_type = bits_type (*)(Value);
        using decoder_type = Value (*)(bits_type);

        constexpr BitField(
                std::string_view name,
                std::string_view auxvar,
                std::string_view description,
                encoder_type encoder = nullptr,
                decoder_type decoder = nullptr)
                : auxspec(AuxSpec::parse(auxvar)),
                  m_name(name),
                  m_description(description),
                  m_encoder(encoder),
                  m_decoder(decoder),
                  m_acc(std::string{auxspec.name}), m_wacc(std::string{auxspec.name}) {

        }

        // ------------------------------------------------------------------------
        // Metadata
        // ------------------------------------------------------------------------

        constexpr std::string_view name() const { return m_name; }

        constexpr std::string_view description() const { return m_description; }

        // ------------------------------------------------------------------------
        // Get the value directly from an AOD object.
        // ------------------------------------------------------------------------

        Value value(const SG::AuxElement &obj) const {

            if constexpr (std::is_integral_v<AuxValue>)
            { // ensures have bitwise operators

                /*if constexpr(has_mask) {
                    return (m_acc(obj) & Mask) >> shift;*/
                if (auxspec.has_mask) {
                    return (m_acc(obj) & auxspec.mask) >> auxspec.shift;
                }
            }

            return m_acc(obj);

        }

        // if truncate = true, then will do an encode-decode to truncate the stored value
        Value value(const SG::AuxElement& obj, bool truncate) const {
            if(!truncate) return value(obj);
            return decode(encode(value(obj)));
        }

        AuxValue& store(SG::AuxElement &obj, Value value) const {
            if constexpr (std::is_integral_v<AuxValue>)
            { // ensures have bitwise operators, has_mask is not constexpr so must hide this from invalid AuxValue types
                // leaving this commented as may revery to constexpr again at some point
                //            if constexpr (has_mask) {
                //                auto& val = m_wacc(obj);
                //                val = (val & ~Mask) | ((value << shift) & Mask);
                if (auxspec.has_mask) {
                    auto &val = m_wacc(obj);
                    return (val = (val & ~auxspec.mask) | ((value << auxspec.shift) & auxspec.mask));
                }
            }
            return (m_wacc(obj) = value);

        }

        // ------------------------------------------------------------------------
        // Encode a Value into this field's bit representation.
        // ------------------------------------------------------------------------

        bits_type encode(Value value) const {
            if (m_encoder)
                return m_encoder(value);

            // Default behaviour: integral conversion.

            // should we check for out-of-range given size of bitset?

            return bits_type{
                    static_cast<unsigned long long>(value)
            };
        }

        // ------------------------------------------------------------------------
        // Get and encode the value from an AOD object.
        // ------------------------------------------------------------------------

        bits_type bits(const SG::AuxElement &obj) const {
            return encode(value(obj));
        }

        // ------------------------------------------------------------------------
        // Decode this field's bits into a Value.
        // ------------------------------------------------------------------------

        Value decode(bits_type bits) const {
            if (m_decoder)
                return m_decoder(bits);

            return static_cast<Value>(bits.to_ullong());
        }

        // ------------------------------------------------------------------------
        // Extract this field from a complete specification bitset.
        // ------------------------------------------------------------------------

        template<std::size_t N>
        bits_type extract(const std::bitset <N> &packed) const {
            static_assert(Hi < N, "BitField extends beyond the specification width");

            bits_type result;

            for (unsigned i = 0; i < width; ++i)
                result[i] = packed[Lo + i];

            return result;
        }

        // ------------------------------------------------------------------------
        // Extract and decode this field from a complete specification bitset.
        // ------------------------------------------------------------------------

        template<std::size_t N>
        Value decodeFrom(const std::bitset <N> &packed) const {
            return decode(extract(packed));
        }

        template<std::size_t N>
        void decodeAndAssignFrom(const std::bitset <N> &packed, SG::AuxElement &obj) const {
            store( obj, decode(extract(packed)));
        }

        // ------------------------------------------------------------------------
        // Pack this field from an AOD object into a complete bitset.
        // ------------------------------------------------------------------------

        template<std::size_t N>
        void pack(
                std::bitset <N> &result,
                const SG::AuxElement &obj) const {
            static_assert(
                    Hi < N,
                    "BitField extends beyond the specification width");

            const bits_type field_bits = bits(obj);

            for (unsigned i = 0; i < width; ++i)
                result[Lo + i] = field_bits[i];
        }

        AuxSpec auxspec;
    private:

        std::string_view m_name;
        std::string_view m_description;

        encoder_type m_encoder;
        decoder_type m_decoder;

        SG::ConstAccessor <AuxValue> m_acc;
        SG::Accessor <AuxValue> m_wacc;
    };


    template<typename Field>
    class BitFieldAccessor : public Field {
    public:
        BitFieldAccessor(const Field &field, const SG::AuxElement &obj) : Field(field), m_field(field), m_obj(obj) {
        }

        // this just preserves old syntax behaviour of obj.field().value() and obj.field() = ...
        BitFieldAccessor& operator()() {
            return *this;
        }

        // ------------------------------------------------------------------------
        // Value stored in the AOD object.
        // ------------------------------------------------------------------------

        auto value(bool truncate=false) const {
            if(truncate) return m_field.value(m_obj,true);
            return m_field.value(m_obj);
        }


        BitFieldAccessor &operator=(const Field::value_type &value) {
            m_field.store(const_cast<SG::AuxElement &>(m_obj),value);
            return *this;
        }

        BitFieldAccessor &operator=(const Field::bits_type &bits) {
            // Decode bits and set the value in m_obj
            // ...
            return operator=(m_field.decode(bits));
        }

        // ------------------------------------------------------------------------
        // Encoded representation of the AuxElement value.
        // ------------------------------------------------------------------------

        auto bits() const {
            return m_field.bits(m_obj);
        }

        // ------------------------------------------------------------------------
        // Metadata
        // ------------------------------------------------------------------------
        constexpr auto spec() const { return m_field; }


        // ------------------------------------------------------------------------
        // Extract this field from a complete packed representation.
        // ------------------------------------------------------------------------

        template<std::size_t N>
        auto extract(const std::bitset <N> &packed) const {
            return m_field.extract(packed);
        }

        // ------------------------------------------------------------------------
        // Extract and decode this field from a complete packed representation.
        // ------------------------------------------------------------------------

        template<std::size_t N>
        auto decodeFrom(const std::bitset <N> &packed) const {
            return m_field.decodeFrom(packed);
        }

    private:
        const Field &m_field;
        const SG::AuxElement &m_obj;
    };


// ============================================================================
// BitSpec<N>
//
// Common base class for concrete specifications.
//
// N = total width of the packed representation.
//
// ============================================================================

    template<typename Derived, std::size_t N>
    class BitSpec {
    public:
        static constexpr std::size_t width = N;
        using bitset_type = std::bitset<N>;



        // example use: MySpec::field<0>().name()
        template<std::size_t I> static constexpr decltype(auto) field() {
            return *std::get<I>(Derived::fields);
        }

        // example use: MySpec::numFields()
        static constexpr std::size_t

        numFields() {
            return std::tuple_size_v < std::remove_cvref_t < decltype(Derived::fields) >> ;
        }

        // example use: MySpec::forEachField( [](const auto& field) { std::cout << field.name() << std::endl; } );
        static constexpr void forEachField(auto &&func) {
            std::apply([&](const auto *... field) { (func(*field), ...); }, Derived::fields);
        }

        // example use: MySpec::json()
        static constexpr std::string json() {
            nlohmann::json j;
            j["data_width"] = width;
            j["fields"] = nlohmann::json::array();
            forEachField([&](const auto &field) {
                nlohmann::json f;
                f["name"] = field.name();
                f["description"] = field.description();
                f["start"] = field.lo;
                f["width"] = field.width;
                j["fields"].push_back(f);
            });
            return j.dump(4);
        }

    protected:

        // this method is used in the DECLARE_FIELDS macro to create the tuple .. needed to inject the & symbol
        template<typename... Fields>
        static constexpr auto makeFields(Fields &... fields) {
            return std::tuple{&fields...};
        }


        // ------------------------------------------------------------------------
        // Pack all fields in a field tuple.
        // ------------------------------------------------------------------------
    public:
        template<typename Fields>
        static bitset_type packFields(const Fields &fields, const SG::AuxElement &obj) {
            bitset_type result;

            std::apply(
                    [&](const auto *... field) {
                        (field->pack(result, obj), ...);
                    },
                    fields);

            return result;
        }

    protected:
        // ------------------------------------------------------------------------
        // Validate all fields.
        //
        // Individual field ranges are checked at compile time via:
        //
        //     static_assert(Hi < N)
        //
        // Overlap between different fields is checked here.
        // ------------------------------------------------------------------------


        static consteval bool validateFields()
        {
            return std::apply(
                    []<typename... FieldPtrs>(FieldPtrs... field) {
                        return validateFieldList<
                               std::remove_cvref_t<decltype(*field)>...
                        >();
                    },
                    Derived::fields);
        }

    private:
        // ------------------------------------------------------------------------
        // Compare the first field against every subsequent field.
        // ------------------------------------------------------------------------

        template<typename First, typename... Rest>
        static consteval bool validateFieldList()
        {
            if constexpr (sizeof...(Rest) == 0)
            {
                return validateField<First>();
            }
            else
            {
                return validateField<First>() &&
                       (validateField<Rest>() && ...) &&
                       ((!checkOverlap<First, Rest>()) && ...) &&
                       validateFieldList<Rest...>();
            }
        }

        template<typename Field>
        static consteval bool validateField() {
            return Field::lo <= Field::hi && Field::hi < N;
        }

        // ------------------------------------------------------------------------
        // Check two fields for overlap.
        // ------------------------------------------------------------------------

        template<typename A, typename B>
        static consteval bool checkOverlap() {
            if (A::lo <= B::hi && B::lo <= A::hi) {
                return true;
            }
            return false;
        }

    };

}

// MACROS FOR DEFINING FIELDS IN BITSPECS:


#define FE_1(m, a)        m(a)
#define FE_2(m, a, ...)   m(a) FE_1(m, __VA_ARGS__)
#define FE_3(m, a, ...)   m(a) FE_2(m, __VA_ARGS__)
#define FE_4(m, a, ...)   m(a) FE_3(m, __VA_ARGS__)
#define FE_5(m, a, ...)   m(a) FE_4(m, __VA_ARGS__)
#define FE_6(m, a, ...)   m(a) FE_5(m, __VA_ARGS__)
#define FE_7(m, a, ...)   m(a) FE_6(m, __VA_ARGS__)
#define FE_8(m, a, ...)   m(a) FE_7(m, __VA_ARGS__)
#define FE_9(m, a, ...)   m(a) FE_8(m, __VA_ARGS__)
#define FE_10(m, a, ...)   m(a) FE_9(m, __VA_ARGS__)
#define FE_11(m, a, ...)   m(a) FE_10(m, __VA_ARGS__)
#define FE_12(m, a, ...)   m(a) FE_11(m, __VA_ARGS__)
#define FE_13(m, a, ...)   m(a) FE_12(m, __VA_ARGS__)
#define FE_14(m, a, ...)   m(a) FE_13(m, __VA_ARGS__)
#define FE_15(m, a, ...)   m(a) FE_14(m, __VA_ARGS__)
#define FE_16(m, a, ...)   m(a) FE_15(m, __VA_ARGS__)
#define FE_17(m, a, ...)   m(a) FE_16(m, __VA_ARGS__)
#define FE_18(m, a, ...)   m(a) FE_18(m, __VA_ARGS__)
#define FE_19(m, a, ...)   m(a) FE_19(m, __VA_ARGS__)
#define FE_20(m, a, ...)   m(a) FE_20(m, __VA_ARGS__)

#define GET_FE(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10, \
               _11,_12,_13,_14,_15,_16,_17,_18,_19,_20,NAME,...) NAME

#define FOR_EACH(m, ...) \
    GET_FE(__VA_ARGS__, \
	   FE_20, FE_19, FE_18, FE_17, \
           FE_16, FE_15, FE_14, FE_13, \
           FE_12, FE_11, FE_10, FE_9, \
           FE_8, FE_7, FE_6, FE_5, \
           FE_4, FE_3, FE_2, FE_1)(m, __VA_ARGS__)

#define FIELD_PTR(name) \
    static inline constexpr auto name##_ptr = &name;

#define FIELD_PTR_VALUE(name) name##_ptr,



#define FIELD_ACCESSOR(name) \
    BitFieldAccessor<std::remove_cvref_t<decltype(*name##_ptr)>> name{*name##_ptr,m_obj};

#define DECLARE_FIELDS(...) \
    protected:              \
    FOR_EACH(FIELD_PTR, __VA_ARGS__) \
    public:                 \
    static inline constexpr auto fields =                       \
        std::tuple{ FOR_EACH(FIELD_PTR_VALUE, __VA_ARGS__) };    \
                                                                  \
    static_assert(validateFields(),"Invalid spec: overlapping fields or fields beyond spec");                        \
                            \
    class ObjectAcc                                              \
    {                                                             \
    public:                                                       \
            explicit ObjectAcc(const SG::AuxElement& obj)         \
            : m_obj(obj)                                          \
        {}                  \
    protected:                \
        const SG::AuxElement& m_obj;          \
    public:                 \
                                                                  \
        FOR_EACH(FIELD_ACCESSOR, __VA_ARGS__)                    \
                                                                  \
    }




#endif
