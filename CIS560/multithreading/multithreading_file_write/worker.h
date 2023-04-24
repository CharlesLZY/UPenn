#pragma once
#include <QRunnable>
#include <QFile>
#include <QMutex>
#include <fstream>
using namespace std;

class Worker : public QRunnable {
private:
    ofstream *outFile;
    int numToWrite;
    QMutex *mutex;
public:
    Worker(ofstream *file, QMutex *mutex, int numToWrite);
    void run() override;
};
