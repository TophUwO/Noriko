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
 * \file  project.hpp
 * \brief defines the public API for the NorikoEd project object
 * 
 * A project is the root organizational node in NorikoEd. It represents an abstract
 * deployable product. All creational work in NorikoEd needs to happen within a project
 * context.
 */


#pragma once

/* Qt includes */
#include <QString>
#include <QDir>
#include <QList>

/* NorikoEd includes */
#include <include/NorikoEd/common.hpp>


namespace NkE {
    /**
     * \class Project
     * \brief represents a NorikoEd project
     * \note  The project represents the top-most organizational node in the object tree.
     */
    class Project : public UniversallyNamedItem {
        Q_GADGET
        Q_PROPERTY(QString path READ getQualifiedPath);
        Q_PROPERTY(QString title READ getWorkingTitle);
        Q_PROPERTY(QString author READ getAuthoringOrg);
        Q_PROPERTY(QString description READ getProductDescription);

    public:
        /**
         * \brief constructs a new project instance
         * \param [in] wkTitle working title of the finished product
         * \param [in] author authoring organization identifier
         * \param [in] brDesc brief description of the finished product
         * \param [in] dirRoot root directory on the filesystem
         */
        explicit Project(QString const &wkTitle, QString const &author, QString const &brDesc, QString const &dirRoot);
        /**
         * \brief constructs a new project instance from the given parameters but with a
         *        custom UUID
         * \param [in] uuidStr UUID string representation
         * \param [in] wkTitle working title of the finished product
         * \param [in] author authoring organization identifier
         * \param [in] brDesc brief description of the finished product
         * \param [in] dirRoot root directory on the filesystem
         * \note  This constructor may throw exceptions of type <tt>NkErrorCode</tt>.
         */
        explicit Project(
            QString const &uuidStr,
            QString const &wkTitle,
            QString const &author,
            QString const &brDesc,
            QString const &dirRoot
        );
        ~Project() = default;

        /**
         * \brief  writes the current state of the project (i.e., its main properties) to
         *         the given output device
         * \param  [in,out] ioDevPtr pointer to the I/O device
         * \return \c true on success
         */
        bool writeRootXmlDocument(QIODevice *ioDevPtr) const;

        QString const &getQualifiedPath()      const { return m_qualifiedPath; }
        QString const &getWorkingTitle()       const { return m_workingTitle; }
        QString const &getAuthoringOrg()       const { return m_authoringOrg; }
        QString const &getProductDescription() const { return m_productDescription; }

    private:
        QString m_qualifiedPath;
        QString m_workingTitle;
        QString m_authoringOrg;
        QString m_productDescription;
    };


    /**
     * \class ProjectManager
     * \brief implements the global project manager object
     * 
     * The project manager is responsible for creating, editing, and otherwise managing
     * project files and directories. It also controls the project explorer widget.
     */
    class ProjectManager : public QObject {
        Q_OBJECT

        /**
         * \brief  writes the project file for the given project
         * \param  [in] projRef project object for which the project file is to be
         *              written
         * \return \c true on success, \c false on failure
         * \note   If this function fails, the function may show message boxes that could
         *         block the main thread.
         */
        static bool int_WriteProjectFile(Project const &projRef);

        QList<Project> m_projVec; /**< currently loaded projects */

    public:
        explicit ProjectManager();
        ~ProjectManager();

        /**
         * \brief  creates a new project and adds it to the internal registry
         * \param  [in] wkTitle working title for the new project
         * \param  [in] authorStr authoring organization identifier
         * \param  [in] prodDesc brief product description
         * \param  [in] dirRoot root directory (must exist)
         * \return \c true if the product could be created in its entirety, \c false if
         *         there was an error
         * \note   This function may show message boxes that could block the main thread.
         */
        bool createProject(
            QString const &wkTitle,
            QString const &authorStr,
            QString const &prodDesc,
            QString const &dirRoot
        );
        /**
         * \brief  opens an existing project from its project file
         * \param  [in] projFilePath absolute path to the project file
         * \return \c true on success, \c false on error
         */
        bool openProject(QString const &projFilePath);

        /**
         * \brief  creates the project path based on the given parameters
         * \param  [in] wkName working title
         * \param  [in] parDir parent directory
         * \return QDir object pointing to the generated directory path
         * \note   The directory does not have to exist for this to work.
         */
        static QDir CreateProjectPath(QString const &wkTitle, QString const &parDir);
        /**
         * \brief  transforms the given raw working title into a working title that is
         *         suitable for use in paths
         * \param  [in] wkTitle raw working title
         * \return QString instance representing the transformed working title
         */
        static QString ConvertWorkingTitle(QString const &wkTitle);

    signals:
        /**
         * \brief signal that is emitted whenever a new project was added to the manager
         * \param [in] wkTitle working title of the loaded project
         * \param [in] rootPath root path of the loaded project
         * \note  This signal is emitted when a project is created or when it is opened
         *        from an existing directory.
         */
        void projectLoaded(QString const &wkTitle, QString const &rootPath);
    };
} /* namespace NkE */


