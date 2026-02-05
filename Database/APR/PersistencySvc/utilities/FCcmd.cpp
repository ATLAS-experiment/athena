/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
   FCcmd.cpp -- FileCatalog command line tool to list or manipulate entries in a FileCatalog XML file
*/

#include "PersistencySvc/IFileCatalog.h"
#include "POOLCore/SystemTools.h"

#include <exception>
#include <memory>

using namespace pool;
using std::cerr, std::cout, std::endl;
using std::string;

void printUsage(){
  cout <<
  "Usage: " << endl <<
  "FCcmd <action> <item> [-p PFName -l LFName -n newPFName -g GUID -u catalogName -h]" << endl <<
  "  supported actions are:" <<endl <<
  "     register : <item> = lfn|pfn  (registering a PFN requires a GUID)" <<endl <<
  "     list     : <item> = guid|lfn|pfn" <<endl <<
  "     delete   : deletes an entry using LFN or PFN" <<endl <<
  "     rename   : rename PFName to a new PFName" <<endl <<
  " example:  FCcmd list pfn -l myLogicalFileName" << endl;
}


class Options {
  public:
    Options(int argc, char* argv[]);
    
    inline const string& getProgName() const;
    inline const string& getAction() const;
    inline const string& getItem() const;
    string               getOptByName( char opt ) const;
    inline bool          exists( char opt ) const;
    
  private:
    std::map<char, string>  m_argMap;
    string      m_action;
    string      m_item;
    string      m_progName;
    
    // No copying or assignment is supported
    Options(const Options&) = delete;
    Options& operator=(const Options&) = delete;
};

// Parse command line and keep information about arguments
Options::Options(int argc, char* argv[]) {
    if( argc > 0 ) {
        m_progName = string( argv[0] );
    }
    if( argc > 1 ) {
        m_action = string( argv[1] );
    }
    if( argc > 2 ) {
        m_item   = string( argv[2] );
    }
    // scan from the beginning (in case there is a lone -h)
    int pos = 1;
    while( argc > pos ) {
        char opt {};
        string val;
        string arg = string( argv[pos++] );
        if( arg.length() == 2 && arg[0] == '-' ) {
            opt = arg[1];
        } else continue;
        if( argc > pos ) {
            arg = string( argv[pos] );
            if( arg.length() != 2 or arg[0] != '-' ) {
                val = arg;
                pos++;
            }
        }
        m_argMap[opt] = val;
    }
}

inline string Options::getOptByName( char opt ) const {
    auto it = m_argMap.find( opt );
    return (it != m_argMap.end())?  it->second : "" ;
}

inline const string& Options::getProgName() const { return m_progName; }
inline const string& Options::getAction() const { return m_action; }
inline const string& Options::getItem() const { return m_item; }
inline bool          Options::exists( char opt ) const { return m_argMap.find(opt) != m_argMap.end(); }



int main(int argc, char** argv)
{
    SystemTools::initGaudi();

    Options options(argc, argv);
    // possible options:
    // const char* opts[] = {"n","l","p","g","u","h",0};
    if( options.exists('h') ) {
        printUsage();
        return 0;
    }
    if( argc < 3 ){
        cerr << "ERROR! insufficient number of arguments" << endl;
        printUsage();
        return 1;
    }
    string  catName;
    // In case none of the options below work, the default FC name is specified in IFileCatalog::addCatalog()
    if( options.exists('u') ){
        catName=options.getOptByName('u');
    }else{
        catName=SystemTools::GetEnvStr("POOL_CATALOG");
    }
    string lfn    = options.getOptByName('l');
    string pfn    = options.getOptByName('p');
    string newpfn = options.getOptByName('n');
    string guid   = options.getOptByName('g');
    string action = options.getAction();
    string item   = options.getItem();
    
    try{
        std::unique_ptr<IFileCatalog> mycatalog(new IFileCatalog);
        mycatalog->setWriteCatalog(catName);
        mycatalog->start();

        if( action == "delete" ) {
            if( !lfn.empty() ){
                mycatalog->deleteFID( mycatalog->lookupLFN( lfn ) );
            }else if( !pfn.empty() ) {
                mycatalog->deleteFID( mycatalog->lookupPFN( pfn ) );
            }else {
                cerr << "ERROR! Use PFN or LFN to specify the entry for deletion: " << endl;
                printUsage();
                return 2;
            }

        } else if( action == "list" ) {
            if( item == "lfn" ) {
                pool::IFileCatalog::Strings fids;
                if( !pfn.empty() ) {
                    fids.push_back( mycatalog->lookupPFN( pfn ) );
                }else{
                    mycatalog->getFIDs( fids );
                }
                for( const auto& fid: fids ) {
                    pool::IFileCatalog::Files files;
                    mycatalog->getLFNs( fid, files );
                    for( const auto& file: files ) {
                        cout << file.first << " ,   " << file.second << endl;
                    }
                }
            } 
            else if( item == "pfn" ) {
                pool::IFileCatalog::Strings fids;
                if( !lfn.empty() ) {
                    fids.push_back( mycatalog->lookupLFN( lfn ) );
                } else if( !guid.empty() ) {
                    fids.emplace_back( std::move(guid) );
                } else {
                    // go through all FIDs in the catalog
                    mycatalog->getFIDs( fids );
                }
                for( const auto& fid: fids ) {
                    pool::IFileCatalog::Files files;
                    mycatalog->getPFNs( fid, files );
                    for( const auto& file: files ) {
                        string pf = file.first;
                        string filetype = (file.second.empty()? string("NULL") : file.second);
                        cout<<pf<<"    "<<filetype<<endl;
                    }
                }
            }
            else if( item == "guid" ) {
                pool::IFileCatalog::Strings fids;
                if( !pfn.empty() ){
                    fids.push_back( mycatalog->lookupPFN( pfn ) );
                } else if( !lfn.empty() ){
                    fids.push_back( mycatalog->lookupLFN( lfn ) );
                } else {
                    mycatalog->getFIDs( fids );
                }
                for( const auto& fid: fids ) {
                    cout << fid << endl;
                }
            }
            else {
                cerr<< "ERROR! Unsupported catalog item type for listing: "<< item << endl;
                printUsage();
                return 2;
            }
            
        } else if( action == "register" ) {
            if( item == "lfn" ) {
                if( pfn.empty() || lfn.empty() ){
                    cerr<<"ERROR! You must specify PFName using -p and LFName using -l" << endl;
                    return 3;
                }
                mycatalog->registerLFN( mycatalog->lookupPFN(pfn), lfn );
            }
            else if( item == "pfn" ) {
                if( pfn.empty() or guid.empty() ) {
                    cerr<<"ERROR! You must specify PFName using -p and GUID using -g options" << endl;
                    return 3;
                }
                mycatalog->registerPFN(pfn, "ROOT_All", guid);
            } 
            else {
                cerr << "ERROR! Unsupported catalog item type for registration: "<< item << endl;
                printUsage();
                return 3;
            }

        } else if( action == "rename" ) {
            if( pfn.empty() || newpfn.empty() ){
                cerr<<"ERROR! You must specify the current PFName using -p and the new PFName using -n option" << endl;
                return 4;
            }
            mycatalog->renamePFN(pfn, newpfn);
        }
        else {
            cerr << "ERROR! Unsupported action: " << action << endl;
            printUsage();
            return 10;
        }

        mycatalog->commit();

    }catch (const std::runtime_error& er){
        cerr<<er.what()<<endl;
        return 1;
    }catch (const std::exception& er){
        cerr<<er.what()<<endl;
        return 1;
    }
}