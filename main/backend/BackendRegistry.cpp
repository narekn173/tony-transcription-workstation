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

QVector<BackendId>
BackendRegistry::engineIds() const
{
    QVector<BackendId> ids;
    ids.reserve(m_adapters.size());
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
