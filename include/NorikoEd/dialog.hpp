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
 * \file  dialog.hpp
 * \brief defines the public APIs for all of Noriko's dialogs
 */


#pragma once

/* Qt includes */
#include <QDialog>
/* Qt forms includes */
#include <ui_form_dlg_projnew.h>
#include <ui_form_dlg_projopen.h>

/* NorikoEd includes */
#include <include/NorikoEd/project.hpp>


namespace NkE::dlg {
    /**
     * \class NewProjectDialog
     * \brief implements the user dialog that is shown when the user chooses to create a
     *        new (empty) project
     */
    class NewProjectDialog : public QDialog, private Ui_NewProjectDialog {
        Q_OBJECT

        ProjectManager *mp_projMan;   /**< reference to project manager */
        QString         m_wkTitle;    /**< working title */
        QString         m_projAuthor; /**< project authoring organization */
        QString         m_brDesc;     /**< brief description */
        QString         m_parPath;    /**< project parent path */

        static QString m_DefParent;   /**< default parent directory */

    public:
        /**
         * \brief constructs a new instance of NewProjectDialog
         * \param [in] projManPtr pointer to the global project manager instance
         * \param [in,out] parPtr pointer to the parent widget
         */
        explicit NewProjectDialog(ProjectManager *projManPtr, QWidget *parPtr = nullptr);
        ~NewProjectDialog() = default;

        /**
         * \brief overrides the default \c accept() method of \c QDialog
         * \note  This function's return value does not need to be examined since all
         *        work (that is, creating the project and initializing the directory
         *        structure) is done before this function returns.
         */
        virtual void accept() override;

    private slots:
        void on_btnReset_clicked();
        void on_tbtnBrowse_clicked();
        void on_UpdateDialogState();
    };


    /**
     * \class OpenProjectDialog
     * \brief represents the dialog that is shown when a project is to be imported from
     *        an existing disk file
     */
    class OpenProjectDialog : public QDialog, private Ui_OpenProjectDialog {
        Q_OBJECT

        ProjectManager *mp_projMan; /**< reference to project manager */

    public:
        /**
         * \brief constructs a new open project dialog 
         * \param [in] projManPtr pointer to the project manager
         * \param [in] parPtr pointer to the parent widget
         */
        explicit OpenProjectDialog(ProjectManager *projManPtr, QWidget *parPtr = nullptr);
        ~OpenProjectDialog() = default;

        /**
         * \brief reimplements the \c accept() method of \c QDialog
         */
        virtual void accept() override;

    private slots:
        void on_tbtnBrowse_clicked();
        void on_UpdateDialogState();
    };
} /* namespace NkE::dlg */


