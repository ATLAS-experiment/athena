/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "dqm_algorithms/AFP_LBsOutOfRange.h"

#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <dqm_core/AlgorithmManager.h>
#include <dqm_core/exceptions.h>

#include <CoolApplication/DatabaseSvcFactory.h>
#include <CoolKernel/ChannelSelection.h>
#include <CoolKernel/IDatabase.h>
#include <CoolKernel/IDatabaseSvc.h>
#include <CoolKernel/IField.h>
#include <CoolKernel/IFolder.h>
#include <CoolKernel/IObject.h>
#include <CoolKernel/IObjectIterator.h>
#include <CoolKernel/IRecord.h>
#include <CoolKernel/ValidityKey.h>
#include <CoolKernel/pointers.h>
#include <CoolKernel/types.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TFile.h>

namespace {
    static dqm_algorithms::AFP_LBsOutOfRange instance;
    constexpr uint64_t LB_MASK = 0xffffffff;
    uint64_t getTime( const cool::IObject& obj, const std::string& name ) {
        return obj.payload()[ name ].data<cool::UInt63>();
    }
    uint64_t getStartTime( const cool::IObject& obj ) {
        return getTime( obj, "StartTime" );
    }
    uint64_t getEndTime( const cool::IObject& obj ) {
        return getTime( obj, "EndTime" );
    }
    uint32_t getLB( const cool::IObject& obj ) {
        return obj.since() & LB_MASK;
    }
} // namespace

dqm_algorithms::AFP_LBsOutOfRange::AFP_LBsOutOfRange() {
    dqm_core::AlgorithmManager::instance().registerAlgorithm( "AFP_LBsOutOfRange", this );
}

dqm_algorithms::AFP_LBsOutOfRange::~AFP_LBsOutOfRange() {
}

dqm_algorithms::AFP_LBsOutOfRange*
dqm_algorithms::AFP_LBsOutOfRange::clone() {
    return new AFP_LBsOutOfRange();
}

const dqm_algorithms::AFP_LBsOutOfRange::IOVSet&
dqm_algorithms::AFP_LBsOutOfRange::fetchIOVs( uint32_t run, uint32_t channel ) {
    auto& iovs = m_iov_cache[ std::make_pair( run, channel ) ];
    if ( iovs.empty() ) {
        auto& dbService = cool::DatabaseSvcFactory::databaseService();
        // Get map of LB timestamps to LB numbers
        uint64_t runStartTime = 0xFFffFFffFFffFFff;
        uint64_t runEndTime   = 0;
        std::map<uint64_t, uint32_t> lbs;
        {
            auto databaseName = std::string( "COOLONL_TRIGGER/CONDBR2" );
            auto folderName   = std::string( "/TRIGGER/LUMI/LBLB" );
            auto databasePtr  = dbService.openDatabase( databaseName, true );
            auto folderPtr    = databasePtr->getFolder( folderName );
            uint64_t run64    = run;
            auto since        = cool::ValidityKey( run64 << 32 );
            auto until        = cool::ValidityKey( ( run64 + 1 ) << 32 );
            auto iterPtr      = folderPtr->browseObjects( since, until, cool::ChannelSelection::all() );
            while ( iterPtr->goToNext() ) {
                auto& object     = iterPtr->currentRef();
                auto lbStartTime = getStartTime( object );
                auto lbEndTime   = getEndTime( object );
                lbs.emplace( lbStartTime, getLB( object ) );
                if ( lbStartTime < runStartTime ) runStartTime = lbStartTime;
                if ( lbEndTime > runEndTime ) runEndTime = lbEndTime;
            }
            if ( lbs.empty() ) throw afp::CantReadCool( ERS_HERE, std::move(databaseName), std::move(folderName) );
        }
        // Get IOVs where the station was in physics position,
        // merge the neighbouring ones,
        // and translate the timestamps to LB numbers
        {
            auto databaseName = std::string( "COOLOFL_DCS/CONDBR2" );
            auto folderName   = std::string( "/AFP/DCS/STATION" );
            auto databasePtr  = dbService.openDatabase( databaseName, true );
            auto folderPtr    = databasePtr->getFolder( folderName );
            auto iterPtr      = folderPtr->browseObjects( runStartTime, runEndTime, cool::ChannelSelection( channel ) );
            uint64_t since = 0, until = 0;
            iovs.reserve( iterPtr->size() / 2 );
            while ( iterPtr->goToNext() ) {
                auto& object   = iterPtr->currentRef();
                auto inphysics = object.payload()[ "inphysics" ].data<cool::Bool>();
                if ( inphysics ) {
                    if ( since == 0 ) since = object.since();
                    until = object.until();
                } else if ( since != 0 ) {
                    uint32_t start = lbs.lower_bound( since )->second;
                    uint32_t end = ( --lbs.upper_bound( until ) )->second;
                    iovs.emplace_back( start, end );
                    since = 0;
                }
            }
        }
        // Ensure the list is not empty
        if ( iovs.empty() ) {
            iovs.emplace_back( 0u, 0u );
        }
    }
    return iovs;
}

dqm_core::Result*
dqm_algorithms::AFP_LBsOutOfRange::execute( const std::string& name,
                                            const TObject& object,
                                            const dqm_core::AlgorithmConfig& config ) {
    if ( !object.IsA()->InheritsFrom( "TH1" ) ) {
        throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH1" );
    }

    auto histogram = static_cast<const TH1*>( &object );
    
    if ( histogram->GetDimension() > 1 ) {
        throw dqm_core::BadConfig( ERS_HERE, name, "histogram has more than 1 dimension" );
    }

    auto gthreshold = static_cast<uint32_t>( dqm_algorithms::tools::GetFromMap( "NbadBins", config.getGreenThresholds() ) );
    auto rthreshold = static_cast<uint32_t>( dqm_algorithms::tools::GetFromMap( "NbadBins", config.getRedThresholds() ) );
    
    auto channel   = static_cast<uint32_t>( dqm_algorithms::tools::GetFirstFromMap( "channel", config.getParameters() ) );
    auto RANGE_D   = static_cast<double>( dqm_algorithms::tools::GetFirstFromMap( "RANGE_D", config.getParameters(), -100 ) );
    auto RANGE_U   = static_cast<double>( dqm_algorithms::tools::GetFirstFromMap( "RANGE_U", config.getParameters(), 100 ) );
    auto ignoreval = static_cast<double>( dqm_algorithms::tools::GetFirstFromMap( "ignoreval", config.getParameters(), 0 ) );
    auto minLBs    = static_cast<uint32_t>( dqm_algorithms::tools::GetFirstFromMap( "MinLBs", config.getParameters(), 0 ) );
    
    auto run   = dqm_algorithms::tools::GetFirstFromMap( "run_number", config.getParameters(), 0 );
    auto& iovs = run > 0 ? fetchIOVs( run, channel ) : IOVSet{ { 0, 0 } };

    auto range    = dqm_algorithms::tools::GetBinRange( histogram, config.getParameters() );
    uint32_t xmin = range[ 0 ];
    uint32_t xmax = range[ 1 ] + 1;
    auto xaxis    = histogram->GetXaxis();
    std::vector<uint32_t> badbins;
    uint32_t checkedLBs = 0;
    for ( auto& iov : iovs ) {
        auto xstart = std::max<uint32_t>( xaxis->FindFixBin( iov.first ), xmin );
        auto xend   = std::min<uint32_t>( xaxis->FindFixBin( iov.second ), xmax );
        for ( uint32_t ix = xstart; ix < xend; ++ix ) {
            double binvalue = histogram->GetBinContent( ix );
            if ( binvalue == ignoreval ) continue;
            ++checkedLBs;
            if ( RANGE_D < binvalue && binvalue < RANGE_U ) continue;
            badbins.push_back( ix );
        }
    }

    auto publish     = static_cast<bool>( dqm_algorithms::tools::GetFirstFromMap( "PublishBins", config.getParameters(), true ) );
    auto Nmaxpublish = static_cast<uint32_t>( dqm_algorithms::tools::GetFirstFromMap( "MaxPublish", config.getParameters(), 20 ) );
    auto result      = new dqm_core::Result();

    // publish problematic bins
    result->tags_[ "NBadBins" ] = badbins.size();
    if ( publish ) {
        auto npublish = std::min<uint32_t>( Nmaxpublish, badbins.size() );
        for ( uint32_t i = 0; i < npublish; ++i ) {
            uint32_t ix = badbins[ i ];
            uint32_t lb = std::floor( xaxis->GetBinCenter( ix ) );
            auto tag    = ( std::ostringstream() << "LB " << lb ).str();

            result->tags_[ tag ] = histogram->GetBinContent( ix );
        }
    }

    if ( checkedLBs < minLBs )
        result->status_ = dqm_core::Result::Undefined;
    else if ( badbins.size() > rthreshold )
        result->status_ = dqm_core::Result::Red;
    else if ( badbins.size() > gthreshold )
        result->status_ = dqm_core::Result::Yellow;
    else
        result->status_ = dqm_core::Result::Green;

    return result;
}

void dqm_algorithms::AFP_LBsOutOfRange::printDescriptionTo( std::ostream& out ) {
    out << "AFP_LBsOutOfRange: Print out the bad LBs, during which AFP was in physics position, and which are out of range [RANDE_D,RANGE_U] (default [-100,100])\n"
        << "Required Parameter: channel: channel number of the station in COOLOFL_DCS/CONDBR2 /AFP/DCS/STATION\n"
        << "Optional Parameter: ignoreval: value to be ignored for being processed (default 0)\n"
        << "Optional Parameter: MaxPublish: Max number of bins to save (default 20)\n"
        << "Optional Parameter: MinLBs: Minimum number of LBs in physics with value not equal to ignoreval to assign color (default 0)" << std::endl;
}
