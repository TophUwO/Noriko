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

/* Qt includes */
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>

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

    ExplorerItem::Type ExplorerItem::getItemType() const {
        return Type::Generic;
    }

    NkSize ExplorerItem::getChildCount() const {
        return (NkSize)m_childItems.size();
    }

    NkInt32 ExplorerItem::getItemRow() const {
        if (mp_parentItem == nullptr)
            return 0;

        auto const itemIter = std::find_if(mp_parentItem->m_childItems.cbegin(), mp_parentItem->m_childItems.cend(),
            [&](std::unique_ptr<ExplorerItem> const &itemRef) {
                return this == itemRef.get();
            }
        );

        return static_cast<NkInt32>(std::distance(mp_parentItem->m_childItems.cbegin(), itemIter));
    }

    ExplorerItem *ExplorerItem::getChildAt(int rowPos) const {
        if (rowPos < 0 || rowPos >= m_childItems.size())
            return nullptr;

        return m_childItems.at(rowPos).get();
    }

    ExplorerItem *ExplorerItem::getParent() const {
        return mp_parentItem;
    }


    bool ExplorerItem::setDisplayData(QVariant const &newVal) {
        /* Do nothing by default. */
        return false;
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

    ExplorerItem::Type ExplorerProjectItem::getItemType() const {
        return Type::Project;
    }

    QVariant ExplorerProjectItem::getDecorationData() const {
        return QIcon(":/icons/ico_project.png");
    }


    bool ExplorerProjectItem::setDisplayData(QVariant const &newVal) {
        if (!newVal.isValid())
            return false;

        /* Update the project's name. */
        NK_LOG_WARNING("Updating project names in the project explorer view is not yet implemented.");
        return false;
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

    ExplorerItem::Type ExplorerFilterItem::getItemType() const {
        return Type::Filter;
    }


    bool ExplorerFilterItem::setDisplayData(QVariant const &newVal) {
        if (!newVal.isValid())
            return false;

        m_filterName = newVal.toString();
        return true;
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
        auto a6 = std::make_unique<ExplorerFilterItem>("OUT", a1.get());
        a2->insertChildItem(0, std::move(a3));
        a2->insertChildItem(1, std::move(a4));
        a1->insertChildItem(0, std::move(a2));
        a1->insertChildItem(1, std::move(a5));
        a1->insertChildItem(2, std::move(a6));
        m_rootItem->insertChildItem(0, std::move(a1));
    }


    ExplorerItem *ExplorerModel::getItemPointer(QModelIndex const &modelIndex) const {
        if (modelIndex.isValid())
            if (auto *itemPtr = static_cast<ExplorerItem *>(modelIndex.internalPointer()))
                return itemPtr;

        return m_rootItem.get();
    }


    Qt::ItemFlags ExplorerModel::flags(const QModelIndex &itemIndex) const {
        if (!itemIndex.isValid())
            return Qt::NoItemFlags;

        return Qt::ItemIsEditable | QAbstractItemModel::flags(itemIndex);
    }

    QModelIndex ExplorerModel::index(int rowPos, int colPos, const QModelIndex &parIndex) const {
        if (parIndex.isValid() && parIndex.column() != 0)
            return QModelIndex{};

        /* Get parent item. */
        ExplorerItem *parItem = getItemPointer(parIndex);
        if (parItem == nullptr)
            return QModelIndex{};

        /* Find child at given position and create index from it. */
        if (ExplorerItem *childItem = parItem->getChildAt(rowPos))
            return createIndex(rowPos, colPos, childItem);
        return QModelIndex{};
    }

    QModelIndex ExplorerModel::parent(const QModelIndex &childIndex) const {
        if (!childIndex.isValid())
            return QModelIndex{};

        /* Get item pointer from the given index. */
        ExplorerItem *childItem  = getItemPointer(childIndex);
        ExplorerItem *parentItem = childItem != nullptr ? childItem->getParent() : nullptr;

        /*
         * If the child item is actually the root item, an invalid model index is
         * returned since the root item has no parent.
         */
        return parentItem != m_rootItem.get() && parentItem != nullptr
            ? createIndex(parentItem->getItemRow(), 0, parentItem)
            : QModelIndex{}
        ;
    }

    int ExplorerModel::rowCount(const QModelIndex &parIndex) const {
        if (parIndex.isValid() && parIndex.column() > 0)
            return 0;

        /*
         * Get the item pointer behind the given model index. If the index is invalid,
         * use the topmost (root) item.
         */
        ExplorerItem *itemPtr = getItemPointer(parIndex);

        return itemPtr != nullptr ? static_cast<int>(itemPtr->getChildCount()) : 0;
    }

    int ExplorerModel::columnCount(const QModelIndex &parIndex) const {
        return 1;
    }

    QVariant ExplorerModel::data(QModelIndex const &modelIndex, int roleId) const {
        if (!modelIndex.isValid())
            return QVariant{};

        /* Get pointer to the item. */
        ExplorerItem *itemPtr = getItemPointer(modelIndex);

        /*
         * Qt::DisplayRole ... QString that is to be displayed as the main item text 
         */
        switch (roleId) {
            case Qt::EditRole:
            case Qt::DisplayRole:    return itemPtr->getDisplayData();
            case Qt::DecorationRole: return itemPtr->getDecorationData();
        }

        return QVariant{};
    }


    bool ExplorerModel::setData(QModelIndex const &modelIndex, QVariant const &newVal, int roleId) {
        if (roleId != Qt::EditRole)
            return false;

        /* Update the data. */
        bool editRes = false;
        ExplorerItem *itemPtr = getItemPointer(modelIndex);
        switch (roleId) {
            case Qt::EditRole:
                editRes = itemPtr->setDisplayData(newVal);
                
                break;
        }
        
        /*
         * Emit dataChanged signal as per requirements.
         * See: https://doc.qt.io/qt-6/qabstractitemmodel.html#setData
         */
        if (editRes)
            emit dataChanged(modelIndex, modelIndex, { Qt::DisplayRole, Qt::EditRole });
        return editRes;
    }
} /* namespace NkE */


/* implementation of the custom explorer item delegate */
namespace NkE::priv {
    /**
    * \class ExplorerItemDelegate
    * \brief represents a modified item delegate for the project explorer
    */
    class ExplorerItemDelegate : public QStyledItemDelegate {
        Q_OBJECT

    public:
        /**
         * \brief construct a new explorer item delegate
         * \param [in] parPtr pointer to the parent object
         */
        explicit ExplorerItemDelegate(QObject *parPtr = nullptr)
            : QStyledItemDelegate(parPtr)
        { }


        /**
         * \brief reimplements \c QStyledItemDelegate::setModelData()
         * \see   https://doc.qt.io/qt-6/qstyleditemdelegate.html#setModelData
         * \note  This function causes an empty string not to be accepted when the model
         *        is edited by hand.
         */
        virtual void setModelData(QWidget *ePtr, QAbstractItemModel *mdPtr, QModelIndex const &mdInd) const override {
            if (!mdInd.isValid()) {
                QStyledItemDelegate::setModelData(ePtr, mdPtr, mdInd);

                return;
            }

            /* Get the underlying QLineEdit editor widget. */
            QLineEdit *editorWg = dynamic_cast<QLineEdit *>(ePtr);
            /*
            * If the editor widget's text was modified, check if it's not empty. If it is,
            * show error message and reset editor widget.
            */
            if (editorWg != nullptr && editorWg->isModified()) {
                QString const editorWgTxt = editorWg->text().trimmed();

                if (editorWgTxt.isEmpty()) {
                    /* Show error message. */
                    QMessageBox::critical(
                        nullptr,
                        "Error",
                        "Please enter a valid new name for the selected item.",
                        QMessageBox::Ok
                    );

                    return;
                }
            }

            QStyledItemDelegate::setModelData(ePtr, mdPtr, mdInd);
        }
    };
}


/* implementation of the custom filtering model used by the project explorer view */
namespace NkE::priv {
    /**
     * \class ExplorerFilterModel
     * \brief represents the proxy model that is used to filter the underlying item model
     */
    class ExplorerFilterModel : public QSortFilterProxyModel {
        Q_OBJECT

    public:
        /**
         * \brief constructs a new explorer filter model
         * \param [in] explModelPtr pointer to the source model
         * \param [in] parPtr pointer to the parent object
         */
        ExplorerFilterModel(ExplorerModel *explModelPtr, QObject *parPtr = nullptr)
            : QSortFilterProxyModel(parPtr)
        {
            setSourceModel(explModelPtr);
            setRecursiveFilteringEnabled(true);
        }

    protected:
        /**
         * \brief reimplements \c QSortFilterProxyModel::filterAcceptsRow()
         * \see   https://doc.qt.io/qt-6/qsortfilterproxymodel.html#filterAcceptsRow
         */
        virtual bool filterAcceptsRow(int sourceRowNum, const QModelIndex &srcParentIndex) const override {
            if (!srcParentIndex.isValid())
                return false;

            /* Get pointer to parent item. */
            auto parPtr = static_cast<::NkE::ExplorerItem *>(srcParentIndex.internalPointer());
            if (sourceRowNum >= parPtr->getChildCount())
                return false;

            /* Get child index. */
            QModelIndex currModelIndex   = sourceModel()->index(sourceRowNum, 0, srcParentIndex);
            ::NkE::ExplorerItem *itemPtr = static_cast<::NkE::ExplorerItem *>(currModelIndex.internalPointer());

            return itemPtr->getDisplayData().toString().contains(filterRegularExpression());
        }
    };
}


/* implementation of the explorer widget */
namespace NkE {
    ExplorerWidget::ExplorerWidget(ExplorerModel *explModelPtr, QWidget *parPtr)
        : DockableContainer("Project Explorer", Qt::RightDockWidgetArea, parPtr)
    {
        setupUi(contentWidget());
        /* Setup dynamic widgets. */
        int_setupDynamicWidgets();

        /* Connect slots triggered by actions directly. */
        connect(actCollapseAll, &QAction::triggered, this, &ExplorerWidget::on_actCollapseAll_triggered);
        connect(actExpandAll, &QAction::triggered, this, &ExplorerWidget::on_actExpandAll_triggered);
        connect(actShowSearchBar, &QAction::triggered, this, &ExplorerWidget::on_actShowSearchBar_triggered);
        connect(actEnableRegex, &QAction::triggered, this, &ExplorerWidget::on_actEnableRegex_triggered);
        connect(actCtrlSearchBar, &QAction::triggered, this, &ExplorerWidget::on_actCtrlSearchBar_triggered);
        connect(actCaseSensitivity, &QAction::triggered, this, &ExplorerWidget::on_actCaseSensitivity_triggered);
        /* Connect slots triggered indirectly by other widgets. */
        connect(leSearch, &QLineEdit::textChanged, this, &ExplorerWidget::on_leSearch_textChanged);
        connect(tvExplorer, &QTreeView::customContextMenuRequested, this, &ExplorerWidget::on_customCxtMenu_requested);

        /* Setup view. */
        tvExplorer->setContextMenuPolicy(Qt::CustomContextMenu);
        tvExplorer->itemDelegate()->deleteLater();
        tvExplorer->setItemDelegate(new priv::ExplorerItemDelegate(tvExplorer));
        tvExplorer->setModel(new priv::ExplorerFilterModel(explModelPtr, this));
        tvExplorer->expandAll();

        NK_LOG_INFO("startup: project explorer");
    }

    ExplorerWidget::~ExplorerWidget() {
        NK_LOG_INFO("shutdown: project explorer");
    }


    void ExplorerWidget::int_setupDynamicWidgets() {
        /* Use the hidden menu. */
        mbMain->setVisible(false);
        actSearchOptions->setMenu(menuSearchOptions);

        leSearch->addAction(actSearchOptions, QLineEdit::TrailingPosition);
        leSearch->addAction(actCtrlSearchBar, QLineEdit::TrailingPosition);
    }

    
    void ExplorerWidget::on_actCollapseAll_triggered() {
        tvExplorer->collapseAll();
    }

    void ExplorerWidget::on_actExpandAll_triggered() {
        tvExplorer->expandAll();
    }

    void ExplorerWidget::on_actShowSearchBar_triggered(bool isChecked) {
        leSearch->setVisible(isChecked);

        /* If the search bar is hidden, also reset the search filter. */
        if (!isChecked) {
            priv::ExplorerFilterModel *modelPtr = dynamic_cast<priv::ExplorerFilterModel *>(tvExplorer->model());

            leSearch->clear();
            if (modelPtr != nullptr)
                modelPtr->setFilterRegularExpression("");
        }
    }

    void ExplorerWidget::on_actEnableRegex_triggered(bool isChecked) {
        on_leSearch_textChanged(leSearch->text());
    }

    void ExplorerWidget::on_actCtrlSearchBar_triggered() {
        QString searchBarText = leSearch->text();

        if (!searchBarText.isEmpty())
            leSearch->clear();
    }

    void ExplorerWidget::on_actCaseSensitivity_triggered(bool isChecked) {
        priv::ExplorerFilterModel *modelPtr = dynamic_cast<priv::ExplorerFilterModel *>(tvExplorer->model());

        if (modelPtr != nullptr) {
            modelPtr->setFilterCaseSensitivity(isChecked ? Qt::CaseSensitive : Qt::CaseInsensitive);

            tvExplorer->expandAll();
        }
    }


    void ExplorerWidget::on_customCxtMenu_requested(QPoint const &mousePos) {
        /* Get the proxy index. */
        QModelIndex const itemAtPos = tvExplorer->indexAt(mousePos);
        if (!itemAtPos.isValid() || itemAtPos.constInternalPointer() == nullptr) {
            NK_LOG_WARNING("The Explorer widget's context menu is not yet implemented.");

            return;
        }

        /* Get the actual item index from the proxy index. */
        if (priv::ExplorerFilterModel *modelPtr = dynamic_cast<priv::ExplorerFilterModel *>(tvExplorer->model())) {
            QModelIndex sourceItemInd = modelPtr->mapToSource(itemAtPos);
            ExplorerItem *itemPtr = static_cast<ExplorerItem *>(sourceItemInd.internalPointer());
            if (!sourceItemInd.isValid() || itemPtr == nullptr)
                return;

            /* Call the appropriate context menu based on the item type. */
            QMenu *reqCxtMenu = nullptr;
            switch (itemPtr->getItemType()) {
                case ExplorerItem::Type::Project: reqCxtMenu = menuCxtProject; break;
                default:
                    NK_LOG_WARNING(
                        "Custom context menus for type %i is not yet implemented.",
                        static_cast<int>(itemPtr->getItemType())
                    );

                    return;
            }

            /* Execute the selected context menu. */
            reqCxtMenu->exec(tvExplorer->viewport()->mapToGlobal(mousePos));
        }
    }

    void ExplorerWidget::on_leSearch_textChanged(QString const &newText) {
        /* Update "Control search bar" action. */
        actCtrlSearchBar->setIcon(newText.isEmpty()
            ? QIcon(":/icons/ico_search.png")
            : QIcon(":/icons/ico_delete.png")
        );
        actCtrlSearchBar->setToolTip(newText.isEmpty() ? "" : "Clear search bar");

        /* Update palette if necessary. */
        bool updateRegEx = true;
        QPalette newPalette = QApplication::palette(leSearch);
        QRegularExpression tmpRegEx(newText);
        if (actEnableRegex->isChecked() && !tmpRegEx.isValid()) {
            newPalette.setColor(QPalette::ColorRole::Text, Qt::red);

            /* Set error tooltip. */
            leSearch->setToolTip("Error: " + tmpRegEx.errorString());
            updateRegEx = false;
        } else
            leSearch->setToolTip("");
        leSearch->setPalette(newPalette);

        /* Update filter model. */
        priv::ExplorerFilterModel *modelPtr = dynamic_cast<priv::ExplorerFilterModel *>(tvExplorer->model());
        if (actEnableRegex->isChecked() && updateRegEx)
            modelPtr->setFilterRegularExpression(tmpRegEx);
        else
            modelPtr->setFilterFixedString(newText.trimmed());

        /* Expand the view after the filtering has finished. */
        tvExplorer->expandAll();
    }
} /* namespace NkE */


#include "explorer.moc"

