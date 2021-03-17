
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
#include "RetimingClient.h"
#include "CoreClient.h"
#include "RetimerUtils.h"

#ifdef WIN32
#pragma warning ( push )
#pragma warning ( disable : 4265 )
#endif

#include <thread>

#ifdef WIN32
#pragma warning ( pop )
#endif

#include <algorithm>
#include <iostream>

using namespace ClientUtils;

namespace ViconDataStreamSDK
{

  namespace Core
  {

    
    Result::Enum Adapt(const VSubjectPose::EResult & i_rResult)
    {
      switch( i_rResult )
      {
      case VSubjectPose::ESuccess: return Result::Success;
      case VSubjectPose::ENoData: return Result::NoFrame;
      case VSubjectPose::ENotConnected: return Result::NotConnected;
      case VSubjectPose::EUnknownSubject: return Result::InvalidSubjectName;
      case VSubjectPose::EEarly: return Result::EarlyDataRequested;
      case VSubjectPose::ELate: return Result::LateDataRequested;
      case VSubjectPose::EInvalid: return Result::InvalidOperation;
      };
      return Result::Unknown;
    }

    VRetimingClient::VRetimingClient(std::shared_ptr<VClient> i_rClient)
      : m_pClient(i_rClient)
      , m_bInputStopped(false)
      , m_bOutputStopped(false)
      , m_OutputLatency(0.0)
    {
      // Store a relative start time for our timestamps
      m_Epoch = hrc::now();
    }

    VRetimingClient::~VRetimingClient()
    {
      Disconnect();
    }

    // Connect client to the Vicon Data Stream
    Result::Enum VRetimingClient::Connect(
      std::shared_ptr< ViconCGStreamClientSDK::ICGClient > i_pClient,
      const std::string & i_rHostName,
      bool i_bLightweight)
    {

      // We can afford to buffer a little, as it doesn't matter if frames come in at once.
      m_pClient->SetBufferSize(10);

      Result::Enum Result = m_pClient->Connect(i_pClient, i_rHostName);

      if( Result == Result::Success )
      {
        // Ensure that segments are enabled
        if( i_bLightweight )
        {
          // Use lightweight mode
          m_pClient->EnableLightweightSegmentData();
        }
        else
        {
          m_pClient->EnableSegmentData();
        }

        // We need to work in server push mode
        m_pClient->SetStreamMode(StreamMode::ServerPush);

        // Start frame acquisition thread
        m_pInputThread.reset(new boost::thread(std::bind(&VRetimingClient::InputThread, this)));
      }

      return Result;
    }

    // Disconnect from the Vicon Data Stream
    Result::Enum VRetimingClient::Disconnect()
    {
      // Stop output thread
      StopOutput();

      // Stop input thread
      StopInput();

      return m_pClient->Disconnect();
    }


    Result::Enum VRetimingClient::StartOutput(double i_FrameRate)
    {
      // Can't start if our input thread isn't running
      if( !m_pInputThread )
      {
        return Result::NotConnected;
      }

      StopOutput();

      // Run for a bit first
      unsigned int FrameCount = 0;
      unsigned int RunInPeriod = 20;
      while( FrameCount < RunInPeriod )
      {
        m_pClient->GetFrame();
        ++FrameCount;
      }


      double FrameRate = 0;

      // Set output rate to input rate if none specified
      if( i_FrameRate >= 0 )
      {
        FrameRate = i_FrameRate;
      }
      else
      {
        m_pClient->GetFrameRate(FrameRate);
      }


      boost::recursive_mutex::scoped_lock Lock(m_FrameRateMutex);
      m_FrameRate = FrameRate;

      // start thread
      m_pOutputThread.reset(new boost::thread(std::bind(&VRetimingClient::OutputThread, this)));

      return Result::Success;

    }

    Result::Enum VRetimingClient::StopOutput()
    {
      // stop thread
      if( m_pOutputThread )
      {
        m_bOutputStopped = true;
        m_pOutputThread->join();
        m_pOutputThread.reset();
      }

      m_OutputWait.notify_all();

      return Result::Success;
    }

    void VRetimingClient::StopInput()
    {
      m_bInputStopped = true;

      // Wait for it to stop
      if( m_pInputThread )
      {
        m_pInputThread->join();
        m_pInputThread.reset();
      }
    }

    bool VRetimingClient::IsRunning() const
    {
      return m_pOutputThread != nullptr;
    }


    bool VRetimingClient::InitGet(Result::Enum & o_rResult) const
    {
      o_rResult = Result::Success;

      unsigned int SubjectCount = 0;
      if( !m_pClient->IsConnected() )
      {
        o_rResult = Result::NotConnected;
      }
      else if( m_Retimer.GetSubjectCount(SubjectCount) == VSubjectPose::ENoData )
      {
        o_rResult = Result::NoFrame;
      }

      return ( o_rResult == Result::Success );
    }

    Result::Enum VRetimingClient::GetSubjectCount(unsigned int & o_rSubjectCount) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);
      return Adapt(m_Retimer.GetSubjectCount(o_rSubjectCount));
    }

    Result::Enum VRetimingClient::GetSubjectName(const unsigned int i_SubjectIndex, std::string& o_rSubjectName) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);
      return Adapt(m_Retimer.GetSubjectName(i_SubjectIndex, o_rSubjectName));
    }

    Result::Enum VRetimingClient::GetSubjectRootSegmentName(const std::string & i_rSubjectName, std::string & o_rSegmentName) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);
      Clear(o_rSegmentName);

      std::shared_ptr< const VSubjectPose > pSubject;
      Result::Enum Result = Adapt(m_Retimer.GetSubjectStatic(i_rSubjectName, pSubject));

      if( Result == Result::Success )
      {
        o_rSegmentName = pSubject->RootSegment;
      }

      return Result;
    }

    Result::Enum VRetimingClient::GetSegmentCount(const std::string& i_rSubjectName, unsigned int& o_rSegmentCount) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);
      Clear(o_rSegmentCount);

      std::shared_ptr< const VSubjectPose > pSubject;
      Result::Enum Result = Adapt(m_Retimer.GetSubjectStatic(i_rSubjectName, pSubject));

      if( Result == Result::Success )
      {
        o_rSegmentCount = static_cast< unsigned int >( pSubject->m_SegmentNames.size() );
      }

      return Result;
    }

    Result::Enum VRetimingClient::GetSegmentName(const std::string& i_rSubjectName, const unsigned int i_SegmentIndex, std::string& o_rSegmentName) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);
      Clear(o_rSegmentName);

      std::shared_ptr< const VSubjectPose > pSubject;
      Result::Enum Result = Adapt(m_Retimer.GetSubjectStatic(i_rSubjectName, pSubject));

      if( Result == Result::Success )
      {
        if( i_SegmentIndex < pSubject->m_SegmentNames.size() )
        {
          o_rSegmentName = pSubject->m_SegmentNames[ i_SegmentIndex ];
        }
        else
        {
          Result = Result::InvalidIndex;
        }
      }

      return Result;
    }

    Result::Enum VRetimingClient::GetSegmentChildCount(const std::string& i_rSubjectName, const std::string& i_rSegmentName, unsigned int & o_rSegmentCount) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);
      Clear(o_rSegmentCount);

      std::shared_ptr< const VSubjectPose > pSubject;
      Result::Enum Result = Adapt(m_Retimer.GetSubjectStatic(i_rSubjectName, pSubject));

      if( Result == Result::Success )
      {
        const auto pSegment = pSubject->m_Segments.find(i_rSegmentName);
        if( pSegment != pSubject->m_Segments.end() && pSegment->second )
        {
          o_rSegmentCount = static_cast< unsigned int >( pSegment->second->m_Children.size() );
        }
        else
        {
          Result = Result::InvalidSegmentName;
        }
      }

      return Result;
    }

    Result::Enum VRetimingClient::GetSegmentChildName(const std::string& i_rSubjectName, const std::string& i_rSegmentName, unsigned int i_SegmentIndex, std::string& o_rSegmentName) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);
      Clear(o_rSegmentName);

      std::shared_ptr< const VSubjectPose > pSubject;
      Result::Enum Result = Adapt(m_Retimer.GetSubjectStatic(i_rSubjectName, pSubject));

      if( Result == Result::Success )
      {
        const auto pSegment = pSubject->m_Segments.find(i_rSegmentName);
        if( pSegment != pSubject->m_Segments.end() && pSegment->second )
        {
          if( i_SegmentIndex < pSegment->second->m_Children.size() )
          {
            o_rSegmentName = pSegment->second->m_Children[i_SegmentIndex];
          }
          else
          {
            Result = Result::InvalidIndex;
          }
        }
        else
        {
          Result = Result::InvalidSegmentName;
        }
      }

      return Result;
    }

    Result::Enum VRetimingClient::GetSegmentParentName(const std::string& i_rSubjectName, const std::string& i_rSegmentName, std::string& o_rSegmentName) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);
      Clear(o_rSegmentName);

      std::shared_ptr< const VSubjectPose > pSubject;
      Result::Enum Result = Adapt(m_Retimer.GetSubjectStatic(i_rSubjectName, pSubject));

      if( Result == Result::Success )
      {
        const auto pSegment = pSubject->m_Segments.find(i_rSegmentName);
        if( pSegment != pSubject->m_Segments.end() && pSegment->second )
        {
          o_rSegmentName = pSegment->second->Parent;
        }
        else
        {
          Result = Result::InvalidSegmentName;
        }
      }

      return Result;
    }

    Result::Enum VRetimingClient::GetSegmentStaticTranslation(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3]) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Result::Enum GetResult = Result::Success;
      if( !InitGet(GetResult, o_rThreeVector) )
      {
        return GetResult;
      }

      std::shared_ptr< const VSubjectPose > pSubject;
      GetResult = Adapt(m_Retimer.GetSubjectStatic(i_rSubjectName, pSubject));

      if( GetResult == Result::Success )
      {
        // Get the segment
        auto SegIt = pSubject->m_Segments.find(i_rSegmentName);
        if( SegIt != pSubject->m_Segments.end() )
        {
          std::copy(SegIt->second->T_Stat.begin(), SegIt->second->T_Stat.end(), o_rThreeVector);
        }
      }
      return GetResult;
    }

    Result::Enum VRetimingClient::GetSegmentStaticRotationHelical(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3]) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Clear(o_rThreeVector);
      // InitGet is called by GetSegmentStaticRotationMatrix.

      // Get the answer as a rotation matrix
      double RotationArray[9];
      const Result::Enum _Result = GetSegmentStaticRotationMatrix(i_rSubjectName, i_rSegmentName, RotationArray);

      if( Result::Success == _Result )
      {
        MatrixToHelical(RotationArray, o_rThreeVector);
      }

      return _Result;
    }

    Result::Enum VRetimingClient::GetSegmentStaticRotationMatrix(const std::string & i_rSubjectName, const std::string & i_rSegmentName, double(&o_rRotation)[9]) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Result::Enum GetResult = Result::Success;
      if( !InitGet(GetResult, o_rRotation) )
      {
        return GetResult;
      }

      std::shared_ptr< const VSubjectPose > pSubject;
      GetResult = Adapt(m_Retimer.GetSubjectStatic(i_rSubjectName, pSubject));

      if( GetResult == Result::Success )
      {
        // Get the segment
        auto SegIt = pSubject->m_Segments.find(i_rSegmentName);
        if( SegIt != pSubject->m_Segments.end() )
        {
          std::array< double, 9 > StaticRotation = ClientUtils::ToRotationMatrix(SegIt->second->R_Stat);
          std::copy(StaticRotation.begin(), StaticRotation.end(), o_rRotation);
        }
      }
      return GetResult;
    }

    Result::Enum VRetimingClient::GetSegmentStaticRotationQuaternion(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rFourVector)[4]) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Result::Enum GetResult = Result::Success;
      if( !InitGet(GetResult, o_rFourVector) )
      {
        return GetResult;
      }

      std::shared_ptr< const VSubjectPose > pSubject;
      GetResult = Adapt(m_Retimer.GetSubjectStatic(i_rSubjectName, pSubject));

      if( GetResult == Result::Success )
      {
        // Get the segment
        auto SegIt = pSubject->m_Segments.find(i_rSegmentName);
        if( SegIt != pSubject->m_Segments.end() )
        {
          std::copy(SegIt->second->R_Stat.begin(), SegIt->second->R_Stat.end(), o_rFourVector);
        }
      }

      return GetResult;
    }

    Result::Enum VRetimingClient::GetSegmentStaticRotationEulerXYZ(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3]) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Clear(o_rThreeVector);
      // InitGet is called by GetSegmentStaticRotationMatrix.

      // Get the answer as a rotation matrix
      double RotationArray[9];
      const Result::Enum _Result = GetSegmentStaticRotationMatrix(i_rSubjectName, i_rSegmentName, RotationArray);

      if( Result::Success == _Result )
      {
        MatrixToEulerXYZ(RotationArray, o_rThreeVector);
      }

      return _Result;
    }

    Result::Enum VRetimingClient::GetSegmentStaticScale(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3]) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Result::Enum GetResult = Result::Success;
      if( !InitGet(GetResult, o_rThreeVector) )
      {
        return GetResult;
      }

      std::shared_ptr< const VSubjectPose > pSubject;
      GetResult = Adapt( m_Retimer.GetSubjectStatic(i_rSubjectName, pSubject) );

      if( GetResult == Result::Success )
      {
        // Get the segment
        auto SegIt = pSubject->m_Segments.find(i_rSegmentName);
        if( SegIt != pSubject->m_Segments.end() )
        {
          if( SegIt->second->bHasScale )
          { 
            std::array< double, 3 > StaticScale = SegIt->second->Scale;
            std::copy(StaticScale.begin(), StaticScale .end(), o_rThreeVector);
          }
          else
          {
            GetResult = Result::NotPresent;
          }
        }
      }
      return GetResult;
    }

    Result::Enum VRetimingClient::GetSegmentGlobalTranslation(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3], bool& o_rbOccluded) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Result::Enum GetResult = Result::Success;
      if( !InitGet(GetResult, o_rThreeVector, o_rbOccluded) )
      {
        return GetResult;
      }

      std::shared_ptr< const VSubjectPose > pSubject;
      GetResult = Adapt(m_Retimer.GetSubject(i_rSubjectName, pSubject));

      if( GetResult == Result::Success )
      {
        // Get the segment
        auto SegIt = pSubject->m_Segments.find(i_rSegmentName);
        if( SegIt != pSubject->m_Segments.end() )
        {
          std::copy(SegIt->second->T.begin(), SegIt->second->T.end(), o_rThreeVector);
          o_rbOccluded = SegIt->second->bOccluded;
        }
      }
      return GetResult;
    }

    Result::Enum VRetimingClient::GetSegmentGlobalRotationHelical(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3], bool& o_rbOccluded) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Clear(o_rThreeVector);
      Clear(o_rbOccluded);
      // InitGet is called by GetSegmentGlobalRotationMatrix.

      // Get the answer as a rotation matrix
      double RotationArray[9];
      const Result::Enum _Result = GetSegmentGlobalRotationMatrix(i_rSubjectName, i_rSegmentName, RotationArray, o_rbOccluded);

      if( Result::Success == _Result && !o_rbOccluded )
      {
        MatrixToHelical(RotationArray, o_rThreeVector);
      }

      return _Result;
    }

    Result::Enum VRetimingClient::GetSegmentGlobalRotationMatrix(const std::string & i_rSubjectName, const std::string & i_rSegmentName, double(&o_rRotation)[9], bool & o_rbOccluded) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Result::Enum GetResult = Result::Success;
      if( !InitGet(GetResult, o_rRotation, o_rbOccluded) )
      {
        return GetResult;
      }

      std::shared_ptr< const VSubjectPose > pSubject;
      GetResult = Adapt(m_Retimer.GetSubject(i_rSubjectName, pSubject));

      if( GetResult == Result::Success )
      {
        // Get the segment
        auto SegIt = pSubject->m_Segments.find(i_rSegmentName);
        if( SegIt != pSubject->m_Segments.end() )
        {
          std::array< double, 9 > GlobalRotation = ClientUtils::ToRotationMatrix(SegIt->second->R);
          std::copy(GlobalRotation.begin(), GlobalRotation.end(), o_rRotation);
          o_rbOccluded = SegIt->second->bOccluded;
        }
      }
      return GetResult;
    }

    Result::Enum VRetimingClient::GetSegmentGlobalRotationQuaternion(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rFourVector)[4], bool& o_rbOccluded) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Result::Enum GetResult = Result::Success;
      if( !InitGet(GetResult, o_rFourVector, o_rbOccluded) )
      {
        return GetResult;
      }

      std::shared_ptr< const VSubjectPose > pSubject;
      GetResult = Adapt(m_Retimer.GetSubject(i_rSubjectName, pSubject));

      if( GetResult == Result::Success )
      {
        // Get the segment
        auto SegIt = pSubject->m_Segments.find(i_rSegmentName);
        if( SegIt != pSubject->m_Segments.end() )
        {
          std::copy(SegIt->second->R.begin(), SegIt->second->R.end(), o_rFourVector);
          o_rbOccluded = SegIt->second->bOccluded;
        }
      }
      return GetResult;
    }

    Result::Enum VRetimingClient::GetSegmentGlobalRotationEulerXYZ(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3], bool& o_rbOccluded) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Clear(o_rThreeVector);
      Clear(o_rbOccluded);
      // InitGet is called by GetSegmentGlobalRotationMatrix.

      // Get the answer as a rotation matrix
      double RotationArray[9];
      const Result::Enum _Result = GetSegmentGlobalRotationMatrix(i_rSubjectName, i_rSegmentName, RotationArray, o_rbOccluded);

      if( Result::Success == _Result && !o_rbOccluded )
      {
        MatrixToEulerXYZ(RotationArray, o_rThreeVector);
      }

      return _Result;
    }

    Result::Enum VRetimingClient::GetSegmentLocalTranslation(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3], bool& o_rbOccluded) const
    {
      Result::Enum GetResult = Result::Success;
      if( !InitGet(GetResult, o_rThreeVector, o_rbOccluded) )
      {
        return GetResult;
      }

      std::shared_ptr< const VSubjectPose > pSubject;
      GetResult = Adapt(m_Retimer.GetSubject(i_rSubjectName, pSubject));

      if( GetResult == Result::Success )
      {
        // Get the segment
        auto SegIt = pSubject->m_Segments.find(i_rSegmentName);
        if( SegIt != pSubject->m_Segments.end() )
        {
          std::copy(SegIt->second->T_Rel.begin(), SegIt->second->T_Rel.end(), o_rThreeVector);
          o_rbOccluded = SegIt->second->bOccluded;
        }
      }
      return GetResult;
    }

    Result::Enum VRetimingClient::GetSegmentLocalRotationHelical(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3], bool& o_rbOccluded) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Clear(o_rThreeVector);
      Clear(o_rbOccluded);

      // Get the answer as a rotation matrix
      double RotationArray[9];
      const Result::Enum _Result = GetSegmentLocalRotationMatrix(i_rSubjectName, i_rSegmentName, RotationArray, o_rbOccluded);

      if( Result::Success == _Result && !o_rbOccluded )
      {
        MatrixToHelical(RotationArray, o_rThreeVector);
      }

      return _Result;
    }

    Result::Enum VRetimingClient::GetSegmentLocalRotationMatrix(const std::string & i_rSubjectName, const std::string & i_rSegmentName, double(&o_rRotation)[9], bool & o_rbOccluded) const
    {
      Result::Enum GetResult = Result::Success;
      if( !InitGet(GetResult, o_rRotation, o_rbOccluded) )
      {
        return GetResult;
      }

      std::shared_ptr< const VSubjectPose > pSubject;
      GetResult = Adapt(m_Retimer.GetSubject(i_rSubjectName, pSubject));

      if( GetResult == Result::Success )
      {
        // Get the segment
        auto SegIt = pSubject->m_Segments.find(i_rSegmentName);
        if( SegIt != pSubject->m_Segments.end() )
        {
          std::array< double, 9 > LocalRotation = ClientUtils::ToRotationMatrix(SegIt->second->R_Rel);
          std::copy(LocalRotation.begin(), LocalRotation.end(), o_rRotation);
          o_rbOccluded = SegIt->second->bOccluded;
        }
      }
      return GetResult;
    }

    Result::Enum VRetimingClient::GetSegmentLocalRotationQuaternion(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rFourVector)[4], bool& o_rbOccluded) const
    {
      Result::Enum GetResult = Result::Success;
      if( !InitGet(GetResult, o_rFourVector, o_rbOccluded) )
      {
        return GetResult;
      }

      std::shared_ptr< const VSubjectPose > pSubject;
      GetResult = Adapt(m_Retimer.GetSubject(i_rSubjectName, pSubject));

      if( GetResult == Result::Success )
      {
        // Get the segment
        auto SegIt = pSubject->m_Segments.find(i_rSegmentName);
        if( SegIt != pSubject->m_Segments.end() )
        {
          std::copy(SegIt->second->R_Rel.begin(), SegIt->second->R_Rel.end(), o_rFourVector);
          o_rbOccluded = SegIt->second->bOccluded;
        }
      }
      return GetResult;
    }

    Result::Enum VRetimingClient::GetSegmentLocalRotationEulerXYZ(const std::string& i_rSubjectName, const std::string& i_rSegmentName, double(&o_rThreeVector)[3], bool& o_rbOccluded) const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

      Clear(o_rThreeVector);
      Clear(o_rbOccluded);

      // Get the answer as a rotation matrix
      double RotationArray[9];
      const Result::Enum _Result = GetSegmentLocalRotationMatrix(i_rSubjectName, i_rSegmentName, RotationArray, o_rbOccluded);

      if( Result::Success == _Result && !o_rbOccluded )
      {
        MatrixToEulerXYZ(RotationArray, o_rThreeVector);
      }

      return _Result;
    }



    Result::Enum VRetimingClient::WaitForFrame() const
    {
      boost::mutex::scoped_lock Lock(m_OutputMutex);

      if( !IsRunning() )
      {
        return Result::NoFrame;
      }

      m_OutputWait.wait(Lock);
      return Result::Success;
    }

    void VRetimingClient::SetOutputLatency(double i_OutputLatency)
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);
      m_OutputLatency = i_OutputLatency;
    }

    double VRetimingClient::OutputLatency() const
    {
      boost::recursive_mutex::scoped_lock Lock(m_DataMutex);
      return m_OutputLatency;
    }

    void VRetimingClient::SetMaximumPrediction(double i_MaxPrediction)
    {
      m_Retimer.SetMaximumPrediction(i_MaxPrediction);
    }

    double VRetimingClient::MaximumPrediction() const
    {
      return m_Retimer.MaximumPrediction();
    }

    bool VRetimingClient::SetDebugLogFile(const std::string & i_rLogFile)
    {
      return m_Retimer.SetDebugLogFile(i_rLogFile);
    }

    bool VRetimingClient::SetOutputFile(const std::string & i_rLogFile)
    {
      return m_Retimer.SetOutputFile(i_rLogFile);
    }

    Result::Enum VRetimingClient::UpdateFrame(double i_rOffset)
    {
      if( IsRunning() )
      {
        return Result::InvalidOperation;
      }

      const double Now = std::chrono::duration< double, std::milli >(hrc::now() - m_Epoch).count();
      double PTime = Now + i_rOffset;
      return Adapt(m_Retimer.UpdateFrameAtTime( PTime ));
    }

    void VRetimingClient::InputThread()
    {
      while( !m_bInputStopped )
      {
        if( m_pClient->IsConnected() )
        {

          // Get a frame
          while( m_pClient->GetFrame() != Result::Success && m_pClient->IsConnected() )
          {
            // Sleep a little so that we don't lumber the CPU with a busy poll
#ifdef WIN32
            Sleep(200);
#else
            sleep(1);
#endif
          }

          // Get the system latencies as individual components for debugging
          std::map< std::string, double > Latencies;
          unsigned int LatencySampleCount;
          if( m_pClient->GetLatencySampleCount(LatencySampleCount) == Result::Success )
          {
            for( unsigned int SampleIndex = 0; SampleIndex < LatencySampleCount; ++SampleIndex )
            {
              std::string SampleName;
              if( m_pClient->GetLatencySampleName(SampleIndex, SampleName) == Result::Success )
              {
                double SampleValue = 0;
                if( m_pClient->GetLatencySampleValue(SampleName, SampleValue) == Result::Success )
                {
                  Latencies[SampleName] = SampleValue;
                }
              }
            }
          }

          unsigned int FrameNumber;
          double FrameRateHz;
          Result::Enum FrameNumberResult = m_pClient->GetFrameNumber(FrameNumber);
          Result::Enum FrameRateResult = m_pClient->GetFrameRate(FrameRateHz);

          if( FrameNumberResult == Result::Success && FrameRateResult == Result::Success )
          {
            // Get a lock here so that we add all subjects from one frame together.
            boost::recursive_mutex::scoped_lock Lock(m_DataMutex);

            double FrameReceiptTime = ( 1.0 / FrameRateHz ) * static_cast< double >( FrameNumber );

            // Capture the receipt time now
            double WallReceiptTime = std::chrono::duration< double, std::milli >(hrc::now() - m_Epoch).count(); 
            std::vector < std::shared_ptr< VSubjectPose > > PoseDataItems;

            // Count the number of subjects
            unsigned int SubjectCount;
            m_pClient->GetSubjectCount(SubjectCount);
            for( unsigned int SubjectIndex = 0; SubjectIndex < SubjectCount; ++SubjectIndex )
            {

              std::shared_ptr< VSubjectPose > pPoseData(new VSubjectPose());

              pPoseData->FrameNumber = FrameNumber;
              pPoseData->FrameTime = FrameReceiptTime;
              pPoseData->ReceiptTime = WallReceiptTime;
              pPoseData->Latencies = Latencies;
              pPoseData->Result = VSubjectPose::ESuccess;
              pPoseData->FrameRate = FrameRateHz;

              // Get the subject name
              std::string SubjectName;
              m_pClient->GetSubjectName(SubjectIndex, SubjectName);

              // Get the root segment
              std::string RootSegment;
              m_pClient->GetSubjectRootSegmentName(SubjectName, RootSegment);

              pPoseData->Name = SubjectName;
              pPoseData->RootSegment = RootSegment;

              // Count the number of segments
              unsigned int SegmentCount;
              m_pClient->GetSegmentCount(SubjectName, SegmentCount);
              for( unsigned int SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex )
              {
                std::string SegmentName;
                m_pClient->GetSegmentName(SubjectName, SegmentIndex, SegmentName);

                pPoseData->m_SegmentNames.push_back( SegmentName );

                std::shared_ptr< VSegmentPose > pSegmentPoseData(new VSegmentPose());
                pSegmentPoseData->Name = SegmentName;

                // Get our parent name (if we have one)
                std::string ParentName;
                if( m_pClient->GetSegmentParentName(SubjectName, SegmentName, ParentName) == Result::Success )
                {
                  pSegmentPoseData->Parent = ParentName;
                }

                // Add some information about the children of this segment
                unsigned int ChildSegmentCount;
                if( m_pClient->GetSegmentChildCount(SubjectName, SegmentName, ChildSegmentCount) == Result::Success )
                {
                  for( unsigned int ChildSegmentIndex = 0; ChildSegmentIndex < ChildSegmentCount; ++ChildSegmentIndex )
                  {
                    std::string ChildSegmentName;
                    if( m_pClient->GetSegmentChildName(SubjectName, SegmentName, ChildSegmentIndex, ChildSegmentName) == Result::Success )
                    {
                      pSegmentPoseData->m_Children.push_back(ChildSegmentName);
                    }
                  }
                }

                // Get the global segment translation
                bool bOccluded = false;
                double Translation[3];
                m_pClient->GetSegmentGlobalTranslation(SubjectName, SegmentName, Translation, bOccluded);
                std::copy(Translation, Translation + 3, pSegmentPoseData->T.begin());

                // Get the global segment rotation in quaternion co-ordinates
                double Rotation[4];
                m_pClient->GetSegmentGlobalRotationQuaternion(SubjectName, SegmentName, Rotation, bOccluded);
                std::copy(Rotation, Rotation + 4, pSegmentPoseData->R.begin());

                // Get local translation
                double LocalTranslation[3];
                m_pClient->GetSegmentLocalTranslation(SubjectName, SegmentName, LocalTranslation, bOccluded);
                std::copy(LocalTranslation, LocalTranslation + 3, pSegmentPoseData->T_Rel.begin());

                // And local rotation
                double LocalRotation[4];
                m_pClient->GetSegmentLocalRotationQuaternion(SubjectName, SegmentName, LocalRotation, bOccluded);
                std::copy(LocalRotation, LocalRotation + 4, pSegmentPoseData->R_Rel.begin());

                // Get static translation
                double StaticTranslation[3];
                m_pClient->GetSegmentStaticTranslation(SubjectName, SegmentName, StaticTranslation);
                std::copy(StaticTranslation, StaticTranslation + 3, pSegmentPoseData->T_Stat.begin());

                // And static rotation
                double StaticRotation[4];
                m_pClient->GetSegmentStaticRotationQuaternion(SubjectName, SegmentName, StaticRotation);
                std::copy(StaticRotation, StaticRotation + 4, pSegmentPoseData->R_Stat.begin());

                // and scale
                double Scale[3];
                if( m_pClient->GetSegmentStaticScale(SubjectName, SegmentName, Scale) == Result::Success )
                {
                  std::copy(Scale, Scale + 3, pSegmentPoseData->Scale.begin());
                  pSegmentPoseData->bHasScale = true;
                }
                else
                {
                  pSegmentPoseData->bHasScale = false;
                }

                pSegmentPoseData->bOccluded = bOccluded;

                pPoseData->m_Segments[SegmentName] = pSegmentPoseData;
              }

              PoseDataItems.emplace_back( pPoseData );
            }

            m_Retimer.AddData( PoseDataItems );

          }
        }
      }

      m_bInputStopped = false;

    }

    void VRetimingClient::OutputThread()
    {
      // Keep track of retimed output frame number
      uint64_t CurrentOutputFrame = 0;

      std::chrono::system_clock::time_point WakeTime
        = std::chrono::system_clock::now() + std::chrono::milliseconds(static_cast< unsigned int >( ( 1.0 / m_FrameRate ) * 1000 ));;

      while( !m_bOutputStopped )
      {
        {
          boost::mutex::scoped_lock Lock( m_OutputMutex );

          // Interpolate output positions if required.
          double Now = std::chrono::duration< double, std::milli >(hrc::now() - m_Epoch).count();
          double PTime = Now - m_OutputLatency;
          m_Retimer.UpdateFrameAtTime( PTime );

          // Output the frame 
          m_OutputWait.notify_all();
        }

        // Increment our output frame number
        ++CurrentOutputFrame;

        // Yield until next frame is required.
        std::this_thread::sleep_until( WakeTime );
        WakeTime += std::chrono::milliseconds(static_cast< unsigned int >( ( 1.0 / m_FrameRate ) * 1000 ));

      }

      m_bOutputStopped = false;
    }
  }
}
