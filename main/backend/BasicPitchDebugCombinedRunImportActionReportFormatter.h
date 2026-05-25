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

#ifndef TONY_BASIC_PITCH_DEBUG_COMBINED_RUN_IMPORT_ACTION_REPORT_FORMATTER_H
#define TONY_BASIC_PITCH_DEBUG_COMBINED_RUN_IMPORT_ACTION_REPORT_FORMATTER_H

#include "BasicPitchDebugCombinedRunImportAction.h"

namespace Tony {
namespace Backend {

struct BasicPitchDebugCombinedRunImportActionReportText
{
    QString title;
    QString plainText;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
    bool importedIntoTonyLayers = false;
    bool insertedIntoView = false;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
};

class BasicPitchDebugCombinedRunImportActionReportFormatter
{
public:
    BasicPitchDebugCombinedRunImportActionReportText fromReport(
        const BasicPitchDebugCombinedRunImportActionReport &report) const;
};

}
}

#endif
