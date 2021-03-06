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

import QtQuick 2.4
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.1
import org.kde.purpose 1.0

ApplicationWindow
{
    id: window

    flags: Qt.Dialog
    title: configuration.pluginName

    width: Math.max(screen.width/6, 200)
    height: Math.max(screen.height/4, 300)

    property bool finished: false
    property var configuration
    property QtObject q

    Component.onCompleted: adoptJob()
    onConfigurationChanged: adoptJob()
    minimumHeight: view.Layout.minimumHeight
    minimumWidth: view.Layout.minimumWidth

    onClosing: {
        q.finished(null, 1 /* KIO::ERR_USER_CANCELED */, i18nd("libpurpose_widgets", "Configuration cancelled"));
    }

    function adoptJob() {
        if (configuration == null)
            return;
        if (configuration.isReady) {
            startJob()
        } else {
            view.push(configWizardComponent)
        }
    }

    function startJob() {
        var job = window.configuration.createJob();
        job.start()
        view.push(runningJobComponent, { job: job })
    }

    StackView {
        id: view
        anchors {
            fill: parent
            margins: 5
        }
        Layout.minimumWidth: (currentItem ? currentItem.Layout.minimumWidth : 0) + 2*anchors.margins
        Layout.minimumHeight: (currentItem ? currentItem.Layout.minimumHeight : 0) + 2*anchors.margins
    }

    Component {
        id: configWizardComponent
        Page {
            PurposeWizard {
                id: wiz
                configuration: window.configuration
                focus: true

                anchors.fill: parent
            }

            footer: RowLayout {
                Item {
                    Layout.fillWidth: true
                }
                Button {
                    text: i18nd("libpurpose_widgets", "Send")
                    icon.name: "document-send"
                    enabled: window.configuration && window.configuration.isReady
                    onClicked: {
                        view.pop();
                        window.startJob();
                    }
                }
                Button {
                    text: i18nd("libpurpose_widgets", "Cancel")
                    icon.name: "dialog-cancel"
                    onClicked: {
                        window.q.finished(null, 1 /* KIO::ERR_USER_CANCELED */, i18nd("libpurpose_widgets", "Configuration cancelled"));
                        window.visible = false;
                    }
                }
            }
        }
    }

    Component {
        id: runningJobComponent
        RunningJob {
            onResult: {
                window.q.finished(job.output, job.error, job.errorString);
                window.visible = false
            }
        }
    }
}
