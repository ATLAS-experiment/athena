/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "gtest/gtest.h"

#include "TestSpec.h"


#include "GlobalSimulation/Object.h"

#include "TInterpreter.h"

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
    ASSERT_EQ( TestSpec::field<0>().name(), "f1"); // access field by index
    ASSERT_EQ( TestSpec::field<1>().description(), "Description 2");
    ASSERT_EQ( TestSpec::f3.name(), "f3"); // or if know it, access by name

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


}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS();
}
