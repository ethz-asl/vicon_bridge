
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
#include "CGClient.h"

#include "ICGFrameState.h"

#include <functional>
#include <boost/thread/xtime.hpp>

#include <numeric>

namespace ViconCGStreamClientSDK
{

// Static factory method to create an instance of ICGClient
ICGClient * ICGClient::CreateCGClient()
{
  return new VCGClient();
}

/*********************************************************************/

// Callback class suitable for having on a weak_ptr
class VCGClientCallback : public IViconCGStreamClientCallback
{
private:
  VCGClient & m_rCGClient;
  const size_t m_ConnectionId;

public:
  VCGClientCallback( VCGClient & i_rCGClient, size_t i_ConnectionId )
  : m_rCGClient( i_rCGClient )
  , m_ConnectionId( i_ConnectionId )
  {
  }

  // IViconCGStreamClientCallback
  virtual void OnConnect()
  {
    m_rCGClient.OnConnect( m_ConnectionId );
  }

  virtual void OnStaticObjects(  std::shared_ptr< const VStaticObjects >  i_pStaticObjects )
  {
    m_rCGClient.OnStaticObjects( i_pStaticObjects );
  }

  virtual void OnDynamicObjects( std::shared_ptr< const VDynamicObjects > i_pDynamicObjects )
  {
    m_rCGClient.OnDynamicObjects( i_pDynamicObjects, m_ConnectionId );
  }

  virtual void OnDisconnect()
  {
    m_rCGClient.OnDisconnect( m_ConnectionId );
  }

};

/*********************************************************************/

VCGClient::VCGClient()
: m_bMulticastReceiving( false )
, m_bMulticastController( false )
, m_MaxBufferSize( 1 )
{
}

VCGClient::~VCGClient()
{
}

void VCGClient::Destroy()
{
  delete this;
}

bool VCGClient::PollFrames( std::vector< ICGFrameState > & o_rFrames )
{
  TFrameDeque FrameDeque;

  {
    boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );
    std::swap( m_FrameDeque, FrameDeque );
  }
  
  o_rFrames.resize( FrameDeque.size() );
  if( FrameDeque.empty() )
  {
    return false;
  }

  TFrameDeque::iterator It = FrameDeque.begin();
  TFrameDeque::iterator End = FrameDeque.end();
  unsigned int Index = 0;
  for( ; It != End; ++It, ++Index )
  {
    ICGFrameState & rCGFrameState = o_rFrames[ Index ];
    ReadFramePair( *It, rCGFrameState );
  }
  return true;
}

bool VCGClient::PollFrame( ICGFrameState & o_rFrame )
{
  TFramePair FramePair;
  {
    boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );
    if( m_FrameDeque.empty() )
    {
      return false;
    }
    FramePair = m_FrameDeque.front();
    m_FrameDeque.pop_front();
  }
  
  ReadFramePair( FramePair, o_rFrame );
  return true;
}

void VCGClient::ReadFramePair( const TFramePair& i_rPair, ICGFrameState& o_rFrameState )
{
  const std::shared_ptr< const VStaticObjects > & rpStaticState = i_rPair.first;
  if ( rpStaticState )
  {
    o_rFrameState.m_Stream = rpStaticState->m_StreamInfo;
    if( rpStaticState->m_pCameraCalibrationHealth )
    {
      o_rFrameState.m_CameraCalibrationHealth = *(rpStaticState->m_pCameraCalibrationHealth);
    }
    o_rFrameState.m_CameraCalibrations = rpStaticState->m_CameraCalibrationInfo;
    o_rFrameState.m_Cameras = rpStaticState->m_CameraInfo;
    o_rFrameState.m_CamerasSensorInfo = rpStaticState->m_CameraSensorInfo;
    o_rFrameState.m_Subjects = rpStaticState->m_SubjectInfo;
    o_rFrameState.m_SubjectTopologies = rpStaticState->m_SubjectTopology;
    o_rFrameState.m_SubjectScales = rpStaticState->m_SubjectScale;
    o_rFrameState.m_SubjectHealths = rpStaticState->m_SubjectHealth;
    o_rFrameState.m_ObjectQualities = rpStaticState->m_ObjectQuality;
    o_rFrameState.m_Devices = rpStaticState->m_DeviceInfo;
    o_rFrameState.m_DevicesExtra = rpStaticState->m_DeviceInfoExtra;
    o_rFrameState.m_Channels = rpStaticState->m_ChannelInfo;
    o_rFrameState.m_ChannelUnits = rpStaticState->m_ChannelInfoExtra;
    o_rFrameState.m_ForcePlates = rpStaticState->m_ForcePlateInfo;
    o_rFrameState.m_EyeTrackers = rpStaticState->m_EyeTrackerInfo;
    o_rFrameState.m_ApplicationInfo = rpStaticState->m_ApplicationInfo;
  }

  const std::shared_ptr< const VDynamicObjects > & rpDynamicState = i_rPair.second;
  if ( rpDynamicState )
  {
    o_rFrameState.m_Frame = rpDynamicState->m_FrameInfo;
    o_rFrameState.m_HardwareFrame = rpDynamicState->m_HardwareFrameInfo;
    o_rFrameState.m_Timecode = rpDynamicState->m_Timecode;
    o_rFrameState.m_Latency = rpDynamicState->m_LatencyInfo;
    o_rFrameState.m_FrameRateInfo = rpDynamicState->m_FrameRateInfo;
    o_rFrameState.m_EdgePairs = rpDynamicState->m_EdgePairs;
    o_rFrameState.m_GreyscaleBlobs = rpDynamicState->m_GreyscaleBlobs;
    o_rFrameState.m_GreyscaleSubsampledBlobs = rpDynamicState->m_GreyscaleSubsampledBlobs;
    o_rFrameState.m_Centroids = rpDynamicState->m_Centroids;
    o_rFrameState.m_CentroidTracks = rpDynamicState->m_CentroidTracks;
    o_rFrameState.m_CentroidWeights = rpDynamicState->m_CentroidWeights;
    o_rFrameState.m_UnlabeledRecons.m_UnlabeledRecons = rpDynamicState->m_UnlabeledRecons.m_UnlabeledRecons;
    o_rFrameState.m_LabeledRecons.m_LabeledRecons = rpDynamicState->m_LabeledRecons.m_LabeledRecons;
    o_rFrameState.m_LabeledReconRayAssignments.m_ReconRayAssignments = rpDynamicState->m_LabeledRayAssignments.m_ReconRayAssignments;
    o_rFrameState.m_Voltages = rpDynamicState->m_VoltageFrames;
    o_rFrameState.m_Forces = rpDynamicState->m_ForceFrames;
    o_rFrameState.m_Moments = rpDynamicState->m_MomentFrames;
    o_rFrameState.m_CentresOfPressure = rpDynamicState->m_CentreOfPressureFrames;
    o_rFrameState.m_GlobalSegments = rpDynamicState->m_GlobalSegments;
    o_rFrameState.m_LocalSegments = rpDynamicState->m_LocalSegments;
    o_rFrameState.m_LightweightSegments = rpDynamicState->m_LightweightSegments;
    o_rFrameState.m_CameraWand2d = rpDynamicState->m_CameraWand2d;
    o_rFrameState.m_CameraWand3d = rpDynamicState->m_CameraWand3d;
    o_rFrameState.m_EyeTracks = rpDynamicState->m_EyeTrackerFrames;
  }

  o_rFrameState.m_VideoFrames.clear();
  for( unsigned int Index = 0; rpDynamicState && Index != rpDynamicState->m_VideoFrames.size(); ++Index )
  {
    o_rFrameState.m_VideoFrames.push_back( rpDynamicState->m_VideoFrames[ Index ] );
  }
}

bool VCGClient::WaitFrames( std::vector< ICGFrameState > & o_rFrames, unsigned int i_TimeoutMs )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  boost::xtime WaitDeadline;

  // boost::TIME_UTC has been renamed to boost::TIME_UTC_ in Boost 1.50:
  // https://svn.boost.org/trac/boost/ticket/6940
#if BOOST_VERSION >= 105000
  boost::xtime_get( &WaitDeadline, boost::TIME_UTC_ );
#else
  boost::xtime_get( &WaitDeadline, boost::TIME_UTC );
#endif
  boost::xtime::xtime_sec_t  AdditionalSeconds     = static_cast< boost::xtime::xtime_sec_t  >( static_cast< double >( i_TimeoutMs ) / 1000.0 );
  boost::xtime::xtime_nsec_t AdditionalNanoSeconds = static_cast< boost::xtime::xtime_nsec_t >( ( i_TimeoutMs - ( 1000 * AdditionalSeconds ) ) * 1000000 );
 
  WaitDeadline.sec += AdditionalSeconds;
  WaitDeadline.nsec += AdditionalNanoSeconds;

  while( m_FrameDeque.empty() )
  {
    if( !m_NewFramesCondition.timed_wait( Lock, WaitDeadline ) )
    {
      return false;
    }
  }
  
  return PollFrames( o_rFrames );
}

bool VCGClient::WaitFrame( ICGFrameState& o_rFrame, unsigned int i_TimeoutMs )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  boost::xtime WaitDeadline;

  // boost::TIME_UTC has been renamed to boost::TIME_UTC_ in Boost 1.50:
  // https://svn.boost.org/trac/boost/ticket/6940
#if BOOST_VERSION >= 105000
  boost::xtime_get( &WaitDeadline, boost::TIME_UTC_ );
#else
  boost::xtime_get( &WaitDeadline, boost::TIME_UTC );
#endif
  boost::xtime::xtime_sec_t  AdditionalSeconds     = static_cast< boost::xtime::xtime_sec_t  >( static_cast< double >( i_TimeoutMs ) / 1000.0 );
  boost::xtime::xtime_nsec_t AdditionalNanoSeconds = static_cast< boost::xtime::xtime_nsec_t >( ( i_TimeoutMs - ( 1000 * AdditionalSeconds ) ) * 1000000 );
 
  WaitDeadline.sec += AdditionalSeconds;
  WaitDeadline.nsec += AdditionalNanoSeconds;

  while( m_FrameDeque.empty() )
  {
    if( !m_NewFramesCondition.timed_wait( Lock, WaitDeadline ) )
    {
      return false;
    }
  }
  
  return PollFrame( o_rFrame );
}

void VCGClient::Connect( std::string i_IPAddress, unsigned short i_Port )
{
  Connect({{i_IPAddress, i_Port}});
}

void VCGClient::Connect( const std::vector< std::pair< std::string, unsigned short > >& i_rHosts )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  for( const auto & rHost : i_rHosts)
  {
    std::shared_ptr< VCGClientCallback > pCallback(new VCGClientCallback(*this, m_pCallbacks.size()) );
    std::shared_ptr< VViconCGStreamClient > pClient( new VViconCGStreamClient( pCallback ) );

    pClient->Connect( rHost.first, rHost.second );

    pClient->SetRequiredObjects(m_RequestedObjects.m_Enums);

    m_pCallbacks.push_back( pCallback );
    m_pClients.push_back( pClient );
    m_LastFrameIDs.push_back(-1);
  }
}

void VCGClient::ReceiveMulticastData( std::string i_MulticastIPAddress, std::string i_LocalIPAddress, unsigned short i_Port )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  for (auto pClient : m_pClients)
  {
    pClient->ReceiveMulticastData(i_MulticastIPAddress, i_LocalIPAddress, i_Port);
  }

  m_bMulticastReceiving = !m_pClients.empty();
}

void VCGClient::StopReceivingMulticastData()
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  for (auto pClient : m_pClients)
  {
    pClient->StopReceivingMulticastData();
  }

  m_bMulticastReceiving = false;
}

bool VCGClient::SetRequestTypes( ViconCGStreamType::Enum i_RequestedType, bool i_bEnable )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  if( i_bEnable )
  {
    for (auto pClient : m_pClients)
    {
      if (!pClient->ObjectIsSupported(i_RequestedType))
      {
        return false;
      }
    }

    m_RequestedObjects.m_Enums.insert( i_RequestedType );
  }
  else
  {
    std::set< ViconCGStreamType::Enum >::iterator It = m_RequestedObjects.m_Enums.find( i_RequestedType );
    if( It != m_RequestedObjects.m_Enums.end() )
    {
      m_RequestedObjects.m_Enums.erase( It );
    }
  }

  for (auto pClient : m_pClients)
  {
    pClient->SetRequiredObjects(m_RequestedObjects.m_Enums);
  }

  return true;
}

void VCGClient::SetBufferSize( unsigned int i_MaxFrames )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  m_MaxBufferSize = i_MaxFrames;
  while( m_FrameDeque.size() > i_MaxFrames )
  {
    m_FrameDeque.pop_front();
  }
}

void VCGClient::SetDecodeVideo( bool i_bDecode )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  for (auto pClient : m_pClients)
  {
    pClient->SetVideoHint(i_bDecode ? VViconCGStreamClient::EDecode : VViconCGStreamClient::EPassThrough);
  }
}

void VCGClient::SetStreamMode( bool i_bStream )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  for (auto pClient : m_pClients)
  {
    pClient->SetStreaming(i_bStream);
  }
}

void VCGClient::SetFilter( const ViconCGStream::VFilter & i_rFilter )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  for (auto pClient : m_pClients)
  {
    pClient->SetFilter(i_rFilter);
  }
}

void VCGClient::SendPing()
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  for (auto pClient : m_pClients)
  {
    pClient->SendPing();
  }
}

void VCGClient::RequestFrame()
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  for (auto pClient : m_pClients)
  {
    pClient->RequestFrame();
  }
}

void VCGClient::RequestNextFrame()
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  for (auto pClient : m_pClients)
  {
    pClient->RequestNextFrame();
  }
}

void VCGClient::ClearBuffer()
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );
  TFrameDeque FrameDeque;
  std::swap( m_FrameDeque, FrameDeque );
}

bool VCGClient::SetLogFile(const std::string& i_rLog)
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  if( m_pClients.empty() )
  {
    return false;
  }

  if (m_pClients.size() == 1)
  {
    return m_pClients.front()->SetTimingLogFile(i_rLog);
  }
  else
  {
    std::string::size_type Pos = i_rLog.rfind('.');
    if (Pos == std::string::npos)
    {
      Pos = i_rLog.size();
    }

    bool bOk( true );
    for (auto pClient : m_pClients)
    {
      std::string Log = i_rLog;
      Log.insert(Pos, pClient->HostName());
      bOk = bOk && pClient->SetTimingLogFile( Log );
    }
    return bOk;
  }
  
}

void VCGClient::SetServerToTransmitMulticast( std::string i_MulticastIPAddress, std::string i_ServerIPAddress, unsigned short i_Port )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  for( auto pClient : m_pClients )
  {
    pClient->SetServerToTransmitMulticast( i_MulticastIPAddress, i_ServerIPAddress, i_Port );
  }

  m_bMulticastController = true;
}

void VCGClient::StopMulticastTransmission()
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  for( auto pClient : m_pClients )
  {
    pClient->StopMulticastTransmission();
  }

  m_bMulticastController = false;
}

bool VCGClient::IsMulticastController() const
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  return m_bMulticastController;
}

bool VCGClient::IsConnected() const
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  // If any route is connected, we report true.
  for( const auto & Connected : m_Connected )
  {
    if( Connected.second )
    {
      return true;
    }
  }

  return false;
}

bool VCGClient::IsMulticastReceiving() const
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  return m_bMulticastReceiving;
}

void VCGClient::OnConnect( size_t i_ClientID )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );
  
  m_Connected[i_ClientID] = true;
}

void VCGClient::OnStaticObjects( std::shared_ptr< const VStaticObjects > i_pStaticObjects )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  m_pLastStaticObjects = i_pStaticObjects;
}

void VCGClient::OnDynamicObjects( std::shared_ptr< const VDynamicObjects > i_pDynamicObjects, size_t i_ClientID)
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );


  const ViconCGStreamType::UInt32 ThisFrame = i_pDynamicObjects->m_FrameInfo.m_FrameID;

  m_LastFrameIDs[i_ClientID] = ThisFrame;

  // If we only have one client we always accept the frame.
  if( m_LastFrameIDs.size() > 1 )
  {
    // Check to see if the fame has already been received.
    const ViconCGStreamType::UInt32 LastFrame = [&]() {
      // The last frame received by connections other than this one.
      // Note that the FrameID can get reset by a system reboot.
      ViconCGStreamType::UInt32 MaxFrame = 0;
      for (unsigned int Index = 0; Index < m_LastFrameIDs.size(); ++Index)
      {
        if (Index == i_ClientID)
        {
          continue;
        }
        MaxFrame = std::max(m_LastFrameIDs[Index], MaxFrame);
      }
      return MaxFrame;
    }();

    if (ThisFrame <= LastFrame)
    {
      return;
    }
  }

  m_FrameDeque.push_back( TFramePair( m_pLastStaticObjects, i_pDynamicObjects ) );
  while( m_FrameDeque.size() > m_MaxBufferSize )
  {
    m_FrameDeque.pop_front();
  }

  m_NewFramesCondition.notify_all();
}

void VCGClient::OnDisconnect( size_t i_ClientID )
{
  boost::recursive_mutex::scoped_lock Lock( m_ClientMutex );

  m_Connected[i_ClientID] = false;
  m_NewFramesCondition.notify_all();
}

bool VCGClient::SetApexDeviceFeedback( unsigned int i_DeviceID, bool i_bOn )
{
  if( i_bOn )
  {
    if( m_HapticDeviceOnList.find( i_DeviceID ) != m_HapticDeviceOnList.end() )
    {
      return false;
    }
    m_HapticDeviceOnList.insert( i_DeviceID );
  }
  else
  {
    std::set< unsigned int >::iterator It = m_HapticDeviceOnList.find( i_DeviceID );
    if( It== m_HapticDeviceOnList.end() )
    {
      return false;
    } 
    m_HapticDeviceOnList.erase( It );
    
  }

  for( auto pClient : m_pClients )
  {
    pClient->SetApexDeviceFeedback( m_HapticDeviceOnList );
  }

  return true;
}


}
