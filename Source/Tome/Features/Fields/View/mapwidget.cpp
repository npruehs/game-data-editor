#include "mapwidget.h"

#include <QHeaderView>

#include "mapitemwindow.h"
#include "../../Records/Controller/recordscontroller.h"
#include "../../Types/Controller/typescontroller.h"
#include "../../Types/Model/builtintype.h"
#include "../../../Util/memoryutils.h"


using namespace Tome;


MapWidget::MapWidget(FacetsController& facetsController,
                     ProjectController& projectController,
                     RecordsController& recordsController,
                     TypesController& typesController,
                     QWidget* parent) :
    QWidget(parent),
    mapItemWindow(0),
    facetsController(facetsController),
    projectController(projectController),
    recordsController(recordsController),
    typesController(typesController)
{
    // Create layout.
    this->layout = new QHBoxLayout(this);

    // Add table widget.
    this->tableWidget = new QTableWidget(this);
    connect(
                this->tableWidget,
                SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
                SLOT(editItem(QTableWidgetItem*))
                );
    this->layout->addWidget(this->tableWidget);

    // Add buttons.
    this->buttonLayout = new QVBoxLayout();

    QToolButton* addButton = new QToolButton(this);
    addButton->setArrowType(Qt::LeftArrow);
    connect(
                addButton,
                SIGNAL(clicked()),
                SLOT(addItem())
                );
    buttonLayout->addWidget(addButton);

    QToolButton* removeButton = new QToolButton(this);
    removeButton->setArrowType(Qt::RightArrow);
    connect(
                removeButton,
                SIGNAL(clicked()),
                SLOT(removeItem())
                );
    buttonLayout->addWidget(removeButton);

    this->layout->addLayout(buttonLayout);

    // Set layout.
    this->setLayout(this->layout);
    this->layout->setContentsMargins(0, 0, 0, 0);

    // Init table.
    this->clearTable();
}

MapWidget::~MapWidget()
{
    deleteLayout(this->buttonLayout);
    deleteLayout(this->layout);
}

QString MapWidget::getKeyType() const
{
    return this->keyType;
}

QString MapWidget::getValueType() const
{
    return this->valueType;
}

QVariantMap MapWidget::getMap() const
{
    return this->map;
}

void MapWidget::setKeyType(const QString& keyType)
{
    this->keyType = keyType;
}

void MapWidget::setValueType(const QString& valueType)
{
    this->valueType = valueType;
}

void MapWidget::setMap(const QVariantMap& map)
{
    // Update model.
    this->map = map;

    // Update view.
    this->clearTable();

    // Add all fields.
    this->tableWidget->setRowCount(map.size());

    int row = 0;

    this->tableWidget->setSortingEnabled(false);

    bool hasReferenceKeys = this->typesController.isTypeOrDerivedFromType(this->keyType, BuiltInType::Reference);
    bool hasReferenceValues = this->typesController.isTypeOrDerivedFromType(this->valueType, BuiltInType::Reference);

    for (QVariantMap::const_iterator it = map.cbegin();
         it != map.cend();
         ++it)
    {
        QTableWidgetItem* keyItem = this->createTableWidgetItem(it.key(), hasReferenceKeys);
        this->tableWidget->setItem(row, 0, keyItem);

        QTableWidgetItem* valueItem = this->createTableWidgetItem(it.value(), hasReferenceValues);
        this->tableWidget->setItem(row, 1, valueItem);

        ++row;
    }

    this->tableWidget->resizeColumnsToContents();
    this->tableWidget->horizontalHeader()->setStretchLastSection(true);
    this->tableWidget->verticalHeader()->setVisible(false);
    this->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // Enable sorting.
    this->tableWidget->setSortingEnabled(true);
}

void MapWidget::addItem()
{
    // Prepare window.
    if (!this->mapItemWindow)
    {
        this->mapItemWindow = new MapItemWindow(this->facetsController, this->projectController, this->recordsController, this->typesController, this);
    }

    // Update view.
    this->mapItemWindow->setKeyType(this->keyType);
    this->mapItemWindow->setValueType(this->valueType);

    // Show window.
    int result = this->mapItemWindow->exec();

    if (result == QDialog::Accepted)
    {
        QVariant key = this->mapItemWindow->getKey();
        QVariant value = this->mapItemWindow->getValue();

        // Update model.
        this->map[key.toString()] = value;

        // Update view.
        this->setMap(this->map);
    }
}

void MapWidget::editItem(QTableWidgetItem* item)
{
    // Prepare window.
    if (!this->mapItemWindow)
    {
        this->mapItemWindow = new MapItemWindow(this->facetsController, this->projectController, this->recordsController, this->typesController, this);
    }

    QTableWidgetItem* keyTableWidgetItem = this->tableWidget->item(item->row(), 0);
    QTableWidgetItem* valueTableWidgetItem = this->tableWidget->item(item->row(), 1);

    QVariant currentKey = keyTableWidgetItem->data(Qt::UserRole);
    QVariant currentValue = valueTableWidgetItem->data(Qt::UserRole);

    // Update view.
    this->mapItemWindow->setKeyType(this->keyType);
    this->mapItemWindow->setValueType(this->valueType);

    this->mapItemWindow->setKey(currentKey);
    this->mapItemWindow->setValue(currentValue);

    // Show window.
    int result = this->mapItemWindow->exec();

    if (result == QDialog::Accepted)
    {
        QVariant key = this->mapItemWindow->getKey();
        QVariant value = this->mapItemWindow->getValue();

        // Update model.
        this->map[key.toString()] = value;

        // Update view.
        this->setMap(this->map);
    }
}

void MapWidget::removeItem()
{
    QModelIndexList selectedIndexes = this->tableWidget->selectionModel()->selectedRows();

    if (selectedIndexes.isEmpty())
    {
        return;
    }

    int row = selectedIndexes.first().row();
    QTableWidgetItem* keyTableWidgetItem = this->tableWidget->item(row, 0);

    QVariant actualKey = keyTableWidgetItem->data(Qt::UserRole);

    this->map.remove(actualKey.toString());

    // Update view.
    this->tableWidget->removeRow(row);
}

void MapWidget::clearTable()
{
    this->tableWidget->clear();

    this->tableWidget->setColumnCount(2);

    QStringList headers;
    headers << tr("Key");
    headers << tr("Value");
    this->tableWidget->setHorizontalHeaderLabels(headers);
}

QTableWidgetItem* MapWidget::createTableWidgetItem(QVariant value, bool isReference)
{
    QTableWidgetItem* item = new QTableWidgetItem();

    if (isReference)
    {
        if (this->recordsController.hasRecord(value))
        {
            const Record& record = this->recordsController.getRecord(value);
            item->setText(record.displayName);
        }
        else
        {
            item->setText(value.toString());
        }
    }
    else
    {
        item->setText(value.toString());
    }

    item->setData(Qt::UserRole, value);

    return item;
}
