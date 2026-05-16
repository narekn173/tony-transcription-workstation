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

#ifndef TONY_BACKEND_DISCOVERY_SERVICE_H
#define TONY_BACKEND_DISCOVERY_SERVICE_H

#include "BackendDiscoveryConfig.h"
#include "BackendManifestDirectoryLoader.h"

namespace Tony {
namespace Backend {

struct BackendDiscoveryDirectoryResult
{
    BackendDiscoveryPath directory;
    BackendManifestDirectoryLoadResult loadResult;

    bool isValid() const;
    int manifestCount() const;
    int loadedCount() const;
    int rejectedCount() const;
};

struct BackendDiscoveryResult
{
    BackendDiscoveryConfig config;
    QVector<BackendDiscoveryDirectoryResult> directories;
    ValidationReport report;

    bool isValid() const;
    int directoriesScanned() const;
    int manifestsLoaded() const;
    int manifestsRejected() const;
    int duplicateIds() const;
};

class BackendDiscoveryService
{
public:
    BackendDiscoveryResult discover(const BackendDiscoveryConfig &config,
                                    BackendRegistry &registry) const;
};

}
}

#endif
