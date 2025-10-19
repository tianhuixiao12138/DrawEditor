#pragma once
#ifndef ADDSHAPECOMMAND_H
#define ADDSHAPECOMMAND_H

#include <QUndoCommand>
#include <QString>

// ǰ������
class DrawingArea;
class Shape;
class ConnectionLine;

class AddItemCommand : public QUndoCommand {
public:
    // ���ͼ�εĹ��캯��
    explicit AddItemCommand(DrawingArea* area, Shape* shape, QUndoCommand* parent = nullptr);

    // ����ߵĹ��캯��
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