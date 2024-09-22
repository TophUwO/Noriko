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
 * \file  explorer.hpp
 * \brief defines the public API for the project explorer widget
 * 
 * The project explorer widget is one of NorikoEd's central widgets. It manages the
 * project and asset organization within the application. It is responsible for displaying
 * the contents of projects according to their internal structure and and organization.
 */


#pragma once

/* stdlib includes */
#include <vector>

/* Qt includes */
#include <QVariant>
#include <QString>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QAbstractItemModelTester>
/* Qt forms includes */
#include <ui_form_expl.h>

/* NorikoEd includes */
#include <include/NorikoEd/common.hpp>
#include <include/NorikoEd/project.hpp>
#include <include/NorikoEd/session.hpp>


/* definition of ExplorerModel */
namespace NkE {
    /**
     * \class ExplorerItem
     * \brief represents a generic explorer item for use in the explorer view model
     */
    class ExplorerItem {
    public:
        /**
        * \enum  ExplorerItem::Type
        * \brief explorer item type IDs (used for category identification when assigning
        *        custom context menus, etc.)
        */
        enum class Type {
            Generic, /**< generic type */

            Session, /**< session item ID */
            Project, /**< project item type ID */
            Filter   /**< filter item type ID */
        };

    private:
        using ExplorerItemVector = std::vector<std::unique_ptr<class ExplorerItem>>;

        ExplorerItem::Type  m_itemType;    /**< item type */
        ExplorerItemVector  m_childItems;  /**< list of direct children */
        ExplorerItem       *mp_parentItem; /**< pointer to the parent item */
        QVariant            m_internalVal; /**<  */

    public:
        /**
         * \brief constructs a new explorer item
         * \param [in] parPtr pointer to the parent item
         */
        explicit ExplorerItem(ExplorerItem::Type typeId, ExplorerItem *parPtr = nullptr);
        virtual ~ExplorerItem() = default;

        ExplorerItem::Type  getItemType()          const;
        QVariant            getItemData(int colId) const;
        NkSize              getChildCount()        const;
        NkInt32             getItemRow()           const;
        ExplorerItem       *getChildAt(int rowPos) const;
        ExplorerItem       *getParent()            const;

        /**
         * \brief mutates the type of the current item
         * \param [in] newType ID of the new item type
         * \note  This function only supports mutating the item's type when the current
         *        item type is <tt>ExplorerItem::Type::Generic</tt>.
         */
        void setItemType(ExplorerItem::Type newType);
        bool setItemData(int colId, QVariant const &newVal);

        /**
         * \brief inserts a new child item at the given index
         * \param [in] where2Insert the position of where to insert the new item
         * \param [in] childItem child item that is to be added at the given position
         */
        void insertChildItem(NkInt32 where2Insert, std::unique_ptr<ExplorerItem> &&childItem);
        /**
         * \brief removes the child item at the given position
         * \param [in] where2Remove the index of the item that is to be removed
         * \note  The item, if removed, is deleted.
         */
        void removeChildItem(NkInt32 where2Remove);
    };


    /**
     * \class ExplorerModel
     * \brief represents the item model that is used for the Project Explorer widget
     */
    class ExplorerModel : public QAbstractItemModel {
        Q_OBJECT

        std::unique_ptr<ExplorerItem> m_rootItem; /**< root item */

    public:
        /**
         * \brief constructs a new explorer model
         * \param [in] parPtr pointer to the parent object
         */
        explicit ExplorerModel(QObject *parPtr = nullptr);
        ~ExplorerModel() = default;

        /**
         * \brief  retrieves the raw pointer to the underlying item
         * \param  [in] modelIndex index pointing to the model item
         * \return pointer to the item that corresponds to the given model index
         */
        ExplorerItem *getItemPointer(QModelIndex const &modelIndex) const;

        /**
         * \brief reimplements \c QAbstractItemModel::flags()
         * \see   https://doc.qt.io/qt-6/qabstractitemmodel.html#flags
         */
        virtual Qt::ItemFlags flags(const QModelIndex &itemIndex) const override;
        /**
         * \brief reimplements \c QAbstractItemModel::index()
         * \see   https://doc.qt.io/qt-6/qabstractitemmodel.html#index
         */
        virtual QModelIndex index(int rowPos, int colPos, const QModelIndex &parIndex = QModelIndex()) const override;
        /**
         * \brief reimplements \c QAbstractItemModel::parent()
         * \see   https://doc.qt.io/qt-6/qabstractitemmodel.html#parent
         */
        virtual QModelIndex parent(const QModelIndex &childIndex) const override;
        /**
         * \brief reimplements \c QAbstractItemModel::rowCount()
         * \see   https://doc.qt.io/qt-6/qabstractitemmodel.html#rowCount
         */
        virtual int rowCount(const QModelIndex &parIndex = QModelIndex()) const override;
        /**
         * \brief reimplements \c QAbstractItemModel::columnCount()
         * \see   https://doc.qt.io/qt-6/qabstractitemmodel.html#columnCount
         */
        virtual int columnCount(const QModelIndex &parIndex = QModelIndex()) const override;
        /**
         * \brief reimplements \c QAbstractItemModel::data()
         * \see   https://doc.qt.io/qt-6/qabstractitemmodel.html#data
         */
        virtual QVariant data(QModelIndex const &modelIndex, int roleId) const override;

        /**
         * \brief reimplements \c QAbstractItemModel::setData()
         * \see   https://doc.qt.io/qt-6/qabstractitemmodel.html#setData
         */
        virtual bool setData(QModelIndex const &modelIndex, QVariant const &newVal, int roleId) override;

        /**
         * \brief reimplements \c QAbstractItemModel::insertRows()
         * \see   https://doc.qt.io/qt-6/qabstractitemmodel.html#insertRows
         * \note  This method currently only supports inserting one row.
         * \todo  Implement support for inserting multiple rows.
         */
        virtual bool insertRows(int where2Insert, int numOfRows, const QModelIndex &parIndex = QModelIndex()) override;
    };
} /* namespace NkE */


/* definition of the explorer widget */
namespace NkE {
    /**
     * \class ExplorerWidget
     * \brief represents the dockable explorer widget that holds a tree view by default
     */
    class ExplorerWidget : public DockableContainer, private Ui_ExplorerWidget {
        Q_OBJECT

        /**
         * \brief carries out specific dynamic widget initialization that could not be
         *        done during the Qt Designer stage.
         */
        void int_setupDynamicWidgets();
        /**
         * \brief  retrieves the source item index that is mapped to the given proxy item
         *         index
         * \param  [in] proxyIndex index into the proxy model
         * \return item as given type or \c nullptr if there was an error
         */
        ExplorerItem *int_getSourceItem(QModelIndex const &proxyIndex) const;
        /**
         * \brief expands or collapses all items starting from the given root index
         * \param [in] rootIndex model index of where to start expanding/collapsing
         * \param [in] isExpand whether or not to expand (<tt>true</tt>) or collapse
         *             (<tt>false</tt>) the tree items
         */
        void int_expandOrCollapseRecursively(QModelIndex const &rootIndex, bool isExpand);

#if (defined _DEBUG)
        QAbstractItemModelTester *mp_itemModelTester; /**< item model tester for debugging */
#endif

    public:
        /**
         * \brief constructs a new explorer widget
         * \param [in] explModelPtr pointer to the Explorer's view model
         * \param [in] parPtr pointer to the parent widget
         */
        explicit ExplorerWidget(ExplorerModel *explModelPtr, QWidget *parPtr = nullptr);
        ~ExplorerWidget();

        void createSession(QString const &name);

    private slots:
        /* main toolbar actions */
        void on_actCollapseAll_triggered();
        void on_actExpandAll_triggered();
        void on_actShowSearchBar_triggered(bool isChecked = false);
        void on_actCtrlSearchBar_triggered();
        void on_actHomeView_triggered();
        /* search bar actions */
        void on_actEnableRegex_triggered(bool isChecked = false);
        void on_actCaseSensitivity_triggered(bool isChecked = false);
        /* project context menu actions */
        void on_actOpenInFileExplorer_triggered(QModelIndex const &srcInd, QModelIndex const &proxyInd);
        void on_actExpand_triggered(QModelIndex const &srcInd, QModelIndex const &proxyInd);
        void on_actCollapse_triggered(QModelIndex const &srcInd, QModelIndex const &proxyInd);
        void on_actExpandAllDesc_triggered(QModelIndex const &srcInd, QModelIndex const &proxyInd);
        void on_actCollapseAllDesc_triggered(QModelIndex const &srcInd, QModelIndex const &proxyInd);
        void on_actScopeToThis_triggered(QModelIndex const &srcInd, QModelIndex const &proxyInd);
        void on_actRename_triggered(QModelIndex const &srcInd, QModelIndex const &proxyInd);
        void on_actAddFilter_triggered(QModelIndex const &srcInd, QModelIndex const &proxyInd);

        /* miscellaneous widget slots */
        void on_customCxtMenu_requested(QPoint const &mousePos = QPoint());
        void on_leSearch_textChanged(QString const &newText = "");
    };
} /* namespace NkE */


