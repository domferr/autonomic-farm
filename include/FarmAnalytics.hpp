//
// Created by dferraro on 22/12/23.
//

#ifndef AUTONOMICFARM_FARMANALYTICS_HPP
#define AUTONOMICFARM_FARMANALYTICS_HPP


#include <vector>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

#define CSV_DELIMITER ","

class farm_analytics {
public:
    std::chrono::system_clock::time_point farm_start_time;
    std::vector<std::pair<double, long>> throughput; // pair <timestamp, moving average throughput value>
    std::vector<std::pair<double, long>> throughput_points; // pair <timestamp, throughput value>
    std::vector<std::pair<double, long>> service_time; // pair <timestamp, moving average service time value>
    std::vector<std::pair<double, long>> service_time_points; // pair <timestamp, service time value>
    std::vector<long> arrival_time;

    void throughput_to_file(const char* root_dir, const char* basename, size_t num_workers, size_t stream_size) {
        std::ofstream file;
        auto file_name = open(file, root_dir, basename, num_workers, stream_size);

        std::cout << "Writing throughput moving average data to " << file_name << "..." << std::flush;
        file << "throughput" << CSV_DELIMITER << "time" << CSV_DELIMITER << "numworkers" << CSV_DELIMITER << "streamsize" << std::endl;
        for(auto& th: throughput) {
            file << th.first << CSV_DELIMITER << th.second << CSV_DELIMITER << num_workers << CSV_DELIMITER << stream_size << std::endl;
        }
        file.close();
        std::cout << "DONE!" << std::endl;
    }

    void throughput_points_to_file(const char* root_dir, const char* basename, size_t num_workers, size_t stream_size) {
        std::ofstream file;
        auto file_name = open(file, root_dir, basename, num_workers, stream_size);

        std::cout << "Writing throughput points data to " << file_name << "..." << std::flush;
        file << "throughput" << CSV_DELIMITER << "time" << CSV_DELIMITER << "numworkers" << CSV_DELIMITER << "streamsize" << std::endl;
        for(auto& th: throughput_points) {
            file << th.first << CSV_DELIMITER << th.second << CSV_DELIMITER << num_workers << CSV_DELIMITER << stream_size << std::endl;
        }
        file.close();
        std::cout << "DONE!" << std::endl;
    }

    void arrivaltime_to_file(const char* root_dir, const char* basename, size_t num_workers, size_t stream_size) {
        std::ofstream file;
        auto file_name = open(file, root_dir, basename, num_workers, stream_size);

        std::cout << "Writing arrival time data to " << file_name << "..." << std::flush;
        file << "time" << CSV_DELIMITER << "numworkers" << CSV_DELIMITER << "streamsize" << std::endl;
        for(auto& time: arrival_time) {
            file << time << CSV_DELIMITER << num_workers << CSV_DELIMITER << stream_size << std::endl;
        }
        file.close();
        std::cout << "DONE!" << std::endl;
    }

    void servicetime_to_file(const char* root_dir, const char* basename, size_t num_workers, size_t stream_size) {
        std::ofstream file;
        auto file_name = open(file, root_dir, basename, num_workers, stream_size);

        std::cout << "Writing service time moving average data to " << file_name << "..." << std::flush;
        file << "servicetime" << CSV_DELIMITER << "time" << CSV_DELIMITER << "numworkers" << CSV_DELIMITER << "streamsize" << std::endl;
        for(auto& svt: service_time) {
            file << svt.first << CSV_DELIMITER << svt.second << CSV_DELIMITER << num_workers << CSV_DELIMITER << stream_size << std::endl;
        }
        file.close();
        std::cout << "DONE!" << std::endl;
    }

    void servicetime_points_to_file(const char* root_dir, const char* basename, size_t num_workers, size_t stream_size) {
        std::ofstream file;
        auto file_name = open(file, root_dir, basename, num_workers, stream_size);

        std::cout << "Writing service time points data to " << file_name << "..." << std::flush;
        file << "servicetime" << CSV_DELIMITER << "time" << CSV_DELIMITER << "numworkers" << CSV_DELIMITER << "streamsize" << std::endl;
        for(auto& svt: service_time_points) {
            file << svt.first << CSV_DELIMITER << svt.second << CSV_DELIMITER << num_workers << CSV_DELIMITER << stream_size << std::endl;
        }
        file.close();
        std::cout << "DONE!" << std::endl;
    }

private:
    static std::string open(std::ofstream &file, const char* root_dir, const char* basename, size_t num_workers, size_t stream_size) {
        std::ostringstream oss;
        oss << root_dir << "/" << basename << "-" << num_workers << "-" << stream_size << ".csv";
        auto file_name = oss.str();
        mkdir(root_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        file.open(file_name);
        return file_name;
    }
};


#endif //AUTONOMICFARM_FARMANALYTICS_HPP
