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

#include "BackendSettingsFactory.h"

namespace Tony {
namespace Backend {

namespace {

void
appendIssues(ValidationReport &target, const ValidationReport &source)
{
    for (const auto &issue: source.issues) {
        target.addIssue(issue.severity, issue.code, issue.message);
    }
}

}

bool
BackendSettingsPersistenceComponents::isValid() const
{
    return report.isValid() && config.hasAnyUsableSettingsFilePath();
}

BackendSettingsPersistenceComponents
BackendSettingsFactory::createDefaultPersistenceComponents() const
{
    BackendSettingsPathResolver resolver;
    const BackendSettingsPathResolutionResult resolved =
        resolver.resolveDefaultPath();
    return createPersistenceComponentsFromResolvedPath(resolved);
}

BackendSettingsPersistenceComponents
BackendSettingsFactory::createPersistenceComponents(
    const BackendSettingsPersistenceConfig &config) const
{
    BackendSettingsPersistenceComponents components;
    components.config = config;

    appendIssues(components.report, components.config.validate());

    if (!components.config.hasAnyUsableSettingsFilePath()) {
        components.report.addError(
            "no_settings_file_path",
            "No usable BackendSettings persistence path is configured.");
    }

    return components;
}

BackendSettingsPersistenceComponents
BackendSettingsFactory::createPersistenceComponentsFromResolvedPath(
    const BackendSettingsPathResolutionResult &resolved) const
{
    BackendSettingsPersistenceConfig config;
    BackendSettingsPersistenceComponents components;

    appendIssues(components.report, config.applyResolvedDefaultPath(resolved));

    const BackendSettingsPersistenceComponents configured =
        createPersistenceComponents(config);

    components.config = configured.config;
    appendIssues(components.report, configured.report);

    return components;
}

BackendSettingsPersistenceComponents
BackendSettingsFactory::createTestPersistenceComponents(
    const QString &testOverridePath) const
{
    BackendSettingsPathResolver resolver;
    const BackendSettingsPathResolutionResult resolved =
        resolver.resolveTestOverridePath(testOverridePath);

    BackendSettingsPersistenceConfig config;
    BackendSettingsPersistenceComponents components;

    appendIssues(components.report, resolved.report);
    if (resolved.isValid()) {
        config.testOnlySettingsFilePath = resolved.path;
    }

    const BackendSettingsPersistenceComponents configured =
        createPersistenceComponents(config);

    components.config = configured.config;
    appendIssues(components.report, configured.report);

    return components;
}

}
}
