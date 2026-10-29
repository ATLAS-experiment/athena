inline DatabaseHandler*
DatabaseRegistry::lookupByFID( const std::string& fid )
{
  std::map< std::string, DatabaseHandler* >::iterator idb = m_fidToDb.find( fid );
  if ( idb == m_fidToDb.end() ) return 0;
  else return idb->second;
}

inline DatabaseHandler*
DatabaseRegistry::lookupByPFN( const std::string& pfn )
{
  std::map< std::string, DatabaseHandler* >::iterator idb = m_pfnToDb.find( pfn );
  if ( idb == m_pfnToDb.end() ) return 0;
  else return idb->second;
}

inline DatabaseHandler*
DatabaseRegistry::lookupByLFN( const std::string& lfn )
{
  std::map< std::string, DatabaseHandler* >::iterator idb = m_lfnToDb.find( lfn );
  if ( idb == m_lfnToDb.end() ) return 0;
  else return idb->second;
}

inline std::size_t
DatabaseRegistry::size() const
{
  return m_databases.size();
}

inline DatabaseRegistry::iterator
DatabaseRegistry::begin()
{
  return m_databases.begin();
}

inline DatabaseRegistry::const_iterator
DatabaseRegistry::begin() const
{
  return m_databases.begin();
}

inline DatabaseRegistry::iterator
DatabaseRegistry::end()
{
  return m_databases.end();
}

inline DatabaseRegistry::const_iterator
DatabaseRegistry::end() const
{
  return m_databases.end();
}
