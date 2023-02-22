/*
 * @Author: crescendo
 * @Date: 2021-05-11 01:17:46
 * @Last Modified by: crescendo
 * @Last Modified time: 2021-05-11 03:13:01
 * g++ -std=c++1z -masm=intel calcpogo.cpp -o calcpogo.exe
 * 测试最难收的跳跳
 */

#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include "RTvz.cpp"
using namespace std;

#define TotalTick 1800
#define ZombieNum 1000
#define Repeat 10
#define StartTick 1000
#define IceTime 1
#define PaoNum 1

int x_shift = 36;
int x_size = 42;
int y_shift = 0;
int y_size = 115;

// 计算炮在右时的最左可能爆炸中心
int calculate(int zombie_x, int zombie_y, int zombie_y_offset, int pao_y) {
    int y_upper = zombie_y - zombie_y_offset + y_shift;
    int y_lower = y_upper + y_size;
    int y_dist = pao_y > y_lower ? pao_y - y_lower : (pao_y < y_upper ? y_upper - pao_y : 0);
    if (y_dist > 115) {
        return -1;
    }
    int pao_dist = (int)(sqrt(115 * 115 - y_dist * y_dist));
    int pao_right_dist = pao_dist + x_shift + x_size;
    return zombie_x + pao_right_dist;
}

// 计算炮在左时的最左可能爆炸中心
int calculate2(int zombie_x, int zombie_y, int zombie_y_offset, int pao_y) {
    int y_upper = zombie_y - zombie_y_offset + (-38);
    int y_lower = y_upper + 154;
    int y_dist = pao_y > y_lower ? pao_y - y_lower : (pao_y < y_upper ? y_upper - pao_y : 0);
    if (y_dist > 115) {
        return -1;
    }
    int pao_dist = (int)(sqrt(115 * 115 - y_dist * y_dist));
    int pao_left_dist = pao_dist - (-17);
    return zombie_x - pao_left_dist;
}

int main() {
    game.find();
    game.forbidRestartWindow(1);
    game.allowBackgroundRun(1);
    game.zombieSpawnPause(1);
    game.mushoomAwake(1);
    game.autoCollect(1);
    game.forbidItemDrop(1);
    game.simplifiedSTRCMP(1);
    game.forbidEnterHome(1);
    game.plantInvincible(1);

    system("pause/t");
    cout << "开始测试\n";
    level.clear();
    game.syspause(1);

    ifstream ifile;
    ifile.open("data/iced_slow_0.csv");
    string temp;
    getline(ifile, temp);
    double gargSlow[TotalTick];
    for (int i = 0; i < TotalTick; i++) {
        getline(ifile, temp);
        temp = temp.substr(0, temp.size() - 1);
        int pos = temp.find_last_of(",");
        gargSlow[i] = stod(temp.substr(pos + 1));
    }

    // 录入风炮
    // int paoDist[3 * PaoNum] = {0, 84, 169, 19, 104, 189, 39, 124, 209, 59, 144, 229, 79, 164, 249, 99, 184, 269, 110, 195, 280};
   
    // 后院
    int paoDist[3 * PaoNum] = {120, 205, 290};
    int garg_y[] = {50, 135, 220};

    ofstream mfile;
    time_t t = time(0);  // get time now
    struct tm *now = localtime(&t);
    char buffer[80];
    strftime(buffer, 80, "output/calcPogo(%Y-%m-%d_%H.%M.%S).csv", now);
    mfile.open(buffer);
    mfile << "time"
          << ",";

    for (int i = 0; i < PaoNum; i++) {
        mfile << "pogo_upper,pogo_same,pogo_lower,garg_upper,garg_same,garg_lower,";
    }
    mfile << endl;

    Plant *cob1 = new Plant({1, 6, Plant_COB_CANNON});
    Plant *cob2 = new Plant({1, 4, Plant_COB_CANNON});
    int minTable[PaoNum][3][TotalTick - StartTick];
    for (int i = 0; i < PaoNum; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < TotalTick - StartTick; k++)
                minTable[i][j][k] = 999;
    for (int l = 0; l < Repeat; ++l) {
        Plant *ice = new Plant({1, 0, Plant_ICE_SHROOM});
        level.jumptick(100 - IceTime);

        Zombie *Zombies[ZombieNum];
        for (int i = 0; i < ZombieNum; i++) {
            Zombies[i] = new Zombie({1, -1, Zombie_POGO_ZOMBIE});
        }

        // level.jumptick(52);
        // for (int i = 0; i < ZombieNum; i++) {
        //     float x;
        //     Zombies[i]->read(Zombie::x, &x);
        //     if (int(x) <= 764) {
        //         Zombies[i]->write(Zombie::vanish, true);
        //     }
        // }

        // level.jumptick(StartTick - 52);

        level.jumptick(StartTick);

        for (int i = 0; i < TotalTick - StartTick; ++i) {
            float x, y, y_offset;
            for (int j = 0; j < ZombieNum; ++j) {
                if (Zombies[j] == nullptr) {
                    continue;
                }
                bool vanish;
                Zombies[j]->read(Zombie::vanish, &vanish);
                if (vanish) {
                    continue;
                }
                Zombies[j]->read(Zombie::x, &x);
                Zombies[j]->read(Zombie::y, &y);
                Zombies[j]->read(Zombie::y_offset, &y_offset);
                for (int k = 0; k < PaoNum; k++) {
                    int paoRow = 0;
                    for (int num = k * 3; num < k * 3 + 3; num++) {
                        int res = calculate(int(x), int(y), int(y_offset), paoDist[num]);
                        // cout << paoDist[num] << " ";
                        if (res < minTable[k][paoRow][i])
                            minTable[k][paoRow][i] = res;
                        paoRow++;
                    }
                }
            }
            level.jumptick(1);
        }

        for (int i = 0; i < ZombieNum; i++) {
            Zombies[i]->kill();
        }
        level.renew();
        level.jumptick(1);
        cout << "(" << l << "/" << Repeat << ") 完成\n";
    }

    int gargTable[PaoNum][3][TotalTick - StartTick];
    for (int i = 0; i < TotalTick - StartTick; i++) {
        for (int j = 0; j < PaoNum; j++) {
            int walkTime = int(IceTime + (StartTick + i - 600 - IceTime) / 2.0);
            int x = int(gargSlow[max(0, walkTime)]);
            for (int k = 0; k < 3; k++) {
                int y = int(garg_y[k]);
                int res = calculate2(x, y, 0, paoDist[j * 3 + 1]);
                gargTable[j][k][i] = res;
            }
        }
    }
    for (int i = 0; i < TotalTick - StartTick; ++i) {
        mfile << i + StartTick << ",";
        for (int j = 0; j < PaoNum; j++) {
            mfile << minTable[j][2][i] << "," << minTable[j][1][i] << "," << minTable[j][0][i] << ",";
            mfile << gargTable[j][0][i] << "," << gargTable[j][1][i] << "," << gargTable[j][2][i] << ",";
        }
        mfile << "\n";
    }

    system("pause/t");
    game.syspause(0);
    level.write(Level::Paused, 0);
    mfile.close();
}
