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
 * \file  common.hpp
 * \brief defines some common utility classes and basic includes required throughout the
 *        entire application
 */


#pragma once

/* Windows-specific includes */
#include <msxml6.h>
#include <comutil.h>

/* Qt includes */
#include <QWidget>
#include <QDockWidget>
#include <QMainWindow>

/* Noriko includes */
#include <include/Noriko/noriko.h>


/**
 * \namespace NkE
 * \brief     root namespace of Noriko's editor component
 */
namespace NkE {
    /**
     * \class DockableContainer
     * \brief represents a utility class that wraps arbitrary content into a dockable
     *        widget with full main window support (i.e, toolbars, nested docks, etc.)
     */
    class DockableContainer : public QDockWidget {
        Q_OBJECT

        Qt::DockWidgetArea  m_defDockArea; /**< default dock widget area */
        QMainWindow        *mp_contWidget; /**< container widget */

    public:
        /**
         * \brief constructs a new dockable container
         * \param [in] wndTitle dock widget window title
         * \param [in] defDockArea default dock area to dock widget to
         * \param [in,out] parPtr pointer to the parent widget
         * \note  If \c parPtr is an instance of a QDockWidget, then the container is
         *        automatically added to the parent in the specified dock area.
         */
        explicit DockableContainer(QString const &wndTitle, Qt::DockWidgetArea defDockArea, QWidget *parPtr = nullptr)
            : QDockWidget(wndTitle, parPtr), m_defDockArea(defDockArea)
        {
            /* Create the content widget. */
            mp_contWidget = new QMainWindow(this);
            mp_contWidget->setWindowFlags(Qt::Widget);
            mp_contWidget->show();

            /* Set widget flags. */
            setVisible(true);
            setWidget(mp_contWidget);

            /*
             * If the direct parent is a main window, let the dock widget dock itself to
             * its default area if specified.
             */
            QMainWindow *parAsMainWnd;
            if ((parAsMainWnd = dynamic_cast<QMainWindow *>(parPtr)) != nullptr)
                parAsMainWnd->addDockWidget(defDockArea, this);
        }
        virtual ~DockableContainer() = default;

        /**
         * \brief  retrieves the underlying QMainWindow instance for use as parent widget
         * \return pointer to the underlying QMainWindow instance
         */
        QMainWindow *contentWidget() const { return mp_contWidget; }
    };


    /**
     * \class UniversallyNamedItem
     * \brief represents an item with a universally unique identifier (UUID)
     */
    class UniversallyNamedItem {
        Q_GADGET
        Q_PROPERTY(NkUuid UUID READ getUuid); /**< underlying UUID structure */

    public:
        /**
         * \brief creates a new universally named item
         * \note  The universal identifier is automatically generated when this object is
         *        instantiated.
         */
        explicit UniversallyNamedItem() noexcept {
            NkUuidGenerate(&m_uuidRep);

            NK_LOG_TRACE("Created universally named item with UUID {%s}.", uuidToString().toStdString().c_str());
        }
        /**
         * \brief creates a new universally named item from an existing UUID
         * \param [in] uuidStr string representation of the UUID
         */
        explicit UniversallyNamedItem(QString const &uuidStr) {
            NkErrorCode cvtRes = NkUuidFromString(uuidStr.toStdString().c_str(), &m_uuidRep);
            if (cvtRes != NkErr_Ok) {
                NK_LOG_ERROR("UUID {%s} is malformed.", uuidStr.toStdString().c_str());

                throw cvtRes;
            }

            NK_LOG_TRACE("Created universally named item from existing UUID {%s}.", uuidStr.toStdString().c_str());
        }
        virtual ~UniversallyNamedItem() = default;

        /**
         * \brief  converts the underlying UUID to a string
         * \return UUID represented as string
         */
        QString uuidToString() const {
            char tmpBuf[100] = { 0 };
            NkUuidToString(&m_uuidRep, tmpBuf);

            return tmpBuf;
        }

        /**
         * \brief returns the underlying UUID object
         */
        NkUuid const &getUuid() const { return m_uuidRep; }

    private:
        NkUuid m_uuidRep; /**< underlying UUID representation */
    };
} /* namespace NkE */


Q_DECLARE_METATYPE(NkUuid);


