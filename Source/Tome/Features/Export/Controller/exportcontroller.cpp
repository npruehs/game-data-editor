#include "exportcontroller.h"

#include <stdexcept>

#include <QFile>
#include <QStringBuilder>
#include <QTextStream>

#include "../../Fields//Controller/fielddefinitionscontroller.h"
#include "../../Fields/Model/fielddefinition.h"
#include "../../Records/Controller/recordscontroller.h"
#include "../../Types/Controller/typescontroller.h"
#include "../../Types/Model/builtintype.h"
#include "../../Types/Model/vector.h"

using namespace Tome;

const QString ExportController::PlaceholderComponents = "$RECORD_COMPONENTS$";
const QString ExportController::PlaceholderComponentName = "$COMPONENT_NAME$";
const QString ExportController::PlaceholderFieldId = "$FIELD_ID$";
const QString ExportController::PlaceholderFieldKey = "$FIELD_KEY$";
const QString ExportController::PlaceholderFieldType = "$FIELD_TYPE$";
const QString ExportController::PlaceholderFieldValue = "$FIELD_VALUE$";
const QString ExportController::PlaceholderListItem = "$LIST_ITEM$";
const QString ExportController::PlaceholderRecordFields = "$RECORD_FIELDS$";
const QString ExportController::PlaceholderRecordId = "$RECORD_ID$";
const QString ExportController::PlaceholderRecordParentId = "$RECORD_PARENT$";
const QString ExportController::PlaceholderRecords = "$RECORDS$";


ExportController::ExportController(const FieldDefinitionsController& fieldDefinitionsController, const RecordsController& recordsController, const TypesController& typesController)
    : fieldDefinitionsController(fieldDefinitionsController),
      recordsController(recordsController),
      typesController(typesController)
{
}

const RecordExportTemplate ExportController::getRecordExportTemplate(const QString& name) const
{
    return this->model->value(name);
}

const RecordExportTemplateMap&ExportController::getRecordExportTemplates() const
{
    return *this->model;
}

void ExportController::exportRecords(const RecordExportTemplate& exportTemplate, const QString& filePath)
{
    QFile file(filePath);

    if (file.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        this->exportRecords(exportTemplate, file);
    }
    else
    {
        QString errorMessage = QObject::tr("Destination file could not be written:\r\n") + filePath;
        throw std::runtime_error(errorMessage.toStdString());
    }
}

void ExportController::exportRecords(const RecordExportTemplate& exportTemplate, QIODevice& device)
{
    // Build record file string.
    QString recordsString;

    const RecordSetList& recordSets = this->recordsController.getRecordSets();
    const FieldDefinitionList& fields = this->fieldDefinitionsController.getFieldDefinitions();

    for (int i = 0; i < recordSets.size(); ++i)
    {
        const RecordSet& recordSet = recordSets[i];

        for (int j = 0; j < recordSet.records.size(); ++j)
        {
            const Record& record = recordSet.records[j];

            // [pg-0001]
            if (record.fieldValues.empty())
            {
                continue;
            }

            // Build field values string.
            QString fieldValuesString;

            // Get fields to export.
            RecordFieldValueMap fieldValues;

            if (exportTemplate.exportAsTable)
            {
                // Build field table, filling up with empty values.
                for (int k = 0; k < fields.count(); ++k)
                {
                    const FieldDefinition& field = fields[k];

                    if (record.fieldValues.contains(field.id))
                    {
                        fieldValues[field.id] = record.fieldValues[field.id];
                    }
                    else
                    {
                        fieldValues[field.id] = "";
                    }
                }
            }
            else
            {
                fieldValues = recordsController.getRecordFieldValues(record.id);
            }

            for (RecordFieldValueMap::iterator itFields = fieldValues.begin();
                 itFields != fieldValues.end();
                 ++itFields)
            {
                QString fieldId = itFields.key();
                QVariant fieldValue = itFields.value();
                QString fieldValueText = fieldValue.toString();

                const FieldDefinition& fieldDefinition = this->fieldDefinitionsController.getFieldDefinition(fieldId);

                // Get field type name.
                QString fieldType = fieldDefinition.fieldType;
                QString exportedFieldType = exportTemplate.typeMap.contains(fieldType)
                        ? exportTemplate.typeMap[fieldType]
                        : fieldType;

                // Apply field value template.
                QString fieldValueString = exportTemplate.fieldValueTemplate;

                // Check if list.
                if (this->typesController.isCustomType(fieldType))
                {
                    const CustomType& customType = this->typesController.getCustomType(fieldType);

                    if (customType.isList())
                    {
                        // Use other template.
                        fieldValueString = exportTemplate.listTemplate;
                        fieldValueText = QString();
                        exportedFieldType = exportTemplate.typeMap.contains(customType.getItemType())
                                ? exportTemplate.typeMap[customType.getItemType()]
                                : customType.getItemType();

                        // Build list string.
                        QVariantList list = fieldValue.toList();

                        for (int i = 0; i < list.size(); ++i)
                        {
                            QString listItem = exportTemplate.listItemTemplate;
                            listItem = listItem.replace(PlaceholderFieldId, fieldId);
                            listItem = listItem.replace(PlaceholderFieldType, exportedFieldType);
                            listItem = listItem.replace(PlaceholderListItem, list[i].toString());
                            fieldValueText.append(listItem);

                            if (i < list.size() - 1)
                            {
                                fieldValueText.append(exportTemplate.listItemDelimiter);
                            }
                        }
                    }
                }
                // Check if vector.
                else if (fieldType == BuiltInType::Vector2I || fieldType == BuiltInType::Vector2R ||
                         fieldType == BuiltInType::Vector3I || fieldType == BuiltInType::Vector3R)
                {
                    // Use other template.
                    fieldValueString = exportTemplate.mapTemplate;
                    fieldValueText = QString();

                    // Build vector string.
                    QVariantMap vector = fieldValue.toMap();

                    QVariant x = vector[BuiltInType::Vector::X];
                    QVariant y = vector[BuiltInType::Vector::Y];

                    // X.
                    QString vectorComponent = exportTemplate.mapItemTemplate;
                    vectorComponent = vectorComponent.replace(PlaceholderFieldId, fieldId);
                    vectorComponent = vectorComponent.replace(PlaceholderFieldKey, "X");
                    vectorComponent = vectorComponent.replace(PlaceholderFieldValue, x.toString());
                    fieldValueText.append(vectorComponent);
                    fieldValueText.append(exportTemplate.mapItemDelimiter);

                    // Y.
                    vectorComponent = exportTemplate.mapItemTemplate;
                    vectorComponent = vectorComponent.replace(PlaceholderFieldId, fieldId);
                    vectorComponent = vectorComponent.replace(PlaceholderFieldKey, "Y");
                    vectorComponent = vectorComponent.replace(PlaceholderFieldValue, y.toString());
                    fieldValueText.append(vectorComponent);

                    if (fieldType == BuiltInType::Vector3I || fieldType == BuiltInType::Vector3R)
                    {
                        QVariant z = vector[BuiltInType::Vector::Z];

                        // Z.
                        fieldValueText.append(exportTemplate.mapItemDelimiter);

                        vectorComponent = exportTemplate.mapItemTemplate;
                        vectorComponent = vectorComponent.replace(PlaceholderFieldId, fieldId);
                        vectorComponent = vectorComponent.replace(PlaceholderFieldKey, "Z");
                        vectorComponent = vectorComponent.replace(PlaceholderFieldValue, z.toString());
                        fieldValueText.append(vectorComponent);
                    }
                }

                fieldValueString = fieldValueString.replace(PlaceholderFieldId, fieldId);
                fieldValueString = fieldValueString.replace(PlaceholderFieldType, exportedFieldType);
                fieldValueString = fieldValueString.replace(PlaceholderFieldValue, fieldValueText);

                fieldValuesString.append(fieldValueString);

                // Add delimiter, if necessary.
                if (itFields != fieldValues.end() - 1)
                {
                    fieldValuesString.append(exportTemplate.fieldValueDelimiter);
                }
            }

            // Collect components.
            QStringList components;

            for (QMap<QString, QVariant>::const_iterator itFields = record.fieldValues.begin();
                 itFields != record.fieldValues.end();
                 ++itFields)
            {
                QString fieldId = itFields.key();
                const FieldDefinition& fieldDefinition = this->fieldDefinitionsController.getFieldDefinition(fieldId);

                if (!fieldDefinition.component.isEmpty() && !components.contains(fieldDefinition.component))
                {
                    components.append(fieldDefinition.component);
                }
            }

            // Build components string.
            QString componentsString;

            for (QStringList::iterator itComponents = components.begin();
                 itComponents != components.end();
                 ++itComponents)
            {
                QString& component = *itComponents;

                // Apply component template.
                QString componentString = exportTemplate.componentTemplate;
                componentString = componentString.replace(PlaceholderComponentName, component);

                componentsString.append(componentString);

                // Add delimiter, if necessary.
                if (itComponents != components.end() - 1)
                {
                    componentsString.append(exportTemplate.componentDelimiter);
                }
            }

            // Apply record template.
            QString recordString = exportTemplate.recordTemplate;
            recordString = recordString.replace(PlaceholderRecordId, record.id);
            recordString = recordString.replace(PlaceholderRecordParentId, record.parentId);
            recordString = recordString.replace(PlaceholderRecordFields, fieldValuesString);
            recordString = recordString.replace(PlaceholderComponents, componentsString);

            recordsString.append(recordString);

            if (j < recordSet.records.size() - 1 || i < recordSets.size() - 1)
            {
                recordsString.append(exportTemplate.recordDelimiter);
            }
        }
    }

    // Apply record file template.
    QString recordFileString = exportTemplate.recordFileTemplate;
    recordFileString = recordFileString.replace(PlaceholderRecords, recordsString);

    // Write record file.
    QTextStream textStream(&device);
    textStream << recordFileString;
}

void ExportController::setRecordExportTemplates(RecordExportTemplateMap& model)
{
    this->model = &model;
}
