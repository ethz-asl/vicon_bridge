
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


#include "ClientUtils.h"


#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <array>

#include <numeric>
#include <cmath>

namespace ClientUtils
{
  // These functions clear a value
  void Clear( bool & o_rValue )
  {
    o_rValue = false;
  }

  void Clear( unsigned int & o_rValue )
  {
    o_rValue = 0;
  }

  void Clear(unsigned short & o_rValue)
  {
    o_rValue = 0;
  }

  void Clear(unsigned char & o_rValue)
  {
    o_rValue = 0;
  }

  void Clear( double & o_rValue )
  {
    o_rValue = 0.0;
  }

  void Clear( std::string & o_rValue )
  {
    o_rValue.clear();
  }

  void Clear( ViconDataStreamSDK::Core::TimecodeStandard::Enum & o_rValue )
  {
    o_rValue = ViconDataStreamSDK::Core::TimecodeStandard::None;
  }

  void Clear( ViconDataStreamSDK::Core::DeviceType::Enum & o_rValue )
  {
    o_rValue = ViconDataStreamSDK::Core::DeviceType::Unknown;
  }

  void Clear( ViconDataStreamSDK::Core::Unit::Enum & o_rValue )
  {
    o_rValue = ViconDataStreamSDK::Core::Unit::Unknown;
  }

  // Determine whether an axis is positive or negative
  double ComponentSign( ViconDataStreamSDK::Core::Direction::Enum i_Direction )
  {
    switch( i_Direction )
    {
    case ViconDataStreamSDK::Core::Direction::Forward :
    case ViconDataStreamSDK::Core::Direction::Left    :
    case ViconDataStreamSDK::Core::Direction::Up      : return 1.0;
    case ViconDataStreamSDK::Core::Direction::Backward:
    case ViconDataStreamSDK::Core::Direction::Right   :
    case ViconDataStreamSDK::Core::Direction::Down    : return -1.0;
    default                                           : assert( !"Unknown enum value" );
                                                        return 0.0;
    }
  }

  // Determine the index of an enum
  unsigned int ComponentIndex( ViconDataStreamSDK::Core::Direction::Enum i_Direction )
  {
    switch( i_Direction )
    {
    case ViconDataStreamSDK::Core::Direction::Forward :
    case ViconDataStreamSDK::Core::Direction::Backward: return 0;
    case ViconDataStreamSDK::Core::Direction::Left    :
    case ViconDataStreamSDK::Core::Direction::Right   : return 1;
    case ViconDataStreamSDK::Core::Direction::Up      :
    case ViconDataStreamSDK::Core::Direction::Down    : return 2;
    default                                           : assert( !"Unknown enum value" );
                                                        return 0;
    }
  }

  std::string ComponentName( ViconDataStreamSDK::Core::Direction::Enum i_Direction )
  {
    switch( i_Direction )
    {
    case ViconDataStreamSDK::Core::Direction::Forward: return "Forward";
    case ViconDataStreamSDK::Core::Direction::Backward: return "Backward";
    case ViconDataStreamSDK::Core::Direction::Left: return "Left";
    case ViconDataStreamSDK::Core::Direction::Right: return "Right";
    case ViconDataStreamSDK::Core::Direction::Up: return "Up";
    case ViconDataStreamSDK::Core::Direction::Down: return "Down";
    default: assert( !"Unknown enum value" );
      return "";
    }

  }

  std::string AdaptCameraName( const std::string & i_rCameraName,
                               const std::string & i_rDisplayType,
                               const unsigned int  i_CameraID )
  {
    // Append Camera info to ensure uniqueness, since this name is used as keys in the DSSDK Camera APIs
    return str( boost::format( "%s (%d)" ) %( i_rCameraName.empty()? i_rDisplayType : i_rCameraName ) %i_CameraID );
  }

  std::string AdaptDeviceName( const std::string & i_rDeviceName, 
                               const unsigned int  i_DeviceID )
  {
    if( !i_rDeviceName.empty() )
    {
      return i_rDeviceName;
    }
    else
    {
      return str( boost::format( "Unnamed Device %d" ) % i_DeviceID );
    }
  }

  std::string AdaptDeviceOutputName( const std::string & i_rDeviceOutputName, 
                                     const unsigned int  i_DeviceOutputIndex )
  {
    if( !i_rDeviceOutputName.empty() )
    {
      return i_rDeviceOutputName;
    }
    else
    {
      return str( boost::format( "Unnamed Device Output %d" ) % ( i_DeviceOutputIndex + 1 ) );
    }
  }

  bool IsValidMulticastIP( const std::string & i_MulticastIPAddress )
  {
    boost::asio::io_service Service;
    boost::system::error_code Error;
    boost::asio::ip::address_v4 Address = boost::asio::ip::address_v4::from_string( i_MulticastIPAddress, Error );
    if( Error )
    {
      boost::asio::ip::tcp::resolver Resolver( Service );
      boost::asio::ip::tcp::resolver::query Query( i_MulticastIPAddress, "" );
      
      boost::asio::ip::tcp::resolver::iterator It = Resolver.resolve( Query, Error );
      boost::asio::ip::tcp::resolver::iterator End;
      
      if( ! Error )
      {
        for( ; It != End; ++It )
        {
          Error = boost::system::error_code();
          boost::asio::ip::tcp::endpoint EndPoint( *It );

          // Currently we only handle IPv4
          if( EndPoint.address().is_v4() )
          {
            Address = EndPoint.address().to_v4();
            break;
          }
        }
      }
      else
      {
        Address = boost::asio::ip::address_v4();
      }
    }

    return( Address.is_multicast() || ( Address.to_ulong() == 0xFFFFFFFF ) );
  }


  void MatrixToQuaternion( const double i_M[9], double (&o_Q)[4])
  {
    // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternion Calculus and Fast Animation".

    double Trace = i_M[0*3+0]+i_M[1*3+1]+i_M[2*3+2];
    double Root;

    if( Trace > 0 )
    {
      // |w| > 1/2, may as well choose w > 1/2
      Root = std::sqrt(Trace + 1.0);  // 2w
      o_Q[3] = 0.5*Root;
      Root = 0.5/Root;  // 1/(4w)
      o_Q[0] = (i_M[2*3+1]-i_M[1*3+2])*Root;
      o_Q[1] = (i_M[0*3+2]-i_M[2*3+0])*Root;
      o_Q[2] = (i_M[1*3+0]-i_M[0*3+1])*Root;
    }
    else
    {
      // |w| <= 1/2
      int s_Next[3] = { 1, 2, 0 };
      int i = 0;
      if ( i_M[1*3+1] > i_M[0*3+0] )
          i = 1;
      if ( i_M[2*3+2] > i_M[i*3+i] )
          i = 2;
      int j = s_Next[i];
      int k = s_Next[j];

      Root = std::sqrt(i_M[i*3+i]-i_M[j*3+j]-i_M[k*3+k] + 1.0);

      o_Q[i] = 0.5*Root;
      Root = 0.5/Root;
      o_Q[3] = (i_M[k*3+j]-i_M[j*3+k])*Root;
      o_Q[j] = (i_M[j*3+i]+i_M[i*3+j])*Root;
      o_Q[k] = (i_M[k*3+i]+i_M[i*3+k])*Root;
    }

    // Normalize
    const double InnerProduct = std::inner_product( o_Q, o_Q + 4, o_Q, 0.0 );
    const double Magnitude = std::sqrt( InnerProduct );

    o_Q[0] /= Magnitude;
    o_Q[1] /= Magnitude;
    o_Q[2] /= Magnitude;
    o_Q[3] /= Magnitude;
  }

  
  void MatrixToHelical( const double i_rM[9], double (&o_rAA)[3])
  {
    double Q[4];
    MatrixToQuaternion( i_rM, Q );

    const double Real = Q[3];

    double Len = sqrt( std::inner_product( Q, Q+3, Q, 0.0 ) );
    if( Len > std::numeric_limits< double >::epsilon() * 10 ) 
    {
      const double Angle = 2.0 * atan2( Len, Real );
      const double Scale = Angle / Len;
      std::transform( Q, Q+3, o_rAA, [ Scale ]( double X ){ return Scale * X; } );
    } 
    else 
    {
      std::copy( Q, Q+3, o_rAA );
    }

  }

  void MatrixToEulerXYZ( const double i_M[9], double (&o_rE)[3])
  {
    //  This original algorithm is the XYZ Euler order and is
    //  the default input order argument, so no existing code
    //  should need changing or will give different results

    //  Algorithm: GraphicsGems II - Matrix Techniques VII.1 p 320

    o_rE[1] = asin(i_M[0*3+2]);

    if( fabs( cos(o_rE[1]) ) > std::numeric_limits< double >::epsilon() * 10 )   
    {
      o_rE[0] = std::atan2(-i_M[1*3+2], i_M[2*3+2]);
      o_rE[2] = std::atan2(-i_M[0*3+1], i_M[0*3+0]);
    }
    else
    {
      // cos(y) ~= 0 Gimbal-Lock
      o_rE[0] = (o_rE[1] > 0) ? std::atan2(i_M[1*3+0], i_M[1*3+1]) : -std::atan2(i_M[0*3+1], i_M[1*3+1]);
      o_rE[2] = 0;
    }
  }

  void HelicalToMatrix( const float i_rAA[3], double( &o_rM )[9] )
  {
   double angle, c, s, x, y, z;

    angle = std::sqrt( i_rAA[ 0 ]*i_rAA[ 0 ] + i_rAA[ 1 ]*i_rAA[ 1 ] + i_rAA[ 2 ]*i_rAA[ 2 ] );
    if ( angle < 10 * std::numeric_limits< double >::epsilon() * 10 ) 
    {
      o_rM[ 0 ] = o_rM[ 4 ] = o_rM[ 8 ] = 1.0;
      o_rM[ 1 ] = o_rM[ 2 ] = o_rM[ 3 ] = o_rM[ 5 ] = o_rM[ 6 ] = o_rM[ 7 ] = 0.0;
      return;
    }
    else 
    {
      x = i_rAA[ 0 ] / angle;
      y = i_rAA[ 1 ] / angle;
      z = i_rAA[ 2 ] / angle;
    }
    c = cos( angle );
    s = sin( angle );

    o_rM[ 0] = c + ( 1 - c )*x*x;
    o_rM[ 1 ] = ( 1 - c )*x*y + s * ( -z );
    o_rM[ 2 ] = ( 1 - c )*x*z + s * y;
    o_rM[ 3 ] = ( 1 - c )*y*x + s * z;
    o_rM[ 4 ] = c + ( 1 - c )*y*y;
    o_rM[ 5 ] = ( 1 - c )*y*z + s * ( -x );
    o_rM[ 6 ] = ( 1 - c )*z*x + s * ( -y );
    o_rM[ 7 ] = ( 1 - c )*z*y + s * x;
    o_rM[ 8 ] = c + ( 1 - c )*z*z;

  }

  std::array< double, 3 > operator*( const std::array< double, 9 > & i_rM, const std::array< double, 3 > & i_rX )
  {
    std::array< double, 3 > Result;

    for( size_t i = 0; i < 3; i++) 
    {  
      double Sum = 0;
      for( unsigned k = 0; k < 3; k++ ) 
      {
        Sum += i_rM[i*3+k] * i_rX[k];
      }
      Result[i] = Sum;
    }

  return Result;
  }

  std::array< double, 3 > & operator/=( std::array< double, 3 > & i_rX, double i_Val )
  {
    std::transform( i_rX.begin(), i_rX.end(), i_rX.begin(), [ i_Val ]( double x ){ return x / i_Val; } );
    return i_rX;
  }

  std::array< double, 3 > & operator*=( std::array< double, 3 > & i_rX, double i_Val )
  {
    std::transform( i_rX.begin(), i_rX.end(), i_rX.begin(), [ i_Val ]( double x ){ return x * i_Val; } );
    return i_rX;
  }

  std::array< double, 3 > operator+( const std::array< double, 3 > & i_rX, const std::array< double, 3 > & i_rY )
  {
    std::array< double, 3 > Result;
    std::transform( i_rX.begin(), i_rX.end(), i_rY.begin(), Result.begin(), []( double x, double y ){ return x + y; } );
    return Result;
  }

  std::array< double, 3 > operator-( const std::array< double, 3 > & i_rX, const std::array< double, 3 > & i_rY )
  {
    std::array< double, 3 > Result;
    std::transform( i_rX.begin(), i_rX.end(), i_rY.begin(), Result.begin(), []( double x, double y ){ return x - y; } );
    return Result;
  }
    
  std::array< double, 9 > operator*( const std::array< double, 9 > & i_rM, const std::array< double, 9 > & i_rN )
  {
    std::array< double, 9 > Result;

    for( size_t i = 0; i < 3; i++) 
    {  
      for( size_t j = 0; j < 3; j++) 
      { 
        double Sum = 0;
        for( unsigned k = 0; k < 3; k++ ) 
        {
          Sum += i_rM[i*3+k] * i_rN[k*3+j];
        }
        Result[i*3+j] = Sum;
      }
    }

    return Result;
  }


  std::array< double, 9 > Transpose( const std::array< double, 9 > & i_rM )
  {
    std::array< double, 9 > Result;

    for( size_t i = 0; i < 3; i++) 
    {  
      for( size_t j = 0; j < 3; j++) 
      { 
        Result[i*3+j] = i_rM[j*3+i];
      }
    }

    return Result;
  }

  std::array< double, 16 > Transpose( const std::array< double, 16 > & i_rM )
  {
    std::array< double, 16 > Result;

    for( size_t i = 0; i < 4; i++) 
    {  
      for( size_t j = 0; j < 4; j++) 
      { 
        Result[i*4+j] = i_rM[j*4+i];
      }
    }

    return Result;
  }

  std::array< double, 3 > CrossProduct( std::array< double, 3 > & i_rA, std::array< double, 3 > & i_rB )
  {
    std::array< double, 3 > Result;
    Result[0] = i_rA[1] * i_rB[2] - i_rA[2] * i_rB[1];
    Result[1] = i_rA[2] * i_rB[0] - i_rA[0] * i_rB[2];
    Result[2] = i_rA[0] * i_rB[1] - i_rA[1] * i_rB[0];
    return Result;
  }

  double DotProduct( std::array<double, 3> & i_rA, std::array<double, 3> & i_rB )
  {
    double Result = 0;
    for( unsigned i = 0; i < 3; ++i )
    {
      Result += i_rA[i] * i_rB[i];
    }
    return Result;
  }

  double JitterVal(std::default_random_engine & i_rEngine, double i_Mean, double i_Variance, double i_Spike, int i_SpikeFrequency)
  {
    // Try a smaller amount of jitter around the mean with an occasional random chance of a spike thrown in.

    if( i_SpikeFrequency > 0 )
    {
      // Calculate the probability of a spike, up to the value of Max
      std::uniform_int_distribution<> spike_probability(0, i_SpikeFrequency);
      if( spike_probability(i_rEngine) == i_SpikeFrequency )
      {
        return i_Spike;
      }
    }

    // Otherwise return random values based on an exponential distribution
    std::exponential_distribution< double > Jitter(10);

    double JitterVal = 2.0;
    while( JitterVal > 1.0 )
    {
      JitterVal = Jitter(i_rEngine);
    }

    JitterVal *= i_Variance;
    JitterVal += i_Mean;

    return JitterVal;
  }
}
