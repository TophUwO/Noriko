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
/* Qt forms includes */
#include <ui_form_expl.h>

/* NorikoEd includes */
#include <include/NorikoEd/common.hpp>
#include <include/NorikoEd/project.hpp>


/* definition of ExplorerModel */
namespace NkE {
    /**
     * \class ExplorerItem
     * \brief represents a generic explorer item for use in the explorer view model
     * \note  This class should only ever be instantiated for use as the root item for
     *        an item view that uses explorer items.
     */
    class ExplorerItem {
        std::vector<std::unique_ptr<ExplorerItem>>  m_childItems;  /**< list of direct children */
        ExplorerItem                               *mp_parentItem; /**< pointer to the parent item */

    public:
        /**
         * \brief constructs a new explorer item
         * \param [in] parPtr pointer to the parent item
         */
        explicit ExplorerItem(ExplorerItem *parPtr = nullptr);
        virtual ~ExplorerItem() = default;

        /**
         * \brief gets the data used for the Qt::DisplayRole of the overarching model 
         */
        virtual QVariant      getDisplayData()       const;
        /**
         * \brief gets the decoration data (i.e., the item's icon) for use inside the
         *        view 
         */
        virtual QVariant      getDecorationData()    const;
                NkSize        getChildCount()        const;
                NkInt32       getItemRow()           const;
                ExplorerItem *getChildAt(int rowPos) const;
                ExplorerItem *getParent()            const;

        /**
         * \brief  updates the display value (for Qt::DisplayRole) for the current item
         * \return \c true if the data was updated, \c false if not
         */
        virtual bool setDisplayData(QVariant const &newVal);

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
     * \class ExplorerProjectItem
     * \brief represents the project item type
     */
    class ExplorerProjectItem : public ExplorerItem {
        std::shared_ptr<Project> m_projItem;

    public:
        /**
         * \brief constructs a new project item
         * \param [in] projRef reference to the project object
         * \param [in] parPtr pointer to the parent item
         */
        explicit ExplorerProjectItem(std::shared_ptr<Project> const &projRef, ExplorerItem *parPtr = nullptr);
        ~ExplorerProjectItem() = default;

        /**
         * \brief reimplements \c ExplorerItem::getDisplayData() 
         */
        virtual QVariant getDisplayData() const override;
        /**
         * \brief reimplements \c ExplorerItem::getDecorationData() 
         */
        virtual QVariant getDecorationData() const override;

        /**
         * \brief reimplements \c ExplorerItem::setDisplayData() 
         */
        virtual bool setDisplayData(QVariant const &newVal) override;
    };


    /**
     * \class ExplorerFilterItem
     * \brief represents the filter (virtual directory) item type
     */
    class ExplorerFilterItem : public ExplorerItem {
        QString m_filterName; /**< name (display representation) of the filter */

    public:
        /**
         * \brief constructs a new filter item
         * \param [in] filterName name of the new filter
         * \param [in] parPtr pointer to the parent item
         */
        explicit ExplorerFilterItem(QString const &filterName, ExplorerItem *parPtr = nullptr);
        ~ExplorerFilterItem() = default;

        /**
         * \brief reimplements \c ExplorerItem::getDisplayData() 
         */
        virtual QVariant getDisplayData() const override;
        /**
         * \brief reimplements \c ExplorerItem::getDecorationData() 
         */
        virtual QVariant getDecorationData() const override;

        /**
         * \brief reimplements \c ExplorerItem::setDisplayData() 
         */
        virtual bool setDisplayData(QVariant const &newVal) override;
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

    public:
        /**
         * \brief constructs a new explorer widget
         * \param [in] explModelPtr pointer to the Explorer's view model
         * \param [in] parPtr pointer to the parent widget
         */
        explicit ExplorerWidget(ExplorerModel *explModelPtr, QWidget *parPtr = nullptr);
        ~ExplorerWidget();

    private slots:
        void on_actCollapseAll_triggered();
        void on_actExpandAll_triggered();
        void on_actShowSearchBar_triggered(bool isChecked = false);
        void on_actEnableRegex_triggered(bool isChecked = false);
        void on_actCtrlSearchBar_triggered();

        void on_leSearch_textChanged();
    };
} /* namespace NkE */


