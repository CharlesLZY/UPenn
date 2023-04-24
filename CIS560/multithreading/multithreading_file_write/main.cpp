#include <iostream>
#include "worker.h"
#include <QThreadPool>
#include <QMutex>
#include <fstream>
#include <thread>
#include <mutex>

using namespace std;

#define STD_THREADING
//#define QT_THREADING

void write_to_file(ofstream &file) {
    file << this_thread::get_id() << endl;
}

int main() {
    ofstream file;
    file.open("out.txt", ios::out | ios::trunc);
#ifdef STD_THREADING
    mutex mut;
    vector<thread> spawned_threads;
    for(int i = 0; i < 20; ++i) {
        spawned_threads.push_back(thread(write_to_file, std::ref(file)));
    }
    for(auto &t : spawned_threads) {
        t.join();
    }
    file.close();
    cout << "done" << endl;
    return 0;
#endif
#ifdef QT_THREADING
    QMutex mut;
    for(int i = 0; i < 20; ++i) {
        Worker *w = new Worker(&file, &mut, i);
        QThreadPool::globalInstance()->start(w);
    }
    QThreadPool::globalInstance()->waitForDone();
    file.close();
    cout << "done" << endl;
    return 0;
#endif
}
