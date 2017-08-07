#include <ros/ros.h>
#include <ros/console.h>
#include <robot_state_manager.h>

int main(int argc, char** argv)
{
    ros::init(argc, argv, "robot_state");

    // Change debugging level to "debug" (i.e., print all ROS_* messages)
    if( ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Debug) ) {
       ros::console::notifyLoggerLevelsChanged(); //TODO allow user to set log level for testing not to display errors (if possible...)
    }

    RobotStateManager robot_state_manager;

    ROS_INFO("Entering main loop");
    double rate = 10; // Hz
    ros::Time::init(); // necessary to use ros::Rate, or we get an error during testing
    ros::Rate loop_rate(rate);
    while(ros::ok()){
        ros::spinOnce();
        loop_rate.sleep(); // sleep for 1/rate seconds
    }
    
    ROS_INFO("Shutting node down");
    ros::shutdown();
        
    return 0;
}