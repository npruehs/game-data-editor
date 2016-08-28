#include "recordtreewidget.h"

#include <QMimeData>

#include "recordtreewidgetitem.h"
#include "../Controller/recordscontroller.h"

using namespace Tome;


RecordTreeWidget::RecordTreeWidget(RecordsController& recordsController)
    : recordsController(recordsController)
{
    this->setDragEnabled(true);
    this->viewport()->setAcceptDrops(true);
    this->setDropIndicatorShown(true);
    this->setHeaderHidden(true);
}

void RecordTreeWidget::addRecord(const QString& id, const QString& displayName)
{
    QTreeWidgetItem* newItem = new RecordTreeWidgetItem(id, displayName, QString());

    // [pg-0005]
    const Record &record_data = this->recordsController.getRecord( id );
    newItem->setIcon( 0, QIcon( record_data.fieldValues.empty() ? ":/Media/Icons/gmTome_empty_record.png" : ":/Media/Icons/gmTome_record.png") );

    this->insertTopLevelItem(0, newItem);
    this->sortItems(0, Qt::AscendingOrder);

    // Select new record.
    this->setCurrentItem(newItem);
}

QString RecordTreeWidget::getSelectedRecordId() const
{
    RecordTreeWidgetItem* recordTreeItem = this->getSelectedRecordItem();

    if (recordTreeItem == 0)
    {
        return QString();
    }

    return recordTreeItem->getId();
}

RecordTreeWidgetItem* RecordTreeWidget::getSelectedRecordItem() const
{
    QList<QTreeWidgetItem*> selectedItems = this->selectedItems();

    if (selectedItems.empty())
    {
        return 0;
    }

    return static_cast<RecordTreeWidgetItem*>(selectedItems.first());
}

void RecordTreeWidget::setRecords(const RecordList& records)
{
    // Create record tree items.
    QMap<QString, RecordTreeWidgetItem*> recordItems;

    for (int i = 0; i < records.size(); ++i)
    {
        const Record& record = records[i];
        RecordTreeWidgetItem* recordItem =
                new RecordTreeWidgetItem(record.id, record.displayName, record.parentId);
        recordItems.insert(record.id, recordItem);
        // [pg-0005]
        const Record &record_data = this->recordsController.getRecord( record.id );
        recordItem->setIcon( 0, QIcon( record_data.fieldValues.empty() ? ":/Media/Icons/gmTome_empty_record.png" : ":/Media/Icons/gmTome_record.png") );
    }

    // Build hierarchy and prepare item list for tree widget.
    QList<QTreeWidgetItem* > items;

    for (QMap<QString, RecordTreeWidgetItem*>::iterator it = recordItems.begin();
         it != recordItems.end();
         ++it)
    {
        RecordTreeWidgetItem* recordItem = it.value();
        QString recordItemParentId = recordItem->getParentId();
        if (!recordItemParentId.isEmpty())
        {
            if (recordItems.contains(recordItemParentId))
            {
                // Insert into tree.
                RecordTreeWidgetItem* recordParent = recordItems[recordItemParentId];
                recordParent->addChild(recordItem);
            }
            else
            {
                // Reset parent reference.
                this->recordsController.reparentRecord(recordItem->getId(), QString());
            }
        }

        items.append(recordItem);
    }

    // Fill tree widget.
    this->insertTopLevelItems(0, items);
    this->expandAll();
}

bool RecordTreeWidget::dropMimeData(QTreeWidgetItem* parent, int index, const QMimeData* data, Qt::DropAction action)
{
    Q_UNUSED(index)
    Q_UNUSED(action)

    // Get dropped content.
    QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    // Check if not empty.
    if (!stream.atEnd())
    {
        // Get data.
        int row;
        int col;
        QMap<int,  QVariant> roleDataMap;
        stream >> row >> col >> roleDataMap;

        // Get dragged record.
        QString draggedRecordId = roleDataMap[Qt::UserRole].toString();

        // Get drop target record.
        QString dropTargetRecordId;

        if (parent != 0)
        {
            RecordTreeWidgetItem* dropTarget = static_cast<RecordTreeWidgetItem*>(parent);
            dropTargetRecordId = dropTarget->getId();
        }

        // Emit signal.
        emit recordReparented(draggedRecordId, dropTargetRecordId);
    }

    return true;
}
