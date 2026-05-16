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

#include "BackendRegistry.h"

namespace Tony {
namespace Backend {

bool
BackendRegistry::registerAdapter(const QSharedPointer<BackendAdapter> &adapter)
{
    if (adapter.isNull() || adapter->engineId().isEmpty() ||
        contains(adapter->engineId())) {
        return false;
    }
    m_adapters.push_back(adapter);
    return true;
}

bool
BackendRegistry::addManifest(const BackendManifest &manifest)
{
    const BackendId backendId = manifest.id();
    if (!Tony::Backend::isValidBackendId(backendId) || hasBackend(backendId)) {
        return false;
    }

    BackendManifest stored = manifest;
    if (stored.backendId.isEmpty()) {
        stored.backendId = backendId;
    }
    if (stored.engineId.isEmpty()) {
        stored.engineId = backendId;
    }
    stored.status = BackendStatus::NotConfigured;

    m_manifests.push_back(stored);
    return true;
}

bool
BackendRegistry::removeManifest(const BackendId &backendId)
{
    for (int i = 0; i < m_manifests.size(); ++i) {
        if (m_manifests[i].id() == backendId) {
            m_manifests.removeAt(i);
            return true;
        }
    }
    return false;
}

bool
BackendRegistry::hasBackend(const BackendId &backendId) const
{
    return contains(backendId) || manifestById(backendId).has_value();
}

std::optional<BackendManifest>
BackendRegistry::manifestById(const BackendId &backendId) const
{
    for (const auto &manifest: m_manifests) {
        if (manifest.id() == backendId) {
            return manifest;
        }
    }
    return std::nullopt;
}

QVector<BackendManifest>
BackendRegistry::allManifests() const
{
    QVector<BackendManifest> result = m_manifests;
    for (const auto &adapter: m_adapters) {
        result.push_back(adapter->manifest());
    }
    return result;
}

void
BackendRegistry::clear()
{
    m_adapters.clear();
    m_manifests.clear();
}

QVector<BackendId>
BackendRegistry::engineIds() const
{
    QVector<BackendId> ids;
    ids.reserve(m_adapters.size() + m_manifests.size());
    for (const auto &manifest: m_manifests) {
        ids.push_back(manifest.id());
    }
    for (const auto &adapter: m_adapters) {
        ids.push_back(adapter->engineId());
    }
    return ids;
}

QVector<BackendManifest>
BackendRegistry::manifests() const
{
    QVector<BackendManifest> result;
    result.reserve(m_adapters.size());
    for (const auto &adapter: m_adapters) {
        result.push_back(adapter->manifest());
    }
    return result;
}

QSharedPointer<BackendAdapter>
BackendRegistry::adapter(const BackendId &engineId) const
{
    for (const auto &candidate: m_adapters) {
        if (candidate->engineId() == engineId) {
            return candidate;
        }
    }
    return {};
}

bool
BackendRegistry::contains(const BackendId &engineId) const
{
    return !adapter(engineId).isNull();
}

int
BackendRegistry::size() const
{
    return m_adapters.size();
}

}
}
