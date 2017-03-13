#ifndef MAPKEYTYPEDOESNOTEXISTTASK_H
#define MAPKEYTYPEDOESNOTEXISTTASK_H

#include <QObject>

#include "../../Tasks/Controller/task.h"

namespace Tome
{
    class TaskContext;

    class MapKeyTypeDoesNotExistTask : public QObject, public Task
    {
        public:
            MapKeyTypeDoesNotExistTask();

            const QString getDisplayName() const;
            const MessageList execute(const TaskContext& context) const;

            static const QString MessageCode;
    };
}

#endif // MAPKEYTYPEDOESNOTEXISTTASK_H
