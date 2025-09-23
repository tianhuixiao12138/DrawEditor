#ifndef PASTECOMMAND_H
#define PASTECOMMAND_H

#include <QUndoCommand>
#include <QList>

class DrawingArea;
class Shape;
class ConnectionLine;

class PasteCommand : public QUndoCommand
{
public:
    PasteCommand(DrawingArea* drawingArea,
        const QList<Shape*>& shapesToPaste,
        const QList<ConnectionLine*>& linesToPaste,
        QUndoCommand* parent = nullptr);

    ~PasteCommand() override;

    void undo() override;
    void redo() override;

private:
    DrawingArea* m_drawingArea;
    QList<Shape*> m_pastedShapes;
    QList<ConnectionLine*> m_pastedLines;
    bool m_itemsAddedToArea;
};

#endif // PASTECOMMAND_H