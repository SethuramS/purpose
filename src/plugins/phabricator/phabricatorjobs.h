/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_PLUGIN_PHABRICATORJOBS_H
#define KDEVPLATFORM_PLUGIN_PHABRICATORJOBS_H

#include <QVariant>
#include <QList>
#include <QUrl>

#include <KJob>
#include <QProcess>

class QNetworkReply;

namespace Phabricator
{
    class Q_DECL_EXPORT DifferentialRevision : public KJob
    {
        Q_OBJECT
        public:
            DifferentialRevision(const QString& id, QObject* parent)
                          : KJob(parent), m_id(id) {}
            QString requestId() const { return m_id; }
            void setRequestId(QString id) { m_id = id; }
            virtual bool buildArcCommand(const QString& patchFile=QString());
            virtual void start() override;

        private Q_SLOTS:
            virtual void done(int exitCode, QProcess::ExitStatus exitStatus) = 0;

        protected:
            QProcess m_arcCmd;
        private:
            QString m_id;
    };

    class Q_DECL_EXPORT NewDiffRev : public DifferentialRevision
    {
        Q_OBJECT
        public:
            NewDiffRev(const QString& project, QObject* parent = 0);
            QString diffURI() const
            {
                return m_diffURI;
            }

        private Q_SLOTS:
            void done(int exitCode, QProcess::ExitStatus exitStatus) override;

        private:
            QString m_project;
            QString m_diffURI;
    };

//     class Q_DECL_EXPORT UpdateDiffRev : public DifferentialRevision
//     {
//         Q_OBJECT
//         public:
//             UpdateDiffRev(const QString& id, const QVariantMap& newValues, QObject* parent = nullptr);
// 
//         private Q_SLOTS:
//             void done(int exitCode, QProcess::ExitStatus exitStatus) override;
// 
//         private:
//             QString m_project;
//     };

    class Q_DECL_EXPORT SubmitDiffRev : public DifferentialRevision
    {
        Q_OBJECT
        public:
            SubmitDiffRev(const QUrl& patch, const QString& basedir, const QString& id, QObject* parent);

        private Q_SLOTS:
            void done(int exitCode, QProcess::ExitStatus exitStatus) override;

        private:
            QUrl m_patch;
            QString m_basedir;
    };

    class Q_DECL_EXPORT DiffRevList : public DifferentialRevision
    {
        Q_OBJECT
        public:
            DiffRevList(const QString& reviewStatus, QObject* parent = 0);
            bool buildArcCommand(const QString&) override;
            QVariantList reviews() const;

        private Q_SLOTS:
            void done(int exitCode, QProcess::ExitStatus exitStatus) override;
            void requestReviewList(int startIndex);

        private:
            QString m_reviewStatus;
            QVariantList m_reviews;
    };
}

#endif