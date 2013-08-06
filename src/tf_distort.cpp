/*

Copyright (c) 2011, Markus Achtelik, ASL, ETH Zurich, Switzerland
You can contact the author at <markus dot achtelik at mavt dot ethz dot ch>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of ETHZ-ASL nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ETHZ-ASL BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include "tf_distort.h"

namespace tf_distort{

inline void whiteGaussianNoise(double * n1, double * n2)
{
  // Box Muller Method - from http://www.dspguru.com/dsp/howtos/how-to-generate-white-gaussian-noise
  double v1, v2, s;

  do
  {
    v1 = 2 * static_cast<double> (rand()) / rand_max_ - 1.0;
    v2 = 2 * static_cast<double> (rand()) / rand_max_ - 1.0;
    s = v1 * v1 + v2 * v2;
  } while (s >= 1);

  *n1 = sqrt(-2.0 * log(s) / s) * v1;
  if (n2)
    *n2 = sqrt(-2.0 * log(s) / s) * v2;
}

inline double uniformNoise(const double & mag = 1)
{
  return (static_cast<double> (rand()) / rand_max_ * 2.0 - 1.0) * mag;
}

TfDistort::TfDistort() :
  nh_(""), pnh_("~"), pub_thread_runnning_(false)
{
  srand(ros::Time::now().toNSec());
  info_pub_ = nh_.advertise<vicon_bridge::TfDistortInfo>("tf_distort/info", 1);

  // bring up dynamic reconfigure
  reconf_srv_ = new ReconfServer(pnh_);
  reconf_srv_->setCallback(boost::bind(&TfDistort::reconfCb, this, _1, _2));

  startPubThread();
}

TfDistort::~TfDistort()
{
  stopPubThread();
  delete reconf_srv_;
}

void TfDistort::startPubThread()
{
  if (!pub_thread_runnning_)
  {
    pub_thread_runnning_ = true;
    pub_thread_ = boost::thread(&TfDistort::pubThread, this);
  }
}

void TfDistort::stopPubThread()
{
  pub_thread_runnning_ = false;
  pub_thread_.join();
}

void TfDistort::addNoise(tf::StampedTransform & tf)
{
  const double deg2rad = M_PI / 180.0;
  static ros::Time last_time = ros::Time(0);
  static bool once = false;

  if (!once)
  {
    once = true;
    last_time = tf.stamp_;
    return;
  }

  double dt = (tf.stamp_-last_time).toSec();
  double noise_x = 0, noise_y = 0, noise_z = 0, noise_roll = 0, noise_pitch = 0, noise_yaw = 0;

  if (config_.noise_type == vicon_bridge::tf_distort_NORMAL)
  {
    whiteGaussianNoise(&noise_roll, &noise_pitch);
    whiteGaussianNoise(&noise_yaw, &noise_x);
    whiteGaussianNoise(&noise_y, &noise_z);

    noise_roll *= config_.sigma_roll_pitch * deg2rad;
    noise_pitch *= config_.sigma_roll_pitch * deg2rad;
    noise_yaw *= config_.sigma_yaw * deg2rad;

    noise_x *= config_.sigma_xy;
    noise_y *= config_.sigma_xy;
    noise_z *= config_.sigma_z;
  }
  else if (config_.noise_type == vicon_bridge::tf_distort_UNIFORM)
  {
    noise_roll = uniformNoise(config_.sigma_roll_pitch * deg2rad);
    noise_pitch = uniformNoise(config_.sigma_roll_pitch * deg2rad);
    noise_yaw = uniformNoise(config_.sigma_yaw * deg2rad);
    noise_x = uniformNoise(config_.sigma_xy);
    noise_y = uniformNoise(config_.sigma_xy);
    noise_z = uniformNoise(config_.sigma_z);
  }

  noise_x += random_walk_x_.update(dt) * config_.random_walk_k_xy;
  noise_y += random_walk_y_.update(dt) * config_.random_walk_k_xy;
  noise_z += random_walk_z_.update(dt) * config_.random_walk_k_z;

  tf::Vector3 & p = tf.getOrigin();
  p.setX(p.x() + noise_x);
  p.setY(p.y() + noise_y);
  p.setZ(p.z() + noise_z);

  p *= config_.position_scale;

  tf::Quaternion q;
  q.setRPY(noise_roll, noise_pitch, noise_yaw);
  tf.setRotation(tf.getRotation() * q);

  last_time = tf.stamp_;
}


void TfDistort::reconfCb(Config & config, uint32_t level)
{
  if(tf_cb_.connected())
    tf_cb_.disconnect();

  pub_period_.fromSec(1.0/config.tf_pub_rate);
  delay_.fromNSec(config.delay*1e6);

  random_walk_x_.configure(config.random_walk_sigma_xy, config.random_walk_tau_xy);
  random_walk_y_.configure(config.random_walk_sigma_xy, config.random_walk_tau_xy);
  random_walk_z_.configure(config.random_walk_sigma_z, config.random_walk_tau_z);

  if(config.tf_publish && !tf_cb_.connected()){
    tf_cb_ = tf_listener_.addTransformsChangedListener(boost::bind(&TfDistort::tfCb, this));
  }

  if(config.tf_frame_out != config_.tf_frame_out){
    ros::NodeHandle nh;
    stopPubThread();
    pose_pub_.reset(new ros::Publisher);
    *pose_pub_= nh.advertise<geometry_msgs::TransformStamped>(config.tf_frame_out, 1);
    startPubThread();
  }

  config_ = config;
  publishInfo(config_);
}


void TfDistort::tfCb()
{
  tf::StampedTransform pose;
  static tf::StampedTransform last_pose;
  ros::Time tf_time(0);

  if (tf_listener_.canTransform(config_.tf_ref_frame, config_.tf_frame_in, tf_time))
  {
    tf_listener_.lookupTransform(config_.tf_ref_frame, config_.tf_frame_in, tf_time, pose);

    // check for new pose
    if (pose.getOrigin() != last_pose.getOrigin())
    {
      last_pose = pose;
//      ros::Time time_now = ros::Time::now();
      ros::Time time_now = pose.stamp_;

      if ((time_now - last_pub_time_) > pub_period_)
      {
        pose.child_frame_id_ = config_.tf_frame_out;
        boost::mutex::scoped_lock(tf_queue_mutex_);
        tf_queue_.push(DelayedTransform(pose, time_now + delay_));
        last_pub_time_ = time_now;
      }

    }
  }
}

void TfDistort::pubThread()
{
//  ros::Duration d(0.005);
  ros::Duration d(0.001);
  ros::Time time_now;
  uint32_t cnt = 0;
  uint32_t msg_cnt = 0;
  geometry_msgs::TransformStamped pose;

  while (nh_.ok() && pub_thread_runnning_)
  {
    cnt++;
    d.sleep();
    boost::mutex::scoped_lock lock(tf_queue_mutex_);

    if (tf_queue_.empty())
      continue;

    DelayedTransform & dt = tf_queue_.front();
    time_now = ros::Time::now();

    if (std::abs(time_now.toSec() - dt.time_to_publish.toSec()) < d.toSec() * 0.75
        || dt.time_to_publish.toSec() - time_now.toSec() < 0)
    {
      addNoise(dt.transform);
      tf_broadcaster.sendTransform(dt.transform);
      tf::transformStampedTFToMsg(dt.transform, pose);
//      {
//        boost::mutex::scoped_try_lock lock(pose_pub_mutex_);
//        if(lock.owns_lock())
          pose_pub_->publish(pose);
//      }

      tf_queue_.pop();
      msg_cnt++;
    }
    if (cnt > 1.0 / d.toSec())
    {
//      ROS_INFO("queue size: %d publishing at %d Hz", tf_queue_.size(), msg_cnt);
      msg_cnt = 0;
      cnt = 0;

      publishInfo(config_);
    }
  }
}

void TfDistort::publishInfo(const Config & config){
  vicon_bridge::TfDistortInfoPtr info(new vicon_bridge::TfDistortInfo);
  info->delay = config.delay;
  info->noise_type = config.noise_type;
  info->position_scale = config.position_scale;
  info->random_walk_k_xy = config.random_walk_k_xy;
  info->random_walk_k_z = config.random_walk_k_z;
  info->random_walk_sigma_xy = config.random_walk_sigma_xy;
  info->random_walk_sigma_z = config.random_walk_sigma_z;
  info->random_walk_tau_xy = config.random_walk_tau_xy;
  info->random_walk_tau_z = config.random_walk_tau_z;
  info->sigma_roll_pitch = config.sigma_roll_pitch;
  info->sigma_xy = config.sigma_xy;
  info->sigma_yaw = config.sigma_yaw;
  info->sigma_z = config.sigma_z;
  info->tf_frame_in = config.tf_frame_in;
  info->tf_frame_out = config.tf_frame_out;
  info->tf_pub_rate = config.tf_pub_rate;
  info->tf_ref_frame = config.tf_ref_frame;

  info_pub_.publish(info);
}

}// end namespace tf_distort

int main(int argc, char** argv)
{

  ros::init(argc, argv, "tf_distort"/*, ros::init_options::AnonymousName*/);

  tf_distort::TfDistort tfd;

  ros::spin();

  return 0;
}


