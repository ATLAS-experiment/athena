inline pool::DatabaseHandler*
pool::DatabaseRegistry::lookupByFID( const std::string& fid )
{
  std::map< std::string, pool::DatabaseHandler* >::iterator idb = m_fidToDb.find( fid );
  if ( idb == m_fidToDb.end() ) return 0;
  else return idb->second;
}

inline pool::DatabaseHandler*
pool::DatabaseRegistry::lookupByPFN( const std::string& pfn )
{
  std::map< std::string, pool::DatabaseHandler* >::iterator idb = m_pfnToDb.find( pfn );
  if ( idb == m_pfnToDb.end() ) return 0;
  else return idb->second;
}

inline pool::DatabaseHandler*
pool::DatabaseRegistry::lookupByLFN( const std::string& lfn )
{
  std::map< std::string, pool::DatabaseHandler* >::iterator idb = m_lfnToDb.find( lfn );
  if ( idb == m_lfnToDb.end() ) return 0;
  else return idb->second;
}

inline std::size_t
pool::DatabaseRegistry::size() const
{
  return m_databases.size();
}

inline pool::DatabaseRegistry::iterator
pool::DatabaseRegistry::begin()
{
  return m_databases.begin();
}

inline pool::DatabaseRegistry::const_iterator
pool::DatabaseRegistry::begin() const
{
  return m_databases.begin();
}

inline pool::DatabaseRegistry::iterator
pool::DatabaseRegistry::end()
{
  return m_databases.end();
}

inline pool::DatabaseRegistry::const_iterator
pool::DatabaseRegistry::end() const
{
  return m_databases.end();
}
