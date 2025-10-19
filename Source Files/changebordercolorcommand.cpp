#include "changebordercolorcommand.h"
#include "diagrameditor.h"      // 需要 DrawingArea 完整定义
#include "shape.h"          // 需要 Shape 完整定义
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
        setText(QObject::tr("Change Border Color (No Targets)")); // 无效命令
        return;
    }

    qDebug() << "ChangeBorderColorCommand created for" << targetShapes.count() << "shapes to color" << myNewColor.name();
    setText(QObject::tr("Change the border color of %n shapes", "", targetShapes.count())); // 使用 %n 支持复数

    // 记录每个目标的旧颜色
    for (Shape* shape : targetShapes) {
        if (shape) {
            // 使用 Shape 提供的 getter 获取旧颜色
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

    // 遍历存储的旧颜色映射，恢复每个图形的颜色
    for (auto it = myOldColors.constBegin(); it != myOldColors.constEnd(); ++it) {
        Shape* shape = it.key();
        const QColor& oldColor = it.value();
        if (shape) {
            // 使用 Shape 提供的 setter 恢复颜色
            shape->setBorderColor(oldColor);
            qDebug() << "  - Shape" << shape->getId() << "restored to border color" << oldColor.name();
 
        }
        else {
            qWarning() << "Undo ChangeBorderColorCommand: Found null shape pointer in map.";
        }
    }
    myDrawingArea->update(); // 请求重绘
    myDrawingArea->shapeFormatChanged(); // 通知 FormatPanel
}

void ChangeBorderColorCommand::redo() {
    if (!myDrawingArea || myOldColors.isEmpty()) { // 使用 myOldColors 是否为空判断有效性
        qWarning() << "ChangeBorderColorCommand::redo() called in invalid state.";
        return;
    }
    qDebug() << "Redo ChangeBorderColorCommand to" << myNewColor.name();

    // 遍历旧颜色映射的键 (即目标图形列表)
    for (Shape* shape : myOldColors.keys()) {
        if (shape) {

            shape->setBorderColor(myNewColor);
            qDebug() << "  - Shape" << shape->getId() << "set to border color" << myNewColor.name();
        }
        else {
            qWarning() << "Redo ChangeBorderColorCommand: Found null shape pointer in map keys.";
        }
    }
    myDrawingArea->update(); // 请求重绘
    myDrawingArea->shapeFormatChanged(); // 通知 FormatPanel
}