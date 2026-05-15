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

#include "UnifiedResultSchemaValidator.h"

#include "UnifiedResultParser.h"

namespace Tony {
namespace Backend {

namespace {

QString
unifiedResultSchemaPath()
{
    return "docs/schemas/unified_result.schema.json";
}

}

bool
UnifiedResultSchemaValidationResult::isValid() const
{
    return report.isValid();
}

QString
UnifiedResultSchemaValidationResult::debugSummaryString() const
{
    return QString("schema=%1 full_json_schema=%2 valid=%3 issues=%4")
        .arg(schemaReferencePath)
        .arg(fullJsonSchemaValidationApplied ? "applied" : "deferred")
        .arg(isValid() ? "true" : "false")
        .arg(report.issues.size());
}

UnifiedResultSchemaValidationResult
UnifiedResultSchemaValidator::validateLightweight(const QJsonObject &object) const
{
    UnifiedResultSchemaValidationResult result;
    result.schemaReferencePath = schemaReferencePath();

    UnifiedResultParser parser;
    const UnifiedResultParseResult parsed = parser.parse(object);
    result.report = parsed.report;
    return result;
}

UnifiedResultSchemaValidationResult
UnifiedResultSchemaValidator::validateParsedResult(
    const BackendRequest &request,
    const UnifiedResult &unifiedResult) const
{
    UnifiedResultSchemaValidationResult result;
    result.schemaReferencePath = schemaReferencePath();

    ResultValidator validator;
    result.report = validator.validate(request, unifiedResult);
    return result;
}

bool
UnifiedResultSchemaValidator::hasFullJsonSchemaValidator() const
{
    return false;
}

QString
UnifiedResultSchemaValidator::schemaReferencePath() const
{
    return unifiedResultSchemaPath();
}

}
}
