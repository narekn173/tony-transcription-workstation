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

#ifndef TONY_ANALYSIS_ENGINE_MANAGER_H
#define TONY_ANALYSIS_ENGINE_MANAGER_H

#include "BackendRegistry.h"

namespace Tony {
namespace Backend {

class AnalysisEngineManager
{
public:
    AnalysisEngineManager();

    BackendRegistry &registry();
    const BackendRegistry &registry() const;

    bool registerAdapter(const QSharedPointer<BackendAdapter> &adapter);
    QVector<BackendManifest> availableManifests() const;
    AnalysisRunSummary startAnalysis(const BackendRequest &request) const;

private:
    BackendRegistry m_registry;
};

}
}

#endif
