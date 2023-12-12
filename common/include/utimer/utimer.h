//
// Created by dferraro on 06/12/23.
//

#ifndef PARALLEL_DISTRIBUTED_SYSTEMS_UTIMER_H
#define PARALLEL_DISTRIBUTED_SYSTEMS_UTIMER_H

#include <iostream>
#include <iomanip>
#include <chrono>

#define START(timename) auto timename = std::chrono::system_clock::now();
#define STOP(timename,elapsed) auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - timename).count();

class utimer {
public:
    utimer(const std::string m);
    utimer(const std::function<void(long)>& message_printer);
    utimer(const std::string m, long * us);
    ~utimer();

private:
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point stop;
    long * us_elapsed;
    std::function<void(long)> message_printer;

protected:
    std::string message;

};

#endif //PARALLEL_DISTRIBUTED_SYSTEMS_UTIMER_H
