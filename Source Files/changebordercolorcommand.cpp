#include "changebordercolorcommand.h"
#include "diagrameditor.h"      // ��Ҫ DrawingArea ��������
#include "shape.h"          // ��Ҫ Shape ��������
#include <QDebug>

ChangeBorderColorCommand::ChangeBorderColorCommand(DrawingArea* drawingArea,
    const QList<Shape*>& targetShapes,
    const QColor& newColor,
    QUndoCommand* parent)
    : QUndoCommand(parent),
    myDrawingArea(drawingArea),
    myNewColor(newColor)
{
    if (!myDrawingArea) {
        qWarning() << "ChangeBorderColorCommand Error: DrawingArea pointer is null.";
        return;
    }
    if (targetShapes.isEmpty()) {
        qWarning() << "ChangeBorderColorCommand created with no target shapes.";
        setText(QObject::tr("Change Border Color (No Targets)")); // ��Ч����
        return;
    }

    qDebug() << "ChangeBorderColorCommand created for" << targetShapes.count() << "shapes to color" << myNewColor.name();
    setText(QObject::tr("Change the border color of %n shapes", "", targetShapes.count())); // ʹ�� %n ֧�ָ���

    // ��¼ÿ��Ŀ��ľ���ɫ
    for (Shape* shape : targetShapes) {
        if (shape) {
            // ʹ�� Shape �ṩ�� getter ��ȡ����ɫ
            myOldColors.insert(shape, shape->getBorderColor());
        }
        else {
            qWarning() << "ChangeBorderColorCommand: Found null shape in target list.";
        }
    }
}

void ChangeBorderColorCommand::undo() {
    if (!myDrawingArea || myOldColors.isEmpty()) {
        qWarning() << "ChangeBorderColorCommand::undo() called in invalid state.";
        return;
    }
    qDebug() << "Undo ChangeBorderColorCommand";

    // �����洢�ľ���ɫӳ�䣬�ָ�ÿ��ͼ�ε���ɫ
    for (auto it = myOldColors.constBegin(); it != myOldColors.constEnd(); ++it) {
        Shape* shape = it.key();
        const QColor& oldColor = it.value();
        if (shape) {
            // ʹ�� Shape �ṩ�� setter �ָ���ɫ
            shape->setBorderColor(oldColor);
            qDebug() << "  - Shape" << shape->getId() << "restored to border color" << oldColor.name();
 
        }
        else {
            qWarning() << "Undo ChangeBorderColorCommand: Found null shape pointer in map.";
        }
    }
    myDrawingArea->update(); // �����ػ�
    myDrawingArea->shapeFormatChanged(); // ֪ͨ FormatPanel
}

void ChangeBorderColorCommand::redo() {
    if (!myDrawingArea || myOldColors.isEmpty()) { // ʹ�� myOldColors �Ƿ�Ϊ���ж���Ч��
        qWarning() << "ChangeBorderColorCommand::redo() called in invalid state.";
        return;
    }
    qDebug() << "Redo ChangeBorderColorCommand to" << myNewColor.name();

    // ��������ɫӳ��ļ� (��Ŀ��ͼ���б�)
    for (Shape* shape : myOldColors.keys()) {
        if (shape) {

            shape->setBorderColor(myNewColor);
            qDebug() << "  - Shape" << shape->getId() << "set to border color" << myNewColor.name();
        }
        else {
            qWarning() << "Redo ChangeBorderColorCommand: Found null shape pointer in map keys.";
        }
    }
    myDrawingArea->update(); // �����ػ�
    myDrawingArea->shapeFormatChanged(); // ֪ͨ FormatPanel
}