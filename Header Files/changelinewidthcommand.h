#ifndef CHANGELINEWIDTHCOMMAND_H
#define CHANGELINEWIDTHCOMMAND_H

#include <QUndoCommand>
#include <QList>


class DrawingArea;
class ConnectionLine;

class ChangeLineWidthCommand : public QUndoCommand
{
public:
    
    ChangeLineWidthCommand(DrawingArea* area, const QList<ConnectionLine*>& lines, qreal newWidth, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    DrawingArea* m_drawingArea;
    QList<ConnectionLine*> m_targetLines;
    qreal m_newWidth;
    QList<qreal> m_oldWidths; // ´æ´¢¾É¿í¶È
};

#endif 