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
 * \file  main.cpp
 * \brief entrypoint of the NorikoEd component
 */


/* stdlib includes */
#include <iostream>

/* NorikoEd includes */
#include <include/NorikoEd/application.hpp>


int main(int argc, char **argv) {
    try {
        NkE::Application app(argc, argv);

        return static_cast<int>(app.runApplication());
    } catch (std::exception const &exc) {
        /* Print some information on the unhandled exception. */
        std::cout << exc.what() << std::endl;
    }

    return EXIT_FAILURE;
}


