
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

/// \file
/// Contains the declaration of the ViconCGStream::VGreyscaleBlobs class.

#include "GreyscaleBlobs.h"
#include "GreyscaleBlobsDetail.h"
#include <vector>

namespace ViconCGStream
{
//-------------------------------------------------------------------------------------------------

/// Writes additional sensor pixel stride information for a frame of greyscale blobs to the CG stream.
class VGreyscaleSubsampledBlobs: public VGreyscaleBlobs
{
public:

  /// Object type enum.
  virtual ViconCGStreamType::Enum TypeID() const
  {
    return ViconCGStreamEnum::GreyscaleSubsampledBlobs;
  }
  
  
  /// Read function.
  virtual bool Read( const ViconCGStreamIO::VBuffer & i_rBuffer )
  {
    return i_rBuffer.Read(m_FrameID) &&
           i_rBuffer.Read(m_CameraID) &&
           i_rBuffer.Read( m_TwiceOffsetX ) &&
           i_rBuffer.Read( m_TwiceOffsetY ) &&
           i_rBuffer.Read( m_SensorPixelsPerImagePixelX ) && 
           i_rBuffer.Read( m_SensorPixelsPerImagePixelY ) &&
           i_rBuffer.Read(m_GreyscaleBlobs);
  }
  
  /// Write function.
  virtual void Write( ViconCGStreamIO::VBuffer & i_rBuffer ) const
  {
    i_rBuffer.Write(m_FrameID);
    i_rBuffer.Write(m_CameraID);
    i_rBuffer.Write(m_TwiceOffsetX);
    i_rBuffer.Write( m_TwiceOffsetY );
    i_rBuffer.Write( m_SensorPixelsPerImagePixelX );
    i_rBuffer.Write( m_SensorPixelsPerImagePixelY );
    i_rBuffer.Write(m_GreyscaleBlobs);
  }

};

//-------------------------------------------------------------------------------------------------
};

