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



#ifndef TFDISTORT_H_
#define TFDISTORT_H_

#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>

#include <dynamic_reconfigure/server.h>
#include <vicon_bridge/tf_distortConfig.h>

#include <queue>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#if ROS_VERSION_MINIMUM(1, 11, 7)
#include <boost/signals2.hpp> // ros indigo
typedef boost::signals2::connection Connection;
#else
#include <boost/signals.hpp> // ros hydro or older
typedef boost::signals::connection Connection;
#endif

#include <vicon_bridge/TfDistortInfo.h>

namespace tf_distort{

typedef dynamic_reconfigure::Server<vicon_bridge::tf_distortConfig> ReconfServer;
typedef vicon_bridge::tf_distortConfig Config;

class DelayedTransform
{
public:
  DelayedTransform();
  DelayedTransform(const tf::StampedTransform & _transform, const ros::Time & _time) :
    transform(_transform), time_to_publish(_time)
  {
  }
  ;
  tf::StampedTransform transform;
  ros::Time time_to_publish;
};

typedef std::queue<DelayedTransform> DelayedTransformQueue;

const double rand_max_ = RAND_MAX;
void whiteGaussianNoise(double * n1, double * n2 = NULL);

class RandomWalk
{
private:
  double x_;
  double sigma_gm_;
  double tau_;
public:
  RandomWalk() :
    x_(0), sigma_gm_(0), tau_(0)
  {
    //srand(time(NULL));
  }
  void configure(const double & sigma_gm, const double & tau)
  {
    sigma_gm_ = sigma_gm;
    tau_ = tau;
  }
  double update(const double & dt)
  {
    double n;
    whiteGaussianNoise(&n);
    x_ = n * sqrt(1.0 - exp(-dt / tau_)) + x_ * exp(-dt / tau_);
    return x_;
  }
  double get()
  {
    return x_;
  }
};

class TfDistort
{
private:

  ReconfServer* reconf_srv_;
  ros::NodeHandle nh_, pnh_;
  Config config_;

  ros::Duration pub_period_;
  ros::Duration delay_;
  ros::Time last_pub_time_;

  DelayedTransformQueue tf_queue_;
  boost::mutex tf_queue_mutex_;

  Connection tf_cb_;
  tf::TransformBroadcaster tf_broadcaster;
  tf::TransformListener tf_listener_;
//  boost::mutex pose_pub_mutex_;
  boost::shared_ptr<ros::Publisher> pose_pub_; ///< pose publisher for convenience

  boost::thread pub_thread_;
  bool pub_thread_runnning_;

  RandomWalk random_walk_x_;
  RandomWalk random_walk_y_;
  RandomWalk random_walk_z_;

  ros::Publisher info_pub_;

  void startPubThread();
  void stopPubThread();
  void pubThread();
  void tfCb();
  void reconfCb(Config & config, uint32_t level);
  void addNoise(tf::StampedTransform & transform);
  void publishInfo(const Config & config);

public:
  TfDistort();
  ~TfDistort();
  void poll();
};

}// end namespace tf_distort

#endif /* TFDISTORT_H_ */
