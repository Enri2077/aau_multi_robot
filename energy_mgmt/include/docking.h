#ifndef DOCKING_H
#define DOCKING_H

#include <ros/ros.h>
#include <navfn/navfn_ros.h>
#include <costmap_2d/costmap_2d_ros.h>
#include <costmap_2d/costmap_2d.h>
#include <adhoc_communication/MmListOfPoints.h>
#include <adhoc_communication/ExpFrontier.h>
#include <adhoc_communication/EmAuction.h>
#include <adhoc_communication/EmDockingStation.h>
#include <adhoc_communication/EmRobot.h>
#include <adhoc_communication/SendEmAuction.h>
#include <adhoc_communication/SendEmDockingStation.h>
#include <adhoc_communication/SendEmRobot.h>
//#include <adhoc_communication/SendMmPoint.h>
#include <map_merger/TransformPoint.h>
#include <energy_mgmt/battery_state.h>
#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <explorer/RobotPosition.h>
#include <sstream>

#define SSTR(x) static_cast<std::ostringstream &>((std::ostringstream() << std::dec << x)).str()

#define AUCTION_TIMEOUT             5
#define FORCED_AUCTION_END_TIMEOUT  (AUCTION_TIMEOUT + 2)
#define AUCTION_RESCHEDULING_TIME   (AUCTION_TIMEOUT * 3)

using namespace std;

class docking
{
public:
    /**
     * Constructor.
     */
    docking();
    
    //F
    void detect_ds();
    void compute_optimal_ds();
    
    void robot_position_callback(const geometry_msgs::PointStamped::ConstPtr& msg);
    void adhoc_ds(const adhoc_communication::EmDockingStation::ConstPtr& msg);
    void points(const adhoc_communication::MmListOfPoints::ConstPtr& msg);
    void cb_recharge(const std_msgs::Empty& msg);
    void cb_auction_result(const adhoc_communication::EmAuction::ConstPtr &msg);
    
    void update_robot_state();
    

private:
    /**
     * The node handle.
     */
    ros::NodeHandle nh;

    /**
     * Service client for sending an auction.
     */
    ros::ServiceClient sc_send_auction, sc_send_docking_station, sc_send_robot;

    /**
     * Subscribers for the required topics.
     */
    ros::Subscriber sub_battery, sub_robots, sub_jobs, sub_docking_stations, sub_auction_starting, sub_auction_reply;

    /**
     * Callbacks for the subscribed topics.
     */
    void cb_battery(const energy_mgmt::battery_state::ConstPtr& msg);
    void cb_robots(const adhoc_communication::EmRobot::ConstPtr& msg);
    void cb_jobs(const adhoc_communication::ExpFrontier::ConstPtr& msg);
    void cb_docking_stations(const adhoc_communication::EmDockingStation::ConstPtr& msg);
    void cb_auction(const adhoc_communication::EmAuction::ConstPtr& msg);

    /**
     * Get the likelihood value as a linear combination of the values l1 to l4.
     * @return double: The likelihood value.
     */
    double get_llh();

    /**
     * Update the likelihood value l1.
     */
    void update_l1();

    /**
     * Update the likelihood value l2.
     */
    void update_l2();

    /**
     * Update the likelihood value l3.
     */
    void update_l3();

    /**
     * Update the likelihood value l4.
     */
    void update_l4();

    /**
     * Start or respond to an auction for a docking station.
     * @param int docking_station: The docking station that this robot wants to charge at.
     * @param int id: The ID of the auction. If it is left to default (i.e. 0), then a new auction is started. Otherwise this robot participates at the given auction.
     * @return bool: Success of auction.
     */
    bool auction(int docking_station, int id, int bid);

    /**
     * Send an auction to a multicast group.
     * @param string multicast_group: The multicast group to send the auction to.
     * @param adhoc_communication::EmAuction auction: The auction that will be sent.
     * @param string topic: The topic name which the auction will be published in.
     * @return bool: Success of transmission.
     */
    bool auction_send_multicast(string multicast_group, adhoc_communication::EmAuction auction, string topic);


    /**
     * Compute the length of the trajectory from the robots current position to a given goal.
     * @param double goal_x: The x-coordinate of the goal (in meters).
     * @param double goal_y: The y-coordinate of the goal (in meters).
     * @param bool euclidean: Whether or not to use euclidean distance. If it is left to default (i.e. euclidean=false), then the acutal path is calculated using Dijkstra's algorithm.
     * @return double: The distance between the robots current position and the goal (in meters).
     */
    double distance(double goal_x, double goal_y, bool euclidean=false);

    /**
     * Compute the length of the trajectory from a given start to a given goal.
     * @param double start_x: The x-coordinate of the start (in meters).
     * @param double start_y: The y-coordinate of the start (in meters).
     * @param double goal_x: The x-coordinate of the goal (in meters).
     * @param double goal_y: The y-coordinate of the goal (in meters).
     * @param bool euclidean: Whether or not to use euclidean distance. If it is left to default (i.e. euclidean=false), then the acutal path is calculated using Dijkstra's algorithm.
     * @return double: The distance between the robots current position and the goal (in meters).
     */
    double distance(double start_x, double start_y, double goal_x, double goal_y, bool euclidean=false);

    /**
     * Navigation function and costmap for calculating paths.
     */
    //navfn::NavfnROS nav;
    costmap_2d::Costmap2DROS *costmap;

    /**
     * Distance until which jobs are still considered close by (in meters).
     */
    double distance_close;

    /**
     * The coordinate frame used for calculating path lengths.
     */
    string move_base_frame;

    /**
     * Name and ID of the robot.
     */
    string robot_name, robot_prefix;
    int robot_id;

    /**
     * ID of the last auction.
     */
    int auction_id;

    /**
     * A vector of all robots with their current state.
     */
    int num_robots; // number of robots is known in simulations
    enum state_t {active, going_charging, charging, idle, in_queue_state};
    struct robot_t{
        int id;
        state_t state;
    };
    vector<robot_t> robots;
    
    enum state_next_t {going_charging_next, going_queue, exploring, stay};
    state_next_t robot_state_next;

    /**
     * A vector of all docking stations with coordinates and vacancy.
     */
    struct ds_t{
        int id;
        double x;
        double y;
        bool vacant;
    };
    vector<ds_t> ds;

    /**
     * The battery state containing time needed to fully charge the battery and time left until battery depletion.
     */
    energy_mgmt::battery_state battery;

    /**
     * A vector of all currently available jobs (e.g. frontiers for exploration).
     */
    struct job_t{
        int id;
        double x;
        double y;
    };
    vector<job_t> jobs;

    /**
     * Likelihood values for going recharging. A linear combination of the values is used in the auctions.
     */
    double l1, l2, l3, l4;

    /**
     * The weights for the weighted sum of the likelihood values l1,...,l4.
     */
    double w1, w2, w3, w4;
    
    //F
    ros::Publisher pub_ds, pub_new_target_ds, pub_auction_completed, pub_auction_winner, pub_auction_loser, pub_abort_charging;
    bool test;
    ds_t best_ds;
    ros::Subscriber sub_robot_position, sub_auction_winner_adhoc, sub_in_queue;
    double robot_x, robot_y;
    ros::ServiceServer ss_send_docking_station;
    bool foo(adhoc_communication::SendEmDockingStation::Request &req, adhoc_communication::SendEmDockingStation::Response &res);
    ros::Publisher pub_adhoc_new_best_ds, pub_auction_participation;
    ros::Subscriber sub_adhoc_new_best_ds, sub_all_points, sub_recharge, sub_check_vacancy;
    ros::ServiceClient sc_trasform;
    
    void timerCallback(const ros::TimerEvent&);
    
    ros::Timer timer_restart_auction, timer_finish_auction, timer2;
    
    struct auction_bid_t{
        int robot_id;
        float bid;
    };
    vector<auction_bid_t> auction_bids;
    
    ros::Subscriber sub_vacant_docking_station, sub_charging_completed, sub_need_charging, sub_translate, sub_vacant_ds, sub_occupied_ds, sub_ask_for_vacancy, sub_really_going_charging;
    
    void cb_charging_completed(const std_msgs::Empty& msg);
    void cb_vacant_docking_station(const adhoc_communication::EmDockingStation::ConstPtr &msg);
    void cb_need_charging(const std_msgs::Empty& msg);
    void cb_translate(const adhoc_communication::EmDockingStation::ConstPtr &msg);
    
    state_t robot_state;
    
    void timer_callback_schedure_auction_restarting(const ros::TimerEvent &event);
    
    void translate_coordinates(double a, double b, double*c, double*d);
    
    double origin_absolute_x, origin_absolute_y;
    
    bool optimal_ds_computed_once;
    
    void preload_docking_stations();
    
    bool recharging, in_queue;
    double remaining_time;
  
    void vacant_ds_callback(const std_msgs::Empty::ConstPtr&);
    void occupied_ds_callback(const std_msgs::Empty::ConstPtr&);
    
    struct auction_t {
        int robot_id;
        int auction_id;
    };
    
    vector<auction_t> auctions;
    
    void cb_auction_reply(const adhoc_communication::EmAuction::ConstPtr&);
    void really_going_charging_callback(const std_msgs::Empty::ConstPtr &msg);
    
    bool managing_auction;
    int participating_to_auction;

    void check_vacancy_callback(const std_msgs::Empty::ConstPtr &);
    void ask_for_vacancy_callback(const adhoc_communication::EmDockingStation::ConstPtr &msg);
    
    bool going_to_ds, going_to_check_if_ds_is_free, need_to_charge, charging_completed, going_charging_bool;
    
    std::vector<ros::Timer> timers;
    
    void end_auction_participation_timer_callback(const ros::TimerEvent &event); 
    
    void robot_pose_callback(const geometry_msgs::PoseWithCovarianceStampedConstPtr &pose);
    
    
    ros::Publisher pub_lost_own_auction, pub_won_auction, pub_lost_other_robot_auction;
    bool lost_own_auction, auction_winner, lost_other_robot_auction, update_state_required;
    void robot_in_queue_callback(const std_msgs::Empty::ConstPtr &msg);
    ros::Subscriber sub_robot_in_queue, sub_abort_charging ;
    void abort_charging_callback(const std_msgs::Empty &msg);
    
    ros::Subscriber sub_robot_pose;
    ros::ServiceClient sc_robot_pose;
    
    ds_t next_optimal_ds, target_ds, next_target_ds;

};

#endif  /* DOCKING_H */