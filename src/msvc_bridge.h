#ifndef __MSVC_BRIDGE_H__
#define __MSVC_BRIDGE_H__

#ifdef WIN32
#ifdef _EXPORTING
   #define FUN_DECLSPEC    __declspec(dllexport)
#else
   #define FUN_DECLSPEC    __declspec(dllimport)
#endif // _EXPORTING

#else

#define FUN_DECLSPEC

#endif

#include <Client.h>

/***
 * This was the desperate try to get this compiled for windows so that it can be executed on the PC running Tracker.
 * Mingw doesn't like the datastreamsdk lib for windows which is compiled with visual studio, so this was meant as a bridging library ...
 */

namespace msvcbridge{

	extern ViconDataStreamSDK::CPP::Client client;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetVersion  GetVersion() ;

	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_Connect     Connect( const ViconDataStreamSDK::CPP::String & HostName );
	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_ConnectToMulticast ConnectToMulticast( const ViconDataStreamSDK::CPP::String & HostName, const ViconDataStreamSDK::CPP::String & MulticastIP );
	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_Disconnect  Disconnect();
	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_IsConnected IsConnected() ;
	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_StartTransmittingMulticast StartTransmittingMulticast( const ViconDataStreamSDK::CPP::String & ServerIP,
	                                                                  const ViconDataStreamSDK::CPP::String & MulticastIP );

	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_StopTransmittingMulticast StopTransmittingMulticast();

	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_EnableSegmentData         EnableSegmentData();
	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_EnableMarkerData          EnableMarkerData();
	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_EnableUnlabeledMarkerData EnableUnlabeledMarkerData();
	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_EnableDeviceData          EnableDeviceData();

	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_DisableSegmentData         DisableSegmentData();
	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_DisableMarkerData          DisableMarkerData();
	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_DisableUnlabeledMarkerData DisableUnlabeledMarkerData();
	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_DisableDeviceData          DisableDeviceData();

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_IsSegmentDataEnabled         IsSegmentDataEnabled() ;
	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_IsMarkerDataEnabled          IsMarkerDataEnabled() ;
	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_IsUnlabeledMarkerDataEnabled IsUnlabeledMarkerDataEnabled() ;
	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_IsDeviceDataEnabled          IsDeviceDataEnabled() ;

	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_SetStreamMode SetStreamMode( const ViconDataStreamSDK::CPP::StreamMode::Enum Mode );

	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_SetAxisMapping SetAxisMapping( const ViconDataStreamSDK::CPP::Direction::Enum XAxis, const ViconDataStreamSDK::CPP::Direction::Enum YAxis, const ViconDataStreamSDK::CPP::Direction::Enum ZAxis );
	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetAxisMapping GetAxisMapping() ;

	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_GetFrame GetFrame();
	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetFrameNumber GetFrameNumber() ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetTimecode GetTimecode() ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetLatencySampleCount GetLatencySampleCount() ;
	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetLatencySampleName  GetLatencySampleName( const unsigned int LatencySampleIndex ) ;
	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetLatencySampleValue GetLatencySampleValue( const ViconDataStreamSDK::CPP::String & LatencySampleName ) ;
	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetLatencyTotal       GetLatencyTotal() ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSubjectCount GetSubjectCount() ;
	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSubjectName GetSubjectName( const unsigned int SubjectIndex ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSubjectRootSegmentName GetSubjectRootSegmentName( const ViconDataStreamSDK::CPP::String & SubjectName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentCount GetSegmentCount( const ViconDataStreamSDK::CPP::String  & SubjectName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentName GetSegmentName( const ViconDataStreamSDK::CPP::String       & SubjectName,
	                                          const unsigned int   SegmentIndex ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentChildCount GetSegmentChildCount( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                      const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentChildName GetSegmentChildName( const ViconDataStreamSDK::CPP::String       & SubjectName,
	                                                    const ViconDataStreamSDK::CPP::String       & SegmentName,
	                                                    const unsigned int   SegmentIndex ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentParentName GetSegmentParentName( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                      const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentStaticTranslation GetSegmentStaticTranslation( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                    const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentStaticRotationHelical GetSegmentStaticRotationHelical( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                            const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentStaticRotationMatrix GetSegmentStaticRotationMatrix( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                          const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentStaticRotationQuaternion GetSegmentStaticRotationQuaternion( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                                  const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentStaticRotationEulerXYZ GetSegmentStaticRotationEulerXYZ( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                              const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentGlobalTranslation GetSegmentGlobalTranslation( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                    const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentGlobalRotationHelical GetSegmentGlobalRotationHelical( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                            const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentGlobalRotationMatrix GetSegmentGlobalRotationMatrix( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                          const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentGlobalRotationQuaternion GetSegmentGlobalRotationQuaternion( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                                  const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentGlobalRotationEulerXYZ GetSegmentGlobalRotationEulerXYZ( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                              const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentLocalTranslation GetSegmentLocalTranslation( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                  const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentLocalRotationHelical GetSegmentLocalRotationHelical( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                          const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentLocalRotationMatrix GetSegmentLocalRotationMatrix( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                        const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentLocalRotationQuaternion GetSegmentLocalRotationQuaternion( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                                const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentLocalRotationEulerXYZ GetSegmentLocalRotationEulerXYZ( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                            const ViconDataStreamSDK::CPP::String & SegmentName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetMarkerCount GetMarkerCount( const ViconDataStreamSDK::CPP::String  & SubjectName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetMarkerName GetMarkerName( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                        const unsigned int  MarkerIndex ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetMarkerParentName GetMarkerParentName( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                    const ViconDataStreamSDK::CPP::String & MarkerName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetMarkerGlobalTranslation GetMarkerGlobalTranslation( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                  const ViconDataStreamSDK::CPP::String & MarkerName ) ;


	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetUnlabeledMarkerCount GetUnlabeledMarkerCount() ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetUnlabeledMarkerGlobalTranslation GetUnlabeledMarkerGlobalTranslation( const unsigned int MarkerIndex ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetDeviceCount GetDeviceCount() ;
	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetDeviceName  GetDeviceName( const unsigned int DeviceIndex ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetDeviceOutputCount GetDeviceOutputCount( const ViconDataStreamSDK::CPP::String  & DeviceName ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetDeviceOutputName GetDeviceOutputName( const ViconDataStreamSDK::CPP::String  & DeviceName,
	                                                    const unsigned int   DeviceOutputIndex ) ;

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetDeviceOutputValue GetDeviceOutputValue( const ViconDataStreamSDK::CPP::String & DeviceName,
	                                                      const ViconDataStreamSDK::CPP::String & DeviceOutputName );

};

#endif
