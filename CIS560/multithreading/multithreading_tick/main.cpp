#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <array>
#include <random>
#include <chrono>


using namespace std;

void generate_strings(vector<string>& to_process,
                      mutex& to_process_mutex,
                      vector<string>& output,
                      mutex& output_mutex,
                      bool& shutdown) {
    // Repeatedly check this thread's input vector<string> to_process
    // and whenever it has data, process it, output it, and clear it.
    while(true) {
        // Stop the infinite loop if we are told to shut down
        // by the main thread
        if(shutdown) {
            break;
        }
        to_process_mutex.lock();
        for(auto &s : to_process) {
            string result = s + " PROCESSED";
            // Emulate the long runtime of a complicated operation
            // by forcing the thread to sleep for 2 seconds
            this_thread::sleep_for(chrono::milliseconds(2000));
            output_mutex.lock();
            output.push_back(result);
            output_mutex.unlock();
        }
        to_process.clear();
        to_process_mutex.unlock();
    }
}

int main() {
    // If we wanted to close our threads down when ending
    // the program, we can iterate over this vector and call
    // join() on each thread
    vector<thread> spawned_threads;
    // Set this to true when it is time to end the program
    bool shutdown_threads = false;

    // One vector<string> per thread, and one mutex per vector
    // This allows the 8 worker threads to operate entirely
    // independently instead of all reading from one large
    // shared resource (i.e. one vector<string> across all 8 threads)
    array<vector<string>, 8> strings_needing_work;
    array<mutex, 8> strings_needing_work_mutexes;

    // We assume the worker threads will write their output
    // far less frequently than they will query for input,
    // so we have only one shared resource for the threads to
    // write their results to
    vector<string> completed_strings;
    mutex completed_strings_mutex;

    // As a pre-process, instantiate one thread per core of the CPU
    // If threads > cores, the CPU thread scheduler will have them
    // operate in an interleaved manner while sharing CPU core cycles
    for(int i = 0; i < 8; ++i) {
        // Each std::thread constructor takes in a function to call
        // from the spawned thread, and a set of any arguments the function takes.
        // If a function argument is a reference, it needs to be wrapped in
        // std::ref()
        spawned_threads.push_back(thread(generate_strings,
                                         std::ref(strings_needing_work[i]),
                                         std::ref(strings_needing_work_mutexes[i]),
                                         std::ref(completed_strings),
                                         std::ref(completed_strings_mutex),
                                         std::ref(shutdown_threads)));
    }



    random_device dev;
    mt19937 rng(dev());
    uniform_real_distribution<double> dist(0.0, 1.0);

    int thread_index = 0;

    while(true) {
        // Emulate how MyGL::tick() gets called every 16 ms
        this_thread::sleep_for(chrono::milliseconds(16));
        // With a 25% chance, generate a new string that
        // will be processed by a worker thread
        float r = dist(rng);
        if(r < 0.25) {
            string to_be_processed = to_string(r);
            cout << to_be_processed << " generated" << endl;
            // We need to put the newly-made string into
            // one of the resources our main thread shares
            // with the worker threads: the std::vector strings_needing_work

            // To begin, we lock the mutex associated with one of our 8
            // vector<string>s so that the worker thread that shares this
            // resource with us cannot modify it at the same time we do
            strings_needing_work_mutexes[thread_index].lock();
            // With the mutex safely locked, we add the new string to the vector
            strings_needing_work[thread_index].push_back(to_be_processed);
            // With our work on the vector complete, we unlock the mutex to allow
            // the worker thread to continue its work
            strings_needing_work_mutexes[thread_index].unlock();
            // Increment this index counter so that the next time we make
            // a new string, a different thread operates on it
            thread_index = (thread_index + 1) % 8;
        }

        // Whether or not we generated a new string, we will check to see
        // if any worker threads completed an output string for us to print
        completed_strings_mutex.lock();
        for(auto &s : completed_strings) {
            cout << s << endl;
        }
        completed_strings.clear();
        completed_strings_mutex.unlock();
    }

    // Technically this code will never be reached because the while
    // loop above never ends, but were it to be interrupted this is
    // how we would clean up our worker threads
    for(auto &thread : spawned_threads) {
        thread.join();
    }
    return 0;
}
