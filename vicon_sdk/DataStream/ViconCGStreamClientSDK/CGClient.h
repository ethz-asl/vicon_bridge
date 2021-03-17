
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
#pragma once

#include "ICGClient.h"
#include "ICGFrameState.h"
#include <ViconCGStreamClient/IViconCGStreamClientCallback.h>
#include <ViconCGStreamClient/ViconCGStreamClient.h>
#include <boost/thread/condition.hpp>
#include <deque>

namespace ViconCGStreamClientSDK
{

class ICGFrameState;
class VCGClientCallback;

class VCGClient : public ICGClient
{
public:

  // Constructor
  VCGClient();
  virtual ~VCGClient();

  // ICGClient
  virtual void Destroy() override;
  virtual void Connect( std::string i_IPAddress, unsigned short i_Port ) override;
  virtual void Connect( const std::vector< std::pair< std::string, unsigned short > > & i_rHosts ) override;
  virtual void ReceiveMulticastData( std::string i_MulticastIPAddress, std::string i_LocalIPAddress, unsigned short i_Port ) override;
  virtual void StopReceivingMulticastData( ) override;

  virtual bool IsConnected() const override;
  virtual bool IsMulticastReceiving() const override;

  virtual bool SetRequestTypes( ViconCGStreamType::Enum i_RequestedType, bool i_bEnable = true) override;
  virtual void SetBufferSize( unsigned int i_MaxFrames ) override;
  virtual void SetDecodeVideo( bool i_bDecode ) override;
  virtual void SetStreamMode( bool i_bStream ) override;
  virtual void SetServerToTransmitMulticast( std::string i_MulticastIPAddress, std::string i_ServerIPAddress, unsigned short i_Port ) override;
  virtual void StopMulticastTransmission() override;
  virtual bool IsMulticastController() const override;

  virtual bool PollFrames( std::vector< ICGFrameState > & o_rFrames ) override;
  virtual bool PollFrame( ICGFrameState& o_rFrame ) override;

  virtual bool WaitFrames( std::vector< ICGFrameState > & o_rFrames, unsigned int i_TimeoutMs ) override;
  virtual bool WaitFrame( ICGFrameState& o_rFrame, unsigned int i_TimeoutMs ) override;

  // maintains a list of devices with haptic feedback on. 
  // if on add to the list,
  // if off delete from the list if existed in the list.
  virtual bool SetApexDeviceFeedback( unsigned int i_DeviceID, bool i_bOn ) override;

  /// Allows filtering of items in a group, e.g. only get centroids for a given camera id, etc.
  virtual void SetFilter( const ViconCGStream::VFilter & i_rFilter ) override;

  virtual void SendPing() override;
  virtual bool SetLogFile( const std::string& i_rLog ) override;

  virtual void RequestFrame() override;
  virtual void RequestNextFrame() override;

  virtual void ClearBuffer() override;

  // mirrors IViconCGStreamClientCallback
  virtual void OnConnect( size_t i_ClientID );
  virtual void OnStaticObjects(  std::shared_ptr< const VStaticObjects >  i_pStaticObjects );
  virtual void OnDynamicObjects( std::shared_ptr< const VDynamicObjects > i_pDynamicObjects, size_t i_ClientID );
  virtual void OnDisconnect( size_t i_ClientID );

protected:

  // Buffer
  typedef std::pair< std::shared_ptr< const VStaticObjects >, std::shared_ptr< const VDynamicObjects > > TFramePair;
  typedef std::deque< TFramePair > TFrameDeque;

  void ReadFramePair( const TFramePair& i_rPair, ICGFrameState& o_rFrameState );

  // The C++ client which does all of the work for us
  std::vector< std::shared_ptr< VViconCGStreamClient > > m_pClients;
  std::vector< std::shared_ptr< VCGClientCallback > >    m_pCallbacks;
  std::vector< ViconCGStreamType::UInt32 >               m_LastFrameIDs;
  std::map< size_t, bool >                               m_Connected;
  bool                                      m_bMulticastReceiving;
  bool                                      m_bMulticastController;
  std::set< unsigned int >                  m_HapticDeviceOnList;

  // Configuration
  mutable boost::recursive_mutex      m_ClientMutex;
          ViconCGStream::VObjectEnums m_RequestedObjects;

  std::shared_ptr< const VStaticObjects >   m_pLastStaticObjects;
  TFrameDeque                               m_FrameDeque;
  unsigned int                              m_MaxBufferSize;

  boost::condition                          m_NewFramesCondition; 
};

} // End of namespace ViconCGStreamClientSDK
