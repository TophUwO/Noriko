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
* \file  explorer.cpp
* \brief implements the public API for the project explorer widget
* 
* The project explorer widget is one of NorikoEd's central widgets. It manages the
* project and asset organization within the application. It is responsible for displaying
* the contents of projects according to their internal structure and and organization.
*/


/* stdlib includes */
#include <algorithm>

/* NorikoEd includes */
#include <include/NorikoEd/explorer.hpp>


/* implementation of explorer item */
namespace NkE {
    ExplorerItem::ExplorerItem(ExplorerItem *parPtr)
        : mp_parentItem(parPtr)
    { }


    QVariant ExplorerItem::getDisplayData() const {
        return QVariant{};
    }

    QVariant ExplorerItem::getDecorationData() const {
        return QVariant{};
    }

    NkSize ExplorerItem::getChildCount() const {
        return (NkSize)m_childItems.size();
    }

    NkInt32 ExplorerItem::getItemRow() const {
        if (mp_parentItem == nullptr)
            return 0;

        auto const itemIter = std::find_if(m_childItems.cbegin(), m_childItems.cend(),
            [&](std::unique_ptr<ExplorerItem> const &itemRef) {
                return this == itemRef.get();
            }
        );

        return static_cast<NkInt32>(std::distance(m_childItems.cbegin(), itemIter));
    }

    ExplorerItem *ExplorerItem::getChildAt(int rowPos) const {
        if (rowPos < 0 || rowPos >= m_childItems.size())
            return nullptr;

        return m_childItems.at(rowPos).get();
    }

    ExplorerItem *ExplorerItem::getParent() const {
        return mp_parentItem;
    }


    void ExplorerItem::insertChildItem(NkInt32 where2Insert, std::unique_ptr<ExplorerItem> &&childItem) {
        m_childItems.insert(m_childItems.cbegin() + where2Insert, std::move(childItem));
    }

    void ExplorerItem::removeChildItem(NkInt32 where2Remove) {
        m_childItems.erase(m_childItems.cbegin() + where2Remove);
    }
} /* namespace NkE */


/* implementation of project (explorer) item */
namespace NkE {
    ExplorerProjectItem::ExplorerProjectItem(std::shared_ptr<Project> const &projRef, ExplorerItem *parPtr)
        : ExplorerItem(parPtr), m_projItem{projRef}
    { }


    QVariant ExplorerProjectItem::getDisplayData() const {
        return m_projItem->getProjectName();
    }

    QVariant ExplorerProjectItem::getDecorationData() const {
        return QIcon(":/icons/ico_project.png");
    }
} /* namespace NkE */


/* implementation of filter (explorer) item */
namespace NkE {
    ExplorerFilterItem::ExplorerFilterItem(QString const &filterName, ExplorerItem *parPtr)
        : ExplorerItem(parPtr), m_filterName(filterName) 
    { }


    QVariant ExplorerFilterItem::getDisplayData() const {
        return m_filterName;
    }

    QVariant ExplorerFilterItem::getDecorationData() const {
        return QIcon(":/icons/ico_folderfilter.png");
    }
} /* namespace NkE */


/* implementation of the explorer model */
namespace NkE {
    ExplorerModel::ExplorerModel(QObject *parPtr)
        : QAbstractItemModel(parPtr)
    {
        /* Create root item. */
        m_rootItem = std::make_unique<ExplorerItem>(nullptr);

        /* Setup example model. */
        auto a1 = std::make_unique<ExplorerProjectItem>(std::make_shared<Project>("Ace of Spaces", "", "", QDir()), m_rootItem.get());
        auto a2 = std::make_unique<ExplorerFilterItem>("assets", a1.get());
        auto a3 = std::make_unique<ExplorerFilterItem>("tilesets", a2.get());
        auto a4 = std::make_unique<ExplorerFilterItem>("maps", a2.get());
        auto a5 = std::make_unique<ExplorerFilterItem>("out", a1.get());
        a2->insertChildItem(0, std::move(a3));
        a2->insertChildItem(1, std::move(a4));
        a1->insertChildItem(0, std::move(a2));
        a1->insertChildItem(1, std::move(a5));
        m_rootItem->insertChildItem(0, std::move(a1));
    }


    QModelIndex ExplorerModel::index(int rowPos, int colPos, const QModelIndex &parIndex) const {
        if (!hasIndex(rowPos, colPos, parIndex))
            return QModelIndex{};

        /* Get parent item. */
        auto parItem = parIndex.isValid()
            ? static_cast<ExplorerItem *>(parIndex.internalPointer())
            : m_rootItem.get()
        ;

        /* Find child at given position and create index from it. */
        return createIndex(rowPos, colPos, parItem->getChildAt(rowPos));
    }

    QModelIndex ExplorerModel::parent(const QModelIndex &childIndex) const {
        if (!childIndex.isValid())
            return QModelIndex{};

        /* Get item pointer from the given index. */
        auto parPtr = static_cast<ExplorerItem *>(childIndex.internalPointer())->getParent();

        /*
         * If the child item is actually the root item, an invalid model index is
         * returned since the root item has no parent.
         */
        return parPtr != m_rootItem.get()
            ? createIndex(parPtr->getItemRow(), 0, parPtr)
            : QModelIndex{}
        ;
    }

    int ExplorerModel::rowCount(const QModelIndex &parIndex) const {
        if (parIndex.column() > 0)
            return 0;

        /*
         * Get the item pointer behind the given model index. If the index is invalid,
         * use the topmost (root) item.
         */
        auto itemPtr = parIndex.isValid()
            ? static_cast<ExplorerItem *>(parIndex.internalPointer())
            : m_rootItem.get()
        ;

        return static_cast<int>(itemPtr->getChildCount());
    }

    int ExplorerModel::columnCount(const QModelIndex &parIndex) const {
        return 1;
    }

    QVariant ExplorerModel::data(QModelIndex const &modelIndex, int roleId) const {
        if (!modelIndex.isValid())
            return QVariant{};

        /* Get pointer to the item. */
        auto itemPtr = static_cast<ExplorerItem *>(modelIndex.internalPointer());

        /*
         * Qt::DisplayRole ... QString that is to be displayed as the main item text 
         */
        switch (roleId) {
            case Qt::DisplayRole:    return itemPtr->getDisplayData();
            case Qt::DecorationRole: return itemPtr->getDecorationData();
        }

        return QVariant{};
    }
} /* namespace NkE */


/* implementation of the explorer widget */
namespace NkE {
    ExplorerWidget::ExplorerWidget(ExplorerModel *explModelPtr, QWidget *parPtr)
        : DockableContainer("Project Explorer", Qt::RightDockWidgetArea, parPtr)
    {
        setupUi(contentWidget());
        explModelPtr->setParent(this);

        /* Connect signals for the search bar. */
        connect(leSearch, &QLineEdit::textChanged, this, [&]() {
            QString tmpTxt = leSearch->text();

            if (tmpTxt.isEmpty())
                actCtrlSearchBar->setIcon(QIcon(":/icons/ico_search.png"));
            else
                actCtrlSearchBar->setIcon(QIcon(":/icons/ico_delete.png"));
        });
        connect(actCtrlSearchBar, &QAction::triggered, this, [&]() {
            leSearch->clear();
        });
        
        /* Setup dynamic widgets. */
        leSearch->addAction(actCtrlSearchBar, QLineEdit::TrailingPosition);

        tvExplorer->setModel(explModelPtr);
        tvExplorer->expandAll();
        NK_LOG_INFO("startup: project explorer");
    }

    ExplorerWidget::~ExplorerWidget() {
        NK_LOG_INFO("shutdown: project explorer");
    }
} /* namespace NkE */


