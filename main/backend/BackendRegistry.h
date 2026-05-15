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

#ifndef TONY_BACKEND_REGISTRY_H
#define TONY_BACKEND_REGISTRY_H

#include "BackendAdapter.h"

#include <QSharedPointer>
#include <QVector>

namespace Tony {
namespace Backend {

class BackendRegistry
{
public:
    bool registerAdapter(const QSharedPointer<BackendAdapter> &adapter);

    QVector<BackendId> engineIds() const;
    QVector<BackendManifest> manifests() const;
    QSharedPointer<BackendAdapter> adapter(const BackendId &engineId) const;
    bool contains(const BackendId &engineId) const;
    int size() const;

private:
    QVector<QSharedPointer<BackendAdapter>> m_adapters;
};

}
}

#endif
