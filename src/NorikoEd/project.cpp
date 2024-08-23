/**********************************************************************
 * Noriko - cross-platform 2-D role-playing game (RPG) game engine    *
 *          for desktop and mobile console platforms                  *
 *                                                                    *
 * (c) 2024 TophUwO <tophuwo01@gmail.com>. All rights reserved.       *
 *                                                                    *
 * The source code is licensed under the Apache License 2.0. Refer    *
 * to the LICENSE file in the root directory of this project. If this *
 * file is not present, visit                                         *
 *     https://www.apache.org/licenses/LICENSE-2.0                    *
 **********************************************************************/

/**
 * \file  project.cpp
 * \brief implements the public API for the NorikoEd project object
 *
 * A project is the root organizational node in NorikoEd. It represents an abstract
 * deployable product. All creational work in NorikoEd needs to happen within a project
 * context.
 */


/* Qt includes */
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/* Noriko includes */
#include <include/Noriko/log.h>
/* NorikoEd includes */
#include <include/NorikoEd/project.hpp>


namespace NkE {
    Project::Project(QString const &wkTitle, QString const &author, QString const &brDesc, QDir const &dirRoot)
        : m_projName(wkTitle), m_projAuthor(author), m_projDesc(brDesc), m_qualPath(dirRoot)
    { }


    bool Project::writeRootXmlDocument(QIODevice *ioDevPtr) const {
        if (ioDevPtr == NULL)
            return false;

        QXmlStreamWriter writer(ioDevPtr);
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(4);
        writer.writeStartDocument();

        writer.writeStartElement("nkproject");
        writer.writeTextElement("uuid", uuidToString());
        writer.writeTextElement("working_title", m_projName);
        writer.writeTextElement("project_author", m_projAuthor);
        writer.writeTextElement("product_description", m_projDesc);
        writer.writeTextElement("rootpath", m_qualPath.absolutePath());
        writer.writeEndElement();

        writer.writeEndDocument();
        return true;
    }
} /* namespace NkE */


namespace NkE {
    ProjectManager::ProjectManager() {
        NK_LOG_INFO("startup: project manager");
    }

    ProjectManager::~ProjectManager() {
        NK_LOG_INFO("shutdown: project manager");
    }


    bool ProjectManager::createProject(
        QString const &wkTitle,
        QString const &author,
        QString const &prodDesc,
        QString const &dirRoot
    ) {
        /* Build directory path and setup directory structure. */
        QDir qualDirRoot = ProjectManager::CreateProjectPath(wkTitle, dirRoot);
        if (!qualDirRoot.mkpath(qualDirRoot.absolutePath())) {
            /* Failed to create directory. */
            QMessageBox::critical(
                nullptr,
                "Error",
                QString(
                    "Could not create project directory \"%1\".\n\nCheck root directory permissions, path "
                    "formatting, or disk space."
                ).arg(qualDirRoot.absolutePath()),
                QMessageBox::Ok
            );

            NK_LOG_ERROR(
                "Could not create project directory \"%s\".",
                qualDirRoot.absolutePath().toStdString().c_str()
            );
            return false;
        }

        /* Add project to manager. */
        Project proj{ wkTitle, author, prodDesc, qualDirRoot };
        if (!ProjectManager::int_WriteProjectFile(proj)) {
            QMessageBox::critical(
                nullptr,
                "Error",
                QString("Could not create project file for project \"%1\". Invalid I/O device.").arg(wkTitle),
                QMessageBox::Ok
            );

            NK_LOG_ERROR(
                "Could not write project file for project \"%s\". Invalid I/O device.",
                wkTitle.toStdString().c_str()
            );
            return false;
        }
        m_projVec.append(std::move(proj));

        NK_LOG_INFO("Created new NorikoEd project \"%s\" at %s.",
            wkTitle.toStdString().c_str(),
            qualDirRoot.absolutePath().toStdString().c_str()
        );
        return true;
    }


    QString ProjectManager::ConvertWorkingTitle(QString const &wkTitle) {
        return wkTitle.trimmed().replace(" ", "_");
    }

    QDir ProjectManager::CreateProjectPath(QString const &wkTitle, QString const &parDir) {
        return parDir + "/" + ProjectManager::ConvertWorkingTitle(wkTitle);
    }

    bool ProjectManager::int_WriteProjectFile(Project const &projRef) {
        QFile projFile(
            projRef.getQualifiedPath().absolutePath()
                + "/"
                + ProjectManager::ConvertWorkingTitle(projRef.getProjectName())
                + ".nkproj"
        );

        /* Try to open the project file. */
        if (!projFile.open(QIODeviceBase::Text | QIODeviceBase::WriteOnly)) {
            QMessageBox::critical(
                nullptr,
                "Error",
                QString("Could not open project file \"%1\".\n\nCheck directory permissions and disk space.").arg(
                    projFile.fileName()
                ),
                QMessageBox::Ok
            );

            NK_LOG_ERROR("Could not open project file \"%s\" for writing.", projFile.fileName().toStdString().c_str());
            return false;
        }

        /* Write the project file contents. */
        return projRef.writeRootXmlDocument(&projFile);
    }
} /* namespace NkE */


