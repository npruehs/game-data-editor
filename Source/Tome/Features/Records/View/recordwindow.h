#ifndef RECORDWINDOW_H
#define RECORDWINDOW_H

#include <QDialog>

#include "../Model/recordfieldstate.h"
#include "../../Fields/Model/fielddefinitionlist.h"
#include "../../Components/Model/componentlist.h" // [pg-0003]
#include "../../Records/Model/recordfieldvaluemap.h"

namespace Ui {
    class RecordWindow;
}

class RecordWindow : public QDialog
{
        Q_OBJECT

    public:
        explicit RecordWindow(QWidget *parent = 0);
        ~RecordWindow();

        QString getRecordDisplayName() const;
        QString getRecordId() const;
        QMap<QString, Tome::RecordFieldState::RecordFieldState> getRecordFields() const;

        void clearRecordFields();

        void setDisallowedRecordIds(const QStringList disallowedRecordIds);
        void setRecordDisplayName(const QString& displayName);
        void setRecordId(const QString& id);
        void setRecordField(const QString& fieldId, const QString& fieldComponent, const Tome::RecordFieldState::RecordFieldState state);
        void setRecordFields(const Tome::FieldDefinitionList& fieldDefinitions);
        void setRecordFields(const Tome::FieldDefinitionList& fieldDefinitions, const Tome::ComponentList &componentDefinitions, const Tome::RecordFieldValueMap& ownFieldValues, const Tome::RecordFieldValueMap& inheritedFieldValues); // [pg-0003]
        void setRecordComponent(const QString& componentId, const Tome::RecordFieldState::RecordFieldState state); // [pg-0003]
        void setRecordComponents(const Tome::ComponentList& components); // [pg-0003]
        void clearRecordComponents(); // [pg-0003]

    public slots:
        void accept();

    protected:
        void showEvent(QShowEvent* event);

    private slots:
        void on_lineEditDisplayName_textEdited(const QString &arg1);

        void onCheckBoxStateChanged(int state);
        void onComponentCheckBoxStateChanged(int state); // [pg-0003]

    private:
        static const QString PropertyFieldComponent;
        static const QString PropertyFieldId;
        static const QString PropertyComponentId;

        Ui::RecordWindow *ui;

        QStringList disallowedRecordIds;

        bool validate();
};

#endif // RECORDWINDOW_H
