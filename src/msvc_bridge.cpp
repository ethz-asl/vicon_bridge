/*
 * msvc_bridge.cpp
 *
 *  Created on: Apr 6, 2011
 *      Author: acmarkus
 */

#include "msvc_bridge.h"

namespace msvcbridge{

    ViconDataStreamSDK::CPP::Client client;

    FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetVersion GetVersion() {
		return client.GetVersion();
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_Connect Connect( const ViconDataStreamSDK::CPP::String & HostName ){
		return client.Connect(HostName);
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_ConnectToMulticast ConnectToMulticast( const ViconDataStreamSDK::CPP::String & HostName, const ViconDataStreamSDK::CPP::String & MulticastIP ){
		return client.ConnectToMulticast(HostName, MulticastIP);
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_Disconnect  Disconnect(){
		return client.Disconnect();
	}

    FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_IsConnected IsConnected(){
		return client.IsConnected();
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_StartTransmittingMulticast StartTransmittingMulticast( const ViconDataStreamSDK::CPP::String & ServerIP,
	                                                                  const ViconDataStreamSDK::CPP::String & MulticastIP ){
		return client.StartTransmittingMulticast(ServerIP, MulticastIP);
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_StopTransmittingMulticast StopTransmittingMulticast(){
		return client.StopTransmittingMulticast();
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_EnableSegmentData         EnableSegmentData(){
		return client.EnableSegmentData();
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_EnableMarkerData          EnableMarkerData(){
		return client.EnableMarkerData();
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_EnableUnlabeledMarkerData EnableUnlabeledMarkerData(){
		return client.EnableUnlabeledMarkerData();
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_EnableDeviceData          EnableDeviceData(){
		return client.EnableDeviceData();
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_DisableSegmentData         DisableSegmentData(){
		return client.DisableSegmentData();
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_DisableMarkerData          DisableMarkerData(){
		return client.DisableMarkerData();
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_DisableUnlabeledMarkerData DisableUnlabeledMarkerData(){
		return client.DisableUnlabeledMarkerData();
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_DisableDeviceData          DisableDeviceData(){
		return client.DisableDeviceData();
	}

    FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_IsSegmentDataEnabled         IsSegmentDataEnabled(){
		return client.IsSegmentDataEnabled();
	}

    FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_IsMarkerDataEnabled          IsMarkerDataEnabled(){
		return client.IsMarkerDataEnabled();
	}

    FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_IsUnlabeledMarkerDataEnabled IsUnlabeledMarkerDataEnabled(){
		return client.IsUnlabeledMarkerDataEnabled();
	}

    FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_IsDeviceDataEnabled          IsDeviceDataEnabled(){
		return client.IsDeviceDataEnabled();
	}

    FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_SetStreamMode SetStreamMode( const ViconDataStreamSDK::CPP::StreamMode::Enum Mode ){
		return client.SetStreamMode(Mode);
	}

	ViconDataStreamSDK::CPP::Output_SetAxisMapping SetAxisMapping( const ViconDataStreamSDK::CPP::Direction::Enum XAxis, const ViconDataStreamSDK::CPP::Direction::Enum YAxis, const ViconDataStreamSDK::CPP::Direction::Enum ZAxis ){
		return client.SetAxisMapping(XAxis, YAxis, ZAxis);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetAxisMapping GetAxisMapping(){
		return client.GetAxisMapping();
	}

	FUN_DECLSPEC ViconDataStreamSDK::CPP::Output_GetFrame GetFrame(){
		return client.GetFrame();
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetFrameNumber GetFrameNumber(){
		return client.GetFrameNumber();
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetTimecode GetTimecode(){
		return client.GetTimecode();
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetLatencySampleCount GetLatencySampleCount(){
		return client.GetLatencySampleCount();
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetLatencySampleName  GetLatencySampleName( const unsigned int LatencySampleIndex ){
		return client.GetLatencySampleName(LatencySampleIndex);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetLatencySampleValue GetLatencySampleValue( const ViconDataStreamSDK::CPP::String & LatencySampleName ) {
		return client.GetLatencySampleValue(LatencySampleName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetLatencyTotal       GetLatencyTotal() {
		return client.GetLatencyTotal();
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSubjectCount GetSubjectCount() {
		return client.GetSubjectCount();
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSubjectName GetSubjectName( const unsigned int SubjectIndex ) {
		return client.GetSubjectName(SubjectIndex);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSubjectRootSegmentName GetSubjectRootSegmentName( const ViconDataStreamSDK::CPP::String & SubjectName ){
		return client.GetSubjectRootSegmentName(SubjectName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentCount GetSegmentCount( const ViconDataStreamSDK::CPP::String  & SubjectName ) {
		return client.GetSegmentCount(SubjectName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentName GetSegmentName( const ViconDataStreamSDK::CPP::String       & SubjectName,
	                                          const unsigned int   SegmentIndex ) {
		return client.GetSegmentName(SubjectName, SegmentIndex);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentChildCount GetSegmentChildCount( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                      const ViconDataStreamSDK::CPP::String & SegmentName ) {
		return client.GetSegmentChildCount(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentChildName GetSegmentChildName( const ViconDataStreamSDK::CPP::String       & SubjectName,
	                                                    const ViconDataStreamSDK::CPP::String       & SegmentName,
	                                                    const unsigned int   SegmentIndex ) {
		return client.GetSegmentChildName(SubjectName, SegmentName, SegmentIndex);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentParentName GetSegmentParentName( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                      const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentParentName(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentStaticTranslation GetSegmentStaticTranslation( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                    const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentStaticTranslation(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentStaticRotationHelical GetSegmentStaticRotationHelical( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                            const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentStaticRotationHelical(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentStaticRotationMatrix GetSegmentStaticRotationMatrix( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                          const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentStaticRotationMatrix(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentStaticRotationQuaternion GetSegmentStaticRotationQuaternion( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                                  const ViconDataStreamSDK::CPP::String & SegmentName ) {
		return client.GetSegmentStaticRotationQuaternion(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentStaticRotationEulerXYZ GetSegmentStaticRotationEulerXYZ( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                              const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentStaticRotationEulerXYZ(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentGlobalTranslation GetSegmentGlobalTranslation( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                    const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentGlobalTranslation(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentGlobalRotationHelical GetSegmentGlobalRotationHelical( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                            const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentGlobalRotationHelical(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentGlobalRotationMatrix GetSegmentGlobalRotationMatrix( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                          const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentGlobalRotationMatrix(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentGlobalRotationQuaternion GetSegmentGlobalRotationQuaternion( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                                  const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentGlobalRotationQuaternion(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentGlobalRotationEulerXYZ GetSegmentGlobalRotationEulerXYZ( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                              const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentGlobalRotationEulerXYZ(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentLocalTranslation GetSegmentLocalTranslation( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                    const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentLocalTranslation(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentLocalRotationHelical GetSegmentLocalRotationHelical( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                            const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentLocalRotationHelical(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentLocalRotationMatrix GetSegmentLocalRotationMatrix( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                          const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentLocalRotationMatrix(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentLocalRotationQuaternion GetSegmentLocalRotationQuaternion( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                                  const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentLocalRotationQuaternion(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetSegmentLocalRotationEulerXYZ GetSegmentLocalRotationEulerXYZ( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                              const ViconDataStreamSDK::CPP::String & SegmentName ){
		return client.GetSegmentLocalRotationEulerXYZ(SubjectName, SegmentName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetMarkerCount GetMarkerCount( const ViconDataStreamSDK::CPP::String  & SubjectName ) {
		return client.GetMarkerCount(SubjectName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetMarkerName GetMarkerName( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                        const unsigned int  MarkerIndex ) {
		return client.GetMarkerName(SubjectName, MarkerIndex);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetMarkerParentName GetMarkerParentName( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                    const ViconDataStreamSDK::CPP::String & MarkerName ){
		return client.GetMarkerParentName(SubjectName, MarkerName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetMarkerGlobalTranslation GetMarkerGlobalTranslation( const ViconDataStreamSDK::CPP::String & SubjectName,
	                                                                  const ViconDataStreamSDK::CPP::String & MarkerName ){
		return client.GetMarkerGlobalTranslation(SubjectName, MarkerName);
	}


	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetUnlabeledMarkerCount GetUnlabeledMarkerCount(){
		return client.GetUnlabeledMarkerCount();
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetUnlabeledMarkerGlobalTranslation GetUnlabeledMarkerGlobalTranslation( const unsigned int MarkerIndex ){
		return client.GetUnlabeledMarkerGlobalTranslation(MarkerIndex);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetDeviceCount GetDeviceCount(){
		return client.GetDeviceCount();
	}
	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetDeviceName  GetDeviceName( const unsigned int DeviceIndex ){
		return client.GetDeviceName(DeviceIndex);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetDeviceOutputCount GetDeviceOutputCount( const ViconDataStreamSDK::CPP::String  & DeviceName ){
		return client.GetDeviceOutputCount(DeviceName);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetDeviceOutputName GetDeviceOutputName( const ViconDataStreamSDK::CPP::String  & DeviceName,
	                                                    const unsigned int   DeviceOutputIndex ){
		return client.GetDeviceOutputName(DeviceName, DeviceOutputIndex);
	}

	FUN_DECLSPEC const ViconDataStreamSDK::CPP::Output_GetDeviceOutputValue GetDeviceOutputValue( const ViconDataStreamSDK::CPP::String & DeviceName,
	                                                      const ViconDataStreamSDK::CPP::String & DeviceOutputName ) {
		return client.GetDeviceOutputValue(DeviceName, DeviceOutputName);
	}



};
