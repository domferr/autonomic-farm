#include <iostream>
#include <iomanip>
#include <chrono>
#include <functional>
#include "../../include/utimer/utimer.h"

utimer::utimer(const std::string m) : message(m),us_elapsed((long *)nullptr),message_printer(nullptr) {
    start = std::chrono::system_clock::now();
}

utimer::utimer(const std::function<void(long)>& message_printer) : message(""),us_elapsed((long *)nullptr),message_printer(message_printer) {
    start = std::chrono::system_clock::now();
}

utimer::utimer(const std::string m, long * us) : message(m),us_elapsed(us),message_printer(nullptr) {
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