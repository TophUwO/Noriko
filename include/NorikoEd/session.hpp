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
 * \file  session.hpp
 * \brief defines the session object; the topmost organizational node in the project
 *        explorer
 * 
 * All context creation, with or without a project can only happen within the context of
 * a 'session'.
 */


#pragma once

/* Qt includes */
#include <QObject>

/* NorikoEd includes */
#include <include/NorikoEd/common.hpp>


namespace NkE {
    /**
     */
    class Item : public QObject, public UniversallyNamedItem {
        Q_OBJECT

    public:
        /**
         */
        enum Type {
            Generic, /**< */
            Session, /**< */
            Filter,  /**< */
            Project, /**< */
            Document /**< */
        };

        /**
         */
        explicit Item(Item::Type itemType, Item *parPtr = nullptr) noexcept;
        virtual ~Item();

        Item::Type  getItemType()            const noexcept;
        Item       *getChildAt(size_t index) const noexcept;
        Item       *getParentItem()          const noexcept;

        /**
         */
        virtual bool addChildItem(Item *beforePtr, Item *itemPtr) noexcept;
        /**
         */
        virtual bool removeChildItem(Item *itemPtr) noexcept;

    private:
        Item                *mp_parentItem; /**< */
        std::vector<Item *>  m_childItems;  /**< */
        Item::Type           m_itemType;    /**< */
    };

    /**
     */
    class SessionItem : public Item {
        Q_OBJECT
        Q_PROPERTY(QString sessionName READ getSessionName WRITE setSessionName);

    public:
        /**
         */
        explicit SessionItem(QString const &sessionName, Item *parPtr = nullptr);
        ~SessionItem();

        QString getSessionName() const;

        void setSessionName(QString const &newName);

        /**
         */
        virtual bool addChildItem(Item *beforePtr, Item *itemPtr) noexcept override;

    private:
        QString m_sessionName;
    };
} /**< namespace NkE */


