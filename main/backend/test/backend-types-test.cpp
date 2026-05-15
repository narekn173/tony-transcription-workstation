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

#include <QCoreApplication>
#include <QtTest>

#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("tony");
    app.setApplicationName("test-backend-types");

    TestBackendTypes t;
    if (QTest::qExec(&t, argc, argv) == 0) {
        std::cerr << "All tests passed" << std::endl;
        return 0;
    } else {
        return 1;
    }
}
