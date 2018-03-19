///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) OMG Plc 2009.
// All rights reserved.  This software is protected by copyright
// law and international treaties.  No part of this software / document
// may be reproduced or distributed in any form or by any means,
// whether transiently or incidentally to some other use of this software,
// without the written permission of the copyright owner.
//
///////////////////////////////////////////////////////////////////////////////
#pragma once

/// \file 
/// The SDK client and supporting definitions.

#ifdef WIN32

#ifdef _EXPORTING
   #define CLASS_DECLSPEC    __declspec(dllexport)
#else
   #define CLASS_DECLSPEC    __declspec(dllimport)
#endif // _EXPORTING

#elif defined( __GNUC__ )

#if __GNUC__ < 4
    #error gcc 4 is required.
  #endif
  #define CLASS_DECLSPEC     __attribute__((visibility("default")))

#else

#define CLASS_DECLSPEC

#endif

#include <string>
#include <vector>

namespace ViconDataStreamSDK
{
namespace CPP
{

class IStringFactory
{
public:
  virtual char * AllocAndCopyString( const char * i_pSource ) = 0;
  virtual void FreeString( char * i_pString ) = 0;
protected:
  virtual ~IStringFactory() {}
};

// A string managing class for DLL boundry safe text.
// It ensures that strings are de-allocated with the same 
// memory manager they are made with.  This allows you to use a debug 
// build of your client with a release SDK.
class String
{
public:
  // A string which we are not responsible for deallocating
  inline
  String( const char * i_pString = 0 )
  : m_pString( 0 )
  , m_pConstString( i_pString )
  , m_pStringFactory( 0 )
  {
  }

  // A string which we are not responsible for deallocating
  String( const std::string & i_rString )
  : m_pString( 0 )
  , m_pConstString( i_rString.c_str() )
  , m_pStringFactory( 0 )
  {
  }

  // Copy constructor
  inline
  String( const String & i_rString )
  {
    m_pConstString = i_rString.m_pConstString;
    m_pStringFactory = i_rString.m_pStringFactory;
    if( m_pStringFactory )
    {
      m_pString = m_pStringFactory->AllocAndCopyString( i_rString.m_pString );
    }
    else
    {
      m_pString = 0;
    }
  }

  inline
  ~String()
  {
    if( m_pStringFactory )
    {
      m_pStringFactory->FreeString( m_pString );
    }
  }

  // A string which we are responsible for deallocating
  inline
  void Set( const char * i_pString, IStringFactory & i_rStringFactory )
  {
    m_pString = i_rStringFactory.AllocAndCopyString( i_pString );
    m_pStringFactory = &i_rStringFactory;
    m_pConstString = 0;
  }

  inline
  operator std::string() const
  {
    if( m_pStringFactory )
    {
      return std::string( m_pString );
    }
    else
    {
      return std::string( m_pConstString );
    }
  }

private:
        char     * m_pString;
  const char     * m_pConstString;
  IStringFactory * m_pStringFactory;
};

// Streaming operator for String
inline std::ostream & operator<<( std::ostream & io_rStream, const String & i_rString )
{
  io_rStream << std::string( i_rString );
  return io_rStream;
}

namespace Direction
{
  enum Enum
  {
    Up,
    Down,
    Left,
    Right,
    Forward,
    Backward
  };
}

namespace ServerOrientation
{
  enum Enum
  {
    Unknown,
    YUp,
    ZUp,
  };
}

namespace StreamMode
{
  enum Enum
  {
    ClientPull,
    ClientPullPreFetch,
    ServerPush
  };
}

namespace TimecodeStandard
{
  enum Enum
  {
    None,
    PAL,
    NTSC,
    NTSCDrop,
    Film
  };
}

namespace DeviceType
{
  enum Enum
  {
    Unknown,
    ForcePlate,
    EyeTracker
  };
}

namespace Unit
{
  enum Enum
  {
    Unknown,
    Volt,
    Newton,
    NewtonMeter,
    Meter,
    Kilogram,
    Second,
    Ampere,
    Kelvin,
    Mole,
    Candela,
    Radian,
    Steradian,
    MeterSquared,
    MeterCubed,
    MeterPerSecond,
    MeterPerSecondSquared,
    RadianPerSecond,
    RadianPerSecondSquared,
    Hertz,
    Joule,
    Watt,
    Pascal,
    Lumen,
    Lux,
    Coulomb,
    Ohm,
    Farad,
    Weber,
    Tesla,
    Henry,
    Siemens,
    Becquerel,
    Gray,
    Sievert,
    Katal
  };

}

namespace Result
{
  /// The Result code indicates the success or failure of a function.
  enum Enum
  {
    Unknown, ///< The result is unknown. Treat it as a failure.
    NotImplemented, ///< The function called has not been implemented in this version of the SDK.
    Success, ///< The function call succeeded.
    InvalidHostName, ///< The "HostName" parameter passed to Connect() is invalid.
    InvalidMulticastIP, ///< The "MulticastIP" parameter was not in the range "224.0.0.0" - "239.255.255.255"
    ClientAlreadyConnected, ///< Connect() was called whilst already connected to a DataStream.
    ClientConnectionFailed, ///< Connect() could not establish a connection to the DataStream server.
    ServerAlreadyTransmittingMulticast, ///< StartTransmittingMulticast() was called when the current DataStream server was already transmitting multicast on behalf of this client.
    ServerNotTransmittingMulticast, ///< StopTransmittingMulticast() was called when the current DataStream server was not transmitting multicasr on behalf of this client.
    NotConnected, ///< You have called a function which requires a connection to the DataStream server, but do not have a connection.
    NoFrame, ///< You have called a function which requires a frame to be fetched from the DataStream server, but do not have a frame.
    InvalidIndex, ///< An index you have passed to a function is out of range.
    InvalidCameraName, ///< The Camera Name you passed to a function is invalid in this frame.
    InvalidSubjectName, ///< The Subject Name you passed to a function is invalid in this frame.
    InvalidSegmentName, ///< The Segment Name you passed to a function is invalid in this frame.
    InvalidMarkerName, ///< The Marker Name you passed to a function is invalid in this frame.
    InvalidDeviceName, ///< The Device Name you passed to a function is invalid in this frame.
    InvalidDeviceOutputName, ///< The Device Output Name you passed to a function is invalid in this frame.
    InvalidLatencySampleName, ///< The Latency Sample Name you passed to a function is invalid in this frame.
    CoLinearAxes, ///< The directions passed to SetAxisMapping() contain input which would cause two or more axis to lie along the same line, e.g. "Up" and "Down" are on the same line.
    LeftHandedAxes, ///< The directions passed to SetAxisMapping() would result in a left handed co-ordinate system. This is not supported in the SDK.
    HapticAlreadySet ///< Haptic feedback is already set
  };
}

  /// The version information returned by Client::GetVersion.
  class Output_GetVersion
  {
  public:
    /// Major version number of the SDK.
    unsigned int Major;
    /// Minor version number of the SDK.
    unsigned int Minor;
    /// Point version number of the SDK.
    unsigned int Point;
  };

  class Output_SimpleResult
  {
  public:
    Result::Enum Result;
  };

  // Output objects that only return a result enum
  class Output_Connect                    : public Output_SimpleResult {};
  class Output_ConnectToMulticast         : public Output_SimpleResult {};
  class Output_Disconnect                 : public Output_SimpleResult {};
  class Output_StartTransmittingMulticast : public Output_SimpleResult {};
  class Output_StopTransmittingMulticast  : public Output_SimpleResult {};
  class Output_EnableSegmentData          : public Output_SimpleResult {};
  class Output_EnableMarkerData           : public Output_SimpleResult {};
  class Output_EnableUnlabeledMarkerData  : public Output_SimpleResult {};
  class Output_EnableMarkerRayData        : public Output_SimpleResult {};
  class Output_EnableDeviceData           : public Output_SimpleResult {};
  class Output_EnableCentroidData         : public Output_SimpleResult {};
  class Output_EnableGreyscaleData        : public Output_SimpleResult {};
  class Output_EnableDebugData            : public Output_SimpleResult {};
  class Output_DisableSegmentData         : public Output_SimpleResult {};
  class Output_DisableMarkerData          : public Output_SimpleResult {};
  class Output_DisableUnlabeledMarkerData : public Output_SimpleResult {};
  class Output_DisableMarkerRayData       : public Output_SimpleResult {};
  class Output_DisableDeviceData          : public Output_SimpleResult {};
  class Output_DisableCentroidData        : public Output_SimpleResult {};
  class Output_DisableGreyscaleData       : public Output_SimpleResult {};
  class Output_DisableDebugData           : public Output_SimpleResult {};
  class Output_SetStreamMode              : public Output_SimpleResult {};
  class Output_SetApexDeviceFeedback      : public Output_SimpleResult {};
  class Output_SetAxisMapping             : public Output_SimpleResult {};
  class Output_GetFrame                   : public Output_SimpleResult {};
  class Output_SetCameraFilter            : public Output_SimpleResult {};

  class Output_EnabledFlag
  {
  public:
    bool Enabled;
  };

  // Output objects that only return an enabled flag
  class Output_IsSegmentDataEnabled         : public Output_EnabledFlag {};
  class Output_IsMarkerDataEnabled          : public Output_EnabledFlag {};
  class Output_IsUnlabeledMarkerDataEnabled : public Output_EnabledFlag {};
  class Output_IsMarkerRayDataEnabled       : public Output_EnabledFlag {};
  class Output_IsDeviceDataEnabled          : public Output_EnabledFlag {};
  class Output_IsCentroidDataEnabled        : public Output_EnabledFlag {};
  class Output_IsGreyscaleDataEnabled       : public Output_EnabledFlag {};
  class Output_IsDebugDataEnabled           : public Output_EnabledFlag {};

  class Output_IsConnected
  {
  public:
    bool Connected;
  };

  class Output_GetAxisMapping
  {
  public:
    Direction::Enum XAxis;
    Direction::Enum YAxis;
    Direction::Enum ZAxis;
  };

  class Output_GetServerOrientation
  {
  public:
    Result::Enum Result;
    ServerOrientation::Enum Orientation;
  };

  class Output_GetFrameNumber
  {
  public:
    Result::Enum Result;
    unsigned int FrameNumber;
  };

  class Output_GetTimecode
  {
  public:
    Result::Enum           Result;
    unsigned int           Hours;
    unsigned int           Minutes;
    unsigned int           Seconds;
    unsigned int           Frames;
    unsigned int           SubFrame;
    bool                   FieldFlag;
    TimecodeStandard::Enum Standard;
    unsigned int           SubFramesPerFrame;
    unsigned int           UserBits;
  };

  class Output_GetFrameRate
  {
  public:
    Result::Enum Result;
    double FrameRateHz;
  };

  class Output_GetLatencySampleCount
  {
  public:
    Result::Enum Result;
    unsigned int Count;
  };

  class Output_GetLatencySampleName
  {
  public:
    Result::Enum Result;
    String       Name;
  };

  class Output_GetLatencySampleValue
  {
  public:
    Result::Enum Result;
    double       Value;
  };

  class Output_GetLatencyTotal
  {
  public:
    Result::Enum Result;
    double       Total;
  };

  class Output_GetFrameRateCount
  {
  public:
    Result::Enum Result;
    unsigned int Count;
  };

  class Output_GetFrameRateName
  {
  public:
    Result::Enum Result;
    String       Name;
  };

  class Output_GetFrameRateValue
  {
  public:
    Result::Enum Result;
    double       Value;
  };

  class Output_GetHardwareFrameNumber
  {
  public:
    Result::Enum Result;
    unsigned int HardwareFrameNumber;
  };


  class Output_GetSubjectCount
  {
  public:
    Result::Enum Result;
    unsigned int SubjectCount;
  };

  class Output_GetSubjectName
  {
  public:
    Result::Enum Result;
    String       SubjectName;
  };

  class Output_GetSubjectRootSegmentName
  {
  public:
    Result::Enum Result;
    String       SegmentName;
  };

  class Output_GetSegmentChildCount
  {
  public:
    Result::Enum Result;
    unsigned int SegmentCount;
  };

  class Output_GetSegmentChildName
  {
  public:
    Result::Enum Result;
    String       SegmentName;
  };

  class Output_GetSegmentParentName
  {
  public:
    Result::Enum Result;
    String       SegmentName;
  };

  class Output_GetSegmentCount
  {
  public:
    Result::Enum Result;
    unsigned int SegmentCount;
  };

  class Output_GetSegmentName
  {
  public:
    Result::Enum Result;
    String       SegmentName;
  };

  class Output_GetSegmentStaticTranslation
  {
  public:
    Result::Enum Result;
    double       Translation[ 3 ];
  };

  class Output_GetSegmentStaticRotationHelical
  {
  public:
    Result::Enum Result;
    double       Rotation[ 3 ];
  };

  class Output_GetSegmentStaticRotationMatrix
  {
  public:
    Result::Enum Result;
    double       Rotation[ 9 ];
  };

  class Output_GetSegmentStaticRotationQuaternion
  {
  public:
    Result::Enum Result;
    double       Rotation[ 4 ];
  };

  class Output_GetSegmentStaticRotationEulerXYZ
  {
  public:
    Result::Enum Result;
    double       Rotation[ 3 ];
  };

  class Output_GetSegmentGlobalTranslation
  {
  public:
    Result::Enum Result;
    double       Translation[ 3 ];
    bool         Occluded;
  };

  class Output_GetSegmentGlobalRotationHelical
  {
  public:
    Result::Enum Result;
    double       Rotation[ 3 ];
    bool         Occluded;
  };

  class Output_GetSegmentGlobalRotationMatrix
  {
  public:
    Result::Enum Result;
    double       Rotation[ 9 ];
    bool         Occluded;
  };

  class Output_GetSegmentGlobalRotationQuaternion
  {
  public:
    Result::Enum Result;
    double       Rotation[ 4 ];
    bool         Occluded;
  };

  class Output_GetSegmentGlobalRotationEulerXYZ
  {
  public:
    Result::Enum Result;
    double       Rotation[ 3 ];
    bool         Occluded;
  };

  class Output_GetSegmentLocalTranslation
  {
  public:
    Result::Enum Result;
    double       Translation[ 3 ];
    bool         Occluded;
  };

  class Output_GetSegmentLocalRotationHelical
  {
  public:
    Result::Enum Result;
    double       Rotation[ 3 ];
    bool         Occluded;
  };

  class Output_GetSegmentLocalRotationMatrix
  {
  public:
    Result::Enum Result;
    double       Rotation[ 9 ];
    bool         Occluded;
  };

  class Output_GetSegmentLocalRotationQuaternion
  {
  public:
    Result::Enum Result;
    double       Rotation[ 4 ];
    bool         Occluded;
  };

  class Output_GetSegmentLocalRotationEulerXYZ
  {
  public:
    Result::Enum Result;
    double       Rotation[ 3 ];
    bool         Occluded;
  };

  class Output_GetObjectQuality
  {
  public:
    Result::Enum Result;
    double Quality;
  };

  class Output_GetMarkerCount
  {
  public:
    Result::Enum Result;
    unsigned int MarkerCount;
  };

  class Output_GetMarkerName
  {
  public:
    Result::Enum Result;
    String       MarkerName;
  };

  class Output_GetMarkerParentName
  {
  public:
    Result::Enum Result;
    String       SegmentName;
  };

  class Output_GetMarkerGlobalTranslation
  {
  public:
    Result::Enum Result;
    double       Translation[ 3 ];
    bool         Occluded;
  };

  class Output_GetMarkerRayContributionCount
  {
  public:
    Result::Enum Result;
    unsigned int RayContributionsCount;
  };

  class Output_GetMarkerRayContribution
  {
  public:
    Result::Enum Result;
    unsigned int CameraID;
    unsigned int CentroidIndex;
  };

  class Output_GetUnlabeledMarkerCount
  {
  public:
    Result::Enum Result;
    unsigned int MarkerCount;
  };

  class Output_GetUnlabeledMarkerGlobalTranslation
  {
  public:
    Result::Enum Result;
    double       Translation[ 3 ];
  };

  class Output_GetLabeledMarkerCount
  {
  public:
    Result::Enum Result;
    unsigned int MarkerCount;
  };

  class Output_GetLabeledMarkerGlobalTranslation
  {
  public:
    Result::Enum Result;
    double       Translation[ 3 ];
  };

  class Output_GetDeviceCount
  {
  public:
    Result::Enum Result;
    unsigned int DeviceCount;
  };

  class Output_GetDeviceName
  {
  public:
    Result::Enum     Result;
    String           DeviceName;
    DeviceType::Enum DeviceType;
  };

  class Output_GetDeviceOutputCount
  {
  public:
    Result::Enum Result;
    unsigned int DeviceOutputCount;
  };

  class Output_GetDeviceOutputName
  {
  public:
    Result::Enum Result;
    String       DeviceOutputName;
    Unit::Enum   DeviceOutputUnit;
  };

  class Output_GetDeviceOutputValue
  {
  public:
    Result::Enum Result;
    double       Value;
    bool         Occluded;
  };

  class Output_GetDeviceOutputSubsamples
  {
  public:
    Result::Enum Result;
    unsigned int DeviceOutputSubsamples;
    bool         Occluded;
  };

  class Output_GetForcePlateCount
  {
  public:
    Result::Enum Result;
    unsigned int ForcePlateCount;
  };

  class Output_GetGlobalForceVector
  {
  public:
    Result::Enum Result;
    double       ForceVector[ 3 ];
  };

  class Output_GetGlobalMomentVector
  {
  public:
    Result::Enum Result;
    double       MomentVector[ 3 ];
  };

  class Output_GetGlobalCentreOfPressure
  {
  public:
    Result::Enum Result;
    double       CentreOfPressure[ 3 ];
  };

  class Output_GetForcePlateSubsamples
  {
  public:
    Result::Enum Result;
    unsigned int ForcePlateSubsamples;
  };

  class Output_GetEyeTrackerCount
  {
  public:
    Result::Enum Result;
    unsigned int EyeTrackerCount;
  };

  class Output_GetEyeTrackerGlobalPosition
  {
  public:
    Result::Enum Result;
    double       Position[ 3 ];
    bool         Occluded;
  };

  class Output_GetEyeTrackerGlobalGazeVector
  {
  public:
    Result::Enum Result;
    double       GazeVector[ 3 ];
    bool         Occluded;
  };

  class Output_GetCameraCount
  {
  public:
    Result::Enum Result;
    unsigned int CameraCount;
  };

  class Output_GetCameraName
  {
  public:
    Result::Enum     Result;
    String           CameraName;
  };

  class Output_GetCameraId
  {
  public:
    Result::Enum Result;
    unsigned int CameraId;
  };

  class Output_GetCameraUserId
  {
  public:
    Result::Enum Result;
    unsigned int CameraUserId;
  };

  class Output_GetCameraType
  {
  public:
    Result::Enum     Result;
    String           CameraType;
  };

  class Output_GetCameraDisplayName
  {
  public:
    Result::Enum     Result;
    String           CameraDisplayName;
  };

  class Output_GetCameraResolution
  {
  public:
    Result::Enum Result;
    unsigned int ResolutionX;
    unsigned int ResolutionY;
  };

  class Output_GetIsVideoCamera
  {
  public:
    Result::Enum Result;
    bool IsVideoCamera;
  };

  class Output_GetCentroidCount
  {
  public:
    Result::Enum Result;
    unsigned int CentroidCount;
  };

  class Output_GetCentroidPosition
  {
  public:
    Result::Enum Result;
    double CentroidPosition[ 2 ];
    double Radius;
//    double Accuracy;
  };

  class Output_GetCentroidWeight
  {
  public:
    Result::Enum Result;
    double Weight;
  };

  class Output_GetGreyscaleBlobCount
  {
  public:
    Result::Enum Result;
    unsigned int BlobCount;
  };

  class Output_GetGreyscaleBlob
  {
  public:
    Result::Enum Result;
    std::vector< unsigned int > BlobLinePositionsX;
    std::vector< unsigned int > BlobLinePositionsY;
    std::vector< std::vector< unsigned char > > BlobLinePixelValues;
  };

  class ClientImpl;

  ///  Vicon DataStream SDK client.
  ///  The core client class for C++.
  class CLASS_DECLSPEC Client
  {
  public:
    /// Construction.
    /// You can create many instances of the Vicon DataStream Client which can connect to multiple Vicon DataStream Servers.
    /// C++ version of the SDK is object oriented, so use the class constructor.
    ///
    ///     {
    ///         ViconDataStreamSDK::CPP::Client StackClient;
    ///         Output_SomeFunction Output = StackClient.SomeFunction();
    ///         // ...
    ///     } 
    ///     // Client is implicitly destroyed as it goes out of scope.
    ///
    /// Alternatively it can be made on the heap.
    ///
    ///     ViconDataStreamSDK::CPP::Client * pHeapClient = new ViconDataStreamSDK::CPP::Client();
    ///     Output_SomeFunction Output = pHeapClient->SomeFunction( Input );
    ///     delete pHeapClient;
    ///
    Client();

    /// Destruction.
    /// Destruction will Disconnect if required.
    /// 
    /// See Client::Client for an example.
    ~Client();

    /// Get the version of the Vicon DataStream SDK.
    /// + **Major**
    ///   When this number increases we break backwards compatibility with previous major versions.
    /// + **Minor**
    ///   When this number increases we have probably added new functionality to the SDK without breaking 
    ///   backwards compatibility with previous versions.
    /// + **Point**
    ///   When this number increases, we have introduced a bug fix or performance enhancement without breaking 
    ///   backwards compatibility with previous versions.
    /// 
    /// The function can be called without the client being connected.
    /// 
    ///      ViconDataStreamSDK::CPP::Client MyClient;
    ///      Output_GetVersion Output = MyClient.GetVersion();
    /// 
    /// \return Output_GetVersion class containing the version information.
    Output_GetVersion  GetVersion() const;

    /// Establish a dedicated connection to a Vicon DataStream Server.
    /// 
    /// See Also : ConnectToMulticast, Disconnect, IsConnected.
    /// 
    /// The function defaults to connecting on port 801. 
    /// You can specify an alternate port number after a colon.
    /// This is for future compatibility, current products serve data on port 801 only.
    /// 
    ///      ViconDataStreamSDK::CPP::Client MyClient;
    ///      Output_Connect Output = MyClient.Connect( "localhost" );
    /// 
    /// \param  HostName The DNS identifiable name, or IP address of the PC hosting the DataStream server. 
    ///                  For example:
    ///                  + "localhost"
    ///                  + "MyViconPC:801"
    ///                  + "10.0.0.2"
    /// \return An Output_Connect class containing the result of the connect operation.
    ///         The Result will be: 
    ///         + Success
    ///         + InvalidHostName
    ///         + ClientAlreadyConnected 
    ///         + ClientConnectionFailed
    Output_Connect Connect( const String & HostName );

    /// Connect to a Vicon DataStream Server's Multicast stream. 
    /// The stream content is managed by a client who calls StartTransmittingMulticast().
    /// 
    /// See Also : Connect, Disconnect, IsConnected, StartTransmittingMulticast, StopTransmittingMulticast
    /// 
    ///      // class Output_ConnectToMulticast
    ///      // {
    ///      // public:
    ///      // Result::Enum Result;
    ///      // };
    ///      
    ///      ViconDataStreamSDK::CPP::Client MyClient;
    ///      Output_ConnectToMulticast Output = MyClient.ConnectToMulticast( "localhost", "224.0.0.0" );
    ///
    ///
    /// \param  LocalIP      The DNS identifiable name, or IP address of the local Ethernet interface 
    ///                      on which you wish to receive multicast data. 
    ///                      You should not specify a port (any port specified will be ignored).
    ///                      For example:
    ///                      + "localhost"
    ///                      + "10.0.0.2"
    /// \param  MulticastIP  The IP Address of the Multicast group on which data will be received. 
    ///                      The address should be in the range "224.0.0.0" - "239.255.255.255" 
    ///                      You may also specify a port by appending it to the end of the IP Address 
    ///                      after a colon. e.g. 224.0.0.0:30001. 
    ///                      If you do not specify a port it will default to 44801.
    /// \return An Output_ConnectToMulticast class containing the result of the connect operation.
    ///         The Result will be: 
    ///         + Success
    ///         + InvalidHostName
    ///         + InvalidMulticastIP
    ///         + ClientAlreadyConnected 
    ///         + ClientConnectionFailed
    Output_ConnectToMulticast ConnectToMulticast( const String & LocalIP, const String & MulticastIP );

    /// Disconnect from the Vicon DataStream Server.
    /// 
    /// See Also : Connect, IsConnected
    /// 
    ///       ViconDataStreamSDK::CPP::Client MyClient;
    ///       MyClient.Connect( "localhost" );
    ///       Output_Disconnect Output = MyClient.Disconnect();
    /// 
    /// \return An Output_Disconnect class containing the result of the operation.
    ///         The Result will be: 
    ///         + Success
    ///         + NotConnected
    Output_Disconnect  Disconnect();

    /// Discover whether client is connected to the Vicon DataStream Server.
    /// 
    /// See Also : Connect, Disconnect
    /// 
    ///       ViconDataStreamSDK::CPP::CPP::Client MyClient;
    ///       Output_IsConnected Output = MyClient.IsConnected()
    ///       // Output.Connected == false
    ///       MyClient.Connect( "localhost" );
    ///       Output_IsConnected Output = MyClient.IsConnected()
    ///       // Output.Connected == true
    ///       // (assuming localhost is serving)
    /// 
    /// \return An Output_IsConnected class containing the true if you are 
    ///         connected to the stream, otherwise false.
    Output_IsConnected IsConnected() const;

    /// Ask the DataStream Server to start transmitting the data you are receiving directly to a Multicast address as well. 
    /// This allows multiple clients to connect to your stream (via ConnectToMulticast() ) whilst minimizing network 
    /// bandwidth use and frame delivery latency.
    /// See Also : Connect, ConnectToMulticast, Disconnect, StopTransmittingMulticast
    /// 
    ///       ViconDataStreamSDK::CPP::CPP::Client MyClient;
    ///       MyClient.Connect( "localhost" );
    ///       MyClient.StartTransmittingMulticast( "10.0.0.1", "224.0.0.0" );
    /// 
    /// \param  ServerIP      The IP Address of the server Ethernet interface from which the Multicast data will be sent. 
    ///                       You should not specify a port number (any port number specified will be ignored).
    /// \param  MulticastIP   The IP Address of the Multicast group to which multicast data will be sent. 
    ///                       The address should be in the range 224.0.0.0 - 239.255.255.255. You may also specify 
    ///                       the port the data will be sent to by appending it to the IP Address after a colon 
    ///                       e.g. 224.0.0.0:30001. If you do not specify a port it will default to 44801. 
    /// \return An Output_StartTransmittingMulticast class containing the result of the operation.
    ///         The Result will be: 
    ///         + Success
    ///         + NotConnected
    ///         + InvalidMulticastIP
    ///         + ServerAlreadyTransmittingMulticast
    Output_StartTransmittingMulticast StartTransmittingMulticast( const String & ServerIP,
                                                                  const String & MulticastIP );
    /// Ask the DataStream Server to stop transmitting the data you are receiving directly to a Multicast address as well. 
    /// You must previously have started a transmission via StartTransmittingMulticast.
    /// 
    /// See Also : Connect, ConnectToMulticast, Disconnect, StartTransmittingMulticast
    /// 
    ///       ViconDataStreamSDK::CPP::CPP::Client MyClient;
    ///       MyClient.Connect( "localhost" );
    ///       MyClient.StartTransmittingMulticast( "10.0.0.1", "224.0.0.0" );
    ///       // Do some stuff
    ///       MyClient.StopTransmittingMulticast();
    /// 
    /// \return An Output_StopTransmittingMulticast class containing the result of the operation.
    ///         The Result will be: 
    ///         + Success
    ///         + NotConnected
    ///         + ServerNotTransmittingMulticast
    Output_StopTransmittingMulticast StopTransmittingMulticast();

    /// Set the number of frames that the client should buffer.
    /// The default value is 1 which always supplies the latest frame.
    /// Choose higher values to reduce the risk of missing frames between calls
    /// to GetFrame()
    void SetBufferSize( unsigned int i_BufferSize );

    Output_EnableSegmentData         EnableSegmentData();
    Output_EnableMarkerData          EnableMarkerData();
    Output_EnableUnlabeledMarkerData EnableUnlabeledMarkerData();
    Output_EnableMarkerRayData       EnableMarkerRayData();
    Output_EnableDeviceData          EnableDeviceData();
    Output_EnableCentroidData        EnableCentroidData();
    Output_EnableGreyscaleData       EnableGreyscaleData();
    Output_EnableDebugData           EnableDebugData();

    Output_DisableSegmentData         DisableSegmentData();
    Output_DisableMarkerData          DisableMarkerData();
    Output_DisableUnlabeledMarkerData DisableUnlabeledMarkerData();
    Output_DisableMarkerRayData       DisableMarkerRayData();
    Output_DisableDeviceData          DisableDeviceData();
    Output_DisableCentroidData        DisableCentroidData();
    Output_DisableGreyscaleData       DisableGreyscaleData();
    Output_DisableDebugData           DisableDebugData();

    Output_IsSegmentDataEnabled         IsSegmentDataEnabled() const;
    Output_IsMarkerDataEnabled          IsMarkerDataEnabled() const;
    Output_IsUnlabeledMarkerDataEnabled IsUnlabeledMarkerDataEnabled() const;
    Output_IsMarkerRayDataEnabled       IsMarkerRayDataEnabled() const;
    Output_IsDeviceDataEnabled          IsDeviceDataEnabled() const;
    Output_IsCentroidDataEnabled        IsCentroidDataEnabled() const;
    Output_IsGreyscaleDataEnabled       IsGreyscaleDataEnabled() const;
    Output_IsDebugDataEnabled           IsDebugDataEnabled() const;

    Output_SetStreamMode SetStreamMode( const StreamMode::Enum Mode );

    Output_SetApexDeviceFeedback SetApexDeviceFeedback( const String& i_rDeviceName, bool i_bOn );

    Output_SetAxisMapping SetAxisMapping( const Direction::Enum XAxis, const Direction::Enum YAxis, const Direction::Enum ZAxis );
    Output_GetAxisMapping GetAxisMapping() const;
    Output_GetServerOrientation GetServerOrientation() const;

    Output_GetFrame GetFrame();
    Output_GetFrameNumber GetFrameNumber() const;

    Output_GetTimecode GetTimecode() const;

    Output_GetFrameRate GetFrameRate() const;

    Output_GetLatencySampleCount  GetLatencySampleCount() const;
    Output_GetLatencySampleName   GetLatencySampleName( const unsigned int LatencySampleIndex ) const;
    Output_GetLatencySampleValue  GetLatencySampleValue( const String & LatencySampleName ) const;
    Output_GetLatencyTotal        GetLatencyTotal() const;
    Output_GetHardwareFrameNumber GetHardwareFrameNumber() const;

    Output_GetFrameRateCount  GetFrameRateCount() const;
    Output_GetFrameRateName   GetFrameRateName( const unsigned int FrameRateIndex ) const;
    Output_GetFrameRateValue  GetFrameRateValue( const String & FrameRateName ) const;


    Output_GetSubjectCount GetSubjectCount() const;
    Output_GetSubjectName GetSubjectName( const unsigned int SubjectIndex ) const;

    Output_GetSubjectRootSegmentName GetSubjectRootSegmentName( const String & SubjectName ) const;

    Output_GetSegmentCount GetSegmentCount( const String  & SubjectName ) const;
    
    Output_GetSegmentName GetSegmentName( const String       & SubjectName,
                                          const unsigned int   SegmentIndex ) const;

    Output_GetSegmentChildCount GetSegmentChildCount( const String & SubjectName,
                                                      const String & SegmentName ) const;

    Output_GetSegmentChildName GetSegmentChildName( const String       & SubjectName,
                                                    const String       & SegmentName,
                                                    const unsigned int   SegmentIndex ) const;

    Output_GetSegmentParentName GetSegmentParentName( const String & SubjectName,
                                                      const String & SegmentName ) const;

    Output_GetSegmentStaticTranslation GetSegmentStaticTranslation( const String & SubjectName,
                                                                    const String & SegmentName ) const;

    Output_GetSegmentStaticRotationHelical GetSegmentStaticRotationHelical( const String & SubjectName,
                                                                            const String & SegmentName ) const;

    Output_GetSegmentStaticRotationMatrix GetSegmentStaticRotationMatrix( const String & SubjectName,
                                                                          const String & SegmentName ) const;

    Output_GetSegmentStaticRotationQuaternion GetSegmentStaticRotationQuaternion( const String & SubjectName,
                                                                                  const String & SegmentName ) const;

    /// Return the static pose rotation of a subject segment in EulerXYZ co-ordinates.
    ///
    /// See Also : GetSegmentStaticTranslation, GetSegmentStaticRotationHelical, GetSegmentStaticRotationMatrix, 
    /// GetSegmentStaticRotationQuaternion, GetSegmentLocalTranslation, GetSegmentLocalRotationHelical, 
    /// GetSegmentLocalRotationMatrix, GetSegmentLocalRotationQuaternion, GetSegmentLocalRotationEulerXYZ.
    /// 
    ///      ViconDataStreamSDK::CPP::Client MyClient;
    ///      MyClient.Connect( "localhost" );
    ///      MyClient.GetFrame();
    ///      Output_GetSegmentStaticRotationEulerXYZ Output;
    ///      Output = MyClient.GetSegmentStaticRotationEulerXYZ( "Alice", "Pelvis" );
    /// 
    /// \param  SubjectName The name of the subject.
    /// \param  SegmentName The name of the segment.
    /// \return A Output_GetSegmentStaticRotationEulerXYZ class containing the result of the request
    ///         and the rotation of the segment \f$(x,y,z)\f$.
    ///         + Success
    ///         + NotConnected
    ///         + NoFrame
    ///         + InvalidSubjectName
    ///         + InvalidSegmentName
    Output_GetSegmentStaticRotationEulerXYZ GetSegmentStaticRotationEulerXYZ( const String & SubjectName,
                                                                              const String & SegmentName ) const;

    Output_GetSegmentGlobalTranslation GetSegmentGlobalTranslation( const String & SubjectName,
                                                                    const String & SegmentName ) const;

    Output_GetSegmentGlobalRotationHelical GetSegmentGlobalRotationHelical( const String & SubjectName,
                                                                            const String & SegmentName ) const;

    Output_GetSegmentGlobalRotationMatrix GetSegmentGlobalRotationMatrix( const String & SubjectName,
                                                                          const String & SegmentName ) const;

    Output_GetSegmentGlobalRotationQuaternion GetSegmentGlobalRotationQuaternion( const String & SubjectName,
                                                                                  const String & SegmentName ) const;

    Output_GetSegmentGlobalRotationEulerXYZ GetSegmentGlobalRotationEulerXYZ( const String & SubjectName,
                                                                              const String & SegmentName ) const;

    Output_GetSegmentLocalTranslation GetSegmentLocalTranslation( const String & SubjectName,
                                                                  const String & SegmentName ) const;

    Output_GetSegmentLocalRotationHelical GetSegmentLocalRotationHelical( const String & SubjectName,
                                                                          const String & SegmentName ) const;

    Output_GetSegmentLocalRotationMatrix GetSegmentLocalRotationMatrix( const String & SubjectName,
                                                                        const String & SegmentName ) const;

    Output_GetSegmentLocalRotationQuaternion GetSegmentLocalRotationQuaternion( const String & SubjectName,
                                                                                const String & SegmentName ) const;

    Output_GetSegmentLocalRotationEulerXYZ GetSegmentLocalRotationEulerXYZ( const String & SubjectName,
                                                                            const String & SegmentName ) const;

    Output_GetObjectQuality GetObjectQuality( const String & ObjectName ) const;

    Output_GetMarkerCount GetMarkerCount( const String  & SubjectName ) const;

    Output_GetMarkerName GetMarkerName( const String & SubjectName,
                                        const unsigned int  MarkerIndex ) const;

    Output_GetMarkerParentName GetMarkerParentName( const String & SubjectName,
                                                    const String & MarkerName ) const;

    Output_GetMarkerGlobalTranslation GetMarkerGlobalTranslation( const String & SubjectName,
                                                                  const String & MarkerName ) const;

    Output_GetMarkerRayContributionCount GetMarkerRayContributionCount( const String & SubjectName,
                                                                        const String & MarkerName ) const;

    Output_GetMarkerRayContribution GetMarkerRayContribution( const String & SubjectName,
                                                                   const String & MarkerName,
                                                                   unsigned int MarkerRayContributionIndex ) const;


    Output_GetUnlabeledMarkerCount GetUnlabeledMarkerCount() const;
    Output_GetUnlabeledMarkerGlobalTranslation GetUnlabeledMarkerGlobalTranslation( const unsigned int MarkerIndex ) const;

    Output_GetLabeledMarkerCount GetLabeledMarkerCount() const;
    Output_GetLabeledMarkerGlobalTranslation GetLabeledMarkerGlobalTranslation( const unsigned int MarkerIndex ) const;

    Output_GetDeviceCount GetDeviceCount() const;
    Output_GetDeviceName  GetDeviceName( const unsigned int DeviceIndex ) const;

    Output_GetDeviceOutputCount GetDeviceOutputCount( const String  & DeviceName ) const;

    Output_GetDeviceOutputName GetDeviceOutputName( const String  & DeviceName,
                                                    const unsigned int   DeviceOutputIndex ) const;
    
    Output_GetDeviceOutputValue GetDeviceOutputValue( const String & DeviceName,
                                                      const String & DeviceOutputName ) const;

    Output_GetDeviceOutputSubsamples GetDeviceOutputSubsamples( const String & DeviceName,
                                                                const String & DeviceOutputName ) const;

    Output_GetDeviceOutputValue GetDeviceOutputValue( const String & DeviceName,
                                                      const String & DeviceOutputName,
                                                      const unsigned int Subsample ) const;

    Output_GetForcePlateCount GetForcePlateCount() const;

    Output_GetGlobalForceVector GetGlobalForceVector( const unsigned int ForcePlateIndex ) const;
    Output_GetGlobalMomentVector GetGlobalMomentVector( const unsigned int ForcePlateIndex ) const;
    Output_GetGlobalCentreOfPressure GetGlobalCentreOfPressure( const unsigned int ForcePlateIndex ) const;

    Output_GetForcePlateSubsamples GetForcePlateSubsamples( const unsigned int ForcePlateIndex ) const;

    Output_GetGlobalForceVector GetGlobalForceVector( const unsigned int ForcePlateIndex, const unsigned int Subsample ) const;
    Output_GetGlobalMomentVector GetGlobalMomentVector( const unsigned int ForcePlateIndex, const unsigned int Subsample ) const;
    Output_GetGlobalCentreOfPressure GetGlobalCentreOfPressure( const unsigned int ForcePlateIndex, const unsigned int Subsample ) const;



    Output_GetEyeTrackerCount GetEyeTrackerCount() const;

    Output_GetEyeTrackerGlobalPosition GetEyeTrackerGlobalPosition( const unsigned int EyeTrackerIndex ) const;
    Output_GetEyeTrackerGlobalGazeVector GetEyeTrackerGlobalGazeVector( const unsigned int EyeTrackerIndex ) const;

    Output_GetCameraCount GetCameraCount() const;
    Output_GetCameraName GetCameraName( unsigned int i_CameraIndex ) const;
    Output_GetCameraId GetCameraId( const std::string & i_rCameraName ) const;
    Output_GetCameraUserId GetCameraUserId( const std::string & i_rCameraName ) const;
    Output_GetCameraType GetCameraType( const std::string & i_rCameraName ) const;
    Output_GetCameraDisplayName GetCameraDisplayName(  const std::string & i_rCameraName ) const;
    
    Output_GetCameraResolution GetCameraResolution( const std::string & i_rCameraName ) const;
    Output_GetIsVideoCamera GetIsVideoCamera( const std::string & i_rCameraName ) const;
    Output_GetCentroidCount GetCentroidCount( const std::string & i_rCameraName ) const;
    Output_GetCentroidPosition GetCentroidPosition( const std::string & i_rCameraName, const unsigned int i_CentroidIndex ) const;
    Output_GetCentroidWeight GetCentroidWeight( const std::string & i_rCameraName, const unsigned int i_CentroidIndex ) const;
    Output_GetGreyscaleBlobCount GetGreyscaleBlobCount( const std::string & i_rCameraName ) const;
    Output_GetGreyscaleBlob GetGreyscaleBlob( const std::string & i_rCameraName, const unsigned int i_BlobIndex ) const;

    Output_SetCameraFilter SetCameraFilter( const std::vector< unsigned int > & i_rCameraIdsForCentroids, const std::vector< unsigned int > & i_rCameraIdsForBlobs );

  private:
    ClientImpl * m_pClientImpl;
  };
} // End of namespace CPP
} // End of namespace ViconDataStreamSDK
