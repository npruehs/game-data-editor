#include "customtypeswindow.h"
#include "ui_customtypeswindow.h"

#include "derivedtypewindow.h"
#include "enumerationwindow.h"
#include "listwindow.h"
#include "mapwindow.h"
#include "../Controller/typescontroller.h"
#include "../Model/builtintype.h"
#include "../../Facets/Controller/facetscontroller.h"
#include "../../Fields/Controller/fielddefinitionscontroller.h"
#include "../../Records/Controller/recordscontroller.h"
#include "../../Search/Controller/findusagescontroller.h"
#include "../../Types/Model/customtype.h"
#include "../../../Util/listutils.h"

using namespace Tome;


CustomTypesWindow::CustomTypesWindow(TypesController& typesController,
                                     FacetsController& facetsController,
                                     FieldDefinitionsController& fieldDefinitionsController,
                                     FindUsagesController& findUsagesController,
                                     RecordsController& recordsController,
                                     QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CustomTypesWindow),
    typesController(typesController),
    facetsController(facetsController),
    fieldDefinitionsController(fieldDefinitionsController),
    findUsagesController(findUsagesController),
    recordsController(recordsController),
    derivedTypeWindow(0),
    enumerationWindow(0),
    listWindow(0),
    mapWindow(0)
{
    ui->setupUi(this);

    // Setup view.
    this->ui->tableWidget->setColumnCount(3);

    QStringList headers;
    headers << tr("Id");
    headers << tr("Type");
    headers << tr("Details");
    this->ui->tableWidget->setHorizontalHeaderLabels(headers);
}

CustomTypesWindow::~CustomTypesWindow()
{
    delete this->ui;

    delete this->derivedTypeWindow;
    delete this->enumerationWindow;
    delete this->listWindow;
    delete this->mapWindow;
}

void CustomTypesWindow::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)

    // Custom types might have changed, i.e. by loading another project.
    this->updateTable();

    // Enable sorting.
    this->ui->tableWidget->setSortingEnabled(true);
}

void CustomTypesWindow::on_actionNew_Derived_Type_triggered()
{
    // Show window.
    if (!this->derivedTypeWindow)
    {
        this->derivedTypeWindow = new DerivedTypeWindow(
                    this->typesController,
                    this->facetsController,
                    this->recordsController,
                    this);
    }

    this->derivedTypeWindow->init();

    int result = this->derivedTypeWindow->exec();

    if (result == QDialog::Accepted)
    {
        // Update model.
        CustomType newType =
                this->typesController.addDerivedType(
                    this->derivedTypeWindow->getTypeName(),
                    this->derivedTypeWindow->getBaseType(),
                    this->derivedTypeWindow->getFacets(),
                    this->derivedTypeWindow->getTypeSetName());

        // Update view.
        this->ui->tableWidget->insertRow(0);
        this->updateRow(0, newType);
    }
}

void CustomTypesWindow::on_actionNew_Custom_Type_triggered()
{
    // Show window.
    if (!this->enumerationWindow)
    {
        this->enumerationWindow = new EnumerationWindow(this);
    }

    this->enumerationWindow->setEnumerationName("");
    this->enumerationWindow->setEnumerationMembers(QStringList());

    const QStringList typeSetNames = this->typesController.getCustomTypeSetNames();
    this->enumerationWindow->setTypeSetNames(typeSetNames);
    this->enumerationWindow->setTypeSetName(typeSetNames.first());

    int result = this->enumerationWindow->exec();

    if (result == QDialog::Accepted)
    {
        // Update model.
        CustomType newType =
                this->typesController.addEnumeration(
                    this->enumerationWindow->getEnumerationName(),
                    this->enumerationWindow->getEnumerationMembers(),
                    this->enumerationWindow->getTypeSetName());

        // Update view.
        this->ui->tableWidget->insertRow(0);
        this->updateRow(0, newType);
    }
}

void CustomTypesWindow::on_actionNew_List_triggered()
{
    // Show window.
    if (!this->listWindow)
    {
        this->listWindow = new ListWindow(this->typesController, this);
    }

    this->listWindow->init();
    int result = this->listWindow->exec();

    if (result == QDialog::Accepted)
    {
        // Update model.
        CustomType newType =
                this->typesController.addList(
                    this->listWindow->getListName(),
                    this->listWindow->getListItemType(),
                    this->listWindow->getTypeSetName());

        // Update view.
        this->ui->tableWidget->insertRow(0);
        this->updateRow(0, newType);
    }
}

void CustomTypesWindow::on_actionNew_Map_triggered()
{
    // Show window.
    if (!this->mapWindow)
    {
        this->mapWindow = new MapWindow(this->typesController, this);
    }

    this->mapWindow->init();
    int result = this->mapWindow->exec();

    if (result == QDialog::Accepted)
    {
        // Update model.
        CustomType newType =
                this->typesController.addMap(
                    this->mapWindow->getMapName(),
                    this->mapWindow->getMapKeyType(),
                    this->mapWindow->getMapValueType(),
                    this->mapWindow->getTypeSetName());

        // Update view.
        this->ui->tableWidget->insertRow(0);
        this->updateRow(0, newType);
    }
}

void CustomTypesWindow::on_actionEdit_Custom_Type_triggered()
{
    // Get selected type.
    QString typeName = this->getSelectedTypeName();

    if (typeName.isEmpty())
    {
        return;
    }

    const CustomType& type = this->typesController.getCustomType(typeName);

    // Check type.
    if (type.isDerivedType())
    {
        this->editDerivedType(typeName, type);
    }
    else if (type.isEnumeration())
    {
        this->editEnumeration(typeName, type);
    }
    else if (type.isList())
    {
        this->editList(typeName, type);
    }
    else if (type.isMap())
    {
        this->editMap(typeName, type);
    }
}

void CustomTypesWindow::on_actionDelete_Custom_Type_triggered()
{
    // Update model.
    QString typeName = this->getSelectedTypeName();

    if (typeName.isEmpty())
    {
        return;
    }

    this->typesController.removeCustomType(typeName);

    // Update view.
    const int index = this->getSelectedTypeIndex();
    this->ui->tableWidget->removeRow(index);
}

void CustomTypesWindow::on_actionFind_Usages_triggered()
{
    // Find usages.
    const QString& typeName = this->getSelectedTypeName();

    if (typeName.isEmpty())
    {
        return;
    }

    this->findUsagesController.findUsagesOfType(typeName);
}

void CustomTypesWindow::on_tableWidget_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    this->on_actionEdit_Custom_Type_triggered();
}

int CustomTypesWindow::getSelectedTypeIndex() const
{
    QModelIndexList selectedIndexes = this->ui->tableWidget->selectionModel()->selectedRows();
    return selectedIndexes.count() > 0 ? selectedIndexes.first().row() : -1;
}

QString CustomTypesWindow::getSelectedTypeName() const
{
    int selectedIndex = this->getSelectedTypeIndex();

    if (selectedIndex < 0)
    {
        return QString();
    }

    return this->ui->tableWidget->item(selectedIndex, 0)->data(Qt::DisplayRole).toString();
}

void CustomTypesWindow::editDerivedType(QString typeName, const CustomType& type)
{
    // Show window.
    if (!this->derivedTypeWindow)
    {
        this->derivedTypeWindow = new DerivedTypeWindow(
                    this->typesController,
                    this->facetsController,
                    this->recordsController,
                    this);
    }

    this->derivedTypeWindow->init();

    // Update view.
    this->derivedTypeWindow->setTypeName(type.name);
    this->derivedTypeWindow->setBaseType(type.getBaseType());
    this->derivedTypeWindow->setFacets(type.constrainingFacets);
    this->derivedTypeWindow->setTypeSetNames(this->typesController.getCustomTypeSetNames());
    this->derivedTypeWindow->setTypeSetName(type.typeSetName);

    int result = this->derivedTypeWindow->exec();

    if (result == QDialog::Accepted)
    {
        // Update type.
        this->updateDerivedType(
                    typeName,
                    this->derivedTypeWindow->getTypeName(),
                    this->derivedTypeWindow->getBaseType(),
                    this->derivedTypeWindow->getFacets(),
                    this->derivedTypeWindow->getTypeSetName());
    }
}

void CustomTypesWindow::editEnumeration(QString typeName, const CustomType& type)
{
    // Show window.
    if (!this->enumerationWindow)
    {
        this->enumerationWindow = new EnumerationWindow(this);
    }

    // Update view.
    this->enumerationWindow->setEnumerationName(type.name);
    this->enumerationWindow->setEnumerationMembers(type.getEnumeration());
    this->enumerationWindow->setTypeSetNames(this->typesController.getCustomTypeSetNames());
    this->enumerationWindow->setTypeSetName(type.typeSetName);

    int result = this->enumerationWindow->exec();

    if (result == QDialog::Accepted)
    {
        // Update type.
        this->updateEnumeration(
                    typeName,
                    this->enumerationWindow->getEnumerationName(),
                    this->enumerationWindow->getEnumerationMembers(),
                    this->enumerationWindow->getTypeSetName());
    }
}

void CustomTypesWindow::editList(QString typeName, const CustomType& type)
{
    // Show window.
    if (!this->listWindow)
    {
        this->listWindow = new ListWindow(this->typesController, this);
    }

    this->listWindow->init();

    // Update view.
    this->listWindow->setListName(type.name);
    this->listWindow->setListItemType(type.getItemType());
    this->listWindow->setTypeSetNames(this->typesController.getCustomTypeSetNames());
    this->listWindow->setTypeSetName(type.typeSetName);

    int result = this->listWindow->exec();

    if (result == QDialog::Accepted)
    {
        // Update type.
        this->updateList(
                    typeName,
                    this->listWindow->getListName(),
                    this->listWindow->getListItemType(),
                    this->listWindow->getTypeSetName());
    }
}

void CustomTypesWindow::editMap(QString typeName, const CustomType& type)
{
    // Show window.
    if (!this->mapWindow)
    {
        this->mapWindow = new MapWindow(this->typesController, this);
    }

    this->mapWindow->init();

    // Update view.
    this->mapWindow->setMapName(type.name);
    this->mapWindow->setMapKeyType(type.getKeyType());
    this->mapWindow->setMapValueType(type.getValueType());
    this->mapWindow->setTypeSetNames(this->typesController.getCustomTypeSetNames());
    this->mapWindow->setTypeSetName(type.typeSetName);

    int result = this->mapWindow->exec();

    if (result == QDialog::Accepted)
    {
        // Update type.
        this->updateMap(
                    typeName,
                    this->mapWindow->getMapName(),
                    this->mapWindow->getMapKeyType(),
                    this->mapWindow->getMapValueType(),
                    this->mapWindow->getTypeSetName());
    }
}

void CustomTypesWindow::updateDerivedType(const QString& oldName, const QString& newName, const QString& baseType, const QVariantMap facets, const QString& typeSetName)
{
    // Update model.
    this->typesController.updateDerivedType(oldName, newName, baseType, facets, typeSetName);

    // Update view.
    this->updateTable();
}

void CustomTypesWindow::updateEnumeration(const QString& oldName, const QString& newName, const QStringList& enumeration, const QString& typeSetName)
{
    // Update model.
    this->typesController.updateEnumeration(oldName, newName, enumeration, typeSetName);

    // Update view.
    this->updateTable();
}

void CustomTypesWindow::updateList(const QString& oldName, const QString& newName, const QString& itemType, const QString& typeSetName)
{
    // Update model.
    this->typesController.updateList(oldName, newName, itemType, typeSetName);

    // Update view.
    this->updateTable();
}

void CustomTypesWindow::updateMap(const QString& oldName, const QString& newName, const QString& keyType, const QString& valueType, const QString& typeSetName)
{
    // Update model.
    this->typesController.updateMap(oldName, newName, keyType, valueType, typeSetName);

    // Update view.
    this->updateTable();
}

void CustomTypesWindow::updateRow(const int index, const CustomType& type)
{
    // Disable sorting before upading data (see http://doc.qt.io/qt-5.7/qtablewidget.html#setItem)
    this->ui->tableWidget->setSortingEnabled(false);

    // Update view.
    this->ui->tableWidget->setItem(index, 0, new QTableWidgetItem(type.name));

    if (type.isDerivedType())
    {
        this->ui->tableWidget->setItem(index, 1, new QTableWidgetItem("Derived Type"));
        this->ui->tableWidget->setItem(index, 2, new QTableWidgetItem(type.getBaseType()));
    }
    else if (type.isEnumeration())
    {
        this->ui->tableWidget->setItem(index, 1, new QTableWidgetItem("Enumeration"));
        this->ui->tableWidget->setItem(index, 2, new QTableWidgetItem(type.getEnumeration().join(", ")));
    }
    else if (type.isList())
    {
        this->ui->tableWidget->setItem(index, 1, new QTableWidgetItem("List"));
        this->ui->tableWidget->setItem(index, 2, new QTableWidgetItem(type.getItemType()));
    }
    else if (type.isMap())
    {
        this->ui->tableWidget->setItem(index, 1, new QTableWidgetItem("Map"));
        this->ui->tableWidget->setItem(index, 2, new QTableWidgetItem(type.getKeyType() + " -> " + type.getValueType()));
    }

    // Enable sorting again.
    this->ui->tableWidget->setSortingEnabled(true);
}

void CustomTypesWindow::updateTable()
{
    const CustomTypeList& types = this->typesController.getCustomTypes();

    if (this->ui->tableWidget->rowCount() != types.length())
    {
        this->ui->tableWidget->setRowCount(types.length());
    }

    for (int i = 0; i < types.length(); ++i)
    {
        this->updateRow(i, types[i]);
    }
}
