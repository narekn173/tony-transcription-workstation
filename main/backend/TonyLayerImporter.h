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

#ifndef TONY_LAYER_IMPORTER_H
#define TONY_LAYER_IMPORTER_H

#include "BackendTypes.h"
#include "ResultValidator.h"
#include "UnifiedResult.h"

#include "data/model/Model.h"

namespace sv {
class Document;
class Layer;
}

namespace Tony {
namespace Backend {

struct TonyLayerImportOptions
{
    double sampleRate = 0.0;
    int resolution = 1;
    bool useFlexiNoteLayer = false;
    sv::Document *document = nullptr;
    bool createDocumentLayer = false;
};

struct TonyLayerImportResult
{
    bool succeeded = false;
    bool importedIntoTonyLayers = false;
    bool modelCreated = false;
    bool modelRegistered = false;
    bool documentLayerCreated = false;
    bool insertedIntoView = false;
    bool sourceMarkedDevMock = false;
    QString createdModelType;
    QString createdLayerType;
    int noteCount = 0;
    sv::ModelId modelId;
    sv::Layer *layer = nullptr;
    ValidationReport report;
    BackendError error;

    bool isValid() const;
    QString debugSummaryString() const;
};

class TonyLayerImporter
{
public:
    TonyLayerImportResult importResult(const UnifiedResult &result) const;
    TonyLayerImportResult importResult(
        const UnifiedResult &result,
        const TonyLayerImportOptions &options) const;
};

}
}

#endif
