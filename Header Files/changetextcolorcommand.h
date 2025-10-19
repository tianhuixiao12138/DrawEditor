#ifndef CHANGETEXTCOLORCOMMAND_H
#define CHANGETEXTCOLORCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QColor>
#include <QMap> 

class DrawingArea;
class Shape; 
class TextBox; 

class ChangeTextColorCommand : public QUndoCommand
{
public:
    
    ChangeTextColorCommand(DrawingArea* area, const QList<Shape*>& shapes, const QColor& newColor, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    DrawingArea* m_drawingArea;
    QList<Shape*> m_targetShapes; 
    QColor m_newColor;
    QMap<TextBox*, QColor> m_oldTextBoxColors;
};

#endif // CHANGETEXTCOLORCOMMAND_H