
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
#include <array>
#include <boost/thread/condition.hpp>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <deque>
#include <fstream>
#include <chrono>

#include "RetimingCore.h"

#include <ViconDataStreamSDKCoreUtils/Constants.h>
#include <ViconDataStreamSDKCoreUtils/ClientUtils.h>

namespace ViconCGStreamClientSDK
{
  class ICGClient;
}

namespace ViconDataStreamSDK
{

  namespace Core
  {
    class VClient;

    class VRetimingClient
    {
    public:

      VRetimingClient( std::shared_ptr<VClient> i_rClient );
      ~VRetimingClient();

      // Connect client to the Vicon Data Stream
      Result::Enum Connect(std::shared_ptr< ViconCGStreamClientSDK::ICGClient > i_pClient,
        const std::string & i_rHostName,
        bool i_bLightweight = false );

      // Disconnect from the Vicon Data Stream
      Result::Enum Disconnect();

      // Interface
      Result::Enum StartOutput( double i_FrameRate );
      Result::Enum StopOutput();
      bool IsRunning() const;

      // Store a predicted pose for all subjects currently in the stream at the current time.
      Result::Enum UpdateFrame(double i_Offset);

      Result::Enum GetSubjectCount(unsigned int & o_rSubjectCount) const;
      Result::Enum GetSubjectName(const unsigned int i_SubjectIndex, std::string& o_rSubjectName) const;
      Result::Enum GetSubjectRootSegmentName(const std::string & i_rSubjectName, std::string & o_rSegmentName) const;

      Result::Enum GetSegmentCount(const std::string& i_rSubjectName, unsigned int& o_rSegmentCount) const;
      Result::Enum GetSegmentName(const std::string& i_rSubjectName, const unsigned int i_SegmentIndex, std::string& o_rSegmentName) const;
      Result::Enum GetSegmentChildCount(const std::string& i_rSubjectName, const std::string& i_rSegmentName, unsigned int & o_rSegmentCount) const;
      Result::Enum GetSegmentChildName(const std::string& i_rSubjectName, const std::string& i_rSegmentName, unsigned int i_SegmentIndex, std::string& o_rSegmentName) const;
      Result::Enum GetSegmentParentName(const std::string& i_rSubjectName, const std::string& i_rSegmentName, std::string& o_rSegmentName) const;

      Result::Enum GetSegmentStaticTranslation(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3]) const;
      Result::Enum GetSegmentStaticRotationHelical(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3]) const;
      Result::Enum GetSegmentStaticRotationMatrix(const std::string & i_rSubjectName, const std::string & i_rSegmentName, double(&o_rRotation)[9]) const;
      Result::Enum GetSegmentStaticRotationQuaternion(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rFourVector)[4]) const;
      Result::Enum GetSegmentStaticRotationEulerXYZ(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3]) const;
      Result::Enum GetSegmentStaticScale ( const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3] ) const;

      Result::Enum GetSegmentGlobalTranslation(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3], bool& o_rbOccluded) const;
      Result::Enum GetSegmentGlobalRotationHelical(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3], bool& o_rbOccluded) const;
      Result::Enum GetSegmentGlobalRotationMatrix(const std::string & i_rSubjectName, const std::string & i_rSegmentName, double(&o_rRotation)[9], bool & o_rbOccluded) const;
      Result::Enum GetSegmentGlobalRotationQuaternion(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rFourVector)[4], bool& o_rbOccluded) const;
      Result::Enum GetSegmentGlobalRotationEulerXYZ(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3], bool& o_rbOccluded) const;

      Result::Enum GetSegmentLocalTranslation(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_pThreeVector)[3], bool& o_rbOccluded) const;
      Result::Enum GetSegmentLocalRotationHelical(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3], bool& o_rbOccluded) const;
      Result::Enum GetSegmentLocalRotationMatrix(const std::string & i_rSubjectName, const std::string & i_rSegmentName, double(&o_rRotation)[9], bool & o_rbOccluded) const;
      Result::Enum GetSegmentLocalRotationQuaternion(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rFourVector)[4], bool& o_rbOccluded) const;
      Result::Enum GetSegmentLocalRotationEulerXYZ(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3], bool& o_rbOccluded) const;

      // Wait for a frame, populates pose of all objects in o_rSubjects mapped to name
      Result::Enum WaitForFrame() const;

      // Set the latency required in the output stream; may be used to avoid the need to predict data forwards, 
      // where a small amount of latency is acceptable.
      void SetOutputLatency( double i_OutputLatency );

      // Return the output latency specified
      double OutputLatency() const;

      // Set the maximum amount by which the interpolation engine will predict forward
      void SetMaximumPrediction( double i_MaxPrediction );

      // Return the maximum prediction used by the system
      double MaximumPrediction() const;

      // Set a log file to write debug output about performance to
      bool SetDebugLogFile(const std::string & i_rLogFile);

      // Set a file to write input data to, allowing offline processing of the subequent file
      bool SetOutputFile(const std::string & i_rLogFile);

    private:

      void InputThread();
      void StopInput();

      typedef std::chrono::high_resolution_clock hrc;
      hrc::time_point m_Epoch;

      bool InitGet(Result::Enum & o_rResult) const;
      template < typename T > bool InitGet(Result::Enum & o_rResult, T & o_rOutput) const;
      template < typename T1, typename T2 > bool InitGet(Result::Enum & o_rResult, T1 & o_rOutput1, T2 & o_rOutput2) const;
      template < typename T1, typename T2, typename T3 > bool InitGet(Result::Enum & o_rResult, T1 & o_rOutput1, T2 & o_rOutput2, T3 & o_rOutput3) const;

    private:


      std::shared_ptr<VClient>  m_pClient;
      
      VRetimingCore m_Retimer;

      void OutputThread();


      mutable boost::recursive_mutex m_DataMutex;

      boost::recursive_mutex m_FrameRateMutex;
      double m_FrameRate;

      mutable boost::mutex     m_OutputMutex;
      mutable boost::condition m_OutputWait;

      std::unique_ptr< boost::thread > m_pInputThread;
      bool m_bInputStopped;

      std::unique_ptr< boost::thread > m_pOutputThread;
      bool m_bOutputStopped;

      // Required output latency (in milliseconds)
      double m_OutputLatency;

    };

    template < typename T >
    bool ViconDataStreamSDK::Core::VRetimingClient::InitGet(Result::Enum & o_rResult, T & o_rOutput) const
    {
      ClientUtils::Clear(o_rOutput);
      return InitGet(o_rResult);
    }

    template < typename T1, typename T2 >
    bool ViconDataStreamSDK::Core::VRetimingClient::InitGet(Result::Enum & o_rResult, T1 & o_rOutput1, T2 & o_rOutput2) const
    {
      ClientUtils::Clear(o_rOutput1);
      ClientUtils::Clear(o_rOutput2);
      return InitGet(o_rResult);
    }

    template < typename T1, typename T2, typename T3 >
    bool ViconDataStreamSDK::Core::VRetimingClient::InitGet(Result::Enum & o_rResult, T1 & o_rOutput1, T2 & o_rOutput2, T3 & o_rOutput3) const
    {
      ClientUtils::Clear(o_rOutput1);
      ClientUtils::Clear(o_rOutput2);
      ClientUtils::Clear(o_rOutput3);
      return InitGet(o_rResult);
    }
  }
}
