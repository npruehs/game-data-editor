#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include <QHBoxLayout>
#include <QListWidget>
#include <QString>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

class ListItemWindow;

namespace Tome
{
    class FacetsController;
    class RecordsController;
    class TypesController;

    /**
     * @brief Allows adding, editing, re-ordering and removing list items.
     */
    class ListWidget : public QWidget
    {
            Q_OBJECT

        public:
            explicit ListWidget(FacetsController& facetsController, RecordsController& recordsController, TypesController& typesController, QWidget *parent = 0);
            ~ListWidget();

            QString getFieldType() const;
            QVariantList getItems() const;

            void setFieldType(const QString& fieldType);
            void setItems(const QVariantList& items);

        private slots:
            void addItem();
            void editItem(QListWidgetItem* item);
            void removeItem();
            void moveItemUp();
            void moveItemDown();

        private:
            QString fieldType;
            QVariantList items;

            ListItemWindow* listItemWindow;

            QHBoxLayout* layout;
            QVBoxLayout* buttonLayout;
            QListWidget* listWidget;

            FacetsController& facetsController;
            RecordsController& recordsController;
            TypesController& typesController;

            int getSelectedItemIndex() const;
    };
}
#endif // LISTWIDGET_H
