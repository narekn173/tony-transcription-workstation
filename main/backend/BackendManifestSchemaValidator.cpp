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

#include "BackendManifestSchemaValidator.h"

#include "BackendManifestParser.h"

namespace Tony {
namespace Backend {

namespace {

QString
backendManifestSchemaPath()
{
    return "docs/schemas/backend_manifest.schema.json";
}

}

bool
BackendManifestSchemaValidationResult::isValid() const
{
    return report.isValid();
}

QString
BackendManifestSchemaValidationResult::debugSummaryString() const
{
    return QString("schema=%1 full_json_schema=%2 valid=%3 issues=%4")
        .arg(schemaReferencePath)
        .arg(fullJsonSchemaValidationApplied ? "applied" : "deferred")
        .arg(isValid() ? "true" : "false")
        .arg(report.issues.size());
}

BackendManifestSchemaValidationResult
BackendManifestSchemaValidator::validateLightweight(
    const QJsonObject &object) const
{
    BackendManifestSchemaValidationResult result;
    result.schemaReferencePath = schemaReferencePath();

    BackendManifestParser parser;
    const BackendManifestParseResult parsed = parser.parse(object);
    result.report = parsed.report;
    return result;
}

BackendManifestSchemaValidationResult
BackendManifestSchemaValidator::validateParsedManifest(
    const BackendManifest &manifest) const
{
    BackendManifestSchemaValidationResult result;
    result.schemaReferencePath = schemaReferencePath();

    if (manifest.contractVersion.isEmpty()) {
        result.report.addError("missing_contract_version",
                               "BackendManifest contract version is missing.");
    }

    if (manifest.id().isEmpty()) {
        result.report.addError("missing_backend_id",
                               "BackendManifest backend ID is required.");
    } else if (!manifest.isValidBackendId()) {
        result.report.addError("invalid_backend_id",
                               "BackendManifest backend ID must be lowercase snake_case.");
    }

    if (manifest.displayName.isEmpty()) {
        result.report.addError("missing_display_name",
                               "BackendManifest display name is required.");
    }

    return result;
}

bool
BackendManifestSchemaValidator::hasFullJsonSchemaValidator() const
{
    return false;
}

QString
BackendManifestSchemaValidator::schemaReferencePath() const
{
    return backendManifestSchemaPath();
}

}
}
