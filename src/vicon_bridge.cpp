/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2010, UC Regents
 *  Copyright (c) 2011, Markus Achtelik, ETH Zurich, Autonomous Systems Lab (modifications)
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the University of California nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

#include <iostream>
#include <map>
#include <unordered_map>

// ROS
#include <ros/ros.h>
#include <tf/tf.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/TransformStamped.h>
#include <vicon_bridge/viconGrabPose.h>
#include <vicon_bridge/Markers.h>
#include <vicon_bridge/Marker.h>
#include <vicon_bridge/viconCalibrateSegment.h>

// Vicon
#include <ViconDataStreamSDK_CPP/DataStreamClient.h>

#include <diagnostic_updater/diagnostic_updater.h>
#include <diagnostic_updater/update_functions.h>

using std::min;
using std::max;
using std::string;
using std::map;

using namespace ViconDataStreamSDK::CPP;

string Adapt(const Direction::Enum i_Direction)
{
  switch (i_Direction)
  {
    case Direction::Forward:
      return "Forward";
    case Direction::Backward:
      return "Backward";
    case Direction::Left:
      return "Left";
    case Direction::Right:
      return "Right";
    case Direction::Up:
      return "Up";
    case Direction::Down:
      return "Down";
    default:
      return "Unknown";
  }
}

string Adapt(const Result::Enum i_result)
{
  switch (i_result)
  {
    case Result::ClientAlreadyConnected:
      return "ClientAlreadyConnected";
    case Result::ClientConnectionFailed:
      return "";
    case Result::CoLinearAxes:
      return "CoLinearAxes";
    case Result::InvalidDeviceName:
      return "InvalidDeviceName";
    case Result::InvalidDeviceOutputName:
      return "InvalidDeviceOutputName";
    case Result::InvalidHostName:
      return "InvalidHostName";
    case Result::InvalidIndex:
      return "InvalidIndex";
    case Result::InvalidLatencySampleName:
      return "InvalidLatencySampleName";
    case Result::InvalidMarkerName:
      return "InvalidMarkerName";
    case Result::InvalidMulticastIP:
      return "InvalidMulticastIP";
    case Result::InvalidSegmentName:
      return "InvalidSegmentName";
    case Result::InvalidSubjectName:
      return "InvalidSubjectName";
    case Result::LeftHandedAxes:
      return "LeftHandedAxes";
    case Result::NoFrame:
      return "NoFrame";
    case Result::NotConnected:
      return "NotConnected";
    case Result::NotImplemented:
      return "NotImplemented";
    case Result::ServerAlreadyTransmittingMulticast:
      return "ServerAlreadyTransmittingMulticast";
    case Result::ServerNotTransmittingMulticast:
      return "ServerNotTransmittingMulticast";
    case Result::Success:
      return "Success";
    case Result::Unknown:
      return "Unknown";
    default:
      return "unknown";
  }
}

class SegmentPublisher
{
public:
  ros::Publisher pub;
  bool is_ready;
  tf::Transform calibration_pose;
  bool calibrated;
  SegmentPublisher() :
    is_ready(false), calibration_pose(tf::Pose::getIdentity()),
        calibrated(false)
  {
  }
  ;
};

typedef map<string, SegmentPublisher> SegmentMap;

class ViconReceiver
{
private:
  ros::NodeHandle nh;
  ros::NodeHandle nh_priv;
  // Diagnostic Updater
  diagnostic_updater::Updater diag_updater;
  double min_freq_;
  double max_freq_;
  diagnostic_updater::FrequencyStatus freq_status_;
  // Parameters:
  string stream_mode_;
  string host_name_;
  string tf_ref_frame_id_;
  string tracked_frame_suffix_;
  // Publisher
  ros::Publisher marker_pub_;
  // TF Broadcaster
  tf::TransformBroadcaster tf_broadcaster_;
  //geometry_msgs::PoseStamped vicon_pose;
  tf::Transform flyer_transform;
  ros::Time now_time;
  // TODO: Make the following configurable:
  ros::ServiceServer m_grab_vicon_pose_service_server;
  ros::ServiceServer calibrate_segment_server_;
  unsigned int lastFrameNumber;
  unsigned int frameCount;
  unsigned int droppedFrameCount;
  ros::Time time_datum;
  unsigned int frame_datum;
  unsigned int n_markers;
  unsigned int n_unlabeled_markers;
  bool segment_data_enabled;
  bool marker_data_enabled;
  bool unlabeled_marker_data_enabled;

  bool broadcast_tf_, publish_tf_, publish_markers_;

  bool grab_frames_;
  // boost::thread grab_frames_thread_;
  // std::unordered_map<std::string, ros::Publisher> segment_publishers_;
  SegmentMap segment_publishers_;
  boost::mutex segments_mutex_;
  std::vector<std::string> time_log_;

  Client vicon_client_;

public:
  void startGrabbing()
  {
    grab_frames_ = true;
    // test grabbing in the main loop and run an asynchronous spinner instead
    grabThread();
    //grab_frames_thread_ = boost::thread(&ViconReceiver::grabThread, this);
  }

  void stopGrabbing()
  {
    grab_frames_ = false;
    //grab_frames_thread_.join();
  }

  ViconReceiver() :
    nh_priv("~"),
    diag_updater(),
    min_freq_(0.1), max_freq_(1000),
    freq_status_(diagnostic_updater::FrequencyStatusParam(&min_freq_, &max_freq_)),
    stream_mode_("ClientPull"),
        host_name_(""), tf_ref_frame_id_("world"), tracked_frame_suffix_("vicon"),
        lastFrameNumber(0), frameCount(0), droppedFrameCount(0), frame_datum(0), n_markers(0), n_unlabeled_markers(0),
        marker_data_enabled(false), unlabeled_marker_data_enabled(false), grab_frames_(false)
  {
    // Diagnostics
    diag_updater.add("ViconReceiver Status", this, &ViconReceiver::diagnostics);
    diag_updater.add(freq_status_);
    diag_updater.setHardwareID("none");
    diag_updater.force_update();
    // Parameters
    nh_priv.param("stream_mode", stream_mode_, stream_mode_);
    nh_priv.param("datastream_hostport", host_name_, host_name_);
    nh_priv.param("tf_ref_frame_id", tf_ref_frame_id_, tf_ref_frame_id_);
    nh_priv.param("broadcast_transform", broadcast_tf_, true);
    nh_priv.param("publish_transform", publish_tf_, true);
    nh_priv.param("publish_markers", publish_markers_, true);
    if (init_vicon() == false){
      ROS_ERROR("Error while connecting to Vicon. Exiting now.");
      return;
    }
    // Service Server
    ROS_INFO("setting up grab_vicon_pose service server ... ");
    m_grab_vicon_pose_service_server = nh_priv.advertiseService("grab_vicon_pose", &ViconReceiver::grabPoseCallback,
                                                                this);

    ROS_INFO("setting up segment calibration service server ... ");
    calibrate_segment_server_ = nh_priv.advertiseService("calibrate_segment", &ViconReceiver::calibrateSegmentCallback,
                                                         this);

    // Publishers
    if(publish_markers_)
    {
      marker_pub_ = nh.advertise<vicon_bridge::Markers>(tracked_frame_suffix_ + "/markers", 10);
    }
    startGrabbing();
  }

  ~ViconReceiver()
  {
    for (size_t i = 0; i < time_log_.size(); i++)
    {
      std::cout << time_log_[i] << std::endl;
    }
    if (shutdown_vicon() == false){
      ROS_ERROR("Error while shutting down Vicon.");
    }
  }

private:
  void diagnostics(diagnostic_updater::DiagnosticStatusWrapper& stat)
  {
    stat.summary(diagnostic_msgs::DiagnosticStatus::OK, "OK");
    stat.add("latest VICON frame number", lastFrameNumber);
    stat.add("dropped frames", droppedFrameCount);
    stat.add("framecount", frameCount);
    stat.add("# markers", n_markers);
    stat.add("# unlabeled markers", n_unlabeled_markers);
  }

  bool init_vicon()
  {
    ROS_INFO_STREAM("Connecting to Vicon DataStream SDK at " << host_name_ << " ...");

    ros::Duration d(1);
    Result::Enum result(Result::Unknown);

    while (!vicon_client_.IsConnected().Connected)
    {
      vicon_client_.Connect(host_name_);
      ROS_INFO(".");
      d.sleep();
      ros::spinOnce();
      if (!ros::ok())
        return false;
    }
    ROS_ASSERT(vicon_client_.IsConnected().Connected);
    ROS_INFO_STREAM("... connected!");

    // ClientPullPrefetch doesn't make much sense here, since we're only forwarding the data
    if (stream_mode_ == "ServerPush")
    {
      result = vicon_client_.SetStreamMode(StreamMode::ServerPush).Result;
    }
    else if (stream_mode_ == "ClientPull")
    {
      result = vicon_client_.SetStreamMode(StreamMode::ClientPull).Result;
    }
    else
    {
      ROS_FATAL("Unknown stream mode -- options are ServerPush, ClientPull");
      ros::shutdown();
    }

    ROS_INFO_STREAM("Setting Stream Mode to " << stream_mode_<< ": "<< Adapt(result));

    vicon_client_.SetAxisMapping(Direction::Forward, Direction::Left, Direction::Up); // 'Z-up'
    Output_GetAxisMapping _Output_GetAxisMapping = vicon_client_.GetAxisMapping();

    ROS_INFO_STREAM("Axis Mapping: X-" << Adapt(_Output_GetAxisMapping.XAxis) << " Y-"
        << Adapt(_Output_GetAxisMapping.YAxis) << " Z-" << Adapt(_Output_GetAxisMapping.ZAxis));

    vicon_client_.EnableSegmentData();
    ROS_ASSERT(vicon_client_.IsSegmentDataEnabled().Enabled);

    Output_GetVersion _Output_GetVersion = vicon_client_.GetVersion();
    ROS_INFO_STREAM("Version: " << _Output_GetVersion.Major << "." << _Output_GetVersion.Minor << "."
        << _Output_GetVersion.Point);
    return true;
  }

  void createSegmentThread(const string subject_name, const string segment_name)
  {
    ROS_INFO("creating new object %s/%s ...",subject_name.c_str(), segment_name.c_str() );
    boost::mutex::scoped_lock lock(segments_mutex_);
    SegmentPublisher & spub = segment_publishers_[subject_name + "/" + segment_name];

    // we don't need the lock anymore, since rest is protected by is_ready
    lock.unlock();

    if(publish_tf_)
    {
      spub.pub = nh.advertise<geometry_msgs::TransformStamped>(tracked_frame_suffix_ + "/" + subject_name + "/"
                                                                                                            + segment_name, 10);
    }
    // try to get zero pose from parameter server
    string param_suffix(subject_name + "/" + segment_name + "/zero_pose/");
    double qw, qx, qy, qz, x, y, z;
    bool have_params = true;
    have_params = have_params && nh_priv.getParam(param_suffix + "orientation/w", qw);
    have_params = have_params && nh_priv.getParam(param_suffix + "orientation/x", qx);
    have_params = have_params && nh_priv.getParam(param_suffix + "orientation/y", qy);
    have_params = have_params && nh_priv.getParam(param_suffix + "orientation/z", qz);
    have_params = have_params && nh_priv.getParam(param_suffix + "position/x", x);
    have_params = have_params && nh_priv.getParam(param_suffix + "position/y", y);
    have_params = have_params && nh_priv.getParam(param_suffix + "position/z", z);

    if (have_params)
    {
      ROS_INFO("loaded zero pose for %s/%s", subject_name.c_str(), segment_name.c_str());
      spub.calibration_pose.setRotation(tf::Quaternion(qx, qy, qz, qw));
      spub.calibration_pose.setOrigin(tf::Vector3(x, y, z));
      spub.calibration_pose = spub.calibration_pose.inverse();
    }
    else
    {
      ROS_WARN("unable to load zero pose for %s/%s", subject_name.c_str(), segment_name.c_str());
      spub.calibration_pose.setIdentity();
    }

    spub.is_ready = true;
    ROS_INFO("... done, advertised as \" %s/%s/%s\" ", tracked_frame_suffix_.c_str(), subject_name.c_str(), segment_name.c_str());

  }

  void createSegment(const string subject_name, const string segment_name)
  {
    boost::thread(&ViconReceiver::createSegmentThread, this, subject_name, segment_name);
  }

  void grabThread()
  {
    ros::Duration d(1.0 / 240.0);  // TODO: Configurable

    while (ros::ok() && grab_frames_)
    {
      while (vicon_client_.GetFrame().Result != Result::Success && ros::ok())
      {
        ROS_INFO("getFrame returned false");
        d.sleep();
      }
      now_time = ros::Time::now();

      bool was_new_frame = process_frame();
      ROS_WARN_COND(!was_new_frame, "grab frame returned false");

      diag_updater.update();
    }
  }

  bool shutdown_vicon()
  {
    ROS_INFO_STREAM("stopping grabbing thread");
    stopGrabbing();
    ROS_INFO_STREAM("Disconnecting from Vicon DataStream SDK");
    vicon_client_.Disconnect();
    ROS_ASSERT(!vicon_client_.IsConnected().Connected);
    ROS_INFO_STREAM("... disconnected.");
    return true;
  }

  bool process_frame()
  {
    static ros::Time lastTime;
    Output_GetFrameNumber OutputFrameNum = vicon_client_.GetFrameNumber();

    //frameCount++;
    //ROS_INFO_STREAM("Grabbed a frame: " << OutputFrameNum.FrameNumber);
    int frameDiff = 0;
    if (lastFrameNumber != 0)
    {
      frameDiff = OutputFrameNum.FrameNumber - lastFrameNumber;
      frameCount += frameDiff;
      if ((frameDiff) > 1)
      {
        droppedFrameCount += frameDiff;
        double droppedFramePct = (double)droppedFrameCount / frameCount * 100;
        ROS_DEBUG_STREAM(frameDiff << " more (total " << droppedFrameCount << "/" << frameCount << ", "
            << droppedFramePct << "%) frame(s) dropped. Consider adjusting rates.");
      }
    }
    lastFrameNumber = OutputFrameNum.FrameNumber;

    if (frameDiff == 0)
    {
      return false;
    }
    else
    {
      freq_status_.tick();
      ros::Duration vicon_latency(vicon_client_.GetLatencyTotal().Total);

      if(publish_tf_ || broadcast_tf_)
      {
        process_subjects(now_time - vicon_latency);
      }

      if(publish_markers_)
      {
        process_markers(now_time - vicon_latency, lastFrameNumber);
      }

      lastTime = now_time;
      return true;
    }
  }

  void process_subjects(const ros::Time& frame_time)
  {
    string tracked_frame, subject_name, segment_name;
    unsigned int n_subjects = vicon_client_.GetSubjectCount().SubjectCount;
    SegmentMap::iterator pub_it;
    tf::Transform transform;
    std::vector<tf::StampedTransform, std::allocator<tf::StampedTransform> > transforms;
    geometry_msgs::TransformStampedPtr pose_msg(new geometry_msgs::TransformStamped);
    static unsigned int cnt = 0;

    for (unsigned int i_subjects = 0; i_subjects < n_subjects; i_subjects++)
    {

      subject_name = vicon_client_.GetSubjectName(i_subjects).SubjectName;
      unsigned int n_segments = vicon_client_.GetSegmentCount(subject_name).SegmentCount;

      for (unsigned int i_segments = 0; i_segments < n_segments; i_segments++)
      {
        segment_name = vicon_client_.GetSegmentName(subject_name, i_segments).SegmentName;

        Output_GetSegmentGlobalTranslation trans = vicon_client_.GetSegmentGlobalTranslation(subject_name, segment_name);
        Output_GetSegmentGlobalRotationQuaternion quat = vicon_client_.GetSegmentGlobalRotationQuaternion(subject_name,
                                                                                                        segment_name);

        if (trans.Result == Result::Success && quat.Result == Result::Success)
        {
          if (!trans.Occluded && !quat.Occluded)
          {
            transform.setOrigin(tf::Vector3(trans.Translation[0] / 1000, trans.Translation[1] / 1000,
                                                  trans.Translation[2] / 1000));
            transform.setRotation(tf::Quaternion(quat.Rotation[0], quat.Rotation[1], quat.Rotation[2],
                                                       quat.Rotation[3]));

            tracked_frame = tracked_frame_suffix_ + "/" + subject_name + "/" + segment_name;

            boost::mutex::scoped_try_lock lock(segments_mutex_);

            if (lock.owns_lock())
            {
              pub_it = segment_publishers_.find(subject_name + "/" + segment_name);
              if (pub_it != segment_publishers_.end())
              {
                SegmentPublisher & seg = pub_it->second;
                //ros::Time thisTime = now_time - ros::Duration(latencyInMs / 1000);

                if (seg.is_ready)
                {
                  transform = transform * seg.calibration_pose;
                  transforms.push_back(tf::StampedTransform(transform, frame_time, tf_ref_frame_id_, tracked_frame));
//                  transform = tf::StampedTransform(flyer_transform, frame_time, tf_ref_frame_id_, tracked_frame);
//                  tf_broadcaster_.sendTransform(transform);

                  if(publish_tf_)
                  {
                    tf::transformStampedTFToMsg(transforms.back(), *pose_msg);
                    seg.pub.publish(pose_msg);
                  }
                }
              }
              else
              {
                lock.unlock();
                createSegment(subject_name, segment_name);
              }
            }
          }
          else
          {
            if (cnt % 100 == 0)
              ROS_WARN_STREAM("" << subject_name <<" occluded, not publishing... " );
          }
        }
        else
        {
          ROS_WARN("GetSegmentGlobalTranslation/Rotation failed (result = %s, %s), not publishing...",
              Adapt(trans.Result).c_str(), Adapt(quat.Result).c_str());
        }
      }
    }

    if(broadcast_tf_)
    {
      tf_broadcaster_.sendTransform(transforms);
    }
    cnt++;
  }

  void process_markers(const ros::Time& frame_time, unsigned int vicon_frame_num)
  {
    if (marker_pub_.getNumSubscribers() > 0)
    {
      if (!marker_data_enabled)
      {
        vicon_client_.EnableMarkerData();
        ROS_ASSERT(vicon_client_.IsMarkerDataEnabled().Enabled);
        marker_data_enabled = true;
      }
      if (!unlabeled_marker_data_enabled)
      {
        vicon_client_.EnableUnlabeledMarkerData();
        ROS_ASSERT(vicon_client_.IsUnlabeledMarkerDataEnabled().Enabled);
        unlabeled_marker_data_enabled = true;
      }
      n_markers = 0;
      vicon_bridge::Markers markers_msg;
      markers_msg.header.stamp = frame_time;
      markers_msg.frame_number = vicon_frame_num;
      // Count the number of subjects
      unsigned int SubjectCount = vicon_client_.GetSubjectCount().SubjectCount;
      // Get labeled markers
      for (unsigned int SubjectIndex = 0; SubjectIndex < SubjectCount; ++SubjectIndex)
      {
        std::string this_subject_name = vicon_client_.GetSubjectName(SubjectIndex).SubjectName;
        // Count the number of markers
        unsigned int num_subject_markers = vicon_client_.GetMarkerCount(this_subject_name).MarkerCount;
        n_markers += num_subject_markers;
        //std::cout << "    Markers (" << MarkerCount << "):" << std::endl;
        for (unsigned int MarkerIndex = 0; MarkerIndex < num_subject_markers; ++MarkerIndex)
        {
          vicon_bridge::Marker this_marker;
          this_marker.marker_name = vicon_client_.GetMarkerName(this_subject_name, MarkerIndex).MarkerName;
          this_marker.subject_name = this_subject_name;
          this_marker.segment_name
              = vicon_client_.GetMarkerParentName(this_subject_name, this_marker.marker_name).SegmentName;

          // Get the global marker translation
          Output_GetMarkerGlobalTranslation _Output_GetMarkerGlobalTranslation =
              vicon_client_.GetMarkerGlobalTranslation(this_subject_name, this_marker.marker_name);

          this_marker.translation.x = _Output_GetMarkerGlobalTranslation.Translation[0];
          this_marker.translation.y = _Output_GetMarkerGlobalTranslation.Translation[1];
          this_marker.translation.z = _Output_GetMarkerGlobalTranslation.Translation[2];
          this_marker.occluded = _Output_GetMarkerGlobalTranslation.Occluded;

          markers_msg.markers.push_back(this_marker);
        }
      }
      // get unlabeled markers
      unsigned int UnlabeledMarkerCount = vicon_client_.GetUnlabeledMarkerCount().MarkerCount;
      //ROS_INFO("# unlabeled markers: %d", UnlabeledMarkerCount);
      n_markers += UnlabeledMarkerCount;
      n_unlabeled_markers = UnlabeledMarkerCount;
      for (unsigned int UnlabeledMarkerIndex = 0; UnlabeledMarkerIndex < UnlabeledMarkerCount; ++UnlabeledMarkerIndex)
      {
        // Get the global marker translation
        Output_GetUnlabeledMarkerGlobalTranslation _Output_GetUnlabeledMarkerGlobalTranslation =
            vicon_client_.GetUnlabeledMarkerGlobalTranslation(UnlabeledMarkerIndex);

        if (_Output_GetUnlabeledMarkerGlobalTranslation.Result == Result::Success)
        {
          vicon_bridge::Marker this_marker;
          this_marker.translation.x = _Output_GetUnlabeledMarkerGlobalTranslation.Translation[0];
          this_marker.translation.y = _Output_GetUnlabeledMarkerGlobalTranslation.Translation[1];
          this_marker.translation.z = _Output_GetUnlabeledMarkerGlobalTranslation.Translation[2];
          this_marker.occluded = false; // unlabeled markers can't be occluded
          markers_msg.markers.push_back(this_marker);
        }
        else
        {
          ROS_WARN("GetUnlabeledMarkerGlobalTranslation failed (result = %s)",
              Adapt(_Output_GetUnlabeledMarkerGlobalTranslation.Result).c_str());
        }
      }
      marker_pub_.publish(markers_msg);
    }
  }

  bool grabPoseCallback(vicon_bridge::viconGrabPose::Request& req, vicon_bridge::viconGrabPose::Response& resp)
  {
    ROS_INFO("Got request for a VICON pose");
    tf::TransformListener tf_listener;
    tf::StampedTransform transform;
    tf::Quaternion orientation(0, 0, 0, 0);
    tf::Vector3 position(0, 0, 0);

    string tracked_segment = tracked_frame_suffix_ + "/" + req.subject_name + "/" + req.segment_name;

    // Gather data:
    int N = req.n_measurements;
    int n_success = 0;
    ros::Duration timeout(0.1);
    ros::Duration poll_period(1.0 / 240.0);

    for (int k = 0; k < N; k++)
    {
      try
      {
        if (tf_listener.waitForTransform(tf_ref_frame_id_, tracked_segment, ros::Time::now(), timeout, poll_period))
        {
          tf_listener.lookupTransform(tf_ref_frame_id_, tracked_segment, ros::Time(0), transform);
          orientation += transform.getRotation();
          position += transform.getOrigin();
          n_success++;
        }
      }
      catch (tf::TransformException ex)
      {
        ROS_ERROR("%s", ex.what());
        //    		resp.success = false;
        //    		return false; // TODO: should we really bail here, or just try again?
      }
    }

    // Average the data
    orientation /= n_success;
    orientation.normalize();
    position /= n_success;

    // copy what we used to service call response:
    resp.success = true;
    resp.pose.header.stamp = ros::Time::now();
    resp.pose.header.frame_id = tf_ref_frame_id_;
    resp.pose.pose.position.x = position.x();
    resp.pose.pose.position.y = position.y();
    resp.pose.pose.position.z = position.z();
    resp.pose.pose.orientation.w = orientation.w();
    resp.pose.pose.orientation.x = orientation.x();
    resp.pose.pose.orientation.y = orientation.y();
    resp.pose.pose.orientation.z = orientation.z();

    return true;
  }

  bool calibrateSegmentCallback(vicon_bridge::viconCalibrateSegment::Request& req,
                                vicon_bridge::viconCalibrateSegment::Response& resp)
  {

    std::string full_name = req.subject_name + "/" + req.segment_name;
    ROS_INFO("trying to calibrate %s", full_name.c_str());

    SegmentMap::iterator seg_it = segment_publishers_.find(full_name);

    if (seg_it == segment_publishers_.end())
    {
      ROS_WARN("frame %s not found --> not calibrating", full_name.c_str());
      resp.success = false;
      resp.status = "segment " + full_name + " not found";
      return false;
    }

    SegmentPublisher & seg = seg_it->second;

    if (seg.calibrated)
    {
      ROS_INFO("%s already calibrated, deleting old calibration", full_name.c_str());
      seg.calibration_pose.setIdentity();
    }

    vicon_bridge::viconGrabPose::Request grab_req;
    vicon_bridge::viconGrabPose::Response grab_resp;

    grab_req.n_measurements = req.n_measurements;
    grab_req.subject_name = req.subject_name;
    grab_req.segment_name = req.segment_name;

    bool ret = grabPoseCallback(grab_req, grab_resp);

    if (!ret)
    {
      resp.success = false;
      resp.status = "error while grabbing pose from Vicon";
      return false;
    }

    tf::Transform t;
    t.setOrigin(tf::Vector3(grab_resp.pose.pose.position.x, grab_resp.pose.pose.position.y,
                            grab_resp.pose.pose.position.z - req.z_offset));
    t.setRotation(tf::Quaternion(grab_resp.pose.pose.orientation.x, grab_resp.pose.pose.orientation.y,
                                 grab_resp.pose.pose.orientation.z, grab_resp.pose.pose.orientation.w));

    seg.calibration_pose = t.inverse();

    // write zero_pose to parameter server
    string param_suffix(full_name + "/zero_pose/");
    nh_priv.setParam(param_suffix + "orientation/w", t.getRotation().w());
    nh_priv.setParam(param_suffix + "orientation/x", t.getRotation().x());
    nh_priv.setParam(param_suffix + "orientation/y", t.getRotation().y());
    nh_priv.setParam(param_suffix + "orientation/z", t.getRotation().z());

    nh_priv.setParam(param_suffix + "position/x", t.getOrigin().x());
    nh_priv.setParam(param_suffix + "position/y", t.getOrigin().y());
    nh_priv.setParam(param_suffix + "position/z", t.getOrigin().z());

    ROS_INFO_STREAM("calibration completed");
    resp.pose = grab_resp.pose;
    resp.success = true;
    resp.status = "calibration successful";
    seg.calibrated = true;

    return true;
  }
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "vicon");

  ros::AsyncSpinner aspin(1);
  aspin.start();
  ViconReceiver vr;
  aspin.stop();
  return 0;
}
