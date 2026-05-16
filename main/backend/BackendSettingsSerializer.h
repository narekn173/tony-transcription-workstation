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

#ifndef TONY_BACKEND_SETTINGS_SERIALIZER_H
#define TONY_BACKEND_SETTINGS_SERIALIZER_H

#include "BackendSettingsStore.h"

#include <QJsonArray>
#include <QJsonObject>

namespace Tony {
namespace Backend {

struct BackendSettingsSerializationResult
{
    BackendSettings settings;
    ValidationReport report;

    bool isValid() const;
};

struct BackendSettingsStoreSerializationResult
{
    BackendSettingsStore store;
    ValidationReport report;
    int loadedCount = 0;
    int rejectedCount = 0;

    bool isValid() const;
};

class BackendSettingsSerializer
{
public:
    QJsonObject toJson(const BackendSettings &settings) const;
    BackendSettingsSerializationResult fromJson(const QJsonObject &object) const;

    QJsonArray storeToJsonArray(const BackendSettingsStore &store) const;
    QJsonObject storeToJsonObject(const BackendSettingsStore &store) const;
    BackendSettingsStoreSerializationResult storeFromJsonArray(const QJsonArray &array) const;
    BackendSettingsStoreSerializationResult storeFromJsonObject(const QJsonObject &object) const;

private:
    static QJsonObject environmentToJson(const QMap<QString, QString> &environment);
    static QMap<QString, QString> environmentFromJson(const QJsonObject &object);
};

}
}

#endif
