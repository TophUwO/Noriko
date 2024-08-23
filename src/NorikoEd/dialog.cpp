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
 * \file  dialog.cpp
 * \brief implements the public APIs for all of Noriko's dialogs
 */


/* Qt includes */
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>

/* Noriko includes */
#include <include/Noriko/util.h>
#include <include/Noriko/log.h>
/* NorikoEd includes */
#include <include/NorikoEd/dialog.hpp>


namespace NkE::dlg {
    QString NewProjectDialog::m_DefParent = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();


    NewProjectDialog::NewProjectDialog(ProjectManager *projMan, QWidget *parPtr)
        : QDialog(parPtr), mp_projMan(projMan), m_parPath(m_DefParent)
    {
        setupUi(this);

        /* Connect signals and initialize button row. */
        connect(leWkTitle, &QLineEdit::textChanged, this, &NewProjectDialog::on_UpdateDialogState);
        connect(leRootPath, &QLineEdit::textChanged, this, &NewProjectDialog::on_UpdateDialogState);

        /* Set initial dialog properties. */
        leWkTitle->setFocus();
        leRootPath->setText(m_parPath);
    }


    void NewProjectDialog::accept() {
        /* Update properties. */
        m_parPath    = leRootPath->text();
        m_projAuthor = leAuthor->text();
        m_wkTitle    = leWkTitle->text();
        m_brDesc     = pteDesc->toPlainText();

        /* Check if the target directory already exists. */
        QDir const targetDir = ProjectManager::CreateProjectPath(m_wkTitle, m_parPath);
        if (targetDir.exists()) {
            QMessageBox::critical(
                this,
                "Error",
                QString("Target path \"%1\" already exists.").arg(targetDir.absolutePath()),
                QMessageBox::Ok | QMessageBox::Cancel
            );

            NK_LOG_ERROR("Target path \"%s\" already exists.", targetDir.absolutePath().toStdString().c_str());
            return;
        }

        /* Add the project to project manager. */
        if (mp_projMan->createProject(m_wkTitle, m_projAuthor, m_brDesc, m_parPath))
            QDialog::accept();
    }


    void NewProjectDialog::on_btnReset_clicked() {
        m_wkTitle    = "";
        m_brDesc     = "";
        m_projAuthor = "";
        m_parPath    = m_DefParent;

        leWkTitle->clear();
        leAuthor->clear();
        leRootPath->setText(m_DefParent);
        pteDesc->clear();

        leWkTitle->setFocus();
    }

    void NewProjectDialog::on_tbtnBrowse_clicked() {
        QString const newRootDir = QFileDialog::getExistingDirectory(
            this,
            "Select root directory",
            m_parPath,
            QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly
        );

        /* Update root directory but only if the dialog was accepted. */
        if (!newRootDir.isEmpty()) {
            m_parPath = newRootDir.isEmpty() ? m_parPath : newRootDir;

            leRootPath->setText(m_parPath);
        }
    }

    void NewProjectDialog::on_UpdateDialogState() {
        m_parPath = leRootPath->text();

        /* Update state of error labels. */
        lblInvWkTitle->setVisible(leWkTitle->text().isEmpty());
        lblInvRootPath->setVisible(m_parPath.isEmpty() || !QDir(m_parPath).exists());

        /* Update state of 'Ok' button. */
        btnOk->setEnabled(!leWkTitle->text().isEmpty() && !(m_parPath.isEmpty() || !QDir(m_parPath).exists()));
    }
} /* namespace NkE::dlg */


