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

#ifndef TONY_BASIC_PITCH_DEBUG_MANUAL_RUN_ACTION_REPORT_FORMATTER_H
#define TONY_BASIC_PITCH_DEBUG_MANUAL_RUN_ACTION_REPORT_FORMATTER_H

#include "BasicPitchDebugManualRunAction.h"

namespace Tony {
namespace Backend {

struct BasicPitchDebugManualRunActionReportText
{
    QString title;
    QString plainText;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
    bool importedIntoTonyLayers = false;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
};

class BasicPitchDebugManualRunActionReportFormatter
{
public:
    BasicPitchDebugManualRunActionReportText fromReport(
        const BasicPitchDebugManualRunActionReport &report) const;
};

}
}

#endif
