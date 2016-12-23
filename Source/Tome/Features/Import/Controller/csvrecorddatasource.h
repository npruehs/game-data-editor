#ifndef CSVRECORDDATASOURCE_H
#define CSVRECORDDATASOURCE_H

#include "recorddatasource.h"

namespace Tome
{
    class CsvRecordDataSource : public RecordDataSource
    {
            Q_OBJECT

        public:
            CsvRecordDataSource();

            void importData(const RecordTableImportTemplate& importTemplate, const QVariant& context);

        signals:
            void dataAvailable(const QString& importTemplateName, const QMap<QString, RecordFieldValueMap>& data) const;
            void dataUnavailable(const QString& importTemplateName, const QString& error) const;
    };
}


#endif // CSVRECORDDATASOURCE_H
