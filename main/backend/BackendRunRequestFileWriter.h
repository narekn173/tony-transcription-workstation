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

#ifndef TONY_BACKEND_RUN_REQUEST_FILE_WRITER_H
#define TONY_BACKEND_RUN_REQUEST_FILE_WRITER_H

#include "BackendRunRequestSerializer.h"

#include <QString>

namespace Tony {
namespace Backend {

struct BackendRunRequestFileWriteResult
{
    ValidationReport report;
    QString path;
    qint64 bytesWritten = 0;

    bool isValid() const;
};

class BackendRunRequestFileWriter
{
public:
    BackendRunRequestFileWriteResult write(
        const QString &path,
        const BackendRunRequestBuildResult &request) const;
};

}
}

#endif
