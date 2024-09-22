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
 * \file  session.cpp
 * \brief implements the session object; the topmost organizational node in the project
 *        explorer
 * 
 * All context creation, with or without a project can only happen within the context of
 * a 'session'.
 */


/* Noriko includes */
#include <include/NorikoEd/session.hpp>


/* implementation of item class */
namespace NkE {
    Item::Item(Item::Type itemType, Item *parPtr) noexcept
        : m_itemType(itemType), mp_parentItem(parPtr)
    { }

    Item::~Item() {
        for (auto *it : m_childItems)
            delete it;
    }


    Item::Type Item::getItemType() const noexcept {
        return m_itemType;
    }

    Item *Item::getChildAt(size_t index) const noexcept {
        if (index >= m_childItems.size())
            return nullptr;

        return m_childItems.at(index);
    }

    Item *Item::getParentItem() const noexcept {
        return mp_parentItem;
    }


    bool Item::addChildItem(Item *beforePtr, Item *itemPtr) noexcept {
        /* Find the item the new item is to be inserted before. */
        auto itemIter = std::find_if(m_childItems.cbegin(), m_childItems.cend(), [&](Item const *currItem) {
            return beforePtr == currItem;
        });

        /* Insert the item. */
        try {
            m_childItems.insert(itemIter, itemPtr);
        } catch (...) { return false; }

        return true;
    }

    bool Item::removeChildItem(Item *itemPtr) noexcept {
        /* Find the item that is to be deleted. */
        auto itemIter = std::find_if(m_childItems.cbegin(), m_childItems.cend(), [&](Item const *currItem) {
            return itemPtr == currItem;
        });

        /* Remove the item. */
        if (itemIter != m_childItems.cend()) {
            m_childItems.erase(itemIter);

            return true;
        }
        
        return false;
    }
} /* namespace NkE */


/* implementation of the session item class */
namespace NkE {
    SessionItem::SessionItem(QString const &sessionName, Item *parPtr)
        : Item(Item::Type::Session, parPtr)
    { }

    SessionItem::~SessionItem() { }


    QString SessionItem::getSessionName() const {
        return m_sessionName;
    }

    void SessionItem::setSessionName(QString const &newName) {
        m_sessionName = newName;
    }


    bool SessionItem::addChildItem(Item *beforePtr, Item *itemPtr) noexcept {
        /* Cannot nest sessions. */
        if (itemPtr->getItemType() == Item::Type::Session)
            return false;

        return Item::addChildItem(beforePtr, itemPtr);
    }
};


