This is a driver providing data from VICON motion capture systems. It is based on the vicon_mocap package from the starmac stacks. 
Additionally, it can handle multiple subjects / segments and allows to calibrate an origin of the vehicle(s) as this is somehow tedious with the VICON Tracker.

License: BSD
Author: Markus Achtelik <markus.achtelik@mavt.ethz.ch> 

QUICK START
roslaunch vicon_bridge vicon.launch
You may need to set the "datastream_hostport" parameter to your vicon computer's ip/hostname

OPERATION
The vicon_bridge node initiates a connection with the Vicon data source (e.g. Nexus or Tracker)
via the DataStream API. The parameter ~datastream_hostport should be set to the IP address and port (joined with a colon)
of the DataStream server machine, e.g. 192.168.0.254:801 (801 is the default port). The parameter ~stream_mode is used for the call to the DataStream SetStreamMode method;
valid values are "ServerPush", "ClientPull". 

All available subjects and segments are recognized automatically and published as tf transform and geometry_msgs::TransformStamped

ORIGIN CALIBRATION
when an object is created in vicon tracker, it places the origin more or less arbitrary in the object. 
Setting the desired origin in Tracker is somewhat tedious, therefore we added an origin calibration method. 
Place the vehicle at the vicon origin that you set during vicon calibration, and measure/estimate the height 
(z offset) of your desired origin from the floor. Then run 
>> rosrun vicon_bridge calibrate <subject name> <segment name> <z offset>
Poses are now grabbed, averaged and the vehicle is placed in the vicon's origin. The calibration parameters are stored on the parameter server as
~/<subject_name>/segment_name/zero_pose (see below). On startup, the node checks if calibration parameters are available. 
Do this the first time while running rviz and observe what happens (and if everything went correct ;-) )  


Published topics: 
- vicon/<subject_name>/<segment_name>
  publishes all available subjects/segments
  
- vicon/markers
  publishes all labeled and unlabeled markers. Labeled markers are not affected by origin calibration
  
Services:
- grab_vicon_pose
  allows to grab n poses for a given subject/segment and averages them
  
- calibrate_segment
  calls the origin calibration of a segment
  
Provided tf transforms
- vicon/<subject_name>/<segment_name>

Parameters
- stream_mode
  mode to connect to the DataStream server. Values: "ClientPull", "ServerPush". Default: "ClientPull"
- datastream_hostport
  host:port of the DataStream server
- tf_ref_frame_id
  tf reference frame id. Default: "world"
  
- ~/<subject_name>/segment_name/zero_pose/orientation/w
- ~/<subject_name>/segment_name/zero_pose/orientation/x
- ~/<subject_name>/segment_name/zero_pose/orientation/y
- ~/<subject_name>/segment_name/zero_pose/orientation/z
  quaternion of the pose at origin of the corresponding vehicle
  
- ~/<subject_name>/segment_name/zero_pose/position/x
- ~/<subject_name>/segment_name/zero_pose/position/y
- ~/<subject_name>/segment_name/zero_pose/position/z
  position of the pose at origin of the corresponding vehicle

