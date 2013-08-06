
#include <ros/ros.h>
#include <vicon_bridge/viconCalibrateSegment.h>
#include <iostream>

int main(int argc, char** argv)
{

  ros::init(argc, argv, "call_calibrate_segment");
  ros::NodeHandle nh;

  if (!(argc == 3 || argc == 4))
  {
    std::cout << "usage: calibrate <subject name> <segment name> [z offset in m]" << std::endl;
    return 0;
  }

  ros::ServiceClient client = nh.serviceClient<vicon_bridge::viconCalibrateSegment> ("vicon/calibrate_segment");

  vicon_bridge::viconCalibrateSegment::Request req;
  vicon_bridge::viconCalibrateSegment::Response resp;

  req.n_measurements = 100;

  req.subject_name = argv[1];
  req.segment_name = argv[2];

  if (argc == 4)
  {
    //		req.z_offset = atof(argv[3]);
    std::istringstream i(argv[3]);
    if (!(i >> req.z_offset))
    {
      ROS_ERROR_STREAM("cannot convert z offset to a number");
      return 0;
    }
  }
  else
    req.z_offset = 0;

  ROS_INFO("trying to calibrate %s/%s with z offset = %f m", req.subject_name.c_str(), req.segment_name.c_str(), req.z_offset);

  client.call(req, resp);

  if (resp.success)
  {
    const geometry_msgs::Quaternion & q = resp.pose.pose.orientation;
    const geometry_msgs::Point & t = resp.pose.pose.position;
    ROS_INFO("calibration successful, q=[ %f %f %f %f ], t=[ %f %f %f ] ", q.w, q.x, q.y, q.x, t.x, t.y, t.z);
  }
  else
  {
    ROS_ERROR_STREAM("calibration failed: "<<resp.status);
  }

  return 0;
}
