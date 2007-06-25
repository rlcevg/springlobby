//
// Class: Unitsync
//

#include "unitsync.h"
#include <assert.h>


void* Unitsync::_GetLibFuncPtr( const std::string& name )
{
  assert( m_loaded );
  void* ptr = NULL;
#ifdef WIN32
  ptr = (void*)GetProcAddress( m_libhandle, name.c_str() );
#else
  ptr = dlsym( m_libhandle, name.c_str() );
#endif
  ASSERT_RUNTIME( ptr != NULL, "Couldn't load " + name + " from unitsync library" );
  return ptr;
}


bool Unitsync::LoadUnitsyncLib()
{
  if ( m_loaded ) return true;
  //system( "cd C:\\Program Files\\Spring" );
  // Load the library.

  std::string loc;
  if ( sett().GetUnitsyncUseDefLoc() ) loc = sett().GetSpringDir() + dllname;
  else loc = sett().GetUnitsyncLoc();

#ifdef WIN32

  m_libhandle = LoadLibrary( loc.c_str() );
  if (m_libhandle == NULL) {
    debug_error( "Couldn't load the unitsync library" );
    return false;
  }

#else

  m_libhandle = dlopen( loc.c_str(), RTLD_LOCAL | RTLD_LAZY );
  if (m_libhandle == NULL) {
    debug_error( "Couldn't load the unitsync library" );
    return false;
  }

#endif

  m_loaded = true;

  // Load all function from library.
  try {
    m_init_ptr = (InitPtr)_GetLibFuncPtr("Init");
    m_get_map_count_ptr = (GetMapCountPtr)_GetLibFuncPtr("GetMapCount");
    m_init_ptr( true, 1 );
  }
  catch ( std::runtime_error& e ) {
    debug_error( e.what() );
    FreeUnitsyncLib();
    return false;
  }

  return true;
}


void Unitsync::FreeUnitsyncLib()
{
  if ( !m_loaded ) return;

#ifdef WIN32
  FreeLibrary(m_libhandle);
#else
  dlclose(m_libhandle);
#endif
  
  m_loaded = false;
}


bool Unitsync::IsLoaded() const
{
  return m_loaded;
}


int Unitsync::GetNumMaps() const
{
  assert( m_loaded );
  return m_get_map_count_ptr();
}

