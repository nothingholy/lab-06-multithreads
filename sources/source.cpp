#include "../third-party/PicoSHA2/picosha2.h"
#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup.hpp>

#include <thread>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
using namespace std;


void PluckHash(size_t threadAmount){
    size_t i = 0;
    while(i < 1024 * 1024 / threadAmount) {
        string newString = to_string(rand());
        string hash = picosha2::hash256_hex_string(newString);

        if (hash.substr(hash.size() - 4) == "0000")
            BOOST_LOG_TRIVIAL(info) << "0000 found in hash '" << hash << "' in string \"" << newString << "\"";
        else
            BOOST_LOG_TRIVIAL(trace) << "Hash '" << hash << "' from string \"" << newString << "\"";

        i++;
    }
}

void log_setup(){
    boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level , char >("Severity");

    auto FileLog = boost::log::add_file_log(
            //file name
            boost::log::keywords::file_name = "../logs/log_%N.log",
            //file size(10 MB)
            boost::log::keywords::rotation_size = 10 * 1024 * 1024,
            //time file rotation
            boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point{0,0,0},
            // format
            boost::log::keywords::format = "[%TimeStamp%][%Severity%][%ThreadID%]: %Message%"
    );

    auto ConsoleLog = boost::log::add_console_log(
            //console output
            cout,
            // format
            boost::log::keywords::format = "[%TimeStamp%][%Severity%][%ThreadID%]: %Message%"
    );

    //Filtration
    FileLog->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::trace
    );
    ConsoleLog->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::info
    );
}

int main(int argc,char *argv[])
{
    srand(time(nullptr));
    size_t threadAmount = 0;
    if(argc >= 2)
        threadAmount = boost::lexical_cast<size_t>(argv[1]);
    else
        threadAmount = boost::thread::hardware_concurrency();

    log_setup();
    boost::log::add_common_attributes();

    BOOST_LOG_TRIVIAL(trace) << "Number of threads: " << threadAmount;

    vector<boost::thread> threads;
    threads.reserve(threadAmount);
    for(size_t i = 0; i < threadAmount; i++)
        threads.emplace_back(PluckHash,threadAmount);

    for(boost::thread &thread : threads)
        thread.join();

    return 0;
}
