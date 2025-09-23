#ifndef CHANGEFONTCOMMAND_H
#define CHANGEFONTCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QFont>
#include <QMap> 


class DrawingArea;
class Shape;
class TextBox;

class ChangeFontCommand : public QUndoCommand
{
public:
    
    ChangeFontCommand(DrawingArea* area, const QList<Shape*>& shapes, const QFont& newFont, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    DrawingArea* m_drawingArea;
    QList<Shape*> m_targetShapes; 
    QFont m_newFont;
    
    QMap<TextBox*, QFont> m_oldTextBoxFonts;
};

#endif // CHANGEFONTCOMMAND_H