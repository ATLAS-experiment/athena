/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "gtest/gtest.h"

#include "TestSpec.h"


#include "GlobalSimulation/Object.h"


#include "TInterpreter.h"

TEST( GlobalSimObjectTest, Bitset ) {
    using namespace GlobalSim;

    // test clamped unsigned bitset
    Bitset<8,true,false> cubs{300ull};
    ASSERT_EQ( cubs.to_ulong() , 255ul );
    ASSERT_EQ( cubs.code(), 255ul );

    // clamped signed bitset
    Bitset<8,true,true> csbs1{300ull};
    ASSERT_EQ( csbs1.to_ulong() , 127ul );
    ASSERT_EQ( csbs1.code(), 127l );

    Bitset<8,true,true> csbs2{-300ll};
    ASSERT_EQ( csbs2.to_ulong() , 128ul ); // since this is -128 in unsigned
    ASSERT_EQ( csbs2.code(), -128l );

}

TEST( GlobalSimObjectTest, BitFieldTests ) {
    using namespace GlobalSim;

    // an unsigned, clamped bitfield, with a simple linear encoding where centre=0 so that codes correspond to lower
    BitField<0, 3, float> bf4{{.name="bf4", .auxvar="bf4", .description="4-bit field", .scale=0.1, .centre=0}};
    ASSERT_EQ( bf4.maxCode, 15 );
    ASSERT_EQ( bf4.encode(1.05).to_ulong(), 10 );
    ASSERT_EQ( bf4.encode(-1).code(), 0 ); // clamped, so negative energies should clamp to lowest code
    ASSERT_EQ( bf4.encode(150).code(), 15 ); // clamped, so highest energies should clamp to highest code
    ASSERT_EQ( bf4.decode(bf4.encode(0.15)), 0.1f ); // centre=0, so will return rounded down value
}

TEST( GlobalSimObjectTest, test1 ) {

    using namespace GlobalSim;

    // here we load the TestSpec into the interpreter .. necessary for class that doesn't have a dictionary

    // cppcheck-suppress syntaxError
    const char* theText =
    #include "TestSpec_source.h"
    ;
    gInterpreter->ProcessLine(theText);

    // The BitSpec can be interrogated in a static manner:
    ASSERT_EQ( TestSpec::width, 64u );
    ASSERT_EQ( TestSpec::numFields() , 4u);
    ASSERT_EQ( TestSpec::field<0>().name, "f1"); // access field by index
    ASSERT_EQ( TestSpec::field<1>().description, "Description 2");
    ASSERT_EQ( TestSpec::f3.name, "f3"); // or if know it, access by name

    TestSpec::json(); // generates the json describing the bit specification (names, descriptions, widths)

    // encoding/decoding between value type and bits of field
    // this functionality may be needed when translating menu values during run configuration?
    ASSERT_EQ( TestSpec::f1.encode(3.1), std::bitset<13>{12} ); // checks how value "3.1" would be encoded
    ASSERT_EQ( TestSpec::f1.decode(TestSpec::f1.encode(3.1)), 3.0 ); // how scheme on this field rounds value


    SG::AuxElement ae;
    ae.makePrivateStore(); // creates a private auxstore for ae

    Object<TestSpec> obj(ae); // now we can interact with ae through the interface

    obj.f1 = 4.1; // assigning using the value type of the field
    ASSERT_FLOAT_EQ( obj.f1.value() , 4.1 );     // currently returns the actual value from auxvar
    ASSERT_FLOAT_EQ( obj.f1.value(true) , 4.0 ); // but can get truncated value like this
    ASSERT_EQ( obj.f1.bits(), std::bitset<obj.f1.width>(16)); // since encoder scale was 0.25

    obj.dump(); // prints auxvar contents

    obj = std::bitset<obj.spec.width>{10};  // can assign all bits like this
    ASSERT_EQ( obj.bits(), std::bitset<obj.spec.width>{10}); // access all bits

    obj.dump(); // print again. Now *all* auxvar are defined, because of above assignment

    obj.f4 = std::bitset<obj.f4.width>{0b10}; // assignment with bits, not value type
    ASSERT_EQ( obj.f4.value(), 2 );
    SG::ConstAccessor<uint8_t> acc("auxvar3"); // how to access an auxvar of type uint8_t
    ASSERT_EQ( acc(ae), 2 << obj.f4.auxspec.shift ); // auxspec.shift is bitspec metadata

    // constructing std::functions to dynamically call methods of BitSpecs

    auto myFunc = GlobalSim::BitSpecFunction<uint64_t(float)>("TestSpec::f1.encode","code()");
    ASSERT_EQ( myFunc(55.23), TestSpec::f1.encode(55.23).code() );

    auto myFunc2 = GlobalSim::BitSpecFunction<unsigned int()>("TestSpec::f1.width"); // can also evaluate a static data member
    ASSERT_EQ( myFunc2(), TestSpec::f1.width );


}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS();
}
