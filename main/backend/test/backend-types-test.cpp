/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Tony
    An intonation analysis and annotation tool
    Centre for Digital Music, Queen Mary, University of London.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "TestBackendTypes.h"
#include "TestUnifiedResult.h"

#include <QCoreApplication>
#include <QThread>
#include <QtTest>

#include <iostream>

namespace {

int runExternalProcessHelper(int argc, char *argv[])
{
    int helperIndex = -1;
    for (int i = 1; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == "--external-process-helper") {
            helperIndex = i;
            break;
        }
    }

    if (helperIndex < 0 || helperIndex + 1 >= argc) {
        return -1;
    }

    const QString command = QString::fromLocal8Bit(argv[helperIndex + 1]);

    if (command == "success") {
        return 0;
    }
    if (command == "failure") {
        return 7;
    }
    if (command == "stdout") {
        const QString text =
            helperIndex + 2 < argc ?
                QString::fromLocal8Bit(argv[helperIndex + 2]) :
                QString("stdout");
        std::cout << text.toStdString() << std::endl;
        return 0;
    }
    if (command == "stderr") {
        const QString text =
            helperIndex + 2 < argc ?
                QString::fromLocal8Bit(argv[helperIndex + 2]) :
                QString("stderr");
        std::cerr << text.toStdString() << std::endl;
        return 0;
    }
    if (command == "stdout-stderr") {
        const QString stdoutText =
            helperIndex + 2 < argc ?
                QString::fromLocal8Bit(argv[helperIndex + 2]) :
                QString("stdout");
        const QString stderrText =
            helperIndex + 3 < argc ?
                QString::fromLocal8Bit(argv[helperIndex + 3]) :
                QString("stderr");
        std::cout << stdoutText.toStdString() << std::endl;
        std::cerr << stderrText.toStdString() << std::endl;
        return 0;
    }
    if (command == "sleep") {
        const unsigned long msec =
            helperIndex + 2 < argc ?
                QString::fromLocal8Bit(argv[helperIndex + 2]).toULong() :
                1000UL;
        QThread::msleep(msec);
        return 0;
    }

    std::cerr << "unknown helper command" << std::endl;
    return 99;
}

}

int main(int argc, char *argv[])
{
    const int helperExitCode = runExternalProcessHelper(argc, argv);
    if (helperExitCode >= 0) {
        return helperExitCode;
    }

    QCoreApplication app(argc, argv);
    app.setOrganizationName("tony");
    app.setApplicationName("test-backend-types");

    TestBackendTypes backendTypes;
    TestUnifiedResult unifiedResult;

    int failures = 0;
    failures += QTest::qExec(&backendTypes, argc, argv);
    failures += QTest::qExec(&unifiedResult, argc, argv);

    if (failures == 0) {
        std::cerr << "All tests passed" << std::endl;
        return 0;
    } else {
        return 1;
    }
}
