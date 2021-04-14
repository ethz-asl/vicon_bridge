
//////////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2017 Vicon Motion Systems Ltd
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//////////////////////////////////////////////////////////////////////////////////
#include "WirelessConfiguration.h"


#ifdef _WIN32
#include <windows.h>

#include <strsafe.h>
#include <wlanapi.h>

#include <map>
#include <cassert>

#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "strsafe.lib")

namespace
{
  std::string ErrorString( DWORD i_ErrorCode )
  {
    std::string Error;
    LPVOID pMsgBuf = nullptr;

    if ( FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      i_ErrorCode,
      MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
      ( LPTSTR )&pMsgBuf,
      0, NULL ) > 0 )
    {
      const size_t NonMsgBufStringLength = 64;
      LPVOID pDisplayBuf = ( LPVOID )LocalAlloc( LMEM_ZEROINIT, ( lstrlen( ( LPCTSTR )pMsgBuf ) + NonMsgBufStringLength ) * sizeof( TCHAR ) );
      if ( pDisplayBuf != nullptr )
      {
        StringCchPrintf( ( LPTSTR )pDisplayBuf,
                         LocalSize( pDisplayBuf ) / sizeof( TCHAR ),
                         TEXT( "Wireless configuration failed with error %d: %s" ),
                         i_ErrorCode,
                         pMsgBuf );

        Error = ( LPTSTR )pDisplayBuf;

        LocalFree( pDisplayBuf );
      }

      LocalFree( pMsgBuf );
    }

    return Error;
  }

  void LogError( std::string & io_rError, const std::string & i_rCategory, const std::string & i_rError )
  {
    if( !io_rError.empty() )
    {
      io_rError += "\n";
    }

    io_rError += i_rCategory + " : " + i_rError;
  }
}

class VWirelessConfiguration::VImpl
{
public:

  VImpl()
  {
  }

  bool SetInterface( std::string & o_rError, WLAN_INTF_OPCODE i_OpCode, bool i_State )
  {
    const BOOL State = i_State ? TRUE : FALSE;

    for ( unsigned int InterfaceIndex = 0; InterfaceIndex < m_pIfList->dwNumberOfItems; InterfaceIndex++ )
    {
      PWLAN_INTERFACE_INFO pIfInfo = (WLAN_INTERFACE_INFO *) & m_pIfList->InterfaceInfo[ InterfaceIndex ];

      DWORD PreviousStateSize;
      BOOL* pPreviousState;

      const DWORD QueryResultCode = WlanQueryInterface( m_hClient,
                                     &pIfInfo->InterfaceGuid,
                                     i_OpCode,
                                     NULL,
                                     &PreviousStateSize,
                                     (PVOID*)&pPreviousState,
                                     NULL );

      if( QueryResultCode != ERROR_SUCCESS )
      {
        LogError( o_rError, "Query", ErrorString( QueryResultCode ) );
        continue;
      }

      if( pPreviousState == NULL )
      {
        LogError( o_rError, "Query", "Previous null." );
        continue;
      }

      if( *pPreviousState == State )
      {
        WlanFreeMemory( pPreviousState );
        pPreviousState = NULL;
        continue;
      }

      const DWORD SetResultCode = WlanSetInterface( m_hClient,
                                                 & pIfInfo->InterfaceGuid,
                                                 i_OpCode,
                                                 sizeof( State ),
                                                 ( PVOID )&State,
                                                 nullptr );

      if( SetResultCode != ERROR_SUCCESS )
      {
        LogError( o_rError, "Set", ErrorString( SetResultCode ) );
        continue;
      }

      m_Changes[std::make_pair(InterfaceIndex, i_OpCode)] = *pPreviousState;
      WlanFreeMemory( pPreviousState );
    }

    return o_rError.empty();
  }

  ~VImpl()
  {
    for( const auto& rChange : m_Changes )
    {
      PWLAN_INTERFACE_INFO pIfInfo = (WLAN_INTERFACE_INFO*)&m_pIfList->InterfaceInfo[ rChange.first.first ];

      const DWORD ResetResultCode = WlanSetInterface( m_hClient,
                                                      &pIfInfo->InterfaceGuid,
                                                      rChange.first.second,
                                                      sizeof( rChange.second ),
                                                      (PVOID)&rChange.second,
                                                      nullptr );

      assert( ResetResultCode == ERROR_SUCCESS );
    }

    if( m_pIfList )
    {
      WlanFreeMemory( m_pIfList );
      m_pIfList = nullptr;
    }

    if( m_hClient )
    {
      WlanCloseHandle( m_hClient, nullptr );
      m_hClient = nullptr;
    }
  }


  HANDLE m_hClient = nullptr;
  PWLAN_INTERFACE_INFO_LIST m_pIfList = nullptr;
  std::map< std::pair< unsigned int, WLAN_INTF_OPCODE >, BOOL > m_Changes;
};



std::shared_ptr< VWirelessConfiguration >  VWirelessConfiguration::Create( std::string & o_rError )
{
  std::shared_ptr< VImpl > pImpl( new VImpl() );

  const DWORD dwMaxClient = 2; // Client version for Windows Vista and Windows Server 2008
  DWORD CurrentVersion = 0;
  const DWORD OpenResultCode = WlanOpenHandle( dwMaxClient, nullptr, &CurrentVersion, &pImpl->m_hClient );
  if ( OpenResultCode != ERROR_SUCCESS ) {
    o_rError = ErrorString( OpenResultCode );
    return std::shared_ptr< VWirelessConfiguration >();
  }


  const DWORD EnumerateResultCode = WlanEnumInterfaces( pImpl->m_hClient, nullptr, &pImpl->m_pIfList );
  if ( EnumerateResultCode != ERROR_SUCCESS ) {
    o_rError = ErrorString( EnumerateResultCode );
    return std::shared_ptr< VWirelessConfiguration >();
  }

  return std::shared_ptr< VWirelessConfiguration >( new VWirelessConfiguration( pImpl ) ); 
}



VWirelessConfiguration::VWirelessConfiguration( std::shared_ptr< VImpl > i_pImpl )
  : m_pImpl( i_pImpl )
{
}

VWirelessConfiguration::~VWirelessConfiguration()
{
}

bool VWirelessConfiguration::BackgroundScan( std::string & o_rError, bool i_Enable )
{
  return m_pImpl->SetInterface( o_rError, wlan_intf_opcode_background_scan_enabled, i_Enable);
}

bool VWirelessConfiguration::StreamingMode( std::string & o_rError, bool i_Enable )
{
  return m_pImpl->SetInterface( o_rError, wlan_intf_opcode_media_streaming_mode, i_Enable);
}

#else
class VWirelessConfiguration::VImpl
{
};

std::shared_ptr< VWirelessConfiguration > VWirelessConfiguration::Create( std::string& o_rError )
{
  o_rError = "Supported on Windows only.";
  return std::shared_ptr< VWirelessConfiguration >();
}

VWirelessConfiguration::VWirelessConfiguration( std::shared_ptr< VImpl > i_pImpl )
: m_pImpl( i_pImpl )
{
}

VWirelessConfiguration::~VWirelessConfiguration()
{
}

bool VWirelessConfiguration::BackgroundScan( std::string& o_rError, [[maybe_unused]] bool i_Enable )
{
  o_rError = "Supported on Windows only.";
  return false;
}

bool VWirelessConfiguration::StreamingMode( std::string& o_rError, [[maybe_unused]] bool i_Enable )
{
  o_rError = "Supported on Windows only.";
  return false;
}


#endif
