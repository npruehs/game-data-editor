#ifndef FACET_H
#define FACET_H

#include <QString>
#include <QWidget>

namespace Tome
{
    class Facet
    {
        public:
            Facet();
            virtual ~Facet();

            virtual QWidget* createWidget() const = 0;
            virtual const QString getDisplayName() const = 0;
            virtual const QString getKey() const = 0;
            virtual const QString getTargetType() const = 0;
            virtual const QVariant getWidgetValue(QWidget* widget) const = 0;
            virtual void setWidgetValue(QWidget* widget, const QVariant value) const = 0;
    };
}

#endif // FACET_H
