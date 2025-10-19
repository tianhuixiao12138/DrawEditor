#include "changefillcolorcommand.h"
#include "diagrameditor.h"
#include "shape.h"
#include <QDebug>

ChangeFillColorCommand::ChangeFillColorCommand(DrawingArea* drawingArea,
    const QList<Shape*>& targetShapes,
    const QColor& newColor, // �µ������ɫ
    QUndoCommand* parent)
    : QUndoCommand(parent),
    myDrawingArea(drawingArea),
    myNewColor(newColor) // ֱ�Ӵ洢����ɫ
{
    if (!myDrawingArea) { return; }
    if (targetShapes.isEmpty()) { return; }

    setText(QObject::tr("Change the fill color of %n shapes", "", targetShapes.count()));

    // ��¼ÿ��Ŀ��ľ����״̬����ɫ
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
            shape->setFilled(oldIsFilled);      // �ָ����״̬
            shape->setFillColor(oldColor);    // �ָ������ɫ

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
            // ��������ɫ������ͨ���ı������ɫ��ζ��Ҫ�������
            shape->setFillColor(myNewColor);
            shape->setFilled(true); // ͨ�����������ɫʱ���������
        }
    }
    myDrawingArea->update();
    myDrawingArea->shapeFormatChanged();
}