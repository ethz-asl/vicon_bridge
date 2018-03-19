% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Copyright (C) OMG Plc 2009.
% All rights reserved.  This software is protected by copyright
% law and international treaties.  No part of this software / document
% may be reproduced or distributed in any form or by any means,
% whether transiently or incidentally to some other use of this software,
% without the written permission of the copyright owner.
%
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Part of the internal QA test suite for Vicon DataStream SDK for MATLAB.
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


% Load the SDK
fprintf( 'Loading SDK...' );
Client.LoadViconDataStreamSDK();
fprintf( 'done\n' );

% Program options
HostName = 'localhost:801';

% Make a new client
MyClient = Client();

% Connect to a server
fprintf( 'Connecting to %s ...', HostName );
while ~MyClient.IsConnected().Connected
  % Direct connection
  MyClient.Connect( HostName );
  
  fprintf( '.' );
end
fprintf( '\n' );

% Enable some different data types
MyClient.EnableSegmentData();
MyClient.EnableDeviceData();

% Set the streaming mode
MyClient.SetStreamMode( StreamMode.ClientPull );

MyClient.SetAxisMapping( Direction.Forward, ...
                         Direction.Left,    ...
                         Direction.Up );    % Z-up
%MyClient.SetAxisMapping( Direction.Forward, ...
%                         Direction.Up,      ...
%                         Direction.Right ); % Y-up

% Discover the version number
Output_GetVersion = MyClient.GetVersion();
version = sprintf( 'Vicon DataStreamSDK Version: %d.%d.%d\n', Output_GetVersion.Major, ...
                                                              Output_GetVersion.Minor, ...
                                                              Output_GetVersion.Point );
  
% A dialog to stop the loop
MessageBox = msgbox( 'Stop DataStream Client', version );
 
% Loop until the message box is dismissed
while ishandle( MessageBox )
  drawnow;
    
  % Get a frame
  while MyClient.GetFrame().Result.Value ~= Result.Success
    fprintf( '.' );
  end% while

  % Get the frame number
  Output_GetFrameNumber = MyClient.GetFrameNumber();
  fprintf( 'Frame Number: %d\n', Output_GetFrameNumber.FrameNumber );

  % Count the number of subjects
    
  % Get the subject name
  SubjectName = MyClient.GetSubjectName( 1 ).SubjectName;
    
  % Get the root segment
  RootSegment = MyClient.GetSubjectRootSegmentName( SubjectName ).SegmentName;

  % Get the global segment translation
  Output_GetSegmentGlobalTranslation = MyClient.GetSegmentGlobalTranslation( SubjectName, RootSegment );
  fprintf( '%s %s\n  Global Translation:\n  X : %g\n  Y : %g\n  Z : %g\n\n', ...
                     SubjectName,                                            ...
                     RootSegment,                                            ...
                     Output_GetSegmentGlobalTranslation.Translation( 1 ),    ...
                     Output_GetSegmentGlobalTranslation.Translation( 2 ),    ...
                     Output_GetSegmentGlobalTranslation.Translation( 3 ) );
  
  % Get the global segment rotation in helical co-ordinates
  Output_GetSegmentGlobalRotationHelical = MyClient.GetSegmentGlobalRotationHelical( SubjectName, RootSegment );
  fprintf( ' Global Rotation (Helical):\n  X : %g\n  Y : %g\n  Z : %g\n\n',             ...
                     Output_GetSegmentGlobalRotationHelical.Rotation( 1 ), ...
                     Output_GetSegmentGlobalRotationHelical.Rotation( 2 ), ...
                     Output_GetSegmentGlobalRotationHelical.Rotation( 3 ) );
                 
  % Count the number of Devices
  DeviceCount = MyClient.GetDeviceCount().DeviceCount;
  fprintf( 'Device Count: %d\n', DeviceCount );
                 
  % Get the device name and type
  Output_GetDeviceName = MyClient.GetDeviceName( 1 );
  fprintf( 'Device: %s\n', Output_GetDeviceName.DeviceName );
    
  % Count the number of device outputs
  DeviceOutputCount = MyClient.GetDeviceOutputCount( Output_GetDeviceName.DeviceName ).DeviceOutputCount;
  fprintf( '  Device Outputs (%d):\n', DeviceOutputCount );
  for DeviceOutputIndex = 1:DeviceOutputCount
    % Get the device output name and unit
    Output_GetDeviceOutputName = MyClient.GetDeviceOutputName( Output_GetDeviceName.DeviceName, DeviceOutputIndex );

    Output_GetDeviceOutputSubsamples = MyClient.GetDeviceOutputSubsamples( Output_GetDeviceName.DeviceName, Output_GetDeviceOutputName.DeviceOutputName );
    fprintf( 'Device Output Subsamples: %d\n', Output_GetDeviceOutputSubsamples.Subsamples );
    
    for DeviceOutputSubsample = 1:Output_GetDeviceOutputSubsamples.Subsamples
      % Get the device output value
      Output_GetDeviceOutputValue = MyClient.GetDeviceOutputValue( Output_GetDeviceName.DeviceName, Output_GetDeviceOutputName.DeviceOutputName, DeviceOutputSubsample );

      fprintf( '    Device Output #%d - %d: %s %g %s %d\n',                      ...
                         DeviceOutputIndex,                                      ...
                         DeviceOutputSubsample,                                  ...
                         Output_GetDeviceOutputName.DeviceOutputName,            ...
                         Output_GetDeviceOutputValue.Value,                      ...
                         Output_GetDeviceOutputName.DeviceOutputUnit.ToString(), ...
                         Output_GetDeviceOutputValue.Occluded );
    end% DeviceOutputSubsample 
  end% DeviceOutputIndex
    
  % Count the number of Plates
  ForcePlateCount = MyClient.GetForcePlateCount().ForcePlateCount;
  fprintf( 'Force Plates Count: %d\n', ForcePlateCount );
  
  Output_GetGlobalForceVector = MyClient.GetGlobalForceVector( 1, 3 );
  fprintf( 'Force (N):\n  X : %g\n  Y : %g\n  Z : %g\n\n',         ...
                     Output_GetGlobalForceVector.ForceVector( 1 ), ...
                     Output_GetGlobalForceVector.ForceVector( 2 ), ...
                     Output_GetGlobalForceVector.ForceVector( 3 ) );
  
  Output_GetGlobalMomentVector = MyClient.GetGlobalMomentVector( 1, 3 );
  fprintf( 'Moment (NM):\n  X : %g\n  Y : %g\n  Z : %g\n\n',         ...
                     Output_GetGlobalMomentVector.MomentVector( 1 ), ...
                     Output_GetGlobalMomentVector.MomentVector( 2 ), ...
                     Output_GetGlobalMomentVector.MomentVector( 3 ) );

  Output_GetGlobalCentreOfPressure = MyClient.GetGlobalCentreOfPressure( 1, 3 );
  fprintf( 'CoP (M):\n  X : %g\n  Y : %g\n  Z : %g\n\n',                     ...
                     Output_GetGlobalCentreOfPressure.CentreOfPressure( 1 ), ...
                     Output_GetGlobalCentreOfPressure.CentreOfPressure( 2 ), ...
                     Output_GetGlobalCentreOfPressure.CentreOfPressure( 3 ) );                 
                     
  % Count the number of Eye Trackers
  EyeTrackerCount = MyClient.GetEyeTrackerCount().EyeTrackerCount;
  fprintf( 'Eye Tracker Count: %d\n', EyeTrackerCount );
  
  Output_GetEyeTrackerGlobalPosition = MyClient.GetEyeTrackerGlobalPosition( 1 );
  fprintf( 'Position (mm):\n  X : %g\n  Y : %g\n  Z : %g\n\n',         ...
                     Output_GetEyeTrackerGlobalPosition.Position( 1 ), ...
                     Output_GetEyeTrackerGlobalPosition.Position( 2 ), ...
                     Output_GetEyeTrackerGlobalPosition.Position( 3 ) );
  
  Output_GetEyeTrackerGlobalGazeVector = MyClient.GetEyeTrackerGlobalGazeVector( 1 );
  fprintf( 'Gaze Vector (mm):\n  X : %g\n  Y : %g\n  Z : %g\n\n',          ...
                     Output_GetEyeTrackerGlobalGazeVector.GazeVector( 1 ), ...
                     Output_GetEyeTrackerGlobalGazeVector.GazeVector( 2 ), ...
                     Output_GetEyeTrackerGlobalGazeVector.GazeVector( 3 ) );

end% while true  

% Disconnect and dispose
MyClient.Disconnect();

% Unload the SDK
fprintf( 'Unloading SDK...' );
Client.UnloadViconDataStreamSDK();
fprintf( 'done\n' );