
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
#include "SegmentPoseReader.h"

#include <ViconDataStreamSDKCoreUtils/ClientUtils.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <random>

namespace ViconDataStreamSDK
{
namespace Core
{

VSegmentPoseReader::VSegmentPoseReader()
{

}

bool VSegmentPoseReader::Load(const std::string & i_rFile)
{
  std::ifstream FileReader;
  FileReader.open(i_rFile);

  if (!FileReader.good())
  {
    return false;
  }

  return Read(FileReader);
}

bool VSegmentPoseReader::Read( std::istream & i_rStream )
{
  unsigned int CurrentFrameNumber = -1;
  double CurrentFrameTime;

  std::map< std::string, std::shared_ptr< VSubjectPose > > FrameMap;

  std::vector< std::string > HeaderItems;

  std::string LineString;
  while (std::getline(i_rStream, LineString) )
  {
    if (boost::starts_with(LineString, "FrameNumber"))
    {
      boost::algorithm::split( HeaderItems, LineString, boost::is_any_of( "," ) );

      // Skip the header line
      continue;
    }

    std::shared_ptr< VSubjectPose > pPose = ReadLine(LineString, HeaderItems );
    if (pPose)
    {
      if (std::find(m_Subjects.begin(), m_Subjects.end(), pPose->Name) == m_Subjects.end())
      {
        m_Subjects.push_back(pPose->Name);
      }

      if (pPose->FrameNumber != CurrentFrameNumber)
      {
        constexpr unsigned int NoFrameNumber(-1);
        if (CurrentFrameNumber != NoFrameNumber)
        {
          m_Data[ CurrentFrameNumber ] = FrameMap;
          m_FrameToTime[ CurrentFrameTime ] = static_cast< unsigned int >( CurrentFrameNumber );
          FrameMap.clear();
        }

        CurrentFrameNumber = static_cast< unsigned int >( pPose->FrameNumber );
        CurrentFrameTime = pPose->ReceiptTime;
      }

      FrameMap[ pPose->Name ] = pPose;
    }
    else
    {
      return false;
    }
  }

  // Add the last frame
  if (!FrameMap.empty())
  {
    m_Data[ CurrentFrameNumber ] = FrameMap;
  }

  return true;
}

void VSegmentPoseReader::GenerateTestData(unsigned int i_NumFrames, double i_FrameRate, double i_TransmissionLatency, double i_TransmissionJitter, double i_TransmissionSpike, int i_TransmissionSpikeFrequency )
{
  m_Subjects.clear();
  m_Data.clear();

  std::string SubjectName = "TestObject";

  std::default_random_engine JitterGenerator;

  m_Subjects.push_back( SubjectName );

  for (unsigned int FrameNum = 0; FrameNum < i_NumFrames; ++FrameNum)
  {
    std::shared_ptr< VSubjectPose > pPose = std::make_shared< VSubjectPose >();
    pPose->Result = VSubjectPose::ESuccess;
    pPose->FrameNumber = FrameNum;
    pPose->FrameRate = i_FrameRate;
    pPose->Name = SubjectName;
    double FramePeriod = 1.0 / i_FrameRate * 1000;
    pPose->Latencies[ "Network" ] = ClientUtils::JitterVal( JitterGenerator, i_TransmissionLatency, i_TransmissionJitter, i_TransmissionSpike, i_TransmissionSpikeFrequency );
    pPose->Latencies[ "Processing" ] = ClientUtils::JitterVal(JitterGenerator, FramePeriod, FramePeriod / 10.0, 0.0, 0 );
    pPose->FrameTime = static_cast<double>(FrameNum) / i_FrameRate * 1000.0;
    pPose->ReceiptTime = pPose->FrameTime + pPose->TotalLatency();

    std::shared_ptr< VSegmentPose > pRoot = std::make_shared< VSegmentPose >();
    pRoot->bOccluded = false;
    pRoot->Name = "Root";
    pRoot->Parent = "";

    pRoot->T[0] = 10.0 * sin( ( FrameNum % 360 ) * 3.1415 / 180.0 );
    pRoot->T[1] = 10.0 * cos( ( FrameNum % 360 ) * 3.1415 / 180.0);
    pRoot->T[2] = 10.0 * sin( 2 * ( FrameNum % 360 ) * 3.1415 / 180.0 );

    pPose->m_Segments.insert(std::make_pair( pRoot->Name, pRoot ) );

    std::map < std::string, std::shared_ptr< VSubjectPose > > FrameData;
    FrameData.insert( std::make_pair( pPose->Name, pPose ) );
    m_Data[ FrameNum ] = FrameData;
    m_FrameToTime[ pPose->ReceiptTime ] = FrameNum;
  }
}

template< typename T >
bool ReadValue(const std::string & i_rInput, T & o_rOutput )
{
  try
  {
    std::string Input = boost::algorithm::trim_copy(i_rInput);
    o_rOutput = boost::lexical_cast< T >(Input);
    return true;
  }
  catch (boost::bad_lexical_cast & blc)
  {
    std::cout << "Error parsing " << i_rInput << " as " << typeid( T ).name() << ": " << blc.what() << std::endl;
    return false;
  }
}

std::shared_ptr< VSubjectPose > VSegmentPoseReader::ReadLine(const std::string & i_rLine, const std::vector< std::string > & i_rHeaderItems ) const
{
  std::vector< std::string > Tokens;

  boost::algorithm::split(Tokens, i_rLine, boost::is_any_of(","));

  std::shared_ptr< VSubjectPose > pPose(new VSubjectPose());


  bool bOK = true;

  unsigned int TokenIndex = 0;

  // Tokens should have minimum size as detailed in the subject pose definition
  if (Tokens.size() >= VSubjectPose::NumElements)
  {
    bOK = bOK && ReadValue<double>(Tokens[TokenIndex++], pPose->FrameNumber);

    unsigned int PoseResult = VSubjectPose::ESuccess;
    bOK = bOK && ReadValue<unsigned int>(Tokens[TokenIndex++], PoseResult );
    if (bOK)
    {
      pPose->Result = static_cast<VSubjectPose::EResult>(PoseResult);
    }

    bOK = bOK && ReadValue<double>(Tokens[TokenIndex++], pPose->FrameRate);
    bOK = bOK && ReadValue<double>(Tokens[TokenIndex++], pPose->ReceiptTime);
    unsigned int NumLatencies = 0;
    bOK = bOK && ReadValue<unsigned int >(Tokens[TokenIndex++ ], NumLatencies );
    for( unsigned int LatencyIndex = 0; LatencyIndex < NumLatencies; ++LatencyIndex )
    {
      double Latency = 0;
      std::string LatencyType;
      if( TokenIndex < i_rHeaderItems.size() )
      {
        LatencyType = i_rHeaderItems[TokenIndex];
      }
      bOK = bOK && ReadValue< double >( Tokens[ TokenIndex++ ], Latency );
      if( bOK ) pPose->Latencies[LatencyType] = Latency;
    }
    bOK = bOK && ReadValue< std::string >(Tokens[TokenIndex++], pPose->Name);
    bOK = bOK && ReadValue< std::string >(Tokens[TokenIndex++], pPose->RootSegment);

    if (bOK)
    {
      // We will only cope with sequences that start numbering from zero
      pPose->FrameTime = pPose->FrameNumber / pPose->FrameRate * 1000.0;
    }

    size_t NumSegments;
    if (bOK && ReadValue<size_t>(Tokens[TokenIndex++], NumSegments))
    {
      if (Tokens.size() >= NumSegments * VSegmentPose::NumElements)
      {
        for (size_t SegmentIndex = 0; SegmentIndex < NumSegments; ++SegmentIndex)
        {
          std::shared_ptr< VSegmentPose > pSegment(new VSegmentPose());
          bOK = bOK && ReadValue< std::string >(Tokens[TokenIndex++], pSegment->Name);
          bOK = bOK && ReadValue< std::string >(Tokens[TokenIndex++], pSegment->Parent);
          bOK = bOK && ReadValue<bool>(Tokens[TokenIndex++], pSegment->bOccluded);
          for (unsigned int i = 0; i < 3; ++i)  bOK = bOK && ReadValue<double>(Tokens[TokenIndex++], pSegment->T[i]);
          for (unsigned int i = 0; i < 4; ++i)  bOK = bOK && ReadValue<double>(Tokens[TokenIndex++], pSegment->R[i]);
          for (unsigned int i = 0; i < 3; ++i)  bOK = bOK && ReadValue<double>(Tokens[TokenIndex++], pSegment->T_Rel[i]);
          for (unsigned int i = 0; i < 4; ++i)  bOK = bOK && ReadValue<double>(Tokens[TokenIndex++], pSegment->R_Rel[i]);
          for (unsigned int i = 0; i < 3; ++i)  bOK = bOK && ReadValue<double>(Tokens[TokenIndex++], pSegment->T_Stat[i]);
          for (unsigned int i = 0; i < 4; ++i)  bOK = bOK && ReadValue<double>(Tokens[TokenIndex++], pSegment->R_Stat[i]);
          size_t NumChildren;
          if (bOK && ReadValue<size_t>(Tokens[TokenIndex++], NumChildren))
          {
            for (size_t ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
            {
              std::string ChildName;
              if (bOK && ReadValue< std::string >(Tokens[TokenIndex++], ChildName))
              {
                pSegment->m_Children.push_back(ChildName);
              }
            }
          }

          if (bOK)
          {
            pPose->m_Segments.insert(std::make_pair(pSegment->Name, pSegment));
          }
        }
      }
    }
  }

  if( bOK )
  { 
    return pPose;
  }
  else
  {
    return std::shared_ptr< VSubjectPose >();
  }
}

unsigned int VSegmentPoseReader::StartFrame() const
{
  if( !m_Data.empty() )
  {
    return m_Data.begin()->first;
  }
  return 0;
}

unsigned int VSegmentPoseReader::EndFrame() const
{
  if( !m_Data.empty() )
  {
    const auto & rEnd = m_Data.rbegin();
    return rEnd->first;
  }
  return 0;
}

unsigned int VSegmentPoseReader::FrameCount() const
{
  return EndFrame() - StartFrame();
}

unsigned int VSegmentPoseReader::SubjectCount() const
{
  return static_cast< unsigned int >( m_Subjects.size() );
}

bool VSegmentPoseReader::SubjectName(unsigned int i_Index, std::string & i_rName) const
{
  if ( i_Index >= m_Subjects.size() )
  {
    return false;
  }
  else
  {
    i_rName = m_Subjects[i_Index];
    return true;
  }
}

std::shared_ptr< VSubjectPose > VSegmentPoseReader::PoseAt(unsigned int i_Frame, const std::string & i_rSubject) const
{
  std::shared_ptr< VSubjectPose > pResult;
  unsigned int Start = StartFrame();
  unsigned int End  = EndFrame();

  if (i_Frame < Start || i_Frame > End )
  {
    return pResult;
  }

  const auto & rFrameIt = m_Data.find( i_Frame );
  if( rFrameIt != m_Data.end() )
  { 
    const auto & rFrame = rFrameIt->second;
    const auto rIt = rFrame.find(i_rSubject);
    if (rIt != rFrame.end())
    {
      return rIt->second;
    }
  }
  return pResult;
}

bool VSegmentPoseReader::FrameIndexClosestToTime( double i_Time, unsigned int & o_rFrame ) const
{
  auto It = m_FrameToTime.lower_bound( i_Time );

  // Return the last frame if i_Time is later than all samples
  if (It == m_FrameToTime.end())
  {
    o_rFrame = m_FrameToTime.rbegin()->second;
    return true;
  }

  if (It == m_FrameToTime.begin())
  {
    return false;
  }
  else
  {
    --It;
    o_rFrame = It->second;
    return true;
  }
}

}
}

