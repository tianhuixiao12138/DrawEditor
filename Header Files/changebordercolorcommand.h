#ifndef CHANGEBORDERCOLORCOMMAND_H
#define CHANGEBORDERCOLORCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QColor>
#include <QMap> // ���ڴ洢ÿ������ľ���ɫ

class DrawingArea;
class Shape; 

class ChangeBorderColorCommand : public QUndoCommand
{
public:
    ChangeBorderColorCommand(DrawingArea* drawingArea,
        const QList<Shape*>& targetShapes,
        const QColor& newColor,
        QUndoCommand* parent = nullptr);
    ~ChangeBorderColorCommand() override = default;

    void undo() override;
    void redo() override;


private:
    DrawingArea* myDrawingArea;
    QColor myNewColor;
    QMap<Shape*, QColor> myOldColors;
};


#endif // CHANGEBORDERCOLORCOMMAND_H