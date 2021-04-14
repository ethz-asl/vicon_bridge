
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

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

#include <array>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <deque>
#include <fstream>
#include <set>
#include <numeric>

#include <ViconDataStreamSDKCoreUtils/Constants.h>

class VCGStreamPostalService;

namespace ViconCGStreamClientSDK
{
  class ICGClient;
}

namespace ViconDataStreamSDK
{

  namespace Core
  {


    class VSegmentPose
    {
    public:

      static const unsigned int NumElements = 25;

      VSegmentPose() 
        : Scale({ 1.0, 1.0, 1.0 } )
        , bOccluded(false) {}

      std::string Name;
      std::string Parent;

      std::array< double, 3 > T;
      std::array< double, 4 > R;

      std::array< double, 3 > T_Rel;
      std::array< double, 4 > R_Rel;

      std::array< double, 3 > T_Stat;
      std::array< double, 4 > R_Stat;

      bool bHasScale;
      std::array< double, 3 > Scale;

      std::vector< std::string > m_Children;

      bool   bOccluded;

      bool operator==(const VSegmentPose & i_rOther);

      friend std::ostream & operator<<(std::ostream & o_rStream, const VSegmentPose & i_Pose);
    };

    class VSubjectPose
    {
    public:

      static const unsigned int NumElements = 8;

      enum EResult
      {
        ESuccess,
        ENoData,
        ENotConnected,
        EUnknownSubject,
        EEarly,
        ELate,
        EInvalid
      };

      static std::string ResultString( const EResult& i_rResult )
      {
        switch( i_rResult )
        {
        case ESuccess:
          return "Success";
        case ENoData:
          return "No Data";
        case ENotConnected:
          return "Not Connected";
        case EUnknownSubject:
          return "Unknown Subject";
        case EEarly:
          return "Early";
        case ELate:
          return "Late";
        case EInvalid:
          return "Invalid";
        default:
          return "Unknown Error";
        }
      }

      VSubjectPose()
        : Result(EInvalid)
        , FrameTime(0)
        , FrameNumber(0)
        , ReceiptTime(0)
        , FrameRate( 0 )
      {}

      static std::string OutputHeader( const std::shared_ptr< VSubjectPose > & i_rPose );

      EResult Result;

      std::string Name;
      std::string RootSegment;

      std::map< std::string, double > Latencies;
      double TotalLatency() const
      {
        double Total = 0;
        for( const auto & rLatency : Latencies )
        {
          Total += rLatency.second;
        }
        return Total;
        //return std::accumulate(Latencies.begin(), Latencies.end(), 0, [] ( double value, const std::map< std::string, double >::value_type & p ) { return value + p.second; } );
      }

      double       FrameTime;
      double       FrameNumber;
      double       ReceiptTime;
      double       FrameRate;

      bool operator==(const VSubjectPose & i_rOther);

      friend std::ostream & operator<<(std::ostream & o_rStream, const VSubjectPose & i_Pose);

      // Segment data
      std::map< std::string, std::shared_ptr< VSegmentPose > > m_Segments;

      // Store the segment names in an ordered container, to allow us to present the segment indexing in a consistent way to the non-retimed client
      std::vector< std::string > m_SegmentNames;
    };
    
    class VRetimingCore
    {
    public:

      VRetimingCore();
      ~VRetimingCore();

      // Set the maximum amount by which the interpolation engine will predict forward
      void SetMaximumPrediction( double i_MaxPrediction );

      // Return the maximum prediction used by the system
      double MaximumPrediction() const;

      // Set a log file to write debug output about performance to
      bool SetDebugLogFile(const std::string & i_rLogFile);

      // Write our data to an output file, to allow for offline running
      bool SetOutputFile(const std::string & i_rLogFile);

      void AddData( std::vector< std::shared_ptr< VSubjectPose > > i_pData );

      // Store a predicted pose for all subjects at the specified time
      VSubjectPose::EResult UpdateFrameAtTime(double i_Time);

      VSubjectPose::EResult GetSubjectCount(unsigned int & o_rSubjectCount) const;

      VSubjectPose::EResult GetSubjectName(const unsigned int i_SubjectIndex, std::string& o_rSubjectName) const;

      VSubjectPose::EResult GetSubject(const std::string & i_rSubjectName, std::shared_ptr< const VSubjectPose > & o_rpSubject) const;

      VSubjectPose::EResult GetSubjectStatic(const std::string & i_rSubjectName, std::shared_ptr< const VSubjectPose > & o_rpSubject) const;

    public:

      // This needs to be public, or make the unit test a friend
      std::shared_ptr< const VSubjectPose > Predict(std::shared_ptr< const VSubjectPose > p1, std::shared_ptr< const VSubjectPose > p2, double t) const;

    private:

      void PurgeOldData( double i_TimeNow );

      mutable boost::recursive_mutex m_DataMutex;
      std::map< std::string, std::deque< std::shared_ptr< const VSubjectPose > > > m_Data;
      std::map< std::string, std::shared_ptr< const VSubjectPose > > m_LatestOutputPoses;


      // Maximum time we should predict forwards (in milliseconds)
      double m_MaxPredictionTime;

      // Postal service for logging
      std::shared_ptr< VCGStreamPostalService > m_pPostalService;

      // Output logs
      void OutputLogFunction( const std::shared_ptr< VSubjectPose > i_pSubjectPose ) const;
      void OutputLog( const std::shared_ptr< VSubjectPose > i_pPose ) const;
      bool CreateOutputLog( const std::string& i_rFilename );
      void CloseOutputLog();
      mutable boost::mutex m_OutputLogMutex;
      mutable std::ofstream m_OutputLog;
      mutable bool m_bOutputLogHeaderWritten;


      void DebugLogFunction( const std::string & i_rLogMessage ) const;
      void DebugLog( const std::string & i_rLogMessage ) const;
      bool CreateDebugLog( const std::string& i_rFilename );
      void CloseDebugLog();
      mutable boost::mutex m_DebugLogMutex;
      mutable std::ofstream m_DebugLog;
    };


  }
}
