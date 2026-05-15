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

#ifndef TONY_RESULT_VALIDATOR_H
#define TONY_RESULT_VALIDATOR_H

#include "BackendTypes.h"

namespace Tony {
namespace Backend {

enum class ValidationSeverity {
    Info,
    Warning,
    Error
};

struct ValidationIssue
{
    ValidationSeverity severity = ValidationSeverity::Error;
    QString code;
    QString message;
};

struct ValidationReport
{
    QVector<ValidationIssue> issues;

    bool isValid() const;
    void addError(const QString &code, const QString &message);
};

class ResultValidator
{
public:
    ValidationReport validate(const BackendRequest &request,
                              const UnifiedResult &result) const;
};

}
}

#endif
