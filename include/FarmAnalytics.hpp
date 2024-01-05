#ifndef AUTONOMICFARM_FARMANALYTICS_HPP
#define AUTONOMICFARM_FARMANALYTICS_HPP


#include <vector>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include "ProgramArgs.hpp"

#define CSV_DELIMITER ","

class farm_analytics {
public:
    std::chrono::system_clock::time_point farm_start_time; // timepoint when the farm's run method was called
    std::vector<std::pair<double, long>> throughput; // pair <timestamp, moving average throughput value>
    std::vector<std::pair<double, long>> throughput_points; // pair <timestamp, throughput value>
    std::vector<std::pair<double, long>> service_time; // pair <timestamp, moving average service time value>
    std::vector<std::pair<double, long>> service_time_points; // pair <timestamp, service time value>
    std::vector<std::pair<size_t, long>> num_workers; // pair <timestamp, number of nodes>
    std::vector<long> arrival_time;

    void throughput_to_file(const char* root_dir, const char* basename, program_args &args) {
        long epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(farm_start_time.time_since_epoch()).count();
        std::ofstream file;
        auto file_name = open(file, root_dir, basename, args, epoch_ms);

        std::cout << "Writing throughput moving average data to " << file_name << "..." << std::flush;
        file << "throughput" << CSV_DELIMITER << "time" << std::endl;
        for(auto& th: throughput) {
            file << th.first << CSV_DELIMITER << th.second << std::endl;
        }
        file.close();
        std::cout << "DONE!" << std::endl;
    }

    void throughput_points_to_file(const char* root_dir, const char* basename, program_args &args) {
        long epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(farm_start_time.time_since_epoch()).count();
        std::ofstream file;
        auto file_name = open(file, root_dir, basename, args, epoch_ms);

        std::cout << "Writing throughput points data to " << file_name << "..." << std::flush;
        file << "throughput" << CSV_DELIMITER << "time" << std::endl;
        for(auto& th: throughput_points) {
            file << th.first << CSV_DELIMITER << th.second << std::endl;
        }
        file.close();
        std::cout << "DONE!" << std::endl;
    }

    void arrivaltime_to_file(const char* root_dir, const char* basename, program_args &args) {
        long epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(farm_start_time.time_since_epoch()).count();
        std::ofstream file;
        auto file_name = open(file, root_dir, basename, args, epoch_ms);

        std::cout << "Writing arrival time data to " << file_name << "..." << std::flush;
        file << "time" << std::endl;
        for(auto& time: arrival_time) {
            file << time << std::endl;
        }
        file.close();
        std::cout << "DONE!" << std::endl;
    }

    void servicetime_to_file(const char* root_dir, const char* basename, program_args &args) {
        long epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(farm_start_time.time_since_epoch()).count();
        std::ofstream file;
        auto file_name = open(file, root_dir, basename, args, epoch_ms);

        std::cout << "Writing service time moving average data to " << file_name << "..." << std::flush;
        file << "servicetime" << CSV_DELIMITER << "time" << std::endl;
        for(auto& svt: service_time) {
            file << svt.first << CSV_DELIMITER << svt.second << std::endl;
        }
        file.close();
        std::cout << "DONE!" << std::endl;
    }

    void servicetime_points_to_file(const char* root_dir, const char* basename, program_args &args) {
        long epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(farm_start_time.time_since_epoch()).count();
        std::ofstream file;
        auto file_name = open(file, root_dir, basename, args, epoch_ms);

        std::cout << "Writing service time points data to " << file_name << "..." << std::flush;
        file << "servicetime" << CSV_DELIMITER << "time" << std::endl;
        for(auto& svt: service_time_points) {
            file << svt.first << CSV_DELIMITER << svt.second << std::endl;
        }
        file.close();
        std::cout << "DONE!" << std::endl;
    }

    void num_workers_to_file(const char* root_dir, const char* basename, program_args &args) {
        long epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(farm_start_time.time_since_epoch()).count();
        std::ofstream file;
        auto file_name = open(file, root_dir, basename, args, epoch_ms);

        std::cout << "Writing number of workers data to " << file_name << "..." << std::flush;
        file << "num_workers" << CSV_DELIMITER << "time" << std::endl;
        for(auto& num: num_workers) {
            file << num.first << CSV_DELIMITER << num.second << std::endl;
        }
        file.close();
        std::cout << "DONE!" << std::endl;
    }

    void metadata_to_file(const char* root_dir, const char* basename, program_args &args) {
        long epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(farm_start_time.time_since_epoch()).count();
        std::ofstream file;
        auto file_name = open(file, root_dir, basename, args, epoch_ms);

        std::cout << "Writing metadata to " << file_name << "..." << std::flush;
        file << "min_num_workers" << CSV_DELIMITER << "max_num_workers" << CSV_DELIMITER << "initial_num_workers" << CSV_DELIMITER;
        file << "target_service_time" << CSV_DELIMITER << "stream_size" << CSV_DELIMITER << "service_times" << CSV_DELIMITER << "arrival_times" << std::endl;
        file << args.min_num_workers << CSV_DELIMITER << args.max_num_workers << CSV_DELIMITER << args.num_workers << CSV_DELIMITER;
        file << args.target_service_time << CSV_DELIMITER << args.stream_size;
        file << CSV_DELIMITER << "\"[";
        for (int i = 0; i < args.stream_size; ++i) {
            auto service_time_index = (args.serviceTimes.size() * i) / args.stream_size;
            file << args.serviceTimes[service_time_index];
            if (i < args.stream_size - 1) file << CSV_DELIMITER;
        }
        file << "]\"" << CSV_DELIMITER << "\"[";
        for (int i = 0; i < args.stream_size; ++i) {
            auto arrival_time_index = (args.arrivalTimes.size() * i) / args.stream_size;
            file << args.arrivalTimes[arrival_time_index];
            if (i < args.stream_size - 1) file << CSV_DELIMITER;
        }
        file << "]\"" << std::endl;
        file.close();
        std::cout << "DONE!" << std::endl;
    }

private:
    static std::string open(std::ofstream &file, const char* root_dir, const char* basename, program_args &args, long timestamp) {
        std::ostringstream oss;
        oss << root_dir << "/" << basename << "-" << args.min_num_workers << "-" << args.max_num_workers << "-" << args.stream_size << "-" << args.target_service_time << "-" << timestamp << ".csv";
        auto file_name = oss.str();
        mkdir(root_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        file.open(file_name);
        return file_name;
    }
};


#endif //AUTONOMICFARM_FARMANALYTICS_HPP
