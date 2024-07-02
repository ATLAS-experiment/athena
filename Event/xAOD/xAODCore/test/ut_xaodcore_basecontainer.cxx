#include "xAODCore/BaseContainer.h"
#include "xAODCore/AuxContainerBase.h"

int main() {
    xAOD::BaseContainer interface;
    xAOD::AuxContainerBase aux;
    interface.setStore(&aux);

    SG::AuxElement* obj = nullptr;
    
    obj = new SG::AuxElement();
    interface.push_back( obj );
    SG::Accessor< int > AnInt( "AnInt" );
    SG::Accessor< float > AFloat( "AFloat" );
    AnInt( *obj ) = 7;
    AFloat( *obj ) = 0.7;


    obj = new SG::AuxElement();
    interface.push_back( obj );
    AnInt( *obj ) = 9;
    AFloat( *obj ) = 1.9;

    for ( auto el: interface) {
       std::cout << AnInt( *el ) << " " << AFloat( *el ) << std::endl;
    }



}
