#include "color.h"
#include <iostream>
#include <cctype>
#include <string>

// ROS INCLUDE
#include "tf2/exceptions.h"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"
#include <tf2_ros/create_timer_ros.h>

#include <geometry_msgs/msg/transform_stamped.hpp>
#include "geometry_msgs/msg/pose.hpp"

// EIGEN INCLUDE
#include <eigen3/Eigen/Geometry>

namespace lv
{
    // Press y to continue, n to exit (other inputs not ammitted)
    bool askContinue(const std::string &prompt = "")
    {
        char ans;
        std::cout << prompt << BOLDBLUE << "- Press y to continue or n to exit: " << RESET;
        std::cin >> ans;

        switch (tolower(ans))
        {
        case 'y':
            return true;
        case 'n':
            return false;
        default:
            std::cout << BOLDRED << "[ERROR]:" << RESET << BOLDWHITE << "Invalid input! Valid inputs are y (yes) or n (no)" << RESET << std::endl;
        }

        throw std::runtime_error("USER STOP!");
    }

    // Get homogeneous matrix from source frame to reference frame {reference_frame}_T_{source_frame}
    bool getTransform(rclcpp::Node::SharedPtr node_, const std::string &target_frame, const std::string &source_frame, geometry_msgs::msg::TransformStamped &transform)
    {
        std::unique_ptr<tf2_ros::Buffer> tf_buffer_ = std::make_unique<tf2_ros::Buffer>(node_->get_clock());
        std::shared_ptr<tf2_ros::TransformListener> tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

        auto cti = std::make_shared<tf2_ros::CreateTimerROS>(
            node_->get_node_base_interface(), node_->get_node_timers_interface());
        tf_buffer_->setCreateTimerInterface(cti);

        try
        {
            rclcpp::Time now = node_->get_clock()->now();
            tf_buffer_->waitForTransform(target_frame, source_frame, tf2::TimePointZero, tf2::durationFromSec(3.0), [](auto &) {});
            transform = tf_buffer_->lookupTransform(target_frame, source_frame, tf2::TimePointZero, tf2::durationFromSec(3.0));
            return true;
        }
        catch (const tf2::TransformException &ex)
        {
            std::cout << BOLDRED << "Could not transform " << source_frame.c_str() << " to " << target_frame.c_str() << ": " << RESET << ex.what() << std::endl;
            return false;
        }
    }

    // Visualize geometry_msgs::Transform
    void print_geometry_transform(const geometry_msgs::msg::Transform transform)
    {
        std::cout << BOLDCYAN << "Transform: " << RESET << std::endl;
        std::cout << "Translation [x, y, z]: [" << transform.translation.x << ", " << transform.translation.y << ", " << transform.translation.z << "] " << std::endl;
        std::cout << "Quaternion [x, y, z, w]: [" << transform.rotation.x << ", " << transform.rotation.y << ", " << transform.rotation.z << ", " << transform.rotation.w << "] " << std::endl;
    }

    // Convert a geometry_msgs::msg::Pose into geometry_msgs::msg::Transform
    auto geometry_2_transform(const geometry_msgs::msg::Pose &pose_)
    {
        geometry_msgs::msg::Transform transform;
        transform.translation.x = pose_.position.x;
        transform.translation.y = pose_.position.y;
        transform.translation.z = pose_.position.z;

        transform.rotation.x = pose_.orientation.x;
        transform.rotation.y = pose_.orientation.y;
        transform.rotation.z = pose_.orientation.z;
        transform.rotation.w = pose_.orientation.w;

        return transform;
    }

    // Convert a geometry_msgs::msg::Pose into Eigen::Isometry3D
    auto geometry_2_eigen(const geometry_msgs::msg::Pose &pose_)
    {
        Eigen::Quaterniond quaternion(pose_.orientation.w, pose_.orientation.x, pose_.orientation.y, pose_.orientation.z);
        Eigen::Isometry3d T(quaternion);
        T.translation().x() = pose_.position.x;
        T.translation().y() = pose_.position.y;
        T.translation().z() = pose_.position.z;

        return T;
    }

    // Convert a geometry_msgs::msg::Transform into Eigen::Isometry3D
    auto geometry_2_eigen(const geometry_msgs::msg::Transform &transform_)
    {
        Eigen::Quaterniond quaternion(transform_.rotation.w, transform_.rotation.x, transform_.rotation.y, transform_.rotation.z);
        Eigen::Isometry3d T(quaternion);
        T.translation().x() = transform_.translation.x;
        T.translation().y() = transform_.translation.y;
        T.translation().z() = transform_.translation.z;

        return T;
    }

    // Convert a Eigen::Isometry3d into geometry_msgs::msg::Pose
    auto eigen_2_geometry(Eigen::Isometry3d &transform)
    {
        geometry_msgs::msg::Pose pose_;
        Eigen::Quaterniond quaternion(transform.rotation());
        pose_.position.x = transform.translation().x();
        pose_.position.y = transform.translation().y();
        pose_.position.z = transform.translation().z();

        pose_.orientation.w = quaternion.w();
        pose_.orientation.x = quaternion.x();
        pose_.orientation.y = quaternion.y();
        pose_.orientation.z = quaternion.z();

        return pose_;
    }

}