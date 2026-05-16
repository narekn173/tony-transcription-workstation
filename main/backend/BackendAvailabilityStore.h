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

#ifndef TONY_BACKEND_AVAILABILITY_STORE_H
#define TONY_BACKEND_AVAILABILITY_STORE_H

#include "BackendAvailabilityProbe.h"

#include <QVector>

#include <optional>

namespace Tony {
namespace Backend {

class BackendAvailabilityStore
{
public:
    bool setReport(const BackendAvailabilityReport &report);
    bool hasReport(const BackendId &backendId) const;
    std::optional<BackendAvailabilityReport> reportById(
        const BackendId &backendId) const;
    bool removeReport(const BackendId &backendId);
    QVector<BackendAvailabilityReport> allReports() const;
    QVector<BackendId> backendIds() const;
    void clear();
    int size() const;
    QString debugSummaryString() const;

private:
    QVector<BackendAvailabilityReport> m_reports;
};

}
}

#endif
