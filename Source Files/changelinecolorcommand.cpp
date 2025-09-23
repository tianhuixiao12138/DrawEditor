#include "changelinecolorcommand.h"
#include "diagrameditor.h"      
#include "connectionline.h" 
#include <QDebug>

ChangeLineColorCommand::ChangeLineColorCommand(DrawingArea* area, const QList<ConnectionLine*>& lines, const QColor& newColor, QUndoCommand* parent)
    : QUndoCommand(parent),
    m_drawingArea(area),
    m_targetLines(lines), // 直接复制列表
    m_newColor(newColor)
{
    if (!m_drawingArea || m_targetLines.isEmpty()) {
        qWarning("ChangeLineColorCommand: Invalid area or empty line list!");
        setText(QObject::tr("Invalid line color change"));
        return;
    }

    // 记录所有目标线条的旧颜色
    m_oldColors.reserve(m_targetLines.size()); // 预分配空间
    for (ConnectionLine* line : m_targetLines) {
        if (line) {
            m_oldColors.append(line->getColor());
        }
        else {
            m_oldColors.append(QColor()); // 处理空指针的情况，虽然不应该发生
            qWarning("ChangeLineColorCommand: Found null line pointer in list!");
        }
    }

    // 设置命令描述
    setText(QObject::tr("Change line color"));
}

void ChangeLineColorCommand::undo()
{
    if (!m_drawingArea || m_targetLines.size() != m_oldColors.size()) return;

    qDebug() << "Undo: Changing color for" << m_targetLines.size() << "lines back";
    for (int i = 0; i < m_targetLines.size(); ++i) {
        ConnectionLine* line = m_targetLines.at(i);
        if (line) {
            line->setColor(m_oldColors.at(i)); // 恢复旧颜色
        }
    }

    // 更新绘图区并通知属性面板
    m_drawingArea->update();
    m_drawingArea->lineFormatChanged(); // 通知属性面板更新
}

void ChangeLineColorCommand::redo()
{
    if (!m_drawingArea || m_targetLines.isEmpty()) return;

    qDebug() << "Redo: Changing color for" << m_targetLines.size() << "lines to" << m_newColor.name();
    for (ConnectionLine* line : m_targetLines) {
        if (line) {
            line->setColor(m_newColor); // 应用新颜色
        }
    }

    // 更新绘图区并通知属性面板
    m_drawingArea->update();
    m_drawingArea->lineFormatChanged(); // 通知属性面板更新
}