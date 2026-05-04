#include "Arena.h"
#include <iomanip>
#include <ctime>

Arena::Arena(int rows, int cols) : m_rows(rows), m_cols(cols) {
	std::srand(std::time(nullptr));
	//initiallize grid with empty cells
	m_grid.assign(m_rows, std::vector<BoardCell>(m_cols, {' ', nullptr}));
}

Arena::~Arena() {
	//clean up xtra robot pointers, no memory leaks
	for (RobotBase* robot : m_robots) {
		delete robot;
	}
}

void Arena::add_robot(RobotBase* robot, int row, int col) {
	//
	if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
		m_robots.push_back(robot);
		char icon = (robot->m_character > 32) ? robot->m_character : 'R';
		m_grid[row][col] = BoardCell{icon, robot};
		robot->move_to(row, col);
	}
}

void Arena::play_turn() {
	//radar phase
	for (RobotBase* robot : m_robots) {
		perform_radar_phase(robot);
	}
	//shooting phase
	for (RobotBase* robot : m_robots) {
                perform_shooting_phase(robot);
        }
	//movement phase
	for (RobotBase* robot : m_robots) {
                perform_movement_phase(robot);
        }
}

void Arena::perform_radar_phase(RobotBase* robot) {
    //
    int direction;
    robot->get_radar_direction(direction);
    //
    std::vector<RadarObj> results;
    int r, c;
    robot->get_current_location(r, c);
    //
    //calculate delta based on 1-8 directions
    int dr = 0, dc = 0;
    if (direction == 1) dr = -1; //up
    else if (direction == 5) dr = 1; //down
    else if (direction == 3) dc = 1; //right
    else if (direction == 7) dc = -1; //left
    else if (direction == 2) {
	    //up-right
	    dr = -1;
	    dc = 1;
    } else if (direction == 4) {
	    //down-right
	    dr = 1;
	    dc = 1;
    } else if (direction == 6) {
	    //down-left
	    dr = 1;
	    dc = -1;
    } else if (direction == 8) {
	    //up-left
	    dr = -1;
	    dc = -1;
    }

    //scan the line until we hit a wall or a mountain
    r += dr; c += dc;
    while (r >= 0 && r < m_rows && c >= 0 && c < m_cols) {
        if (m_grid[r][c].type != ' ') {
            results.push_back({m_grid[r][c].type, r, c});
            if (m_grid[r][c].type == 'M') break; // Mountains block radar
        }
        r += dr; c += dc;
    }

    robot->process_radar_results(results);
}

void Arena::apply_damage(RobotBase* target, int raw_damage) {
    //
    if (!target || target->get_health() <= 0) return;
    //
    // 1. Calculate armor reduction (10% per armor level)
    double reduction = target->get_armor() * 0.10;
    int final_damage = static_cast<int>(raw_damage * (1.0 - reduction));
    //
    // 2. Apply damage to health
    target->take_damage(final_damage); 
    //
    // 3. Every hit reduces armor by 1
    target->reduce_armor(1);           
    //
    // 4. If they died, mark the grid with an 'X'
    if (target->get_health() <= 0) {
        int r, c;
        target->get_current_location(r, c);
        m_grid[r][c].type = 'X'; 
    }
}

void Arena::perform_shooting_phase(RobotBase* robot) {
	//
	int target_r, target_c;
	//
	if (robot->get_shot_location(target_r, target_c)) {
		//
		WeaponType weapon = robot->get_weapon();
		//
		if (weapon == railgun) {
			handle_railgun(robot, target_r, target_c);
		} else if (weapon == grenade) {
			handle_grenade(robot, target_r, target_c);
		} else if (weapon == hammer) {
			handle_hammer(robot, target_r, target_c);
		} else if (weapon == flamethrower) {
			handle_flamethrower(robot, target_r, target_c);
		}
	}
}

void Arena::perform_movement_phase(RobotBase* robot) {
	if (robot->get_health() <= 0) return; //dead robots don't move
	//
	for (auto trapped : m_trapped_robots) {
		if (trapped == robot) return;
	}
	//
	int direction, distance;
	robot->get_move_direction(direction, distance);
	//
	//cap the distance by the robot's move stat
	int max_move = robot->get_move_speed();
	int actual_dist = std::min(distance, max_move);
	//
	int dr = 0, dc = 0;
	//map directions 1-8 to row/col changes
	if (direction == 1) dr = -1; //up
	else if (direction == 5) dr = 1; //down
	else if (direction == 3) dc = 1; //right
	else if (direction == 7) dc = -1; //left
	else if (direction == 2) {
            	//up-right
            	dr = -1;
            	dc = 1;
    	} else if (direction == 4) {
            	//down-right
            	dr = 1;
            	dc = 1;
    	} else if (direction == 6) {
            	//down-left
            	dr = 1;
            	dc = -1;
    	} else if (direction == 8) {
            	//up-left
            	dr = -1;
            	dc = -1;
    	}
	//
	for (int i = 0; i < actual_dist; ++i) {
		//
		int current_r, current_c;
		robot->get_current_location(current_r, current_c);
		//
		int next_r = current_r + dr;
		int next_c = current_c + dc;
		//
		//check for board bounds
		if (next_r < 0 || next_r >= m_rows || next_c < 0 || next_c >= m_cols) break;
		//
		char cell_type = m_grid[next_r][next_c].type;
		//
		//check for obstacles
		//if (m_grid[next_r][next_c].type != ' ') break; //collision detected, move ends here <--old way
		//--new way--
		//solid obstacles
		if (cell_type == 'M' || cell_type == 'R' || cell_type == 'X') {
			break;
		}
		//pits
		if (cell_type == 'P') {
			char icon = m_grid[current_r][current_c].type;
			m_grid[current_r][current_c] = BoardCell{' ', nullptr};
			robot->move_to(next_r, next_c);
			m_grid[next_r][next_c] = BoardCell{icon, robot};
			//
			m_trapped_robots.push_back(robot);
			break;
		}
		//flamethrowers
		if (cell_type == 'F') {
			//
			int damage = 30 + (std::rand() % 21);
			apply_damage(robot, damage);
			if (robot->get_health() <= 0) break; //stop if flame kills robot
		}
		char icon = m_grid[current_r][current_c].type;
		if (icon == ' ') icon = 'R'; //fallback in case we have ghost robots
		//
		//execute the move
		m_grid[current_r][current_c] = BoardCell{' ', nullptr};
		//
		//update robot's internal state (position)
		robot->move_to(next_r, next_c);
		//
		//update grid's new spot
		m_grid[next_r][next_c] = BoardCell{icon, robot};
	}
}

void Arena::handle_flamethrower(RobotBase* shooter, int target_r, int target_c) {
    int start_r, start_c;
    shooter->get_current_location(start_r, start_c);

    // Determine direction deltas (similar to movement/radar)
    int dr = (target_r > start_r) ? 1 : (target_r < start_r ? -1 : 0);
    int dc = (target_c > start_c) ? 1 : (target_c < start_c ? -1 : 0);

    // If they shot at their own square, don't do anything
    if (dr == 0 && dc == 0) return;

    // A flamethrower is 4 cells deep
    for (int step = 1; step <= 4; ++step) {
        // Center of the flame at this distance
        int center_r = start_r + (dr * step);
        int center_c = start_c + (dc * step);

        // It is 3 cells wide. We need to check the "side" cells.
        // If we are shooting vertically (Up/Down), width is horizontal.
        // If we are shooting horizontally (Left/Right), width is vertical.
        int side_dr = (dr == 0) ? 1 : 0;
        int side_dc = (dc == 0) ? 1 : 0;

        // Check 3 cells: center, side1, and side2
        for (int offset = -1; offset <= 1; ++offset) {
            int r = center_r + (side_dr * offset);
            int c = center_c + (side_dc * offset);

            // Bounds check
            if (r >= 0 && r < m_rows && c >= 0 && c < m_cols) {
                if (m_grid[r][c].robot_ptr != nullptr) {
                    int damage = 30 + (std::rand() % 21); // 30-50 damage
                    apply_damage(m_grid[r][c].robot_ptr, damage);
                }
            }
        }
    }
}

void Arena::handle_grenade(RobotBase* shooter, int target_r, int target_c) {
    // Grenades have a limit of 10 shots
    // Note: RobotBase handles the count, but Arena handles the area damage.
    for (int r = target_r - 1; r <= target_r + 1; ++r) {
        for (int c = target_c - 1; c <= target_c + 1; ++c) {
            if (r >= 0 && r < m_rows && c >= 0 && c < m_cols) {
                if (m_grid[r][c].robot_ptr != nullptr) {
                    int damage = 10 + (std::rand() % 31); // 10-40 damage
                    apply_damage(m_grid[r][c].robot_ptr, damage);
                }
            }
        }
    }
    shooter->decrement_grenades(); // Official method from RobotBase.h
}

void Arena::handle_hammer(RobotBase* shooter, int target_r, int target_c) {
    // Check if there is a robot at the specific cell targeted
    if (target_r >= 0 && target_r < m_rows && target_c >= 0 && target_c < m_cols) {
        if (m_grid[target_r][target_c].robot_ptr != nullptr) {
            int damage = 50 + (std::rand() % 11); // 50-60 damage
            apply_damage(m_grid[target_r][target_c].robot_ptr, damage);
        }
    }
}

void Arena::handle_railgun(RobotBase* shooter, int target_r, int target_c) {
    int start_r, start_c;
    shooter->get_current_location(start_r, start_c);

    // Calculate the direction deltas (normalized to -1, 0, or 1)
    int dr = (target_r > start_r) ? 1 : (target_r < start_r ? -1 : 0);
    int dc = (target_c > start_c) ? 1 : (target_c < start_c ? -1 : 0);

    // Prevent infinite loops if the robot somehow shoots its own square
    if (dr == 0 && dc == 0) return;

    // Start one cell away from the shooter
    int r = start_r + dr;
    int c = start_c + dc;

    // The railgun travels to the edge of the arena regardless of target_r/c
    while (r >= 0 && r < m_rows && c >= 0 && c < m_cols) {
        // If a robot is in this cell, they take damage
        if (m_grid[r][c].robot_ptr != nullptr) {
            int damage = 10 + (std::rand() % 11); // Railgun: 10-20 damage
            apply_damage(m_grid[r][c].robot_ptr, damage);
        }
        
        // Move to the next cell in the line
        r += dr;
        c += dc;
    }
}

bool Arena::is_game_over() const {
    int alive_count = 0;
    for (RobotBase* robot : m_robots) {
        if (robot->get_health() > 0) {
            alive_count++;
        }
    }
    return alive_count <= 1; // Game over if 1 winner or everyone is dead
}

void Arena::display_board() const {
    // Top border
    std::cout << "+" << std::string(m_cols * 2, '-') << "+" << std::endl;

    for (int r = 0; r < m_rows; ++r) {
        std::cout << "|"; // Left border
        for (int c = 0; c < m_cols; ++c) {
            char cell = m_grid[r][c].type;
            // If the cell is empty, maybe print a dot to show the grid
            if (cell == ' ') std::cout << " .";
            else std::cout << " " << cell;
        }
        std::cout << " |" << std::endl; // Right border
    }

    // Bottom border
    std::cout << "+" << std::string(m_cols * 2, '-') << "+" << std::endl;
    
    // Optional: Print a status line for health
    for (RobotBase* robot : m_robots) {
        if (robot->get_health() > 0) {
	    int r, c;
	    robot->get_current_location(r, c);
	    char icon = m_grid[r][c].type;
            std::cout << icon << ": " << robot->get_health() << " HP  ";
        }
    }
    std::cout << "\n" << std::endl;
}
//
