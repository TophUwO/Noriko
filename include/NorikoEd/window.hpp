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
 * \file  mainwnd.hpp
 * \brief defines the API for the NorikoEd main window class
 */


#pragma once

/* Qt includes */
#include <QMainWindow>
/* Qt form files */
#include <ui_form_window.h>

/* Noriko includes */
#include <include/Noriko/noriko.h>


namespace NkE {
    /**
     */
    class MainWindow : public QMainWindow, private Ui_MainWindow {
        Q_OBJECT

    public:
        MainWindow(QString const &title, QSize const &defSize, QWidget *parPtr = nullptr);
        ~MainWindow() = default;
    };
} /* namespace NkE */


