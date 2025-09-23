#ifndef CHANGEFILLCOLORCOMMAND_H
#define CHANGEFILLCOLORCOMMAND_H
#include <QUndoCommand>
#include <QList>
#include <QColor>
#include <QMap>
class DrawingArea;
class Shape;

class ChangeFillColorCommand : public QUndoCommand
{
public:
    ChangeFillColorCommand(DrawingArea* drawingArea,
        const QList<Shape*>& targetShapes,
        const QColor& newColor,
        QUndoCommand* parent = nullptr);
    ~ChangeFillColorCommand() override = default;
    void undo() override;
    void redo() override;
private:
    DrawingArea* myDrawingArea;
    QColor myNewColor;
    QMap<Shape*, QPair<bool, QColor>> myOldFillStates;
};
#endif 