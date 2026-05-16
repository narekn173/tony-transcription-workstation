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

#ifndef TONY_BACKEND_DISCOVERY_CONFIG_H
#define TONY_BACKEND_DISCOVERY_CONFIG_H

#include "ResultValidator.h"

#include <QString>
#include <QVector>

namespace Tony {
namespace Backend {

enum class BackendDiscoveryPathSource {
    DefaultLocal,
    UserConfigured,
    TestOnly
};

struct BackendDiscoveryPath
{
    BackendDiscoveryPathSource source = BackendDiscoveryPathSource::DefaultLocal;
    QString path;

    bool isUsable() const;
    QString sourceName() const;
};

struct BackendDiscoveryConfig
{
    QString defaultLocalManifestDirectoryPath;
    QString userConfiguredManifestDirectoryPath;
    QString testOnlyManifestDirectoryPath;

    static BackendDiscoveryConfig safeDefaults();
    static QString defaultLocalManifestDirectoryPathConcept();

    QVector<BackendDiscoveryPath> manifestDirectoryCandidates() const;
    ValidationReport validate() const;
    bool hasAnyUsableManifestDirectory() const;
};

}
}

#endif
