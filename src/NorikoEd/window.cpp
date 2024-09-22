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
 * \file  window.cpp
 * \brief implements the main window class
 */


/* stdlib includes */
#include <iostream>

/* NorikoEd includes */
#include <include/NorikoEd/window.hpp>
#include <include/NorikoEd/dialog.hpp>
#include <include/NorikoEd/application.hpp>


namespace NkE {
    MainWindow::MainWindow(QString const &wndTitle) {
        setupUi(this);

        /* Set basic properties. */
        setWindowTitle(wndTitle);
        setWindowState(Qt::WindowMaximized);
        setVisible(true);

        /* Query global components. */
        m_projExplInst = std::make_shared<ExplorerWidget>(new ExplorerModel(this), this);
    }


    void MainWindow::on_actionProjNew_triggered() {
        //dlg::NewProjectDialog newProjDlg(m_projManInst.get(), this);

        //newProjDlg.exec();
    }

    void MainWindow::on_actionProjOpen_triggered() {
        //dlg::OpenProjectDialog openProjDlg(m_projManInst.get(), this);

        //openProjDlg.exec();
    }

    void MainWindow::on_actionFileExit_triggered() {
        QApplication::exit(EXIT_SUCCESS);
    }
} /* namespace NkE */


