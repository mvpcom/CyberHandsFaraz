#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <tf/transform_broadcaster.h>
#include <sstream>

#define DEG2RAD(x) (0.017453292519*x)
#define RAD2DEG(x) (57.29577951308*x)

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

int main(int argc, char** argv){
	ros::init(argc, argv, "simple_navigation_goals");

	double x = 0.0;
	double thetadeg = 0.0;

	if(argc > 1)
	{
		std::istringstream is( argv[1] );
		is >> x;
	}
	
	if(argc>2)
	{
		std::istringstream is( argv[2] );
		is >> thetadeg;
	}
	
	//tell the action client that we want to spin a thread by default
	MoveBaseClient ac("move_base", true);
	
	//wait for the action server to come up
	while(!ac.waitForServer(ros::Duration(5.0))){
		ROS_INFO("Waiting for the move_base action server to come up");	
	}

	move_base_msgs::MoveBaseGoal goal;
	
	//we'll send a goal to the robot to move 1 meter forward
	goal.target_pose.header.frame_id = "base_link";
	goal.target_pose.header.stamp = ros::Time::now();

	goal.target_pose.pose.position.x = x;	
	geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw( DEG2RAD( thetadeg ));
	goal.target_pose.pose.orientation.x = odom_quat.x;
	goal.target_pose.pose.orientation.y = odom_quat.y;
	goal.target_pose.pose.orientation.z = odom_quat.z;
	goal.target_pose.pose.orientation.w = odom_quat.w;
		
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);

	ac.waitForResult();

	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
		ROS_INFO("Hooray, the base moved successfuly");
	else
		ROS_INFO("The base failed to move for some reason");
	return 0;
}
