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

#include "TonyLayerImporter.h"

namespace Tony {
namespace Backend {

TonyLayerImportResult
TonyLayerImporter::importResult(const UnifiedResult &) const
{
    TonyLayerImportResult result;
    result.succeeded = false;
    result.error = {
        BackendErrorCode::NotImplemented,
        "Tony layer import is intentionally disabled in the skeleton.",
        true
    };
    return result;
}

}
}
