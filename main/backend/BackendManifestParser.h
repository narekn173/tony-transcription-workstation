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

#ifndef TONY_BACKEND_MANIFEST_PARSER_H
#define TONY_BACKEND_MANIFEST_PARSER_H

#include "BackendTypes.h"
#include "ResultValidator.h"

#include <QJsonObject>

namespace Tony {
namespace Backend {

struct BackendManifestParseResult
{
    BackendManifest manifest;
    ValidationReport report;

    bool isValid() const;
};

class BackendManifestParser
{
public:
    BackendManifestParseResult parse(const QJsonObject &object) const;

private:
    static BackendRuntimeType parseRuntimeType(const QString &value,
                                               ValidationReport &report);
    static QStringList parseStringArray(const QJsonObject &object,
                                        const QString &key,
                                        ValidationReport &report);
    static bool parseBool(const QJsonObject &object,
                          const QString &key,
                          ValidationReport &report);
};

}
}

#endif
