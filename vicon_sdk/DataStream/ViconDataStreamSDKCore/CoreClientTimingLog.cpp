
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
#include "CoreClientTimingLog.h"

#include <functional>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <ViconCGStreamClient/CGStreamPostalService.h>

namespace
{
  // Number of frames to wait for before starting to log, in the hope of getting all of the
  // columns in.
  static const size_t s_LogPreamble = 200;
}

namespace ViconDataStreamSDK
{
namespace Core
{


VClientTimingLog::VClientTimingLog()
{
}

VClientTimingLog::~VClientTimingLog()
{
  CloseLog();
}


std::vector< std::string > VClientTimingLog::LogHeadings(const std::vector< ViconCGStreamDetail::VLatencyInfo_Sample > & i_rSamples ) const
{
  std::vector< std::string > Headings;
  for (const auto& rLatency : i_rSamples)
  {
    Headings.push_back( rLatency.m_Name );
  }
  return Headings;
}

VClientTimingLog::EHeadingComparison VClientTimingLog::CompareHeadings( const std::vector< std::string >& i_rHeadings1, const std::vector< std::string >& i_rHeadings2, const VClientTimingLog::EHeadingComparison i_SubsetType ) const
 {
  if (i_rHeadings1 == i_rHeadings2)
  {
    return ESame;
  }

  size_t Found = 0;
  for( size_t Index = 0; Index < i_rHeadings1.size(); ++Index )
  {
    if (Found < i_rHeadings2.size())
    {
      if (i_rHeadings1[Index] == i_rHeadings2[Found])
      {
        ++Found;
      }
    }
  }

  return ( Found == i_rHeadings2.size() ) ? i_SubsetType : EDifferent;
}

VClientTimingLog::EHeadingComparison VClientTimingLog::CompareHeadings( const std::vector< std::string >& i_rHeadings1, const std::vector< std::string >& i_rHeadings2 ) const
{

  if( i_rHeadings1.size() >= i_rHeadings2.size() )
  {
    return CompareHeadings( i_rHeadings1, i_rHeadings2, ESubset );
  }
  else
  {
    return CompareHeadings( i_rHeadings2, i_rHeadings1, ESuperset );
  }
}

void VClientTimingLog::LogLatencies(std::ostream & i_rStream, const std::vector< std::string > & i_rHeadings, const std::vector< ViconCGStreamDetail::VLatencyInfo_Sample >& i_rLatencies) const
{
  EHeadingComparison Result = CompareHeadings( i_rHeadings, LogHeadings( i_rLatencies ) );
  if( Result <= ESubset )
  { 
    unsigned int LatencyIndex = 0;
    for( const auto& Heading : i_rHeadings)
    {
      if(LatencyIndex < i_rLatencies.size() )
      {
        if( Heading == i_rLatencies[ LatencyIndex ].m_Name )
        { 
          i_rStream << ", " << i_rLatencies[ LatencyIndex ].m_Latency;
          ++LatencyIndex;
        }
        else
        {
          i_rStream << ", ";
        }
      }
      else
      {
        i_rStream << ", ";
      }
    }
  }
  // We need a DSSDK error log
  //else
  //{
  //  i_rStream << ", Headings are " << ( Result == ESuperset ) ? "Superset, " : "Different, ";
  //  for (const auto& rHeading : LogHeadings(i_rLatencies))
  //  {
  //    i_rStream << rHeading << ", ";
  //  }
  //}
}

void VClientTimingLog::LogTimingFunction( const unsigned int i_FrameNumber, const double i_ReceiptTime, const std::vector< ViconCGStreamDetail::VLatencyInfo_Sample >& i_rLatencies )
{
  boost::mutex::scoped_lock LogLock( m_LogMutex );
  if( m_Log.good() )
  {
    // We haven't written a header yet
    if( m_LogHeadings.empty() )
    {
      // Store up a buffer of items before we start to log, in order to
      // get an idea of all of the column items.
      if( m_PreLoggedItems.size() < s_LogPreamble )
      {
        LogCacheItem Item;
        Item.m_FrameNumber = i_FrameNumber;
        Item.m_ReceiptTime = i_ReceiptTime;
        Item.m_Latencies = i_rLatencies;
        m_PreLoggedItems.push_back( Item );
      }
      else
      {
        // We have filled our buffer; now work out what the set of headings is
        for (const auto& rLatency : m_PreLoggedItems)
        {
          const auto Headings = LogHeadings( rLatency.m_Latencies );
          const auto Result = CompareHeadings( Headings, m_LogHeadings );
        
          if (Result == ESubset)
          {
            m_LogHeadings = Headings;
          }
        }

        m_Log << "Frame Number, Receipt Time";
        for( const auto& Heading : m_LogHeadings)
        {
          m_Log << ", " << Heading;
        }
        m_Log << std::endl;

        // Now write out the rest of our buffer
        for (const auto& rLatency : m_PreLoggedItems)
        {
          m_Log << std::fixed << rLatency.m_FrameNumber << ", " << rLatency.m_ReceiptTime;
          LogLatencies( m_Log, m_LogHeadings, rLatency.m_Latencies );
          m_Log << std::endl;
        }

        m_PreLoggedItems.clear();
      }
    }
    else
    { 
      m_Log << std::fixed << i_FrameNumber << ", " << i_ReceiptTime;
      LogLatencies( m_Log, m_LogHeadings, i_rLatencies );
      m_Log << std::endl;
    }
  }
}

void VClientTimingLog::WriteToLog( const unsigned int i_FrameNumber, const std::vector< ViconCGStreamDetail::VLatencyInfo_Sample >& i_rLatencies )
{
  if( m_pPostalService )
  {
    const double ReceiptTime = std::chrono::duration< double, std::milli >( std::chrono::high_resolution_clock::now().time_since_epoch() ).count();
    m_pPostalService->Post( std::bind( &VClientTimingLog::LogTimingFunction, this, i_FrameNumber, ReceiptTime, i_rLatencies ) );
  }
}

bool VClientTimingLog::CreateLog( const std::string& i_rFilename )
{
  boost::mutex::scoped_lock LogLock( m_LogMutex );

  bool bSuccess = false;

  if( !m_Log.is_open() )
  {
    m_Log.open( i_rFilename );
    bSuccess = m_Log.good();
  }

  if( bSuccess )
  {
    if( !m_pPostalService )
    {
      m_pPostalService = std::make_shared< VCGStreamPostalService >();
    }

    bSuccess = m_pPostalService->StartService();
  }

  return bSuccess;
}

void VClientTimingLog::CloseLog()
{
  boost::mutex::scoped_lock LogLock( m_LogMutex );

  if( m_pPostalService )
  {
    m_pPostalService->StopService();
  }

  if( m_Log.is_open() )
  {
    m_Log.close();
  }
}

} // End of namespace Core
} // End of namespace ViconDataStreamSDK
