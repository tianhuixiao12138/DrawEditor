#ifndef CHANGETEXTCOMMAND_H
#define CHANGETEXTCOMMAND_H

#include <QUndoCommand>
#include <QString>


class DrawingArea;
class TextBox; 

class ChangeTextCommand : public QUndoCommand
{
public:
    ChangeTextCommand(DrawingArea* drawingArea,
        TextBox* targetTextBox,
        const QString& oldText,
        const QString& newText,
        QUndoCommand* parent = nullptr);

    ~ChangeTextCommand() override = default;

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand* other) override;
    int id() const override; 

private:
    DrawingArea* myDrawingArea;
    TextBox* myTargetTextBox; 
    QString myOldText;        
    QString myNewText;        
};

const int ChangeTextCommandId = 5678; // 选择一个唯一的整数 ID

#endif 