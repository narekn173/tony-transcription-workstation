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

#include "TestBackendTypes.h"
#include "TestUnifiedResult.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QtTest>

#include <iostream>

namespace {

bool hasArgument(int argc, char *argv[], const QString &flag)
{
    for (int i = 1; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == flag) {
            return true;
        }
    }

    return false;
}

QString argumentValue(int argc, char *argv[], const QString &flag)
{
    for (int i = 1; i + 1 < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == flag) {
            return QString::fromLocal8Bit(argv[i + 1]);
        }
    }

    return QString();
}

QJsonArray testOnlyStringArray(const QStringList &values)
{
    QJsonArray array;
    for (const QString &value : values) {
        array.append(value);
    }

    return array;
}

QJsonObject makeDevMockUnifiedResult(const QJsonObject &request, const QString &requestPath)
{
    const QString backendId = request.value("backend_id").toString("dev_mock_backend");
    const QString runId = request.value("run_id").toString("dev_mock_run");
    const QString inputPath = request.value("input_audio_path").toString();
    const QString outputPath = request.value("output_result_json_path").toString();
    const QJsonObject selectedRegion = request.value("selected_region").toObject();

    double noteStart = 0.25;
    double noteEnd = 0.75;
    if (!selectedRegion.isEmpty()) {
        const double regionStart = selectedRegion.value("start_sec").toDouble(0.0);
        const double regionEnd = selectedRegion.value("end_sec").toDouble(regionStart + 1.0);
        noteStart = regionStart + 0.1;
        noteEnd = qMin(regionEnd, noteStart + 0.5);
        if (noteEnd < noteStart) {
            noteEnd = noteStart;
        }
    }

    QJsonObject noteSource;
    noteSource.insert("engine_id", backendId);
    noteSource.insert("dev_mock", true);
    noteSource.insert("test_only", true);
    noteSource.insert("production_transcription", false);

    QJsonObject note;
    note.insert("id", "dev_mock_note_0001");
    note.insert("start_sec", noteStart);
    note.insert("end_sec", noteEnd);
    note.insert("midi_pitch", 60);
    note.insert("frequency_hz", 261.63);
    note.insert("velocity", 64);
    note.insert("confidence", 0.5);
    note.insert("label", "dev-mock-test-only");
    note.insert("source", noteSource);
    note.insert("flags", testOnlyStringArray({"dev_mock", "test_only"}));

    QJsonObject warning;
    warning.insert("code", "DEV_MOCK_TEST_ONLY");
    warning.insert("severity", "warning");
    warning.insert("message", "Dev/mock backend output is test-only and not production transcription.");
    warning.insert("details", QJsonObject{
        {"dev_mock", true},
        {"test_only", true},
        {"production_transcription", false}
    });

    QJsonObject engine;
    engine.insert("engine_id", backendId);
    engine.insert("display_name", "Dev Mock Backend (test only)");
    engine.insert("engine_version", "0.1.0-test");
    engine.insert("adapter_version", "0.1.0-test");
    engine.insert("runtime_type", "development_test");
    engine.insert("device_used", "cpu");

    QJsonObject audio;
    audio.insert("path", inputPath);
    audio.insert("duration_sec", 2.0);
    audio.insert("sample_rate_hz", 44100);
    audio.insert("channels", 1);

    QJsonObject summary;
    summary.insert("note_count", 1);
    summary.insert("pitch_point_count", 0);
    summary.insert("pitch_bend_count", 0);
    summary.insert("technique_label_count", 0);
    summary.insert("dev_mock", true);
    summary.insert("test_only", true);

    QJsonObject provenance;
    provenance.insert("created_by", "CODEX-088 dev/mock backend helper");
    provenance.insert("created_at_utc", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    provenance.insert("dev_mock", true);
    provenance.insert("test_only", true);
    provenance.insert("production_transcription", false);
    provenance.insert("request_json_path", requestPath);
    provenance.insert("output_result_json_path", outputPath);

    QJsonObject result;
    result.insert("contract_version", "0.1");
    result.insert("result_id", QString("dev_mock_result_%1").arg(runId));
    result.insert("request_id", runId);
    result.insert("created_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    result.insert("engine", engine);
    result.insert("status", "completed_with_warnings");
    result.insert("audio", audio);
    result.insert("region", selectedRegion.isEmpty() ? QJsonValue(QJsonValue::Null) : QJsonValue(selectedRegion));
    result.insert("summary", summary);
    result.insert("notes", QJsonArray{note});
    result.insert("pitch_curve", QJsonArray());
    result.insert("pitch_bends", QJsonArray());
    result.insert("technique_labels", QJsonArray());
    result.insert("warnings", QJsonArray{warning});
    result.insert("errors", QJsonArray());
    result.insert("files", QJsonArray());
    result.insert("provenance", provenance);

    return result;
}

int runDevMockBackendHelper(int argc, char *argv[])
{
    if (!hasArgument(argc, argv, "--dev-mock-backend")) {
        return -1;
    }

    const QString requestPath = argumentValue(argc, argv, "--request");
    if (requestPath.trimmed().isEmpty()) {
        std::cerr << "dev-mock-backend: missing --request argument" << std::endl;
        return 40;
    }

    QFile requestFile(requestPath);
    if (!requestFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "dev-mock-backend: request file is missing or unreadable: "
                  << requestPath.toStdString() << std::endl;
        return 41;
    }

    QJsonParseError parseError;
    const QJsonDocument requestDocument = QJsonDocument::fromJson(requestFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        std::cerr << "dev-mock-backend: invalid request JSON: "
                  << parseError.errorString().toStdString() << std::endl;
        return 42;
    }

    if (!requestDocument.isObject()) {
        std::cerr << "dev-mock-backend: request JSON top level must be an object" << std::endl;
        return 43;
    }

    const QJsonObject request = requestDocument.object();
    const QString outputPath = request.value("output_result_json_path").toString();
    if (outputPath.trimmed().isEmpty()) {
        std::cerr << "dev-mock-backend: request JSON is missing output_result_json_path" << std::endl;
        return 44;
    }

    const QString mode = argumentValue(argc, argv, "--dev-mock-mode");
    if (mode == "nonzero") {
        std::cerr << "dev-mock-backend: intentional non-zero exit after reading request" << std::endl;
        return 45;
    }

    if (mode == "missing-result") {
        std::cout << "dev-mock-backend: intentionally leaving result file absent" << std::endl;
        return 0;
    }

    QFileInfo outputInfo(outputPath);
    if (!outputInfo.absoluteDir().exists()) {
        std::cerr << "dev-mock-backend: result parent directory does not exist: "
                  << outputInfo.absolutePath().toStdString() << std::endl;
        return 46;
    }

    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        std::cerr << "dev-mock-backend: cannot write result file: "
                  << outputPath.toStdString() << std::endl;
        return 47;
    }

    if (mode == "invalid-result") {
        outputFile.write("{ invalid dev mock result json\n");
        std::cout << "dev-mock-backend: wrote intentionally invalid result JSON" << std::endl;
        return 0;
    }

    const QJsonObject result = makeDevMockUnifiedResult(request, requestPath);
    outputFile.write(QJsonDocument(result).toJson(QJsonDocument::Indented));
    std::cout << "dev-mock-backend: wrote test-only result "
              << outputPath.toStdString() << std::endl;

    return 0;
}

int runExternalProcessHelper(int argc, char *argv[])
{
    int helperIndex = -1;
    for (int i = 1; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == "--external-process-helper") {
            helperIndex = i;
            break;
        }
    }

    if (helperIndex < 0 || helperIndex + 1 >= argc) {
        return -1;
    }

    const QString command = QString::fromLocal8Bit(argv[helperIndex + 1]);

    if (command == "success") {
        return 0;
    }
    if (command == "failure") {
        return 7;
    }
    if (command == "stdout") {
        const QString text =
            helperIndex + 2 < argc ?
                QString::fromLocal8Bit(argv[helperIndex + 2]) :
                QString("stdout");
        std::cout << text.toStdString() << std::endl;
        return 0;
    }
    if (command == "stderr") {
        const QString text =
            helperIndex + 2 < argc ?
                QString::fromLocal8Bit(argv[helperIndex + 2]) :
                QString("stderr");
        std::cerr << text.toStdString() << std::endl;
        return 0;
    }
    if (command == "stdout-stderr") {
        const QString stdoutText =
            helperIndex + 2 < argc ?
                QString::fromLocal8Bit(argv[helperIndex + 2]) :
                QString("stdout");
        const QString stderrText =
            helperIndex + 3 < argc ?
                QString::fromLocal8Bit(argv[helperIndex + 3]) :
                QString("stderr");
        std::cout << stdoutText.toStdString() << std::endl;
        std::cerr << stderrText.toStdString() << std::endl;
        return 0;
    }
    if (command == "sleep") {
        const unsigned long msec =
            helperIndex + 2 < argc ?
                QString::fromLocal8Bit(argv[helperIndex + 2]).toULong() :
                1000UL;
        QThread::msleep(msec);
        return 0;
    }

    std::cerr << "unknown helper command" << std::endl;
    return 99;
}

}

int main(int argc, char *argv[])
{
    const int devMockHelperExitCode = runDevMockBackendHelper(argc, argv);
    if (devMockHelperExitCode >= 0) {
        return devMockHelperExitCode;
    }

    const int helperExitCode = runExternalProcessHelper(argc, argv);
    if (helperExitCode >= 0) {
        return helperExitCode;
    }

    QCoreApplication app(argc, argv);
    app.setOrganizationName("tony");
    app.setApplicationName("test-backend-types");

    TestBackendTypes backendTypes;
    TestUnifiedResult unifiedResult;

    int failures = 0;
    failures += QTest::qExec(&backendTypes, argc, argv);
    failures += QTest::qExec(&unifiedResult, argc, argv);

    if (failures == 0) {
        std::cerr << "All tests passed" << std::endl;
        return 0;
    } else {
        return 1;
    }
}
