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

#ifndef TONY_BASIC_PITCH_DEBUG_POST_IMPORT_PROOF_ACTION_REPORT_FORMATTER_H
#define TONY_BASIC_PITCH_DEBUG_POST_IMPORT_PROOF_ACTION_REPORT_FORMATTER_H

#include "BasicPitchDebugPostImportProofAction.h"

namespace Tony {
namespace Backend {

struct BasicPitchDebugPostImportProofActionReportText
{
    QString title;
    QString plainText;
    bool productionTranscription = false;
    bool testOnlyDebugOnly = true;
    bool readyInstalledCompletedMutation = false;

    bool isValid() const;
};

class BasicPitchDebugPostImportProofActionReportFormatter
{
public:
    BasicPitchDebugPostImportProofActionReportText fromReport(
        const BasicPitchDebugPostImportProofActionReport &report) const;
};

}
}

#endif
