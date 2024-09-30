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
 * \brief implements NorikoEd's application class
 */


/* NorikoEd includes */
#include <include/NorikoEd/application.hpp>


namespace NkE {
    Application::Application(int argc, char **argv)
        : QApplication(argc, argv), m_platInfo{ sizeof m_platInfo }
    {
        /* Initialize Noriko's integrated logging facility. */
        NK_IGNORE_RETURN_VALUE(NkLogStartup());

        /* Create main components. */
        m_mainWnd = std::make_unique<MainWindow>("NorikoEd indev");

        /* Initialize other Noriko components. */
        NK_IGNORE_RETURN_VALUE(NkPRNGInitialize());
        int_PrintPlatformInformation();
    }

    Application::~Application() {
        NkPRNGUninitialize();
        NkLogShutdown();
    }


    NkErrorCode Application::runApplication() {
        /* Set app style. */
        setStyle("fusion");

        return (NkErrorCode)exec();
    }


    void Application::int_PrintPlatformInformation() {
        /* Query Noriko platform information. */
        NkQueryPlatformInformation(&m_platInfo);

        /* Print platform information. */
        NK_LOG_INFO("Noriko v%s (build: %s) - %s",
            m_platInfo.m_prodVersion.mp_dataPtr,
            m_platInfo.m_buildDate.mp_dataPtr,
            m_platInfo.m_prodConfig.mp_dataPtr
        );
        NK_LOG_INFO("Noriko Engine %s", m_platInfo.m_prodCopyright.mp_dataPtr);
    }
} /* namespace NkE */


