#include <boost/bind.hpp>

#include <stdio.h>
//ros libraries
#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include <nav_msgs/Odometry.h>
#include "geometry_msgs/Vector3.h"
#include "geometry_msgs/Quaternion.h"
//gazebo libraries
#include "gazebo/gazebo.hh"
#include "gazebo/physics/physics.hh"
#include "gazebo/common/common.hh"
#include "gazebo/sensors/RaySensor.hh"
#include "gazebo/sensors/SensorManager.hh"
#include "gazebo/math/Vector3.hh"
#include <gazebo/math/gzmath.hh>
static float vlin=0.0, vang=0.0;
namespace gazebo
{
class ROSModelPlugin : public ModelPlugin
{
public: ROSModelPlugin()
{
// Start up ROS
// name will also be the sufix of the rostopic
// ex1: name = "robot_0", with the below cmd_vel topic, results in the topic name: /robot_0/cmd_vel
// ex2: name = "", will lead to the topic name: /cmd_vel , commonly used in simulations with only 1 robot
std::string name = "robot_0";
int argc = 0;
ros::init(argc, NULL, name);
}
public: ~ROSModelPlugin()
{
delete this->nh;
}
public: void Load(physics::ModelPtr _parent, sdf::ElementPtr /*_sdf*/)
{
// Store the pointer to the model
this->model = _parent;
// ROS Nodehandle
this->nh = new ros::NodeHandle("~");
// ROS Subscriber
this->subvel = this->nh->subscribe<geometry_msgs::Twist>("cmd_vel", 1000,
&ROSModelPlugin::VelocityCallback, this );
// ROS publishers
this->nh = new ros::NodeHandle("~");
this->pubscan = this->nh->advertise<sensor_msgs::LaserScan>("base_scan",1000);
this->nh = new ros::NodeHandle("~");
this->pubodom = this->nh->advertise<nav_msgs::Odometry>("odom", 1000);
//************** LASER MODEL ***********************
// Search the model for a sensor named "laser"
sensors::SensorPtr sensor = sensors::SensorManager::Instance()->GetSensor("laser");
if(!sensor)
printf("sensor is NULL\n");
//this->raysensor = boost::shared_dynamic_cast<sensors::RaySensor>(sensor);
this->raysensor = boost::dynamic_pointer_cast<sensors::RaySensor>(sensor);
if(!this->raysensor)
printf("raysensor is NULL\n");
//****************************************************
// Listen to the update event. This event is broadcast every
// simulation iteration.
// for gazebo 1.5 or higher it should be ConnectWorldUpdateBegin
//this->updateConnection = event::Events::ConnectWorldUpdateStart(
//boost::bind( &ROSModelPlugin::OnUpdate, this));
this->updateConnection = event::Events::ConnectWorldUpdateBegin(
boost::bind( &ROSModelPlugin::OnUpdate, this));
}
// Called by the world update start event
// this function runs on every world "iteration", the "main" function
public: void OnUpdate()
{
std::vector<double> rangesgz;
static double qw,qx,qy,qz, Rrad, Prad, Yrad;
static double px,py,pz;
// ************* QUATERNION / POSE DATA ******************
//returns the model's cartisian position
math::Vector3 p = model->GetWorldPose().pos;
px=p.x; py=p.y; pz=p.z;
//returns the model's angular position
math::Quaternion r = model->GetWorldPose().rot;
//from quaternin to Roll Pitch Yaw, in radians
qw=r.w;
qx=r.x; qy=r.y; qz=r.z;
Rrad=atan2( 2*(qw*qx+qy*qz), 1-2*(qx*qx+qy*qy) );
Prad=asin(2*(qw*qy-qz*qx));
Yrad=atan2( 2*(qw*qz+qx*qy), 1-2*(qy*qy+qz*qz) );
//Roll
//Pitch
//Yaw
// *********** ROS Times ******************************
ros::Time current_time, last_time;
current_time = ros::Time::now();
last_time = ros::Time::now();
// *********** SCAN DATA *****************************
//Publish Scan msg
sensor_msgs::LaserScan scan2ros;
scan2ros.header.stamp=current_time; //ros::Time::now();
scan2ros.header.frame_id="base_scan";
scan2ros.angle_min=this->raysensor->GetAngleMin().Radian();
scan2ros.angle_max=this->raysensor->GetAngleMax().Radian();
scan2ros.angle_increment=this->raysensor->GetAngleResolution();
scan2ros.time_increment=0.0;
scan2ros.scan_time=0.0;
scan2ros.range_min=this->raysensor->GetRangeMin();
float rmn=scan2ros.range_min;
scan2ros.range_max=this->raysensor->GetRangeMax();
float rmx=scan2ros.range_max;
// ***************************
// it is necessary to pass the whole laser values to the msg
this->raysensor->GetRanges(rangesgz);
int raynumber=this->raysensor->GetRangeCount();
scan2ros.ranges.resize(raynumber);
for (int iray=0;iray<raynumber;iray++)
{
// the laser scan subtracts, to each laser measure,
// the min range defined on the laser model
// to "correct" this when building the scan message for ROS we add
// the value of the min range if the sum is not greater than max range
float rg = this->raysensor->GetRange(iray);
if(rg+rmn<=rmx)
{scan2ros.ranges[iray]=rg+rmn;}
else {scan2ros.ranges[iray]=rmx;}
}
// **************************
// publish the message
this->pubscan.publish(scan2ros);
// *****************************************************************
// *********** ODOMETRY DATA ************************************
geometry_msgs::Quaternion odom_quat;
odom_quat.x=qx;
odom_quat.y=qy;
odom_quat.z=qz;
odom_quat.w=qw;
//publish the odometry message over to ROS
nav_msgs::Odometry odom;
odom.header.stamp =ros::Time::now(); //current_time;
odom.header.frame_id = "odom";
//set the position for odom message
odom.pose.pose.position.x = px;
odom.pose.pose.position.y = py;
odom.pose.pose.position.z = 0.0; //since the model moves in a 2D plane
odom.pose.pose.orientation = odom_quat;
//set the velocity for odom message
odom.child_frame_id = "base_link";
odom.twist.twist.linear.x = vlin;
odom.twist.twist.linear.y = 0.0;
odom.twist.twist.angular.z = vang;
//publish the message
this->pubodom.publish(odom);
// ******************************************************************
// ********* SET MODEL Velocity **************************************
//set velocities
float velx,vely;
velx=vlin*cos(Yrad);
vely=vlin*sin(Yrad);
this->model->SetLinearVel(math::Vector3(velx, vely, 0));
this->model->SetAngularVel(math::Vector3(0, 0, vang));
// ***********************************************
ros::spinOnce();
}
// This Callback function is "linked" to the cmd_vel topic created above
// Everytime data is sent to their topic Callback functions will execute
void VelocityCallback(const geometry_msgs::Twist::ConstPtr& msg)
{
// updates the model linear and angular velocities values
vlin=msg->linear.x;
vang=msg->angular.z;
}
// Pointer to the model
private: physics::ModelPtr model;
// Pointer to Laser model
private: sensors::RaySensorPtr raysensor;
// Pointer to the update event connection
private: event::ConnectionPtr updateConnection;
// *** ROS ************
// ROS Nodehandle
private: ros::NodeHandle* nh;
// ROS Subscriber
ros::Subscriber subvel;
// ROS Publisher
ros::Publisher pubscan;
ros::Publisher pubodom;
//*************
};
// Register this plugin with the simulator
GZ_REGISTER_MODEL_PLUGIN(ROSModelPlugin)
}

