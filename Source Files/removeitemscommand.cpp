#include "removeitemscommand.h"
#include "diagrameditor.h"
#include "shape.h"
#include "ConnectionLine.h"
#include <QDebug>
#include <QMutableListIterator>

RemoveItemsCommand::RemoveItemsCommand(DrawingArea* drawingArea,
    const QList<Shape*>& shapesToRemove,
    const QList<ConnectionLine*>& linesToRemove,
    QUndoCommand* parent)
    : QUndoCommand(parent),
    myDrawingArea(drawingArea),
    myShapesToRemove(shapesToRemove),
    myLinesToRemove(linesToRemove),
    itemsRemovedFromScene(false)
{
    if (!myDrawingArea) {
        qWarning() << QObject::tr("RemoveItemsCommand Error: DrawingArea pointer is null.");
        myShapesToRemove.clear();
        myLinesToRemove.clear();
        return;
    }

    for (Shape* s : myShapesToRemove) {
        if (s) removedShapeIdsForDebug << s->getId();
    }
    for (ConnectionLine* l : myLinesToRemove) {
        if (l) removedLineIdsForDebug << l->getId();
    }

    for (ConnectionLine* line : myDrawingArea->allLines) {
        if (!line) continue;
        if (myLinesToRemove.contains(line)) continue;

        Shape* startShape = line->getStartShape();
        if (startShape && myShapesToRemove.contains(startShape)) {
            detachedStartConnections.insert(line, { startShape, line->getStartPointIndex() });
        }

        Shape* endShape = line->getEndShape();
        if (endShape && myShapesToRemove.contains(endShape)) {
            detachedEndConnections.insert(line, { endShape, line->getEndPointIndex() });
        }
    }

    setText(QObject::tr("Delete %1 shapes and %2 lines").arg(myShapesToRemove.count()).arg(myLinesToRemove.count()));
}

RemoveItemsCommand::~RemoveItemsCommand() {
    if (itemsRemovedFromScene) {
        qDeleteAll(myShapesToRemove);
        qDeleteAll(myLinesToRemove);
    }

    myShapesToRemove.clear();
    myLinesToRemove.clear();
}

void RemoveItemsCommand::undo() {
    if (!myDrawingArea) {
        qWarning() << QObject::tr("RemoveItemsCommand::undo() called with null DrawingArea.");
        return;
    }

    for (Shape* shape : myShapesToRemove) {
        if (shape && !myDrawingArea->shapes.contains(shape)) {
            myDrawingArea->shapes.append(shape);
        }
    }

    for (ConnectionLine* line : myLinesToRemove) {
        if (line && !myDrawingArea->allLines.contains(line)) {
            myDrawingArea->allLines.append(line);
        }
    }

    for (auto it = detachedStartConnections.begin(); it != detachedStartConnections.end(); ++it) {
        ConnectionLine* line = it.key();
        Shape* shape = it.value().first;
        int index = it.value().second;
        if (line && shape) {
            if (myDrawingArea->shapes.contains(shape) && myDrawingArea->allLines.contains(line)) {
                line->attachStart(shape, index);
            }
        }
    }

    for (auto it = detachedEndConnections.begin(); it != detachedEndConnections.end(); ++it) {
        ConnectionLine* line = it.key();
        Shape* shape = it.value().first;
        int index = it.value().second;
        if (line && shape) {
            if (myDrawingArea->shapes.contains(shape) && myDrawingArea->allLines.contains(line)) {
                line->attachEnd(shape, index);
            }
        }
    }

    itemsRemovedFromScene = false;

    myDrawingArea->selectedShapes.clear();
    myDrawingArea->selectedLines.clear();
    for (Shape* s : myShapesToRemove) {
        myDrawingArea->selectedShapes.append(s);
    }
    for (ConnectionLine* l : myLinesToRemove) {
        myDrawingArea->selectedLines.append(l);
    }

    myDrawingArea->updateSelectionStates();
    myDrawingArea->showControlPoints = !myShapesToRemove.isEmpty() || !myLinesToRemove.isEmpty();

    myDrawingArea->update();
    myDrawingArea->shapeFormatChanged();
    myDrawingArea->lineFormatChanged();
}

void RemoveItemsCommand::redo() {
    if (!myDrawingArea) {
        qWarning() << QObject::tr("RemoveItemsCommand::redo() called with null DrawingArea.");
        return;
    }

    QMutableListIterator<ConnectionLine*> lineIter(myDrawingArea->allLines);
    while (lineIter.hasNext()) {
        ConnectionLine* line = lineIter.next();
        if (!line || myLinesToRemove.contains(line)) {
            continue;
        }

        Shape* startShape = line->getStartShape();
        if (startShape && myShapesToRemove.contains(startShape)) {
            line->detachStart();
        }

        Shape* endShape = line->getEndShape();
        if (endShape && myShapesToRemove.contains(endShape)) {
            line->detachEnd();
        }
    }

    int shapesRemovedCount = 0;
    for (Shape* shape : myShapesToRemove) {
        if (shape) {
            bool removed = myDrawingArea->shapes.removeOne(shape);
            if (removed) {
                shapesRemovedCount++;
                myDrawingArea->selectedShapes.removeOne(shape);
            }
        }
    }

    int linesRemovedCount = 0;
    for (ConnectionLine* line : myLinesToRemove) {
        if (line) {
            bool removed = myDrawingArea->allLines.removeOne(line);
            if (removed) {
                linesRemovedCount++;
                myDrawingArea->selectedLines.removeOne(line);
            }
        }
    }

    myDrawingArea->updateSelectionStates();
    myDrawingArea->showControlPoints = false;

    itemsRemovedFromScene = true;

    myDrawingArea->update();
    myDrawingArea->shapeFormatChanged();
    myDrawingArea->lineFormatChanged();
}