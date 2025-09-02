#include <ros/ros.h>
#include <geometry_msgs/Pose.h>
#include <sensor_msgs/JointState.h>
#include <std_msgs/String.h>
#include <std_msgs/Bool.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <array>
#include <string>
#include <map>

uint32_t read_version(void* addr) {
    uint32_t version = 0;
    std::memcpy(&version, addr, sizeof(uint32_t));
    return version;
}

template <typename T>
bool read_from_shm_if_updated(const std::string& path, size_t size, uint32_t& prev_version, T& out_data) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) return false;

    void* addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) { close(fd); return false; }

    uint32_t current_version;
    std::memcpy(&current_version, addr, sizeof(uint32_t));

    if (current_version != prev_version) {
        std::memcpy(&out_data, (char*)addr + sizeof(uint32_t), sizeof(T));
        prev_version = current_version;
        munmap(addr, size);
        close(fd);
        return true;
    }

    munmap(addr, size);
    close(fd);
    return false;
}

bool read_string_from_shm_if_updated(const std::string& path, size_t size, uint32_t& prev_version, std::string& out_str) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) return false;

    void* addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) { close(fd); return false; }

    uint32_t current_version;
    std::memcpy(&current_version, addr, sizeof(uint32_t));

    if (current_version != prev_version) {
        char buffer[256] = {0};
        std::memcpy(buffer, (char*)addr + sizeof(uint32_t), size - sizeof(uint32_t));
        out_str = std::string(buffer);
        prev_version = current_version;
        munmap(addr, size);
        close(fd);
        return true;
    }

    munmap(addr, size);
    close(fd);
    return false;
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "shm_reader_pap_node");
    ros::NodeHandle nh;

    ros::Publisher pub_block_pose = nh.advertise<geometry_msgs::Pose>("/block_pose", 10);
    ros::Publisher pub_grab = nh.advertise<std_msgs::Bool>("/grab", 10);

    ros::Rate rate(50);

    // 버전 트래킹 변수
    uint32_t ver_block_pose = 0, ver_grab = 0;

    while (ros::ok()) {
        // 1. block_pose
        struct PoseData {
        float x, y, z;
        float ox, oy, oz, ow;
        } pose_data;

        if (read_from_shm_if_updated("/dev/shm/ros_bridge_block_pose", 32, ver_block_pose, pose_data)) {
            geometry_msgs::Pose pose_msg;
            pose_msg.position.x = pose_data.x;
            pose_msg.position.y = pose_data.y;
            pose_msg.position.z = pose_data.z;
            pose_msg.orientation.x = pose_data.ox;
            pose_msg.orientation.y = pose_data.oy;
            pose_msg.orientation.z = pose_data.oz;
            pose_msg.orientation.w = pose_data.ow;
            pub_block_pose.publish(pose_msg);
        }

        // 2. grab
        uint8_t grab_val = 0;
        if (read_from_shm_if_updated("/dev/shm/ros_bridge_grab", 5, ver_grab, grab_val)) {
            std_msgs::Bool msg;
            msg.data = grab_val != 0;
            pub_grab.publish(msg);
        }

        ros::spinOnce();
        rate.sleep();
    }

    return 0;
}
