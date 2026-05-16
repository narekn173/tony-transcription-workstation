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

#ifndef TONY_BACKEND_SETTINGS_PATH_RESOLVER_H
#define TONY_BACKEND_SETTINGS_PATH_RESOLVER_H

#include "ResultValidator.h"

#include <QString>

namespace Tony {
namespace Backend {

struct BackendSettingsPathResolutionResult
{
    ValidationReport report;
    QString path;
    QString baseDirectory;
    QString fileName;
    bool usedTestOverride = false;

    bool isValid() const;
};

class BackendSettingsPathResolver
{
public:
    static QString defaultFileName();
    static ValidationReport validateFileName(const QString &fileName);

    BackendSettingsPathResolutionResult
    resolveDefaultPath(const QString &fileName = defaultFileName()) const;

    BackendSettingsPathResolutionResult
    resolveDefaultPathFromBaseDirectory(
        const QString &baseDirectory,
        const QString &fileName = defaultFileName()) const;

    BackendSettingsPathResolutionResult
    resolveTestOverridePath(const QString &path) const;
};

}
}

#endif
