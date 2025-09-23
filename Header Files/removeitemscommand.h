#ifndef REMOVEITEMSCOMMAND_H
#define REMOVEITEMSCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>

class DrawingArea;
class Shape;
class ConnectionLine;

class RemoveItemsCommand : public QUndoCommand
{
public:
    RemoveItemsCommand(DrawingArea* drawingArea,
        const QList<Shape*>& shapesToRemove,
        const QList<ConnectionLine*>& linesToRemove,
        QUndoCommand* parent = nullptr);

    ~RemoveItemsCommand() override;

    void undo() override;
    void redo() override;

private:
    DrawingArea* myDrawingArea;

    QList<Shape*> myShapesToRemove;
    QList<ConnectionLine*> myLinesToRemove;

    QMap<ConnectionLine*, QPair<Shape*, int>> detachedStartConnections;
    QMap<ConnectionLine*, QPair<Shape*, int>> detachedEndConnections;

    bool itemsRemovedFromScene;

    QStringList removedShapeIdsForDebug;
    QStringList removedLineIdsForDebug;
};

#endif // REMOVEITEMSCOMMAND_H