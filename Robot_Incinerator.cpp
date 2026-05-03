#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include "RobotBase.h"

//EVERYTHING is in this class, including methods AND implementation.
//Kinda like a python class, rather than a split C++ class.
//this is just a reminder for myself, I'll delete it when I'm done here.
class Robot_Incinerator : public RobotBase {
private:
        //
	int m_radar_sweep;
        int m_target_row;
        int m_target_col;
	bool m_has_target;
public:
	//constructor
	//Stat points to spend for move speed and armor: 7
	Robot_Incinerator() : RobotBase(4, 3, flamethrower) {
		//i'm kinda half-referencing Incineroar from Pokemon, but Incinerator sounds more robot-like
		m_name = "Incinerator";
		m_character = 'I';
		m_radar_sweep = 1;
		m_has_target = false;
	}
	//
	void get_radar_direction(int& radar_direction) override {
		//implementing a similar sweep to flame_e_o
		if (m_has_target) {
			radar_direction = m_radar_sweep;
		} else {
			radar_direction = m_radar_sweep;
			m_radar_sweep = (m_radar_sweep % 8) + 1;
		}
		//i feel like i'm missing something...
	}
	//
    	void process_radar_results(const std::vector<RadarObj>& radar_results) override {
		//
		m_has_target = false;
	}
	//
    	bool get_shot_location(int& shot_row, int& shot_col) override {
		return false;
	}
	//
    	void get_move_direction(int &direction,int &distance) override {
		direction = 0;
		distance = 0;
	}
};

//extern "C" functions for test_robot executable
extern "C" {
	RobotBase* create_robot() {
		return new Robot_Incinerator();
	}
	//
	const char* robot_summary() {
		return "Incinerator: your flames of disaster.";
	}
}
//
