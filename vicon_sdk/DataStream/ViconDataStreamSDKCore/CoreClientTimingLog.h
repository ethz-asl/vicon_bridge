
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

#include "RetimingClient.h"

#include <ViconDataStreamSDKCoreUtils/Constants.h>
#include <ViconDataStreamSDKCoreUtils/AxisMapping.h>
#include <ViconDataStreamSDKCoreUtils/ClientUtils.h>


#include <memory>
#include <array>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <ViconCGStreamClientSDK/ICGClient.h>
#include <ViconCGStreamClientSDK/CGClient.h>
#include <ViconCGStreamClientSDK/ICGFrameState.h>

class VCGStreamPostalService;


namespace ViconDataStreamSDK
{
namespace Core
{



class VClientTimingLog
{

public:
  enum EHeadingComparison
  {
    ESame,     /// The heading items are the same
    ESubset,   /// Headings2 is completely contained within headings1
    ESuperset, /// Headings1 is completely contained within headings2
    EDifferent /// The items in the 2 heading vectors are different, or in a different order
  };

public:

  VClientTimingLog();
  virtual ~VClientTimingLog();

  bool CreateLog( const std::string& i_rFilename );
  void WriteToLog( const unsigned int i_FrameNumber, const std::vector< ViconCGStreamDetail::VLatencyInfo_Sample >& i_rLatencies );
  void CloseLog();

  EHeadingComparison CompareHeadings( const std::vector< std::string >& i_rHeadings1, const std::vector< std::string >& i_rHeadings2 ) const;
  std::vector< std::string > LogHeadings( const std::vector< ViconCGStreamDetail::VLatencyInfo_Sample >& i_rSamples ) const;
  void LogLatencies( std::ostream& i_rStream, const std::vector< std::string >& i_rHeadings, const std::vector< ViconCGStreamDetail::VLatencyInfo_Sample >& i_rLatencies ) const;

protected:

  void LogTimingFunction( const unsigned int i_FrameNumber, const double i_ReceiptTime, const std::vector< ViconCGStreamDetail::VLatencyInfo_Sample >& i_rLatencies );
  EHeadingComparison CompareHeadings( const std::vector< std::string >& i_rHeadings1, const std::vector< std::string >& i_rHeadings2, const EHeadingComparison i_SubsetType ) const;

private:

  std::shared_ptr< VCGStreamPostalService > m_pPostalService;
  boost::mutex m_LogMutex;
  std::ofstream m_Log;
  
  struct LogCacheItem
  {
    unsigned int m_FrameNumber;
    double m_ReceiptTime;
    std::vector< ViconCGStreamDetail::VLatencyInfo_Sample > m_Latencies;
  };

  std::deque< LogCacheItem > m_PreLoggedItems;
  std::vector< std::string > m_LogHeadings;
};



} // End of namespace Core
} // End of namespace ViconDataStreamSDK
