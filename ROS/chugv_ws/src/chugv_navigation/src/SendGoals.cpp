/*
 * SendGoals.cpp
 *
 *  Created on: Dec 7, 2013
 *      Author: roiyeho
 */

#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <tf/transform_datatypes.h>

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

int main(int argc, char** argv) {
	ros::init(argc, argv, "send_goals_node");

	// Read x, y and angle parameters
	double x, y, theta;
	ros::NodeHandle nh;
	nh.getParam("goal_x", x);
	nh.getParam("goal_y", y);
	nh.getParam("goal_theta", theta);

	// create the action client
	// true causes the client to spin its own thread
	MoveBaseClient ac("move_base", true);

	// Wait 60 seconds for the action server to become available
	ROS_INFO("Waiting for the move_base action server");
	ac.waitForServer(ros::Duration(60));

	ROS_INFO("Connected to move base server");

	// Send a goal to move_base
	move_base_msgs::MoveBaseGoal goal;
	goal.target_pose.header.frame_id = "map";
	goal.target_pose.header.stamp = ros::Time::now();

	//goal.target_pose.pose.position.x = 18.174;
	//goal.target_pose.pose.position.y = 28.876;
	//goal.target_pose.pose.orientation.w = 1;

	goal.target_pose.pose.position.x = x;
	goal.target_pose.pose.position.y = y;

	// Convert the Euler angle to quaternion
	double radians = theta * (M_PI/180);
	tf::Quaternion quaternion;
	quaternion = tf::createQuaternionFromYaw(radians);

	geometry_msgs::Quaternion qMsg;
	tf::quaternionTFToMsg(quaternion, qMsg);

	goal.target_pose.pose.orientation = qMsg;

	ROS_INFO("Sending goal to: x = %f, y = %f, theta = %f", x, y, theta);
	ac.sendGoal(goal);

	// Wait for the action to return
	ac.waitForResult();

	if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
		ROS_INFO("You have reached the goal!");
	else
		ROS_INFO("The base failed for some reason");

	return 0;
}





