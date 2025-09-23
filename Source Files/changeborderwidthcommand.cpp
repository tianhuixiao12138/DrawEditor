#include "ChangeBorderWidthCommand.h"
#include <QDebug>
#include <QtMath> 
#include "diagrameditor.h"      
#include "shape.h"          

ChangeBorderWidthCommand::ChangeBorderWidthCommand(DrawingArea* area,
    const QList<Shape*>& shapes,
    qreal newWidth,
    QUndoCommand* parent)
    : QUndoCommand(parent),
    myDrawingArea(area),
    myNewWidth(newWidth)
{
    if (!myDrawingArea) return;

    for (Shape* shape : shapes) {
        if (shape) {
            myOldWidths.insert(shape, shape->getBorderWidth());
            myOldColors.insert(shape, shape->getBorderColor());
        }
    }

    setText(QObject::tr("Change the border width of %n shapes", "", shapes.size()).arg(myNewWidth, 0, 'f', 1));
}

void ChangeBorderWidthCommand::undo() {
    if (!myDrawingArea || myOldWidths.isEmpty()) return;

    for (auto it = myOldWidths.constBegin(); it != myOldWidths.constEnd(); ++it) {
        Shape* shape = it.key();
        qreal oldWidth = it.value();
        QColor oldColor = myOldColors.value(shape);

        if (shape && myDrawingArea->shapes.contains(shape)) {
            shape->setBorderWidth(oldWidth);
            shape->setBorderColor(oldColor);
        }
    }

    myDrawingArea->update();
    myDrawingArea->shapeFormatChanged();
}

void ChangeBorderWidthCommand::redo() {
    if (!myDrawingArea || myOldWidths.isEmpty()) return;

    for (auto it = myOldWidths.constBegin(); it != myOldWidths.constEnd(); ++it) {
        Shape* shape = it.key();
        if (shape && myDrawingArea->shapes.contains(shape)) {
            shape->setBorderWidth(myNewWidth);
            if (myNewWidth == 0.0) {
                shape->setBorderColor(Qt::transparent);
            }
            else {
                // 如果原来的宽度是 0 ，则设为黑色
                bool wasNoBorder = qFuzzyCompare(it.value(), 0.0);
                QColor oldColor = myOldColors.value(shape);

                if (wasNoBorder) {
                    shape->setBorderColor(Qt::black); // 默认黑色
                }
                else {
                    shape->setBorderColor(oldColor);
                }
            }
        }
    }

    myDrawingArea->update();
    myDrawingArea->shapeFormatChanged();
}