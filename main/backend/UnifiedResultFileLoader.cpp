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

#include "UnifiedResultFileLoader.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

namespace Tony {
namespace Backend {

bool
UnifiedResultFileLoadResult::isValid() const
{
    return report.isValid();
}

UnifiedResultFileLoadResult
UnifiedResultFileLoader::load(const QString &path) const
{
    UnifiedResultFileLoadResult loaded;
    loaded.path = path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        loaded.report.addError("file_open_failed",
                               QString("Unable to open UnifiedResult file: %1")
                                   .arg(file.errorString()));
        return loaded;
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        loaded.report.addError("invalid_json",
                               QString("Unable to parse UnifiedResult JSON: %1")
                                   .arg(parseError.errorString()));
        return loaded;
    }

    if (!document.isObject()) {
        loaded.report.addError("invalid_top_level_json",
                               "UnifiedResult JSON must have an object at the top level.");
        return loaded;
    }

    UnifiedResultParser parser;
    const UnifiedResultParseResult parsed = parser.parse(document.object());
    loaded.result = parsed.result;
    loaded.report = parsed.report;
    return loaded;
}

}
}
