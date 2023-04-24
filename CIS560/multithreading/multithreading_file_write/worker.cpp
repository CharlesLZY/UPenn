#include "worker.h"
#include <QThread>

Worker::Worker(ofstream *file, QMutex *mutex, int numToWrite)
    : outFile(file), numToWrite(numToWrite), mutex(mutex)
{}

#define MUTEX 1

void Worker::run() {
    QString msg("My number to write is " + QString::number(numToWrite) + "\n");
    QThread::msleep(50);
#if MUTEX
    mutex->lock();
#endif
    *outFile << msg.toStdString().c_str();
#if MUTEX
    mutex->unlock();
#endif
}
