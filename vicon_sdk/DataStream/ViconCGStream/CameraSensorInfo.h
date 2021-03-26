
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

#include "Item.h"
#include <string>

namespace ViconCGStream
{
//-------------------------------------------------------------------------------------------------

/// Contains information about the camera sensor mode 
class VCameraSensorInfo : public VItem
{
public:

  /// Camera identifier
  ViconCGStreamType::UInt32 m_CameraID;

  /// Camera sensor mode: Normal, Binned, Subsampled
  std::string m_SensorMode;

  /// Window left
  ViconCGStreamType::UInt32 m_WindowOffsetX;

  /// Window top
  ViconCGStreamType::UInt32 m_WindowOffsetY;

  /// Window width
  ViconCGStreamType::UInt32 m_WindowWidth;

  /// Window height
  ViconCGStreamType::UInt32 m_WindowHeight;

  // Add some padding in case we need to add anything in future
  ViconCGStreamType::UInt32 m_Future1;
  ViconCGStreamType::UInt32 m_Future2;
  ViconCGStreamType::UInt32 m_Future3;
  ViconCGStreamType::UInt32 m_Future4;

  /// Equality function
  bool IsEqual( const VCameraSensorInfo & i_rOther ) const
  {
    return m_CameraID == i_rOther.m_CameraID &&
           m_SensorMode == i_rOther.m_SensorMode &&
           m_WindowOffsetX == i_rOther.m_WindowOffsetX &&
           m_WindowOffsetY == i_rOther.m_WindowOffsetY &&
           m_WindowWidth == i_rOther.m_WindowWidth &&
           m_WindowHeight == i_rOther.m_WindowHeight ;
  }

  /// Equality operator
  bool operator == ( const VCameraSensorInfo & i_rOther ) const
  {
    return IsEqual( i_rOther );
  }

  /// Object type enum.
  virtual ViconCGStreamType::Enum TypeID() const
  {
    return ViconCGStreamEnum::CameraSensorInfo;
  }

  /// Filter ID
  virtual ViconCGStreamType::UInt32 FilterID() const
  {
    return m_CameraID;
  }

  /// Read function.  
  virtual bool Read( const ViconCGStreamIO::VBuffer & i_rBuffer )
  {
    return i_rBuffer.Read( m_CameraID ) &&
           i_rBuffer.Read( m_SensorMode ) &&
           i_rBuffer.Read( m_WindowOffsetX ) &&
           i_rBuffer.Read( m_WindowOffsetY ) &&
           i_rBuffer.Read( m_WindowWidth ) &&
           i_rBuffer.Read( m_WindowHeight ) &&
           i_rBuffer.Read( m_Future1 ) &&
           i_rBuffer.Read( m_Future2 ) &&
           i_rBuffer.Read( m_Future3 ) &&
           i_rBuffer.Read( m_Future4 ) ;
  }

  /// Write function.  
  virtual void Write( ViconCGStreamIO::VBuffer & i_rBuffer ) const
  {
    i_rBuffer.Write( m_CameraID );
    i_rBuffer.Write( m_SensorMode );
    i_rBuffer.Write( m_WindowOffsetX );
    i_rBuffer.Write( m_WindowOffsetY );
    i_rBuffer.Write( m_WindowWidth );
    i_rBuffer.Write(m_WindowHeight);
    i_rBuffer.Write( m_Future1 );
    i_rBuffer.Write( m_Future2 );
    i_rBuffer.Write( m_Future3 );
    i_rBuffer.Write( m_Future4 );
  }

};

//-------------------------------------------------------------------------------------------------
};

