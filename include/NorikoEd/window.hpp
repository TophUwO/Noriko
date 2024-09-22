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
 * \file  window.hpp
 * \brief defines the API for the NorikoEd main window class
 */


#pragma once

/* Qt includes */
#include <QMainWindow>
/* Qt form files */
#include <ui_form_window.h>

/* Noriko includes */
#include <include/Noriko/noriko.h>
/* NorikoEd includes */
#include <include/NorikoEd/session.hpp>
#include <include/NorikoEd/project.hpp>
#include <include/NorikoEd/explorer.hpp>


namespace NkE {
    /**
     * \class MainWindow
     * \brief represents the main window for the NorikoEd application
     */
    class MainWindow : public QMainWindow, private Ui_MainWindow {
        Q_OBJECT

        std::shared_ptr<ExplorerWidget> m_projExplInst; /**< global project explorer widget instance */

    public:
        /**
         * \brief constructs a new main window
         * \param [in] wndTitle window title
         */
        MainWindow(QString const &wndTitle);
        ~MainWindow() = default;

    private slots:
        void on_actionProjNew_triggered();
        void on_actionProjOpen_triggered();
        void on_actionFileExit_triggered();
    };
} /* namespace NkE */


