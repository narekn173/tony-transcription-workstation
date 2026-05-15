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

#include "AnalysisEngineManager.h"

namespace Tony {
namespace Backend {

AnalysisEngineManager::AnalysisEngineManager() = default;

BackendRegistry &
AnalysisEngineManager::registry()
{
    return m_registry;
}

const BackendRegistry &
AnalysisEngineManager::registry() const
{
    return m_registry;
}

bool
AnalysisEngineManager::registerAdapter(const QSharedPointer<BackendAdapter> &adapter)
{
    return m_registry.registerAdapter(adapter);
}

QVector<BackendManifest>
AnalysisEngineManager::availableManifests() const
{
    return m_registry.manifests();
}

AnalysisRunSummary
AnalysisEngineManager::startAnalysis(const BackendRequest &request) const
{
    AnalysisRunSummary summary = AnalysisRunSummary::notImplemented(request.engineId);
    summary.runId = request.requestId;
    if (!m_registry.contains(request.engineId)) {
        summary.errors.push_back({
            BackendErrorCode::BackendNotConfigured,
            "No backend adapter is registered for this engine.",
            true
        });
    }
    return summary;
}

}
}
