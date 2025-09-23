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
            m_oldTextBoxFonts.insert(textBox, textBox->getFont()); // �洢ָ��;�����
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
    // �����洢�� TextBox ָ��;�����
    for (auto it = m_oldTextBoxFonts.constBegin(); it != m_oldTextBoxFonts.constEnd(); ++it) {
        TextBox* textBox = it.key();
        const QFont& oldFont = it.value();
        if (textBox) {
            textBox->setFont(oldFont); // �ָ�������
        }
    }

    // ���»�ͼ����֪ͨ�������
    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged(); // ������ͼ�θ�ʽ��һ����
}

void ChangeFontCommand::redo()
{
    if (!m_drawingArea || m_oldTextBoxFonts.isEmpty()) return;

    qDebug() << "Redo: Changing font for" << m_oldTextBoxFonts.size() << "TextBoxes to" << m_newFont.toString();
    // �����洢�� TextBox ָ��
    for (TextBox* textBox : m_oldTextBoxFonts.keys()) {
        if (textBox) {
            textBox->setFont(m_newFont); // Ӧ��������
        }
    }

    // ���»�ͼ����֪ͨ�������
    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged();
}