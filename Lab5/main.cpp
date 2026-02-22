
#include <string>
#include <iostream>
#include <sstream>
#include <random>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <utility>
#include <cstdlib>
#include <map>
#include <numeric>

class strategye {
public:
    int active = 0;
    uint64_t destroyed = 0;

    virtual void shoot(int width, int height) {
    }

};

class strategy_stupid : public strategye {
public:
    int x = -1, y = 0;

    void shoot(int width, int height) override {
        x++;
        if (x == width) {
            x = 0;
            y++;
        }
        std::cout << x << ' ' << y;
    }
};

class strategy_clever : public strategye {
public:

    std::map<std::pair<uint64_t, uint64_t>, uint8_t> mm;

    void shoot(int width, int height) override {
        while (1) {
            uint64_t x, y;
            x = std::rand() % width;
            y = std::rand() % height;
            if (mm[{x, y}] == 0) {
                mm[{x, y}] = 1;
                std::cout << x << ' ' << y;
                return;
            }
        }
    }
};

struct ship {
    uint64_t X, Y;
    uint8_t direction;
    int size;
    uint8_t mask;

    ship(uint64_t x, uint64_t y, int dir, int sz) : X(x), Y(y), direction(dir), size(sz) {
        mask = 0;
        mask = (1 << (sz)) - 1;
    }
};

class game {
protected:
    uint64_t width;
    uint64_t heigth;
    uint64_t sum = 0;
    strategye *strategy = new strategye();
    std::vector<ship> ships;
    std::vector<int> count_of_ships;
    uint64_t number_destroyed = 0;
    uint8_t finished = 0, win = 0, lose = 0;
public:
    game() {
        count_of_ships.resize(4, 0);
    }

    virtual void start() {

    }

    void set_ship_finally() {
        int ind = 0;
        sum = std::accumulate(std::begin(count_of_ships), std::end(count_of_ships), 0);
        for (int i = 0; i < width;) {
            if (count_of_ships[ind] > 0) {
                ship sh(i, 0, 0, ind + 1);
                ships.push_back(sh);
                i += ind + 2;
                count_of_ships[ind]--;
            } else {
                ind++;
                if (ind == 4) {
                    return;
                }
            }
        }
        for (int i = 0; i < width;) {
            if (count_of_ships[ind] > 0) {
                ship sh(i, heigth - 1, 0, ind + 1);
                ships.push_back(sh);
                i += ind + 2;
                count_of_ships[ind]--;
            } else {
                ind++;
                if (ind == 4) {
                    return;
                }
            }
        }
        for (int i = 1; i < heigth - 1;) {
            if (count_of_ships[ind] > 0) {
                ship sh(0, i, 1, ind + 1);
                ships.push_back(sh);
                i += ind + 2;
                count_of_ships[ind]--;
            } else {
                ind++;
                if (ind == 4) {
                    return;
                }
            }
        }
        for (int i = 1; i < heigth - 1;) {
            if (count_of_ships[ind] > 0) {
                ship sh(width - 1, i, 1, ind + 1);
                ships.push_back(sh);
                i += ind + 2;
                count_of_ships[ind]--;
            } else {
                ind++;
                if (ind == 4) {
                    return;
                }
            }
        }

        for (int x = 2; x < width - 2; x += 2) {
            for (int y = 2; y < heigth - 2;) {
                if (count_of_ships[ind] > 0) {
                    if (y + ind + 1 < heigth - 2) {
                        ship sh(x, y, 2, ind + 1);
                        ships.push_back(sh);
                        y += ind + 2;
                        count_of_ships[ind]--;
                    } else {
                        y += 1;
                    }
                } else {
                    ind++;
                    if (ind == 4) {
                        return;
                    }
                }
            }
        }
    }

    void stop() {
        strategy->active = 0;
    }

    virtual void set_width(uint64_t x) {

    }

    void get_width() {
        std::cout << width;
    }

    virtual void set_height(uint64_t y) {

    }

    void get_height() {
        std::cout << heigth;
    }

    virtual void set_ship(int type, int count) {

    }

    void get_count_of_ship(int type) {
        std::cout << count_of_ships[type - 1];
    }

    void set_strateg(int n) {
        if (n == 1) {
            strategy = dynamic_cast<strategye *>(new strategy_stupid());
        } else {
            strategy = dynamic_cast<strategye *>(new strategy_clever());
        }
        std::cout << "ok";
    }

    void me_shoot(int x, int y) {
        for (int i = 0; i < ships.size(); i++) {
            if (ships[i].direction == 1) {
                if (x == ships[i].X && ships[i].Y <= y && y <= ships[i].size - 1 + ships[i].Y) {
                    int sdvig = y - ships[i].Y;
                    int mask = (1 << (sdvig));
                    mask = ~mask;
                    ships[i].mask = ships[i].mask & mask;
                    if (ships[i].mask == 0) {
                        number_destroyed++;
                        if (number_destroyed == sum)lose = 1;
                        std::cout << "kill";
                    } else {
                        std::cout << "hit";
                    }
                    return;
                }
            }
            if (ships[i].direction == 0) {

                if (y == ships[i].Y && ships[i].X <= x && x <= -ships[i].X + ships[i].size - 1) {
                    int sdvig = x - ships[i].X;
                    int mask = (1 << (sdvig));
                    mask = ~mask;
                    ships[i].mask = ships[i].mask & mask;
                    if (ships[i].mask == 0) {
                        std::cout << "kill";
                    } else {
                        std::cout << "hit";
                    }
                    return;
                }
            }
        }
        std::cout << "miss";
    }

    void i_shoot() {
        strategy->shoot(width, heigth);
    }

    void get_result_of_my_shoot(std::string x) {
        if (x == "kill") {
            strategy->destroyed++;
            if (strategy->destroyed == sum) {
                win = 1;
            }
        }
        std::cout << "ok";
    }

    void is_finished() {
        if (win || lose) {
            std::cout << "yes";
        } else {
            std::cout << "no";
        }
    }

    void is_win() {
        if (win) {
            std::cout << "yes";
        } else {
            std::cout << "no";
        }
    }

    void is_lose() {
        if (lose) {
            std::cout << "yes";
        } else {
            std::cout << "no";
        }
    }

    void save_in(std::string path) {
        std::ofstream input(path);
        if (!input.is_open()) {
            std::cout << "fault";
            return;
        }
        input << width << ' ' << heigth << '\n';
        std::unordered_map<int, char> check;
        check[1] = 'h';
        check[0] = 'v';
        for (auto x: ships) {
            input << x.size << ' ' << check[x.direction] << ' ' << x.X << ' ' << x.Y << '\n';
        }
        std::cout << "ok";
        input.close();
    }

    void load(std::string path) {
        std::fstream input(path);
        std::string line;
        if (!input.is_open()) {
            return;
        }
        int i = 0;
        while (std::getline(input, line)) {
            if (i == 0) {
                std::istringstream iss(line);
                iss >> width >> heigth;
                i++;
            } else {
                std::istringstream iss(line);
                uint64_t x, y, size;
                std::string dir;
                iss >> size >> dir >> x >> y;
                int f = 0;
                if (dir == "h")f = 1;
                ship a(x, y, f, size);
                ships.push_back(a);
            }
        }
        input.close();
    }
};

class game_slave : public game {
    void set_width(uint64_t x) override {
        std::cout << "failed";
    }

    void start() override {
        strategy->active = 1;
        std::cout << "ok";
    }

    void set_height(uint64_t y) override {
        std::cout << "failed";
    }

    void set_ship(int type, int count) override {
        std::cout << "ok";
    }
};

class game_master : public game {
    void set_width(uint64_t x) override {
        if (strategy->active == 0) {
            width = x;
            std::cout << "ok";
        } else {
            std::cout << "failed";
        }
    }

    void start() override {
        strategy->active = 1;
        set_ship_finally();
        std::cout << "ok";
    }

    void set_height(uint64_t y) override {
        if (strategy->active == 0) {
            heigth = y;
            std::cout << "ok";
        } else {
            std::cout << "failed";
        }
    }

    void set_ship(int type, int count) override {
        count_of_ships[type - 1] += count;
        std::cout << "ok";
    }
};

int main() {

    game* my_game;
    while (true) {
        std::string cmd;
        std::getline(std::cin, cmd);

        if (cmd == "exit") {
            return 0;
        } else if (cmd == "ping") {
            std::cout << "pong";
        } else if (cmd == "start") {
            my_game->start();
        } else if (cmd.substr(0,6) == "create") {
            if(cmd.substr(7) == "master"){
                my_game = dynamic_cast<game*>(new game_master());
            }else{
                my_game = dynamic_cast<game*>(new game_slave());
            }
        }
        else if (cmd == "stop") {
            my_game->stop();
        } else if (cmd.substr(0, 9) == "set width") {
            std::string value = cmd.substr(10);
            my_game->set_width(stoi(value));
        } else if (cmd == "get width") {
            my_game->get_width();
        } else if (cmd.substr(0, 10) == "set height") {
            std::string value = cmd.substr(10);
            my_game->set_height(stoi(value));
        } else if (cmd == "get height") {
            my_game->get_height();
        } else if (cmd.substr(0, 9) == "set count") {
            std::istringstream iss(cmd);
            std::string type;
            iss >> type, iss >> type, iss >> type;
            std::string val;
            iss >> val;
            my_game->set_ship(stoi(type), stoi(val));
        } else if (cmd.substr(0, 9) == "get count") {
            std::string type = std::to_string(cmd[10]);
            my_game->get_count_of_ship(stoi(type));
        } else if (cmd.substr(0, 12) == "set strategy") {
            std::string type = cmd.substr(13);
            if (type == "ordered") {
                my_game->set_strateg(1);
            } else {
                my_game->set_strateg(2);
            }
        } else if (cmd.size() > 4 && cmd.substr(0, 4) == "shot") {
            std::istringstream iss(cmd);
            std::string type;
            iss >> type;
            std::string x, y;
            iss >> x;
            iss >> y;
            my_game->me_shoot(stoi(x), stoi(y));
        } else if (cmd == "shot") {
            my_game->i_shoot();
        } else if (cmd == "set result") {
            my_game->get_result_of_my_shoot(cmd.substr(10));
        } else if (cmd == "finished") {
            my_game->is_finished();
        } else if (cmd == "win") {
            my_game->is_win();
        } else if (cmd == "lose") {
            my_game->is_lose();
        } else if (cmd.substr(0, 4) == "dump") {
            my_game->save_in(cmd.substr(5));
        } else if (cmd.substr(0, 4) == "load") {
            my_game->load(cmd.substr(5));
        } else {
            std::cout << "Uncorrect command";
        }
        std::cout << std::endl;
    }
//create master
//set width 100
//set height 100
//set count 2 10
//set strategy ordered
//start


    return 0;
}