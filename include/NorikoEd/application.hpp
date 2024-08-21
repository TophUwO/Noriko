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
 * \file  application.hpp
 * \brief defines the API for the NorikoEd application class
 */


#pragma once

/* Qt includes */
#include <QApplication>

/* Noriko includes */
#include <include/Noriko/noriko.h>
/* NorikoEd includes */
#include <include/NorikoEd/window.hpp>


namespace NkE {
    /**
     * \class Application
     * \brief main application class for NorikoEd instance
     */
    class Application : public QApplication {
        Q_OBJECT

    public:
        /**
         * \brief constructs a new application instance
         * \param [in] argc number of command-line parameters
         * \param [in] argv command-line parameters as array of C-strings
         */
        Application(int argc, char **argv);
        ~Application();

        /**
         * \brief  enters the main loop and runs the application
         * \return error code for host platform
         */
        NkErrorCode runApplication();

    private:
        /**
         * \brief prints some platform information on the debug console
         */
        void int_PrintPlatformInformation();

        std::unique_ptr<MainWindow> m_mainWnd;  /**< main window */
        NkPlatformInformation       m_platInfo; /**< Noriko platform information */
    };
} /* namespace NkE */


