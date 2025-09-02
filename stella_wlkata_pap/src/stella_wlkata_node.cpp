#include <string>
#include <ros/ros.h>
#include <serial/serial.h>
#include <std_msgs/String.h>
#include <std_msgs/Empty.h>
#include <std_msgs/UInt16.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Bool.h>
#include <sensor_msgs/JointState.h>
#include <geometry_msgs/Pose.h>
#include <math.h>
#include <unistd.h>
#include <tf/transform_datatypes.h>


serial::Serial _serial;				// serial object
bool HomingComplete = false;
bool CommandComplete = false;
bool MoveComplete = false;
bool GripperComplete = false;
bool ArmMoveComplete = false;
bool CanMove = false;
bool GrabStart = false;

//pick and place?
void grab_write_callback(const std_msgs::Bool msg)
{
	std::string Gcode = "";

	if(msg.data==true) GrabStart = true;
	else
	{
		Gcode = (std::string)"M21 G90 G01 X0.00 Y38.90 Z21.30 A0.00 B-60.30 C0.00 F2000.00" + "\r\n";
		_serial.write(Gcode.c_str());
		while (ros::ok())
		{
			Gcode = (std::string)"?" + "\r\n";
			_serial.write(Gcode.c_str());
			ros::Duration(0.1).sleep();
			std_msgs::String result;
			result.data = _serial.read(_serial.available());
			ArmMoveComplete = (result.data.npos == result.data.find("Idle"))?false:true;
			if(ArmMoveComplete == true)break;
		}
		ros::Duration(1).sleep();

		Gcode = (std::string)"M21 G90 G01 X0.00 Y48.00 Z17.30 A0.00 B-65.30 C0.00 F2000.00" + "\r\n";
		_serial.write(Gcode.c_str());
		while (ros::ok())
		{
			Gcode = (std::string)"?" + "\r\n";
			_serial.write(Gcode.c_str());
			ros::Duration(0.1).sleep();
			std_msgs::String result;
			result.data = _serial.read(_serial.available());
			ArmMoveComplete = (result.data.npos == result.data.find("Idle"))?false:true;
			if(ArmMoveComplete == true)break;
		}
		ros::Duration(1).sleep();

		Gcode = (std::string)"M3S40" + "\r\n";
                _serial.write(Gcode.c_str());
                while (ros::ok())
                {
                        ros::Duration(0.1).sleep();
                        std_msgs::String result;
                        result.data = _serial.read(_serial.available());
                        GripperComplete = (result.data.npos == result.data.find("ok"))?false:true;
                        if(GripperComplete == true)break;
                }
                ros::Duration(1).sleep();

		Gcode = (std::string)"M21 G90 G01 X0.00 Y38.90 Z21.30 A0.00 B-60.30 C0.00 F2000.00" + "\r\n";
                _serial.write(Gcode.c_str());
                while (ros::ok())
                {
                        Gcode = (std::string)"?" + "\r\n";
                        _serial.write(Gcode.c_str());
                        ros::Duration(0.1).sleep();
                        std_msgs::String result;
                        result.data = _serial.read(_serial.available());
                        ArmMoveComplete = (result.data.npos == result.data.find("Idle"))?false:true;
                        if(ArmMoveComplete == true)break;
                }
                ros::Duration(1).sleep();

		Gcode = (std::string)"M21 G90 G01 X90.00 Y-40.00 Z40.00 A0.00 B0.00 C0.00 F2000.00" + "\r\n";
                _serial.write(Gcode.c_str());
                while (ros::ok())
                {
                        Gcode = (std::string)"?" + "\r\n";
                        _serial.write(Gcode.c_str());
                        ros::Duration(0.1).sleep();
                        std_msgs::String result;
                        result.data = _serial.read(_serial.available());
                        ArmMoveComplete = (result.data.npos == result.data.find("Idle"))?false:true;
                        if(ArmMoveComplete == true)break;
                }
                ros::Duration(1).sleep();

		Gcode = (std::string)"M3S60" + "\r\n";
                _serial.write(Gcode.c_str());
                while (ros::ok())
                {
                        ros::Duration(0.1).sleep();
                        std_msgs::String result;
                        result.data = _serial.read(_serial.available());
                        GripperComplete = (result.data.npos == result.data.find("ok"))?false:true;
                        if(GripperComplete == true)break;
                }
                ros::Duration(1).sleep();
	}
}

//pick and place? 2
void block_pose_write_callback(const geometry_msgs::Pose msg)
{
	if(GrabStart == true)
	{
		std::string Gcode = "";
		char poseX[10];
		char poseY[10];
		char poseZ[10];

		float x,y,z;
		x = msg.position.x * 1000;
		y = msg.position.y * 1000;
		z = 50;

		sprintf(poseX, "%.2f", x);
		sprintf(poseY, "%.2f", y);
		sprintf(poseZ, "%.2f", z);

		if(HomingComplete == true && CanMove == true)
		{
			Gcode = (std::string)"M3S40" + "\r\n";
	        	_serial.write(Gcode.c_str());
	                while (ros::ok())
	                {
	                        ros::Duration(0.1).sleep();
	                        std_msgs::String result;
	                        result.data = _serial.read(_serial.available());
	                        GripperComplete = (result.data.npos == result.data.find("ok"))?false:true;
	                        if(GripperComplete == true)break;
	                }
			ros::Duration(1).sleep();

	        	Gcode = (std::string)"M20 G90 G01 X" + poseX + " Y" + poseY + " Z" + poseZ + "A  0.00  B  0.00  C  0.00" + " F2000" + "\r\n";
			_serial.write(Gcode.c_str());
			while (ros::ok())
			{
				ros::Duration(0.1).sleep();
				std_msgs::String result;
				result.data = _serial.read(_serial.available());
				CommandComplete = (result.data.npos == result.data.find("ok"))?false:true;
				if(CommandComplete == true)break;
			}
			ros::Duration(0.1).sleep();

			while (ros::ok())
			{
				Gcode = (std::string)"?" + "\r\n";
				_serial.write(Gcode.c_str());
				ros::Duration(0.1).sleep();
				std_msgs::String result;
				result.data = _serial.read(_serial.available());
				ArmMoveComplete = (result.data.npos == result.data.find("Idle"))?false:true;
				if(ArmMoveComplete == true)break;
			}
			ros::Duration(1).sleep();

			z = 30;
			sprintf(poseZ, "%.2f", z);
			Gcode = (std::string)"M20 G90 G01 X" + poseX + " Y" + poseY + " Z" + poseZ + "A  0.00  B  0.00  C  0.00" + " F2000" + "\r\n";
                        _serial.write(Gcode.c_str());
                        while (ros::ok())
                        {
                                ros::Duration(0.1).sleep();
                                std_msgs::String result;
                                result.data = _serial.read(_serial.available());
                                CommandComplete = (result.data.npos == result.data.find("ok"))?false:true;
                                if(CommandComplete == true)break;
                        }
                        ros::Duration(0.1).sleep();

                        while (ros::ok())
                        {
                                Gcode = (std::string)"?" + "\r\n";
                                _serial.write(Gcode.c_str());
                                ros::Duration(0.1).sleep();
                                std_msgs::String result;
                                result.data = _serial.read(_serial.available());
                                ArmMoveComplete = (result.data.npos == result.data.find("Idle"))?false:true;
                                if(ArmMoveComplete == true)break;
                        }
			ros::Duration(1).sleep();

			Gcode = (std::string)"M3S60" + "\r\n";
			_serial.write(Gcode.c_str());
			while (ros::ok())
	                {
	                        ros::Duration(0.1).sleep();
	                        std_msgs::String result;
	                        result.data = _serial.read(_serial.available());
	                        GripperComplete = (result.data.npos == result.data.find("ok"))?false:true;
	                        if(GripperComplete == true)break;
	                }
	                ros::Duration(1).sleep();

			Gcode = (std::string)"M21 G90 G01 X90.00 Y-40.00 Z40.00 A0.00 B0.00 C0.00 F2000.00" + "\r\n";
	             	_serial.write(Gcode.c_str());
	                while (ros::ok())
	                {
				Gcode = (std::string)"?" + "\r\n";
	                        _serial.write(Gcode.c_str());
	                        ros::Duration(0.1).sleep();
	                        std_msgs::String result;
	                        result.data = _serial.read(_serial.available());
	                        ArmMoveComplete = (result.data.npos == result.data.find("Idle"))?false:true;
                        	if(ArmMoveComplete == true)break;
                	}
			ros::Duration(1).sleep();
		}

		GrabStart = false;
	}
}

int main(int argc, char** argv)
{
	ros::init(argc, argv, "mirobot_write_pickandplace_node");
	ros::NodeHandle nh;

	ros::Subscriber sub_block_pose = nh.subscribe("/block_pose", 1, block_pose_write_callback);
	ros::Subscriber sub_grab = nh.subscribe("/grab",1,grab_write_callback);
	ros::Rate loop_rate(50);

	std::string Gcode = "";
	std_msgs::String result;
	std::stringstream ss;
	float jointAngle[7] = {0,0,0,0,0,0,0};
	sensor_msgs::JointState JointState;

	try
	{
		_serial.setPort("/dev/WLKATA");
		_serial.setBaudrate(115200);
		serial::Timeout to = serial::Timeout::simpleTimeout(1000);
		_serial.setTimeout(to);
		_serial.open();
		ros::Duration(1).sleep();
		std::string Gcode = "";
		Gcode = (std::string)"$h" + "\r\n";
		_serial.write(Gcode.c_str());
		while (ros::ok())
		{
			Gcode = (std::string)"?" + "\r\n";
			_serial.write(Gcode.c_str());
			ros::Duration(0.1).sleep();
			result.data = _serial.read(_serial.available());
			HomingComplete = (result.data.npos == result.data.find("ok"))?false:true;
			if(HomingComplete == true)break;
		}
		ros::Duration(1).sleep();

		Gcode = (std::string)"M21 G90 G01 X90.00 Y-40.00 Z40.00 A0.00 B0.00 C0.00 F2000.00" + "\r\n";
		_serial.write(Gcode.c_str());
		while (ros::ok())
		{
			Gcode = (std::string)"?" + "\r\n";
			_serial.write(Gcode.c_str());
			ros::Duration(0.1).sleep();
			std_msgs::String result;
			result.data = _serial.read(_serial.available());
			ArmMoveComplete = (result.data.npos == result.data.find("Idle"))?false:true;
			if(ArmMoveComplete == true)break;
		}
		ros::Duration(1).sleep();

		Gcode = (std::string)"M3S60" + "\r\n";
		_serial.write(Gcode.c_str());
		while (ros::ok())
		{
			ros::Duration(0.1).sleep();
			std_msgs::String result;
			result.data = _serial.read(_serial.available());
			GripperComplete = (result.data.npos == result.data.find("ok"))?false:true;
			if(GripperComplete == true)break;
		}
		ros::Duration(1).sleep();

		ROS_INFO_STREAM("Port has been open successfully");
	}
	catch (serial::IOException& e)
	{
		ROS_ERROR_STREAM("Unable to open port");
		return -1;
	}

	if (_serial.isOpen())
	{
		ros::Duration(1).sleep();
		ROS_INFO_STREAM("Attach and wait for commands");
	}

	while (ros::ok())
	{
		Gcode = (std::string)"?" + "\r\n";
		_serial.write(Gcode.c_str());
		ros::Duration(0.1).sleep();
		result.data = _serial.read(_serial.available());
		CanMove = (result.data.npos == result.data.find("Idle"))?false:true;
		ros::spinOnce();
		loop_rate.sleep();
	}

	return 0;
}


