#include "recordwindow.h"
#include "ui_recordwindow.h"

#include <QCheckBox>
#include <QMessageBox>

using namespace Tome;

const QString RecordWindow::PropertyFieldComponent = "FieldComponent";
const QString RecordWindow::PropertyFieldId = "FieldId";
const QString RecordWindow::PropertyComponentId = "ComponentId";


RecordWindow::RecordWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RecordWindow)
{
    ui->setupUi(this);
}

RecordWindow::~RecordWindow()
{
    delete this->ui;
}

void RecordWindow::accept()
{
    // Validate data.
    if (this->validate())
    {
        this->done(Accepted);
    }
}

void RecordWindow::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    this->ui->lineEditDisplayName->setFocus();
}

QString RecordWindow::getRecordDisplayName() const
{
    return this->ui->lineEditDisplayName->text();
}

QString RecordWindow::getRecordId() const
{
    return this->ui->lineEditId->text();
}

QMap<QString, RecordFieldState::RecordFieldState> RecordWindow::getRecordFields() const
{
    QMap<QString, RecordFieldState::RecordFieldState> fields;

    for (int i = 0; i < this->ui->scrollAreaFieldsContents->layout()->count(); ++i)
    {
        QLayoutItem* item = this->ui->scrollAreaFieldsContents->layout()->itemAt(i);
        QCheckBox* checkBox = static_cast<QCheckBox*>(item->widget());

        QString fieldId = checkBox->property(PropertyFieldId.toStdString().c_str()).toString();

        RecordFieldState::RecordFieldState fieldState;

        if (!checkBox->isEnabled())
        {
            fieldState = RecordFieldState::InheritedEnabled;
        }
        else if (checkBox->isChecked())
        {
            fieldState = RecordFieldState::Enabled;
        }
        else
        {
            fieldState = RecordFieldState::Disabled;
        }

        fields.insert(fieldId, fieldState);
    }

    return fields;
}

void RecordWindow::clearRecordFields()
{
    while (!this->ui->scrollAreaFieldsContents->layout()->isEmpty())
    {
        QLayoutItem* item = this->ui->scrollAreaFieldsContents->layout()->takeAt(0);
        delete item->widget();
        delete item;
    }
}

void RecordWindow::setDisallowedRecordIds(const QStringList disallowedRecordIds)
{
    this->disallowedRecordIds = disallowedRecordIds;
}

void RecordWindow::setRecordDisplayName(const QString& displayName)
{
    this->ui->lineEditDisplayName->setText(displayName);
}

void RecordWindow::setRecordId(const QString& id)
{
    this->ui->lineEditId->setText(id);
}

void RecordWindow::setRecordField(const QString& fieldId, const QString& fieldComponent, const RecordFieldState::RecordFieldState state)
{
    // Build check box text.
    QString checkBoxText = fieldId;
    if (!fieldComponent.isEmpty())
    {
        checkBoxText.append(" (" + fieldComponent + ")");
    }

    // Create checkbox.
    QCheckBox* checkBox = new QCheckBox(checkBoxText);
    checkBox->setProperty(PropertyFieldId.toStdString().c_str(), fieldId);
    checkBox->setProperty(PropertyFieldComponent.toStdString().c_str(), fieldComponent);

    // Setup checkbox.
    if (state == RecordFieldState::Enabled || state == RecordFieldState::InheritedEnabled)
    {
        checkBox->setChecked(true);
    }

    if (state == RecordFieldState::InheritedEnabled)
    {
        checkBox->setEnabled(false);
        checkBox->setToolTip(tr("This field is inherited."));
    }

    // Connect to signal.
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxStateChanged(int)) );

    // Add to layout.
    this->ui->scrollAreaFieldsContents->layout()->addWidget(checkBox);
}

void RecordWindow::setRecordFields(const FieldDefinitionList& fieldDefinitions)
{
    // Clear current fields.
    this->clearRecordFields();

    // Add all passed fields.
    for (int i = 0; i < fieldDefinitions.size(); ++i)
    {
        const FieldDefinition& fieldDefinition = fieldDefinitions.at(i);
        this->setRecordField(fieldDefinition.id, fieldDefinition.component, RecordFieldState::Disabled);
    }
}

void RecordWindow::setRecordFields(const FieldDefinitionList& fieldDefinitions, const ComponentList &componentDefinitions, const RecordFieldValueMap& ownFieldValues, const RecordFieldValueMap& inheritedFieldValues) // [pg-0003]
{
    // Clear current fields.
    this->clearRecordFields();

    // [pg-0003] Add all components.
    setRecordComponents( componentDefinitions );


    // Add all passed fields.
    for (int i = 0; i < fieldDefinitions.size(); ++i)
    {
        const FieldDefinition& fieldDefinition = fieldDefinitions.at(i);
        RecordFieldState::RecordFieldState fieldState = RecordFieldState::Disabled;

        // Check if any parent contains field.
        if (inheritedFieldValues.contains(fieldDefinition.id))
        {
            fieldState = RecordFieldState::InheritedEnabled;
        }
        // Check if record itself contains field.
        else if (ownFieldValues.contains(fieldDefinition.id))
        {
            fieldState = RecordFieldState::Enabled;
        }

        // Add to view.
        this->setRecordField(fieldDefinition.id, fieldDefinition.component, fieldState);

        // [pg-0003] Modify state of component checkboxes based on fieldState
        for (int i = 0; i < this->ui->scrollAreaComponentsContents->layout()->count(); ++i)
        {
            QCheckBox* component_cb = static_cast<QCheckBox*>(this->ui->scrollAreaComponentsContents->layout()->itemAt(i)->widget());
            QString component_id = component_cb->property(PropertyComponentId.toStdString().c_str()).toString();
            if (fieldDefinition.component == component_id)
            {
                if ( RecordFieldState::InheritedEnabled == fieldState )
                {
                    component_cb->setCheckState( Qt::CheckState::Checked );
                    component_cb->setEnabled( false );
                }
                else
                {
                    component_cb->setCheckState( RecordFieldState::Enabled == fieldState ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
                }
                break;
            }
        }
    }
}

void RecordWindow::on_lineEditDisplayName_textEdited(const QString& displayName)
{
    this->setRecordId(displayName);
}

void RecordWindow::onCheckBoxStateChanged(int state)
{
    // Get field id.
    QObject* checkbox = sender();
    QString fieldComponent = checkbox->property(PropertyFieldComponent.toStdString().c_str()).toString();

    if (fieldComponent.isEmpty())
    {
        return;
    }

    // [pg-0003] Apply state to component checkbox of fieldComponent type.
    for (int i = 0; i < this->ui->scrollAreaComponentsContents->layout()->count(); ++i)
    {
        QCheckBox* component_cb = static_cast<QCheckBox*>(this->ui->scrollAreaComponentsContents->layout()->itemAt(i)->widget());
        QString component_id = component_cb->property(PropertyComponentId.toStdString().c_str()).toString();
        if (fieldComponent == component_id)
        {
            component_cb->setCheckState((Qt::CheckState)state);
            break;
        }
    }

    // Apply state to all checkboxes of same field component.
    for (int i = 0; i < this->ui->scrollAreaFieldsContents->layout()->count(); ++i)
    {
        QCheckBox* otherCheckBox = static_cast<QCheckBox*>(this->ui->scrollAreaFieldsContents->layout()->itemAt(i)->widget());
        QString otherFieldComponent = otherCheckBox->property(PropertyFieldComponent.toStdString().c_str()).toString();

        if (otherFieldComponent == fieldComponent)
        {
            otherCheckBox->setCheckState((Qt::CheckState)state);
        }
    }
}

bool RecordWindow::validate()
{
    // Id must not be empty.
    if (this->getRecordId().size() == 0)
    {
        QMessageBox::information(
                    this,
                    tr("Missing data"),
                    tr("Please specify an id for the record."),
                    QMessageBox::Close,
                    QMessageBox::Close);
        return false;
    }

    // Display name must not be empty.
    if (this->getRecordDisplayName().size() == 0)
    {
        QMessageBox::information(
                    this,
                    tr("Missing data"),
                    tr("Please specify a name for the record."),
                    QMessageBox::Close,
                    QMessageBox::Close);
        return false;
    }

    // Record ids must be unique.
    if (this->disallowedRecordIds.contains(this->getRecordId()))
    {
        QMessageBox::information(
                    this,
                    tr("Duplicate record id"),
                    tr("Please specify another id for the record."),
                    QMessageBox::Close,
                    QMessageBox::Close);
        return false;
    }

    return true;
}

// [pg-0003]
void RecordWindow::clearRecordComponents()
{
    while (!this->ui->scrollAreaComponentsContents->layout()->isEmpty())
    {
        QLayoutItem* item = this->ui->scrollAreaComponentsContents->layout()->takeAt(0);
        delete item->widget();
        delete item;
    }
}

// [pg-0003]
void RecordWindow::setRecordComponents(const Tome::ComponentList& components)
{
    // Clear current fields.
    this->clearRecordComponents();

    // Add all passed fields.
    for (int i = 0; i < components.size(); ++i)
    {
        const Component& component = components.at(i);
        this->setRecordComponent(component, RecordFieldState::Disabled);
    }
}

// [pg-0003]
void RecordWindow::setRecordComponent(const QString& componentId, const Tome::RecordFieldState::RecordFieldState state)
{
    // Build check box text.
    QString checkBoxText = componentId;

    // Create checkbox.
    QCheckBox* checkBox = new QCheckBox(checkBoxText);
    checkBox->setProperty(PropertyComponentId.toStdString().c_str(), componentId);

    // Setup checkbox.
    if (state == RecordFieldState::Enabled || state == RecordFieldState::InheritedEnabled)
    {
        checkBox->setChecked(true);
    }

    if (state == RecordFieldState::InheritedEnabled)
    {
        checkBox->setEnabled(false);
        checkBox->setToolTip(tr("This field is inherited."));
    }

    // Connect to signal.
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(onComponentCheckBoxStateChanged(int)) );

    // Add to layout.
    this->ui->scrollAreaComponentsContents->layout()->addWidget(checkBox);
}

// [pg-0003]
void RecordWindow::onComponentCheckBoxStateChanged(int state)
{
    // Get field id.
    QObject* checkbox = sender();
    QString component_id = checkbox->property(PropertyComponentId.toStdString().c_str()).toString();

    // Apply state to all checkboxes of same field component.
    for (int i = 0; i < this->ui->scrollAreaFieldsContents->layout()->count(); ++i)
    {
        QCheckBox* otherCheckBox = static_cast<QCheckBox*>(this->ui->scrollAreaFieldsContents->layout()->itemAt(i)->widget());
        QString otherFieldComponent = otherCheckBox->property(PropertyFieldComponent.toStdString().c_str()).toString();

        if (otherFieldComponent == component_id)
        {
            otherCheckBox->setCheckState((Qt::CheckState)state);
        }
    }
}
