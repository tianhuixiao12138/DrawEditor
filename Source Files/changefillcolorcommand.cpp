#include "changefillcolorcommand.h"
#include "diagrameditor.h"
#include "shape.h"
#include <QDebug>

ChangeFillColorCommand::ChangeFillColorCommand(DrawingArea* drawingArea,
    const QList<Shape*>& targetShapes,
    const QColor& newColor, // 新的填充颜色
    QUndoCommand* parent)
    : QUndoCommand(parent),
    myDrawingArea(drawingArea),
    myNewColor(newColor) // 直接存储新颜色
{
    if (!myDrawingArea) { return; }
    if (targetShapes.isEmpty()) { return; }

    setText(QObject::tr("Change the fill color of %n shapes", "", targetShapes.count()));

    // 记录每个目标的旧填充状态和颜色
    for (Shape* shape : targetShapes) {
        if (shape) {
            myOldFillStates.insert(shape, { shape->isFilled(), shape->getFillColor() });
        }
    }
}

void ChangeFillColorCommand::undo() {
    if (!myDrawingArea || myOldFillStates.isEmpty()) { /* ... Error handling ... */ return; }
    qDebug() << "Undo ChangeFillColorCommand";

    for (auto it = myOldFillStates.constBegin(); it != myOldFillStates.constEnd(); ++it) {
        Shape* shape = it.key();
        bool oldIsFilled = it.value().first;
        const QColor& oldColor = it.value().second;
        if (shape) {
            shape->setFilled(oldIsFilled);      // 恢复填充状态
            shape->setFillColor(oldColor);    // 恢复填充颜色

        }
    }
    myDrawingArea->update();
    myDrawingArea->shapeFormatChanged();
}

void ChangeFillColorCommand::redo() {
    if (!myDrawingArea || myOldFillStates.isEmpty()) {  return; }
    qDebug() << "Redo ChangeFillColorCommand to" << myNewColor.name();

    for (Shape* shape : myOldFillStates.keys()) {
        if (shape) {
            // 设置新颜色，并且通常改变填充颜色意味着要启用填充
            shape->setFillColor(myNewColor);
            shape->setFilled(true); // 通常设置填充颜色时会启用填充
        }
    }
    myDrawingArea->update();
    myDrawingArea->shapeFormatChanged();
}