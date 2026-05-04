#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <set>
#include <cmath>
#include <memory>
#include <algorithm>
#include "RobotBase.h"

//EVERYTHING is in this class, including methods AND implementation.
//Kinda like a python class, rather than a split C++ class.
//this is just a reminder for myself, I'll delete it when I'm done here.
class Robot_Incinerator : public RobotBase {
private:
        //
	int m_radar_sweep;
        int m_target_row = -1;
        int m_target_col = -1;
	bool m_has_target = false;
	//
	std::set<std::pair<int, int>> obstacles_memory;
	//this is copied from robot flame-e-o, it's very useful!
	void update_obstacle_memory(const std::vector<RadarObj>& radar_results)
    	{
        	for (const auto& obj : radar_results)
        	{
            	if (obj.m_type == 'M' || obj.m_type == 'P' || obj.m_type == 'F')
            	{
                	obstacles_memory.insert({obj.m_row, obj.m_col});
            	}
        	}
    	}
	//
	bool is_passable(int row, int col) const{
		//
        	return obstacles_memory.find({row, col}) == obstacles_memory.end();
    	}
	//
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
	int calculate_distance(int row1, int row2, int col1, int col2) const{
		return std::abs(row1 - row2) + std::abs(col1 - col2);
	}
	//
	void get_radar_direction(int& radar_direction) override {
		//implementing a similar sweep to flame_e_o
		if (m_has_target) {
			//radar_direction = m_radar_sweep;
			int my_row, my_col;
        		get_current_location(my_row, my_col);
			//
        		int row_diff = m_target_row - my_row;
        		int col_diff = m_target_col - my_col;
        		//simple logic to convert coordinates back to a one-eight direction
        		if (row_diff < 0 && col_diff == 0) radar_direction = 1;      // Up
        		else if (row_diff > 0 && col_diff == 0) radar_direction = 5; // Down
        		else if (row_diff == 0 && col_diff > 0) radar_direction = 3; // Right
        		else if (row_diff == 0 && col_diff < 0) radar_direction = 7; // Left
			else if (row_diff < 0 && col_diff > 0) radar_direction = 2;  // Up-Right
			else if (row_diff > 0 && col_diff > 0) radar_direction = 4;  // Down-Right
			else if (row_diff < 0 && col_diff < 0) radar_direction = 8;  // Up-Left
			else if (row_diff > 0 && col_diff < 0) radar_direction = 6;  // Down-Left
		} else {
			radar_direction = m_radar_sweep;
			m_radar_sweep = (m_radar_sweep % 8) + 1;
		}
		//
	}
	//
    	void process_radar_results(const std::vector<RadarObj>& radar_results) override {
		// reset memory so robot doesn't target empty cell
		m_has_target = false;
		update_obstacle_memory(radar_results);
		//
		int my_row, my_col;
		get_current_location(my_row, my_col);
		int closest_dist = 1000;
		//
		for (const auto& obj : radar_results) {
			if (obj.m_type == 'R') {
				//enemy robot found, lock in coordinates to closest robot
				int dist = calculate_distance(my_row, obj.m_row, my_col, obj.m_col);
				//emergency logic if approched by enemy robot
				/*
				if (dist < 5) {
					//switch target to robot within five units
					m_target_row = obj.m_row;
					m_target_col = obj.m_col;
					m_has_target = true;
					return;
				}
				*/
				//normal hunting if no emergency
				if (dist < closest_dist) {
					closest_dist = dist;
					m_target_row = obj.m_row;
					m_target_col = obj.m_col;
					m_has_target = true;
				}
			}
		}
	}
	//
    	bool get_shot_location(int& shot_row, int& shot_col) override {
		//
		if (m_has_target) {
			//
			int my_row, my_col;
			get_current_location(my_row, my_col);
			//
			if (calculate_distance(my_row, my_col, m_target_row, m_target_col) <= 4) {
				//
				shot_row = m_target_row;
				shot_col = m_target_col;
				return true;
			} else {
				//
				m_has_target = false;
				//fixed radar = false;
			}
		}
		return false;
	}
	//
    	void get_move_direction(int &direction,int &distance) override {
		/*
		if (!m_has_target) {
			direction = 0;
			distance = 0;
			return;
		}
		*/
		//
		int my_row, my_col;
		get_current_location(my_row, my_col);
		//
		int row_diff = m_target_row - my_row;
		int col_diff = m_target_col - my_col;
		//
		//pick ideal next step based on where enemy is
    		int next_row = my_row + (row_diff > 0 ? 1 : (row_diff < 0 ? -1 : 0));
    		int next_col = my_col + (col_diff > 0 ? 1 : (col_diff < 0 ? -1 : 0));
    		//obstacle Check - Try row movement first
    		if (row_diff != 0 && is_passable(my_row + (row_diff > 0 ? 1 : -1), my_col)) {
        		direction = (row_diff > 0) ? 5 : 1; // Down or Up
    		} //if blocked or rows are same, try column movement
    		else if (col_diff != 0 && is_passable(my_row, my_col + (col_diff > 0 ? 1 : -1))) {
        		direction = (col_diff > 0) ? 3 : 7; // Right or Left
    		} else {
        		//robot be stuck
        		direction = 0;
        		distance = 0;
        		return;
    		}
		//
		int total_dist = std::abs(row_diff) + std::abs(col_diff);
		distance = std::min(4, total_dist);
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
