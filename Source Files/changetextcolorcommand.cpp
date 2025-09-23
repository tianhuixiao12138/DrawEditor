#include "changetextcolorcommand.h"
#include "diagrameditor.h"
#include "shape.h"
#include <QDebug>

ChangeTextColorCommand::ChangeTextColorCommand(DrawingArea* area, const QList<Shape*>& shapes, const QColor& newColor, QUndoCommand* parent)
    : QUndoCommand(parent),
    m_drawingArea(area),
    m_targetShapes(shapes),
    m_newColor(newColor)
{
    if (!m_drawingArea || m_targetShapes.isEmpty()) {
        qWarning("ChangeTextColorCommand: Invalid area or empty shape list!");
        setText(QObject::tr("Invalid text color change"));
        return;
    }

    if (!m_newColor.isValid()) {
        qWarning("ChangeTextColorCommand: Invalid new color provided!");
    }

    for (Shape* shape : m_targetShapes) {
        if (TextBox* textBox = dynamic_cast<TextBox*>(shape)) {
            m_oldTextBoxColors.insert(textBox, textBox->getTextColor());
        }
    }

    if (m_oldTextBoxColors.isEmpty()) {
        qWarning("ChangeTextColorCommand: No TextBox found in the selection!");
        setText(QObject::tr("No text box color change"));
    }
    else {
        setText(QObject::tr("Change text color"));
    }
}

void ChangeTextColorCommand::undo()
{
    if (!m_drawingArea || m_oldTextBoxColors.isEmpty()) return;

    qDebug() << "Undo: Changing text color for" << m_oldTextBoxColors.size() << "TextBoxes back";
    for (auto it = m_oldTextBoxColors.constBegin(); it != m_oldTextBoxColors.constEnd(); ++it) {
        TextBox* textBox = it.key();
        const QColor& oldColor = it.value();
        if (textBox) {
            textBox->setTextColor(oldColor);
        }
    }

    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged();
}

void ChangeTextColorCommand::redo()
{
    if (!m_drawingArea || m_oldTextBoxColors.isEmpty() || !m_newColor.isValid()) return;

    qDebug() << "Redo: Changing text color for" << m_oldTextBoxColors.size() << "TextBoxes to" << m_newColor.name();
    for (TextBox* textBox : m_oldTextBoxColors.keys()) {
        if (textBox) {
            textBox->setTextColor(m_newColor);
        }
    }

    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged();
}