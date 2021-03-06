/*
 Copyright 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "helper.h"
#include <KPluginMetaData>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

using namespace Purpose;

QJsonObject Purpose::readPluginType(const QString &pluginType)
{
    const QString lookup = QStringLiteral("purpose/types/") + pluginType + QStringLiteral("PluginType.json");

    QString path = QStringLiteral(":/") + lookup;
    if (!QFileInfo::exists(path)) {
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, lookup);
        if (path.isEmpty()) {
            qWarning() << "Couldn't find" << lookup << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
            return QJsonObject();
        }
    }
    QFile typeFile(path);
    if (!typeFile.open(QFile::ReadOnly)) {
        qWarning() << "Couldn't open" << lookup;
        return QJsonObject();
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(typeFile.readAll(), &error);
    if (error.error) {
        qWarning() << "JSON error in " << path << error.offset << ":" << error.errorString();
        return QJsonObject();
    }

    Q_ASSERT(doc.isObject());
    return doc.object();
}

KPluginMetaData Purpose::createMetaData(const QString &file)
{
    const QFileInfo fi(file);
    const QString fileName = fi.absoluteFilePath();
    QJsonObject metaData;

    QFile f(fileName);
    if (f.open(QIODevice::ReadOnly)) {
        const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        metaData = doc.object();
    }

    QDir dir = fi.dir().filePath(QStringLiteral("contents/code"));
    QStringList mainFile = dir.entryList({QStringLiteral("main.*")}, QDir::Files);
    if (mainFile.isEmpty()) {
        qWarning() << "no main file for" << file;
        return KPluginMetaData();
    }

    auto info = KPluginMetaData(metaData, dir.absoluteFilePath(mainFile.first()), fileName);
    Q_ASSERT(info.isValid() && !info.rawData().isEmpty());
    return info;
}
