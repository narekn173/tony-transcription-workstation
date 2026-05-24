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

#ifndef TONY_UNIFIED_RESULT_FILE_WRITER_H
#define TONY_UNIFIED_RESULT_FILE_WRITER_H

#include "ResultValidator.h"

#include <QString>

namespace Tony {
namespace Backend {

struct UnifiedResultFileWriteResult
{
    ValidationReport report;
    QString path;
    qint64 bytesWritten = 0;
    bool wroteFile = false;
    bool createdParentDirectory = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class UnifiedResultFileWriter
{
public:
    UnifiedResultFileWriteResult write(const QString &path,
                                       const UnifiedResult &result) const;
};

}
}

#endif
