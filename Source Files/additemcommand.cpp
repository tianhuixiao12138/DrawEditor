#include "additemcommand.h"
#include "diagrameditor.h" 
#include "ConnectionLine.h"
#include "shape.h"       
#include <QDebug>

AddItemCommand::AddItemCommand(DrawingArea* drawingArea, Shape* shapeToAdd, QUndoCommand* parent)
    : QUndoCommand(parent),
    myDrawingArea(drawingArea),
    myShape(shapeToAdd),
    myLine(nullptr),
    shapeAddedToScene(false),
    lineAddedToScene(false)
{
    if (!myDrawingArea) {
        qWarning() << "AddShapeCommand Error: DrawingArea pointer is null.";
        myShape = nullptr;
        return;
    }
    if (!myShape) {
        qWarning() << "AddShapeCommand Error: Shape pointer is null.";
        return;
    }

    shapeIdForDebug = myShape->getId();

    QString typeName = QObject::tr("Shape");
    if (dynamic_cast<Rectangle*>(myShape)) typeName = QObject::tr("Rectangle");
    else if (dynamic_cast<Circle*>(myShape)) typeName = QObject::tr("Circle");
    else if (dynamic_cast<Triangle*>(myShape)) typeName = QObject::tr("Triangle");
    else if (dynamic_cast<Diamond*>(myShape)) typeName = QObject::tr("Diamond");
    else if (dynamic_cast<RoundedRectangle*>(myShape)) typeName = QObject::tr("Rounded Rectangle");
    else if (dynamic_cast<Parallelogram*>(myShape)) typeName = QObject::tr("Parallelogram");
    else if (dynamic_cast<TextBox*>(myShape)) typeName = QObject::tr("Text Box");

;
    qDebug() << "AddShapeCommand created for shape ID:" << shapeIdForDebug;
}

AddItemCommand::AddItemCommand(DrawingArea* drawingArea, ConnectionLine* lineToAdd, QUndoCommand* parent)
    : QUndoCommand(parent),
    myDrawingArea(drawingArea),
    myShape(nullptr),
    myLine(lineToAdd),
    shapeAddedToScene(false),
    lineAddedToScene(false)
{
    if (!myDrawingArea) {
        qWarning() << "AddShapeCommand (line) Error: DrawingArea pointer is null.";
        myLine = nullptr;
        return;
    }
    if (!myLine) {
        qWarning() << "AddShapeCommand (line) Error: Line pointer is null.";
        return;
    }

    QString lineTypeName = QObject::tr("Connection Line");
    //setText(QObject::tr("添加 %1 '%2'").arg(lineTypeName).arg(myLine->getId()));
    qDebug() << "AddShapeCommand created for line ID:" << myLine->getId();
}

void AddItemCommand::undo() {
    if (!myDrawingArea) {
        qWarning() << "AddShapeCommand::undo() called with null DrawingArea.";
        return;
    }

    if (myShape && shapeAddedToScene) {
        myDrawingArea->shapes.removeOne(myShape);
        shapeAddedToScene = false;
    }

    if (myLine && lineAddedToScene) {
        myDrawingArea->allLines.removeOne(myLine);
        lineAddedToScene = false;
    }

    myDrawingArea->selectedShapes.removeAll(myShape);
    myDrawingArea->selectedLines.removeAll(myLine);
    myDrawingArea->updateSelectionStates();
    myDrawingArea->shapeFormatChanged(); // 通知属性面板更新
    myDrawingArea->lineFormatChanged();  // 清除线格式显示
    myDrawingArea->update(); // 请求重绘
}

void AddItemCommand::redo() {
    if (!myDrawingArea) {
        qWarning() << "AddShapeCommand::redo() called with null DrawingArea.";
        return;
    }

    if (myShape && !myDrawingArea->shapes.contains(myShape)) {
        myDrawingArea->shapes.append(myShape);
        shapeAddedToScene = true;
    }

    if (myLine && !myDrawingArea->allLines.contains(myLine)) {
        myDrawingArea->allLines.append(myLine);
        lineAddedToScene = true;
    }

    // 更新选中状态
    myDrawingArea->selectedShapes.clear();
    if (myShape) {
        myDrawingArea->selectedShapes.append(myShape);
    }

    myDrawingArea->selectedLines.clear();
    if (myLine) {
        myDrawingArea->selectedLines.append(myLine);
    }

    myDrawingArea->updateSelectionStates();
    myDrawingArea->showControlPoints = true;
    myDrawingArea->update(); // 重绘
    myDrawingArea->shapeFormatChanged(); // 通知属性面板更新
    myDrawingArea->lineFormatChanged();  // 清除线格式显示
}

AddItemCommand::~AddItemCommand() {
    if (!shapeAddedToScene && myShape) {
        qDebug() << "AddShapeCommand destructor deleting shape ID:" << shapeIdForDebug;
        delete myShape;
        myShape = nullptr;
    }
    else if (myShape) {
        qDebug() << "AddShapeCommand destructor: shape ID" << shapeIdForDebug << "is managed elsewhere.";
    }

    if (!lineAddedToScene && myLine) {
        qDebug() << "AddShapeCommand destructor: deleting line ID" << myLine->getId();
        delete myLine;
        myLine = nullptr;
    }
    else if (myLine) {
        qDebug() << "AddShapeCommand destructor: line ID" << myLine->getId() << "is managed elsewhere.";
    }

    qDebug() << "AddShapeCommand destroyed.";
}