
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

#include <ViconDataStreamSDKCore/RetimingCore.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ViconDataStreamSDK
{
  namespace Core
  {
    class VSegmentPoseReader
    {
    public:
      VSegmentPoseReader();

      bool Load(const std::string & i_rFile);
      bool Read( std::istream & i_rStream );
      void GenerateTestData(unsigned int i_NumFrames, double i_FrameRate, double i_TransmissionLatency, double i_TransmissionJitter, double i_TransmissionSpike, int i_TransmissionSpikeFrequency);

      unsigned int StartFrame() const;
      unsigned int EndFrame() const;
      unsigned int FrameCount() const;
      unsigned int SubjectCount() const;

      bool SubjectName(unsigned int i_Index, std::string & i_rName) const;

      std::shared_ptr< VSubjectPose > PoseAt(unsigned int i_Frame, const std::string & i_rSubject) const;
      bool FrameIndexClosestToTime( double i_Time, unsigned int & o_rFrame ) const;
    private:

      std::shared_ptr< VSubjectPose > ReadLine(const std::string & i_rLine, const std::vector< std::string > & i_rHeaderItems ) const;

      std::vector< std::string > m_Subjects;
      std::map< unsigned int, std::map< std::string, std::shared_ptr< VSubjectPose > > > m_Data;
      std::map< double, unsigned int > m_FrameToTime;
    };
  }
}

