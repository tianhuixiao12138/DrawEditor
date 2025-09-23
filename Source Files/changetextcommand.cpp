#include "changetextcommand.h"
#include "diagrameditor.h"
#include "shape.h"
#include <QDebug>

ChangeTextCommand::ChangeTextCommand(DrawingArea* drawingArea,
    TextBox* targetTextBox,
    const QString& oldText,
    const QString& newText,
    QUndoCommand* parent)
    : QUndoCommand(parent),
    myDrawingArea(drawingArea),
    myTargetTextBox(targetTextBox),
    myOldText(oldText),
    myNewText(newText)
{
    if (!myDrawingArea) {
        qWarning() << "ChangeTextCommand Error: DrawingArea is null.";
        return;
    }
    if (!myTargetTextBox) {
        qWarning() << "ChangeTextCommand Error: TextBox is null.";
        return;
    }

    qDebug() << "ChangeTextCommand created for TextBox" << myTargetTextBox->getId()
        << "from" << oldText << "to" << newText;
    setText(QObject::tr("Change text"));
}

void ChangeTextCommand::undo() {
    if (!myTargetTextBox) {
        qWarning() << "ChangeTextCommand::undo() called with null TextBox.";
        return;
    }
    qDebug() << "Undo ChangeTextCommand: Restoring text for" << myTargetTextBox->getId() << "to:" << myOldText;
    myTargetTextBox->setText(myOldText);
    myDrawingArea->update();
    myDrawingArea->shapeFormatChanged();
}

void ChangeTextCommand::redo() {
    if (!myTargetTextBox) {
        qWarning() << "ChangeTextCommand::redo() called with null TextBox.";
        return;
    }
    qDebug() << "Redo ChangeTextCommand: Setting text for" << myTargetTextBox->getId() << "to:" << myNewText;
    myTargetTextBox->setText(myNewText);
    myDrawingArea->update();
    myDrawingArea->shapeFormatChanged();
}

int ChangeTextCommand::id() const {
    return ChangeTextCommandId;
}

bool ChangeTextCommand::mergeWith(const QUndoCommand* other) {
    const ChangeTextCommand* otherCmd = dynamic_cast<const ChangeTextCommand*>(other);
    if (!otherCmd) {
        return false;
    }

    if (otherCmd->myTargetTextBox != this->myTargetTextBox) {
        return false;
    }

    myNewText = otherCmd->myNewText;
    setText(QObject::tr("Change text (accumulated)"));

    return true;
}