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

#ifndef TONY_UNIFIED_RESULT_SCHEMA_VALIDATOR_H
#define TONY_UNIFIED_RESULT_SCHEMA_VALIDATOR_H

#include "ResultValidator.h"
#include "UnifiedResult.h"

#include <QJsonObject>
#include <QString>

namespace Tony {
namespace Backend {

struct UnifiedResultSchemaValidationResult
{
    ValidationReport report;
    QString schemaReferencePath;
    bool fullJsonSchemaValidationApplied = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class UnifiedResultSchemaValidator
{
public:
    /*
       This boundary intentionally performs only the lightweight validation
       currently available in Tony: JSON-object parsing, required field checks
       in UnifiedResultParser, and semantic checks in ResultValidator.

       Full JSON Schema validation against docs/schemas/unified_result.schema.json
       is deferred because the repository does not yet carry an approved JSON
       Schema engine dependency. Adding one affects Windows build, packaging,
       and license review, so the future implementation should plug into this
       class without changing parser or importer call sites.
    */
    UnifiedResultSchemaValidationResult validateLightweight(
        const QJsonObject &object) const;

    UnifiedResultSchemaValidationResult validateParsedResult(
        const BackendRequest &request,
        const UnifiedResult &result) const;

    bool hasFullJsonSchemaValidator() const;
    QString schemaReferencePath() const;
};

}
}

#endif
