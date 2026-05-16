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

#ifndef TONY_BACKEND_MANIFEST_SCHEMA_VALIDATOR_H
#define TONY_BACKEND_MANIFEST_SCHEMA_VALIDATOR_H

#include "BackendTypes.h"
#include "ResultValidator.h"

#include <QJsonObject>
#include <QString>

namespace Tony {
namespace Backend {

struct BackendManifestSchemaValidationResult
{
    ValidationReport report;
    QString schemaReferencePath;
    bool fullJsonSchemaValidationApplied = false;

    bool isValid() const;
    QString debugSummaryString() const;
};

class BackendManifestSchemaValidator
{
public:
    /*
       This boundary intentionally performs only the lightweight validation
       currently available in Tony: JSON-object parsing plus the structural
       and semantic checks in BackendManifestParser.

       Full JSON Schema validation against
       docs/schemas/backend_manifest.schema.json is deferred because the
       repository does not yet carry an approved JSON Schema engine
       dependency. Adding one affects Windows build, packaging, and license
       review, so the future implementation should plug into this class
       without changing parser, file-loader, registry, or manager call sites.
    */
    BackendManifestSchemaValidationResult validateLightweight(
        const QJsonObject &object) const;

    BackendManifestSchemaValidationResult validateParsedManifest(
        const BackendManifest &manifest) const;

    bool hasFullJsonSchemaValidator() const;
    QString schemaReferencePath() const;
};

}
}

#endif
