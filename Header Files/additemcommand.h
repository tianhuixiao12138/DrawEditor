#pragma once
#ifndef ADDSHAPECOMMAND_H
#define ADDSHAPECOMMAND_H

#include <QUndoCommand>
#include <QString>

// 前向声明
class DrawingArea;
class Shape;
class ConnectionLine;

class AddItemCommand : public QUndoCommand {
public:
    // 添加图形的构造函数
    explicit AddItemCommand(DrawingArea* area, Shape* shape, QUndoCommand* parent = nullptr);

    // 添加线的构造函数
    explicit AddItemCommand(DrawingArea* area, ConnectionLine* line, QUndoCommand* parent = nullptr);

    ~AddItemCommand() override;

    void undo() override;
    void redo() override;

private:
    DrawingArea* myDrawingArea;
    Shape* myShape;
    ConnectionLine* myLine;
    bool shapeAddedToScene;
    bool lineAddedToScene;
    QString shapeIdForDebug;
};

#endif // ADDSHAPECOMMAND_H