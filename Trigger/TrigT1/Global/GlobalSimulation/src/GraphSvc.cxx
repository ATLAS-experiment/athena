/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GraphSvc.h"
#include "GaudiKernel/Algorithm.h"
#include "Gaudi/Sequence.h"
#include "GaudiKernel/DataHandleHolderVisitor.h"

#include <fstream>

namespace GlobalSim {

    StatusCode GraphSvc::initialize() {
        return StatusCode::SUCCESS;
    }

    StatusCode GraphSvc::start() {

        std::function<void(IAlgorithm*,std::set<IAlgorithm*>&,bool)> func;

        std::regex sequenceNameRegex( m_topSequence.value() );

        // locate the desired algs ...
        func = [&](IAlgorithm* alg, std::set<IAlgorithm*>& thealgs, bool isAdding) {
            isAdding = (isAdding || std::regex_search( std::string(alg->nameKey()) , sequenceNameRegex) ); // activate adding when get to target
            if(isAdding) {
                thealgs.insert(alg);
            }
            if(auto seq = dynamic_cast<Gaudi::Sequence*>( alg )) {
                auto subalgs = seq->subAlgorithms();
                for(auto alg : *subalgs) {
                    func(alg,thealgs,isAdding);
                }
            }
        };

        // Get the list of algorithms
        const std::list<IAlgorithm*>& algos      = m_algResourcePool->getTopAlgList();
        std::set<IAlgorithm*> subalgs;
        for ( IAlgorithm* ialgoPtr : algos ) {
            func(ialgoPtr,subalgs,false);
        }


        DataObjIDColl globalInp, globalOutp;
        std::map<std::string, DataObjIDColl> algosOutputDependenciesMap;
        std::map<std::string, DataObjIDColl> algosInputDependenciesMap;

        for ( IAlgorithm* ialgoPtr : subalgs ) {
            if(ialgoPtr->isSequence()) continue; // don't look at sequences, shouldn't have any inputs/outputs

            Gaudi::Algorithm* algoPtr = dynamic_cast<Gaudi::Algorithm*>( ialgoPtr );
            if ( !algoPtr ) continue;
            DataObjIDColl algoOutputs;
            for ( auto id : algoPtr->outputDataObjs() ) {
                globalOutp.insert( id );
                algoOutputs.insert( id );
            }



            DataObjIDColl i1, i2;
            DHHVisitor    avis( i1, i2 );
            algoPtr->acceptDHVisitor( &avis );

            DataObjIDColl algoDependencies;
            for ( const DataObjID& id : algoPtr->inputDataObjs() ) {
                algoDependencies.insert( id );
                globalInp.insert( id );
            }

            std::string algoName = ialgoPtr->nameKey();
            if(ialgoPtr->type()!= algoName) {
                algoName = ialgoPtr->type() + "/" + algoName;
            }

            algosInputDependenciesMap[algoName] = algoDependencies;
            algosOutputDependenciesMap[algoName] = algoOutputs;

        }


        ATH_MSG_INFO("Creating " << m_fileName.value());
        std::ofstream stream{ m_fileName, std::ofstream::out };
        stream << "digraph datadeps {\n  rankdir=\"LR\";\n"; // left-to-right graph

        auto addNode = [&](std::string_view id, std::string_view name, std::string_view shape="box" ) {
            stream << "  " << id << " [label=\"" << name << "\";shape=" << shape << "];\n"; // adds a node
        };
        auto addEdge = [&](std::string_view srcId, std::string_view tgtId, std::string_view label ) {
            stream << "  " << srcId << " -> " << tgtId << " [label=\"" << label << "\"];\n"; // adds an edge
        };

        // find all inputs that do not have an output. These will be our 'starting nodes' of shape='plaintext'
        std::map<std::string,std::pair<std::set<std::string>,std::set<std::string>>> deps;

        // key is dep name, value is pair of sets, first are "producers" of dep, second a "consumers" of dep
        // if has no producer, is a global input. if has no consumer, is a global output.

        for ( const auto& [algName, ideps] : algosInputDependenciesMap ) {
            for (const auto &dep: ideps) {
                deps[dep.key()].second.insert(algName); // alg is a consumer
            }
        }
        for ( const auto& [algName, odeps] : algosOutputDependenciesMap ) {
            for (const auto &dep: odeps) {
                deps[dep.key()].first.insert(algName); // alg is a producer
            }
        }

        std::size_t algoIndex = 0ul;
        std::map<std::string,std::string> keyToName;
        std::string inputs,outputs;
        // start by creating nodes of global inputs and outputs
        for( auto& [dep, pcs] : deps) {
            auto& [producers,consumers] = pcs;
            if(producers.empty()) {
                std::string algIndex = "Input_" + std::to_string( algoIndex );
                addNode( algIndex, dep, "plaintext" );
                keyToName[dep] = algIndex;
                algoIndex++;
                producers.insert(dep); // its a self-producer
                inputs += algIndex + "; ";
            } else if(consumers.empty()) {
                std::string algIndex = "Output_" + std::to_string( algoIndex );
                addNode( algIndex, dep, "plaintext" );
                keyToName[dep] = algIndex;
                algoIndex++;
                consumers.insert(dep); // its a self-consumer
                outputs += algIndex + "; ";
            }
        }
        // create nodes of all algorithms
        for ( const auto& [algName, ideps] : algosInputDependenciesMap ) {
            std::string algIndex = "Alg_" + std::to_string( algoIndex );
            addNode( algIndex, algName, "box" );
            keyToName[algName] = algIndex;
            algoIndex++;
        }
        // now go through deps and create link from every producer to every consumer
        for( const auto& [dep, pcs] : deps) {
            auto& [producers,consumers] = pcs;
            for(auto& producer : producers) {
                for(auto& consumer: consumers) {
                    addEdge(keyToName.at(producer),keyToName.at(consumer),dep!=producer && dep!=consumer ? dep : " ");
                }
            }
        }

        if(!inputs.empty()) {
            stream << "   { rank = same; " << inputs << "}\n";
        }
        if(!outputs.empty()) {
            stream << "   { rank = same; " << outputs << "}\n";
        }

        stream << "}\n";
        stream.close();

        return StatusCode::SUCCESS;

    }

    StatusCode GraphSvc::finalize() {
        return StatusCode::SUCCESS;

    }

}

