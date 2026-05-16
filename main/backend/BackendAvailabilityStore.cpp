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

#include "BackendAvailabilityStore.h"

namespace Tony {
namespace Backend {

bool
BackendAvailabilityStore::setReport(const BackendAvailabilityReport &report)
{
    if (!Tony::Backend::isValidBackendId(report.backendId)) {
        return false;
    }

    for (int i = 0; i < m_reports.size(); ++i) {
        if (m_reports[i].backendId == report.backendId) {
            m_reports[i] = report;
            return true;
        }
    }

    m_reports.push_back(report);
    return true;
}

bool
BackendAvailabilityStore::hasReport(const BackendId &backendId) const
{
    return reportById(backendId).has_value();
}

std::optional<BackendAvailabilityReport>
BackendAvailabilityStore::reportById(const BackendId &backendId) const
{
    if (!Tony::Backend::isValidBackendId(backendId)) {
        return std::nullopt;
    }

    for (const auto &report: m_reports) {
        if (report.backendId == backendId) {
            return report;
        }
    }

    return std::nullopt;
}

bool
BackendAvailabilityStore::removeReport(const BackendId &backendId)
{
    if (!Tony::Backend::isValidBackendId(backendId)) {
        return false;
    }

    for (int i = 0; i < m_reports.size(); ++i) {
        if (m_reports[i].backendId == backendId) {
            m_reports.removeAt(i);
            return true;
        }
    }

    return false;
}

QVector<BackendAvailabilityReport>
BackendAvailabilityStore::allReports() const
{
    return m_reports;
}

QVector<BackendId>
BackendAvailabilityStore::backendIds() const
{
    QVector<BackendId> ids;
    ids.reserve(m_reports.size());
    for (const auto &report: m_reports) {
        ids.push_back(report.backendId);
    }
    return ids;
}

void
BackendAvailabilityStore::clear()
{
    m_reports.clear();
}

int
BackendAvailabilityStore::size() const
{
    return m_reports.size();
}

QString
BackendAvailabilityStore::debugSummaryString() const
{
    return QString("availability_reports=%1").arg(m_reports.size());
}

}
}
