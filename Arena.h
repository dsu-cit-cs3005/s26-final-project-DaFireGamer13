#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include "RobotBase.h"
#include "RadarObj.h"

class Arena {
private:
	//
	int m_rows;
	int m_cols;
	//
	std::vector<RobotBase*> m_robots;
	std::vector<RobotBase*> m_trapped_robots; //this is a list of robots trapped in a pit
	//
	struct BoardCell {
		//
		char type;
		RobotBase* robot_ptr;
	};
	std::vector<std::vector<BoardCell>> m_grid;
	void apply_damage(RobotBase* target, int raw_damage);
	void handle_railgun(RobotBase* shooter, int target_r, int target_c);
	void handle_grenade(RobotBase* shooter, int target_r, int target_c);
	void handle_hammer(RobotBase* shooter, int target_r, int target_c);
	void handle_flamethrower(RobotBase* shooter, int target_r, int target_c);
public:
	//
	Arena(int rows, int cols);
    	~Arena();
    	//Robot management
    	void add_robot(RobotBase* robot, int row, int col);
    	//Core Game loop
    	void play_turn();
    	bool is_game_over() const;
    	//helper methods for turn sequence
    	void perform_radar_phase(RobotBase* robot);
    	void perform_shooting_phase(RobotBase* robot);
    	void perform_movement_phase(RobotBase* robot);
    	//display
    	void display_board() const;
};

//
