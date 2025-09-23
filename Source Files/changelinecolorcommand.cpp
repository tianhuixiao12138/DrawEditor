#include "changelinecolorcommand.h"
#include "diagrameditor.h"      
#include "connectionline.h" 
#include <QDebug>

ChangeLineColorCommand::ChangeLineColorCommand(DrawingArea* area, const QList<ConnectionLine*>& lines, const QColor& newColor, QUndoCommand* parent)
    : QUndoCommand(parent),
    m_drawingArea(area),
    m_targetLines(lines), // ֱ�Ӹ����б�
    m_newColor(newColor)
{
    if (!m_drawingArea || m_targetLines.isEmpty()) {
        qWarning("ChangeLineColorCommand: Invalid area or empty line list!");
        setText(QObject::tr("Invalid line color change"));
        return;
    }

    // ��¼����Ŀ�������ľ���ɫ
    m_oldColors.reserve(m_targetLines.size()); // Ԥ����ռ�
    for (ConnectionLine* line : m_targetLines) {
        if (line) {
            m_oldColors.append(line->getColor());
        }
        else {
            m_oldColors.append(QColor()); // �����ָ����������Ȼ��Ӧ�÷���
            qWarning("ChangeLineColorCommand: Found null line pointer in list!");
        }
    }

    // ������������
    setText(QObject::tr("Change line color"));
}

void ChangeLineColorCommand::undo()
{
    if (!m_drawingArea || m_targetLines.size() != m_oldColors.size()) return;

    qDebug() << "Undo: Changing color for" << m_targetLines.size() << "lines back";
    for (int i = 0; i < m_targetLines.size(); ++i) {
        ConnectionLine* line = m_targetLines.at(i);
        if (line) {
            line->setColor(m_oldColors.at(i)); // �ָ�����ɫ
        }
    }

    // ���»�ͼ����֪ͨ�������
    m_drawingArea->update();
    m_drawingArea->lineFormatChanged(); // ֪ͨ����������
}

void ChangeLineColorCommand::redo()
{
    if (!m_drawingArea || m_targetLines.isEmpty()) return;

    qDebug() << "Redo: Changing color for" << m_targetLines.size() << "lines to" << m_newColor.name();
    for (ConnectionLine* line : m_targetLines) {
        if (line) {
            line->setColor(m_newColor); // Ӧ������ɫ
        }
    }

    // ���»�ͼ����֪ͨ�������
    m_drawingArea->update();
    m_drawingArea->lineFormatChanged(); // ֪ͨ����������
}