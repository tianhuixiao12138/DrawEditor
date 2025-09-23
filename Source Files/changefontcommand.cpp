#include "changefontcommand.h"
#include "diagrameditor.h"
#include "shape.h"       
#include <QDebug>

ChangeFontCommand::ChangeFontCommand(DrawingArea* area, const QList<Shape*>& shapes, const QFont& newFont, QUndoCommand* parent)
    : QUndoCommand(parent),
    m_drawingArea(area),
    m_targetShapes(shapes),
    m_newFont(newFont)
{
    if (!m_drawingArea || m_targetShapes.isEmpty()) {
        qWarning("ChangeFontCommand: Invalid area or empty shape list!");
        setText(QObject::tr("Invalid Font Change"));
        return;
    }


    for (Shape* shape : m_targetShapes) {
        if (TextBox* textBox = dynamic_cast<TextBox*>(shape)) {
            m_oldTextBoxFonts.insert(textBox, textBox->getFont()); // 存储指针和旧字体
        }
    }

    if (m_oldTextBoxFonts.isEmpty()) {
        qWarning("ChangeFontCommand: No TextBox found in the selection!");
        setText(QObject::tr("No TextBox Font Change"));
    }
    else {
        setText(QObject::tr("Change Font"));
    }
}

void ChangeFontCommand::undo()
{
    if (!m_drawingArea || m_oldTextBoxFonts.isEmpty()) return;

    qDebug() << "Undo: Changing font for" << m_oldTextBoxFonts.size() << "TextBoxes back";
    // 遍历存储的 TextBox 指针和旧字体
    for (auto it = m_oldTextBoxFonts.constBegin(); it != m_oldTextBoxFonts.constEnd(); ++it) {
        TextBox* textBox = it.key();
        const QFont& oldFont = it.value();
        if (textBox) {
            textBox->setFont(oldFont); // 恢复旧字体
        }
    }

    // 更新绘图区并通知属性面板
    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged(); // 字体是图形格式的一部分
}

void ChangeFontCommand::redo()
{
    if (!m_drawingArea || m_oldTextBoxFonts.isEmpty()) return;

    qDebug() << "Redo: Changing font for" << m_oldTextBoxFonts.size() << "TextBoxes to" << m_newFont.toString();
    // 遍历存储的 TextBox 指针
    for (TextBox* textBox : m_oldTextBoxFonts.keys()) {
        if (textBox) {
            textBox->setFont(m_newFont); // 应用新字体
        }
    }

    // 更新绘图区并通知属性面板
    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged();
}