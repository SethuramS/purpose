/*
 Copyright 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

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

#include <purpose/pluginbase.h>
#include "mpform.h"
#include <QDebug>
#include <QTimer>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KJob>
#include <KIO/TransferJob>
#include <KIO/StoredTransferJob>

EXPORT_SHARE_VERSION

// Taken from "share" Data Engine
// key associated with plasma-devel@kde.org
// thanks to Alan Schaaf of Imgur (alan@imgur.com)
static const QString apiKey = QStringLiteral("d0757bc2e94a0d4652f28079a0be9379");
static const QUrl imgurUrl(QStringLiteral("https://api.imgur.com/2/upload.json?key=")+apiKey);

class ImgurShareJob : public Purpose::Job
{
    Q_OBJECT
    public:
        ImgurShareJob(QObject* parent)
            : Purpose::Job(parent)
            , m_pendingJobs(0)
        {}

        virtual void start() override
        {
            QJsonArray urls = data().value(QStringLiteral("urls")).toArray();
            qDebug() << "starting..." << data().toVariantMap();
            if (urls.isEmpty()) {
                qWarning() << "no urls to share" << urls << data();
                emitResult();
                return;
            }

            foreach(const QJsonValue &val, urls) {
                QString u = val.toString();
                KIO::StoredTransferJob* job = KIO::storedGet(QUrl(u));
                connect(job, &KJob::finished, this, &ImgurShareJob::fileFetched);
                m_pendingJobs++;
            }
            Q_ASSERT(m_pendingJobs>0);
        }

        void fileFetched(KJob* j)
        {
            KIO::StoredTransferJob* job = qobject_cast<KIO::StoredTransferJob*>(j);
            m_form.addFile(QStringLiteral("image"), job->url(), job->data());
            --m_pendingJobs;
            if (m_pendingJobs == 0)
                performUpload();
        }

        void performUpload()
        {
            m_form.finish();

            KIO::TransferJob *tJob = KIO::http_post(imgurUrl, m_form.formData(), KIO::HideProgressInfo);
            tJob->setMetaData(QMap<QString,QString>{{ QStringLiteral("content-type"), QString::fromLocal8Bit(m_form.contentType()) }});
            connect(tJob, &KIO::TransferJob::data, this, [this](KIO::Job*, const QByteArray& data) { m_resultData += data; });
            connect(tJob, &KJob::result, this, &ImgurShareJob::imagesUploaded);

            m_form.reset(); //we can free some resources already
        }

        void imagesUploaded(KJob* job) {
            QJsonParseError error;
            QJsonObject resultMap = QJsonDocument::fromJson(m_resultData, &error).object();
            if (static_cast<KIO::TransferJob *>(job)->isErrorPage()) {
                setError(3);
                setErrorText(i18n("Error page returned"));
                qDebug() << "Error page :(";
            } else if (job->error()) {
                setError(job->error());
                setErrorText(job->errorText());
            } else if (error.error) {
                setError(1);
                setErrorText(error.errorString());
            } else if ( resultMap.contains(QLatin1String("error")) ) {
                setError(2);
                QJsonObject errorMap = resultMap[QStringLiteral("error")].toObject();
                setErrorText(errorMap[QStringLiteral("message")].toString());
            } else {
                QJsonObject uploadMap = resultMap[QStringLiteral("upload")].toObject();
                QJsonObject linksMap = uploadMap[QStringLiteral("links")].toObject();
                QString url = linksMap[QStringLiteral("original")].toString();
                Q_EMIT infoMessage(this, url, QStringLiteral("<a href='%1'>%1</a>").arg(url));
                Q_EMIT output({ { QStringLiteral("url"), url } });
            }
            emitResult();
        }

        virtual QUrl configSourceCode() const override
        {
            return QUrl();
        }

    private:
        int m_pendingJobs;
        MPForm m_form;
        QByteArray m_resultData;
};

class Q_DECL_EXPORT ImgurPlugin : public Purpose::PluginBase
{
    Q_OBJECT
    public:
        ImgurPlugin(QObject* p, const QVariantList& ) : Purpose::PluginBase(p) {}

        virtual Purpose::Job* share() const override
        {
            return new ImgurShareJob(nullptr);
        }
};

K_PLUGIN_FACTORY_WITH_JSON(ImgurShare, "imgurplugin.json", registerPlugin<ImgurPlugin>();)

#include "imgurplugin.moc"
