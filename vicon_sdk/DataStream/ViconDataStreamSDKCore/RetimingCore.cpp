
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
#include "RetimingCore.h"
#include "RetimerUtils.h"

#include <ViconCGStreamClient/CGStreamPostalService.h>

#pragma warning( push )
#pragma warning( disable : 4265 )
#include <thread>
#pragma warning( pop )

#include <algorithm>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/range/combine.hpp>
#include <boost/format.hpp>

using namespace ClientUtils;

namespace ViconDataStreamSDK
{
  namespace Core
  {
    static unsigned int s_BufSize = 2;
    static double s_PurgeLimit = 500.0;

    VRetimingCore::VRetimingCore()    
    : m_MaxPredictionTime( 100 )
    , m_bOutputLogHeaderWritten( true )
    {
    }

    VRetimingCore::~VRetimingCore()
    {
      CloseDebugLog();
      CloseOutputLog();
    }

    void VRetimingCore::SetMaximumPrediction( double i_MaxPrediction )
    {
      boost::recursive_mutex::scoped_lock Lock( m_DataMutex );
      m_MaxPredictionTime = i_MaxPrediction;
    }

    double VRetimingCore::MaximumPrediction() const
    {
      boost::recursive_mutex::scoped_lock Lock( m_DataMutex );
      return m_MaxPredictionTime;
    }

    // utility to insert the current date and time into a string
    static std::string TimestampFilename( const std::string& i_rFilename )
    {
      std::string DateTimePlaceholder( "[datetime]" );
      std::string InputFilename = i_rFilename;
      auto index = InputFilename.find( DateTimePlaceholder );
      if( index != std::string::npos )
      {
        std::string DateTimeString = boost::posix_time::to_iso_string( boost::posix_time::second_clock::local_time() );
        InputFilename.erase( index, DateTimePlaceholder.length() );
        InputFilename.insert( index, DateTimeString );
      }
      return InputFilename;
    }

    bool VRetimingCore::SetDebugLogFile( const std::string& i_rLogFile )
    {

      return CreateDebugLog( TimestampFilename( i_rLogFile ) );
    }

    bool VRetimingCore::SetOutputFile( const std::string& i_rLogFile )
    {
      return CreateOutputLog( TimestampFilename( i_rLogFile ) );
    }

    void VRetimingCore::AddData( std::vector< std::shared_ptr< VSubjectPose > > i_pData )
    {
      boost::recursive_mutex::scoped_lock Lock( m_DataMutex );

      for( const auto& rpData : i_pData )
      {

        if( rpData )
        {
          // Print all input data to the output log, even if it's occluded and we're not going to add it to
          // the retimer.
          OutputLog( rpData );
        }

        // DSSDK-210. Don't add occluded data, as we will be able to interpolate from good data
        // as long as gaps are not too long. If an object becomes occluded for a significant time,
        // Predict will return ELate when the data becomes too stale.
        bool bOccluded = false;
        // For multi-segment subjects, don't add if any segments are occluded.
        for( const auto& rSegment : rpData->m_Segments )
        {
          if( rSegment.second->bOccluded )
          {
            bOccluded = true;
            break;
          }
        }

        if( !bOccluded )
        {
          auto& rData = m_Data[ rpData->Name ];
          rData.push_back( rpData );

          while( rData.size() > s_BufSize )
          {
            rData.pop_front();
          }
        }
      }
    }

    std::shared_ptr< const VSubjectPose > VRetimingCore::Predict( std::shared_ptr< const VSubjectPose > p1, std::shared_ptr< const VSubjectPose > p2, double t ) const
    {
      std::shared_ptr< VSubjectPose > pOutput( new VSubjectPose() );

      if( !p1 || !p2 )
      {
        return pOutput;
      }

      // Calculate the input frame number that corresponds to this requested time
      double Sample1Index = p1->ReceiptTime;
      double Sample2Index = p2->ReceiptTime;
      double PredictionIndex = t;
      double MaximumPredictionValue = m_MaxPredictionTime;

      if( Sample2Index < Sample1Index )
      {
        pOutput->Result = VSubjectPose::EInvalid;
        DebugLog( str( boost::format( "Invalid Receipt Time, %s, %d, %d, %d" ) % p1->Name % PredictionIndex % Sample1Index % Sample2Index ) );
        return pOutput;
      }

      if( PredictionIndex < Sample1Index )
      {
        pOutput->Result = VSubjectPose::EEarly;
        const std::string Msg = str(boost::format("Early data requested, %d, %d, %d, %d") % p1->Name % PredictionIndex % Sample1Index % Sample2Index);
        DebugLog( Msg );
        return pOutput;
      }

      if( PredictionIndex > Sample2Index )
      {
        if( PredictionIndex - Sample2Index > MaximumPredictionValue )
        {
          pOutput->Result = VSubjectPose::ELate;
          DebugLog( str( boost::format( "Late data requested, %s, %d, %d, %d, %d" ) % p1->Name % PredictionIndex % Sample1Index % Sample2Index % MaximumPredictionValue ) );
          return pOutput;
        }
      }

      DebugLog(str(boost::format("Prediction to time from samples, %d, %d, %d") % PredictionIndex % ( PredictionIndex - Sample1Index ) % ( PredictionIndex - Sample2Index ) ));

      pOutput->FrameTime = t;
      pOutput->ReceiptTime = t;
      pOutput->Result = VSubjectPose::ESuccess;

      // Copy over input data
      pOutput->RootSegment = p1->RootSegment;
      pOutput->Name = p1->Name;
      for( const auto Latency : p1->Latencies )
      {
        pOutput->Latencies.insert( Latency );
      }
      pOutput->FrameRate = p1->FrameRate;


      double PredictedFrameNumber = ClientUtils::PredictVal( p1->FrameNumber, p1->ReceiptTime, p2->FrameNumber, p2->ReceiptTime, t );
      pOutput->FrameNumber = PredictedFrameNumber;

      for( const auto SegIt : p1->m_Segments )
      {
        // Get corresponding segment from p2->
        for( const auto SegIt2 : p2->m_Segments )
        {
          std::shared_ptr< VSegmentPose > pSegment = SegIt.second;
          std::shared_ptr< VSegmentPose > pSegment2 = SegIt2.second;

          if( pSegment2->Name == pSegment->Name )
          {

            std::shared_ptr< VSegmentPose > pOutputSegment( new VSegmentPose() );
            pOutputSegment->Name = pSegment->Name;
            pOutputSegment->Parent = pSegment->Parent;
            std::copy( pSegment->T_Stat.begin(), pSegment->T_Stat.end(), pOutputSegment->T_Stat.begin() );
            std::copy( pSegment->R_Stat.begin(), pSegment->R_Stat.end(), pOutputSegment->R_Stat.begin() );

            pOutputSegment->m_Children.reserve( pSegment->m_Children.size() );
            std::copy( pSegment->m_Children.begin(), pSegment->m_Children.end(), std::back_inserter( pOutputSegment->m_Children ) );

            pOutputSegment->bOccluded = pSegment->bOccluded || pSegment2->bOccluded;

            pOutputSegment->T = ClientUtils::PredictDisplacement( pSegment->T, Sample1Index, pSegment2->T, Sample2Index, PredictionIndex );
            pOutputSegment->R = ClientUtils::PredictRotation( pSegment->R, Sample1Index, pSegment2->R, Sample2Index, PredictionIndex );
            pOutputSegment->T_Rel = ClientUtils::PredictDisplacement( pSegment->T_Rel, Sample1Index, pSegment2->T_Rel, Sample2Index, PredictionIndex );
            pOutputSegment->R_Rel = ClientUtils::PredictRotation( pSegment->R_Rel, Sample1Index, pSegment2->R_Rel, Sample2Index, PredictionIndex );

            pOutput->m_Segments[ pSegment->Name ] = pOutputSegment;

            break;
          }
        }
      }

      if( pOutput->Result != VSubjectPose::ESuccess )
      {
        DebugLog( str( boost::format( "Failed to update frame for, %s, %d, %d, %d, %s" ) % p1->Name % PredictionIndex % Sample1Index % Sample2Index % VSubjectPose::ResultString( pOutput->Result ) ) );
      }
      return pOutput;
    }

    VSubjectPose::EResult VRetimingCore::UpdateFrameAtTime( double i_rTime )
    {
      boost::recursive_mutex::scoped_lock Lock( m_DataMutex );

      m_LatestOutputPoses.clear();

      VSubjectPose::EResult Result = VSubjectPose::ENoData;

      for( const auto& Pair : m_Data )
      {
        std::shared_ptr< const VSubjectPose > pPose;
        const auto& rData = Pair.second;
        if( rData.size() >= 2 )
        {
          pPose = Predict( rData.front(), rData.back(), i_rTime );

          // Return success if prediction was successful for any of the subjects.
          // The individual poses will contain the result for that specific subject
          if( pPose->Result == VSubjectPose::ESuccess )
          {
            Result = pPose->Result;
          }
        }

        m_LatestOutputPoses[ Pair.first ] = pPose;
      }

      return Result;
    }

    VSubjectPose::EResult VRetimingCore::GetSubjectCount( unsigned int& o_rSubjectCount ) const
    {
      boost::recursive_mutex::scoped_lock Lock( m_DataMutex );

      o_rSubjectCount = 0;

      if( m_Data.size() != 0 )
      {
        o_rSubjectCount = static_cast< unsigned int >( m_LatestOutputPoses.size() );
        return VSubjectPose::ESuccess;
      }
      else
      {
        return VSubjectPose::ENoData;
      }
    }

    VSubjectPose::EResult VRetimingCore::GetSubjectName( const unsigned int i_SubjectIndex, std::string& o_rSubjectName ) const
    {
      boost::recursive_mutex::scoped_lock Lock( m_DataMutex );

      if( i_SubjectIndex >= m_LatestOutputPoses.size() )
      {
        return VSubjectPose::EUnknownSubject;
      }

      auto rPair = m_LatestOutputPoses.begin();
      std::advance( rPair, i_SubjectIndex );
      o_rSubjectName = rPair->first;
      return VSubjectPose::ESuccess;
    }

    VSubjectPose::EResult VRetimingCore::GetSubject( const std::string& i_rSubjectName, std::shared_ptr< const VSubjectPose >& o_rpSubject ) const
    {
      boost::recursive_mutex::scoped_lock Lock( m_DataMutex );

      VSubjectPose::EResult OutputResult = VSubjectPose::EUnknownSubject;

      auto DataIt = m_LatestOutputPoses.find( i_rSubjectName );
      if( DataIt != m_LatestOutputPoses.end() )
      {
        o_rpSubject = DataIt->second;

        if( o_rpSubject )
        {
          OutputResult = o_rpSubject->Result;
        }
        else
        {
          OutputResult = VSubjectPose::ENoData;
        }
      }

      return OutputResult;
    }

    VSubjectPose::EResult VRetimingCore::GetSubjectStatic( const std::string& i_rSubjectName, std::shared_ptr< const VSubjectPose >& o_rpSubject ) const
    {
      boost::recursive_mutex::scoped_lock Lock( m_DataMutex );

      VSubjectPose::EResult OutputResult = VSubjectPose::EUnknownSubject;

      auto DataIt = m_Data.find( i_rSubjectName );
      if( DataIt != m_Data.end() )
      {
        if( !DataIt->second.empty() )
        {
          // Use the most recent input data.
          o_rpSubject = DataIt->second.back();
        }

        if( o_rpSubject )
        {
          OutputResult = o_rpSubject->Result;
        }
        else
        {
          OutputResult = VSubjectPose::ENoData;
        }
      }

      return OutputResult;
    }

    bool VSubjectPose::operator==( const VSubjectPose& i_rOther )
    {
      bool bMatches = true;

      bMatches = ( i_rOther.Result == Result );
      bMatches = bMatches && ( i_rOther.Name == Name );
      bMatches = bMatches && ( i_rOther.RootSegment == RootSegment );

      bMatches = bMatches && ( i_rOther.Latencies == Latencies );
      bMatches = bMatches && ( i_rOther.FrameTime == FrameTime );
      bMatches = bMatches && ( i_rOther.FrameNumber == FrameNumber );
      bMatches = bMatches && ( i_rOther.ReceiptTime == ReceiptTime );
      bMatches = bMatches && ( i_rOther.FrameRate == FrameRate );

      if( i_rOther.m_Segments.size() == m_Segments.size() )
      {
        auto SegmentIt = m_Segments.begin();
        auto OtherIt = i_rOther.m_Segments.begin();
        for (; SegmentIt != m_Segments.end() && OtherIt != i_rOther.m_Segments.end(); ++SegmentIt, ++OtherIt)
        {
          bMatches = bMatches && ( SegmentIt->first == OtherIt->first );
          bMatches = bMatches && ( *SegmentIt->second == *OtherIt->second );
        }
      }
      else
      {
        bMatches = false;
      }

      return bMatches;
    }

    std::string VSubjectPose::OutputHeader( const std::shared_ptr< VSubjectPose >& i_rPose )
    {
      std::string HeaderString = "FrameNumber, Result, Frame Rate, ReceiptTime, #Latencies, ";
      for( const auto Latency : i_rPose->Latencies )
      {
        HeaderString += Latency.first + ", ";
      }
      HeaderString += "Name, RootSegment, #Segments, "
                      "Segment Name, Segment Parent, Segment Occluded, T[0], T[1], T[2], R[0], R[1], R[2], R[3], "
                      "T_Rel[0], T_Rel[1], T_Rel[2], R_Rel[0], R_Rel[1], R_Rel[2], R_Rel[3], T_Stat[0], T_Stat[1], "
                      "T_Stat[2], R_Stat[0], R_Stat[1], R_Stat[2], R_Stat[3],#Children,Children...";

      return HeaderString;
    }

    std::ostream& operator<<( std::ostream& o_rStream, const VSubjectPose& i_Pose )
    {
      o_rStream << std::fixed << i_Pose.FrameNumber << ", "
                << i_Pose.Result << ", "
                << i_Pose.FrameRate << ", "
                << i_Pose.ReceiptTime << ", "
                << i_Pose.Latencies.size() << ", ";

      for( const auto LatencySample : i_Pose.Latencies )
      {
        o_rStream << LatencySample.second << ", ";
      }

      o_rStream << i_Pose.Name << ", "
                << i_Pose.RootSegment << ", "
                << i_Pose.m_Segments.size();

      for( const auto rSegment : i_Pose.m_Segments )
      {
        o_rStream << *rSegment.second;
      }
      return o_rStream;
    }

    bool VSegmentPose::operator==( const VSegmentPose& i_rOther )
    {
      bool bMatches = true;

      bMatches = bMatches && ( i_rOther.Name == Name );
      bMatches = bMatches && ( i_rOther.Parent == Parent );

      bMatches = bMatches && ( i_rOther.T == T );
      bMatches = bMatches && ( i_rOther.R == R );

      bMatches = bMatches && ( i_rOther.T_Rel == T_Rel );
      bMatches = bMatches && ( i_rOther.R_Rel == R_Rel );

      bMatches = bMatches && ( i_rOther.T_Stat == T_Stat );
      bMatches = bMatches && ( i_rOther.R_Stat == R_Stat );

      bMatches = bMatches && ( i_rOther.m_Children == m_Children );

      bMatches = bMatches && ( i_rOther.bOccluded == bOccluded );

      return bMatches;
    }

    std::ostream& operator<<( std::ostream& o_rStream, const VSegmentPose& i_Pose )
    {
      o_rStream << ", " << i_Pose.Name << ", " << i_Pose.Parent << ", " << i_Pose.bOccluded << ", "
                << i_Pose.T[ 0 ] << ", " << i_Pose.T[ 1 ] << ", " << i_Pose.T[ 2 ] << ", "
                << i_Pose.R[ 0 ] << ", " << i_Pose.R[ 1 ] << ", " << i_Pose.R[ 2 ] << ", " << i_Pose.R[ 3 ] << ", "
                << i_Pose.T_Rel[ 0 ] << ", " << i_Pose.T_Rel[ 1 ] << ", " << i_Pose.T_Rel[ 2 ] << ", "
                << i_Pose.R_Rel[ 0 ] << ", " << i_Pose.R_Rel[ 1 ] << ", " << i_Pose.R_Rel[ 2 ] << ", " << i_Pose.R_Rel[ 3 ] << ", "
                << i_Pose.T_Stat[ 0 ] << ", " << i_Pose.T_Stat[ 1 ] << ", " << i_Pose.T_Stat[ 2 ] << ", "
                << i_Pose.R_Stat[ 0 ] << ", " << i_Pose.R_Stat[ 1 ] << ", " << i_Pose.R_Stat[ 2 ] << ", " << i_Pose.R_Stat[ 3 ] << ", "
                << i_Pose.m_Children.size();
      for( const auto& rChild : i_Pose.m_Children )
      {
        o_rStream << ", " << rChild;
      }
      return o_rStream;
    }

    void VRetimingCore::OutputLogFunction( const std::shared_ptr< VSubjectPose > i_pSubjectPose ) const
    {
      boost::mutex::scoped_lock LogLock( m_OutputLogMutex );
      if( m_OutputLog.good() )
      {
        if( !m_bOutputLogHeaderWritten )
        {
          m_OutputLog << VSubjectPose::OutputHeader( i_pSubjectPose ) << std::endl;
          m_bOutputLogHeaderWritten = true;
        }

        if( i_pSubjectPose )
        {
          m_OutputLog << std::fixed << *i_pSubjectPose << std::endl;
        }
      }
    }

    void VRetimingCore::OutputLog( const std::shared_ptr< VSubjectPose > i_pSubjectPose ) const
    {
      if( m_pPostalService )
      {
        m_pPostalService->Post( std::bind( &VRetimingCore::OutputLogFunction, this, i_pSubjectPose ) );
      }

    }

    bool VRetimingCore::CreateOutputLog( const std::string& i_rFilename )
    {
      boost::mutex::scoped_lock LogLock( m_OutputLogMutex );

      bool bSuccess = false;
      m_bOutputLogHeaderWritten = false;

      if( !m_OutputLog.is_open() )
      {
        m_OutputLog.open( i_rFilename );
        bSuccess = m_OutputLog.good();
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

    void VRetimingCore::CloseOutputLog()
    {
      boost::mutex::scoped_lock LogLock( m_OutputLogMutex );

      if( m_OutputLog.is_open() )
      {
        m_OutputLog.close();
      }
    }

    void VRetimingCore::DebugLogFunction( const std::string & i_rLogMessage ) const
    {
      boost::mutex::scoped_lock LogLock( m_DebugLogMutex );
      if( m_DebugLog.good() )
      {
        m_DebugLog << i_rLogMessage << std::endl;
      }
    }

    void VRetimingCore::DebugLog( const std::string & i_rLogMessage ) const
    {
      if( m_pPostalService )
      {
        m_pPostalService->Post( std::bind( &VRetimingCore::DebugLogFunction, this, i_rLogMessage ) );
      }
    }

    bool VRetimingCore::CreateDebugLog( const std::string& i_rFilename )
    {
      boost::mutex::scoped_lock LogLock( m_DebugLogMutex );

      bool bSuccess = false;

      if( !m_DebugLog.is_open() )
      {
        m_DebugLog.open( i_rFilename );
        bSuccess = m_DebugLog.good();
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

    void VRetimingCore::CloseDebugLog()
    {
      boost::mutex::scoped_lock LogLock( m_DebugLogMutex );

      if( m_DebugLog.is_open() )
      {
        m_DebugLog.close();
      }
    }

    void VRetimingCore::PurgeOldData( double i_TimeNow )
    {
      boost::recursive_mutex::scoped_lock Lock( m_DataMutex );
      std::vector< std::string > Subjects;
      for( const auto& rSubject : m_Data )
      {
        Subjects.emplace_back(rSubject.first);
      }

      for( auto& rSubject : Subjects )
      {
        auto& rPoses = m_Data[ rSubject ];
        auto PoseIt = rPoses.begin();
        while(  PoseIt != rPoses.end() )
        {
          auto pPose = *PoseIt;
          if( !pPose || pPose->ReceiptTime > i_TimeNow || ( i_TimeNow - pPose->ReceiptTime ) > s_PurgeLimit )
          {
            PoseIt = rPoses.erase(PoseIt);
          }
          else
          {
            ++PoseIt;
          }
        }
      }
    }
  } // namespace Core
} // namespace ViconDataStreamSDK
