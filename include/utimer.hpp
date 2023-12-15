#ifndef AUTONOMICFARM_UTIMER_HPP
#define AUTONOMICFARM_UTIMER_HPP


#include <iostream>
#include <chrono>
#include <functional>
#include <iomanip>

#define START(timename) auto timename = std::chrono::system_clock::now()
#define STOP(timename,elapsed) auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - timename).count()

class utimer {
public:
    utimer(std::string& m);
    utimer(const std::function<void(long)>& message_printer);
    utimer(std::string& m, long * us);
    ~utimer();

private:
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point stop;
    long * us_elapsed;
    std::function<void(long)> message_printer;

protected:
    std::string message;

};

utimer::utimer(std::string& m) : message(m),us_elapsed((long *)nullptr),message_printer(nullptr) {
    start = std::chrono::system_clock::now();
}

utimer::utimer(const std::function<void(long)>& message_printer) : message(""),us_elapsed((long *)nullptr),message_printer(message_printer) {
    start = std::chrono::system_clock::now();
}

utimer::utimer(std::string& m, long * us) : message(m),us_elapsed(us),message_printer(nullptr) {
    start = std::chrono::system_clock::now();
}

utimer::~utimer() {
    stop = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = stop - start;
    auto musec = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    if (message_printer == nullptr) {
        std::cout << message << " computed in " << std::setw(15) << musec << " usec " << std::endl;
    } else {
        message_printer(musec);
    }

    if (us_elapsed != nullptr)
        (*us_elapsed) = musec;
}

#endif //AUTONOMICFARM_UTIMER_HPP
