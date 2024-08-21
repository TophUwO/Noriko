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


namespace NkE {
    MainWindow::MainWindow(QString const &title, QSize const &defSize, QWidget *parPtr)
        : QMainWindow(parPtr)
    {
        /* Initialize static widgets. */
        setupUi(this);

        /* Set basic properties. */
        setWindowTitle(title);
        setVisible(true);
        resize(defSize);
    }
} /* namespace NkE */


