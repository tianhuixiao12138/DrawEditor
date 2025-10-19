#include "changelinewidthcommand.h"
#include "diagrameditor.h"      
#include "connectionline.h" 
#include <QDebug>

ChangeLineWidthCommand::ChangeLineWidthCommand(DrawingArea* area, const QList<ConnectionLine*>& lines, qreal newWidth, QUndoCommand* parent)
    : QUndoCommand(parent),
    m_drawingArea(area),
    m_targetLines(lines),
    m_newWidth(newWidth)
{
    if (!m_drawingArea || m_targetLines.isEmpty()) {
        qWarning("ChangeLineWidthCommand: Invalid area or empty line list!");
        setText(QObject::tr("Invalid line width change"));
        return;
    }

    if (m_newWidth < 0.1) {
        qWarning("ChangeLineWidthCommand: Invalid new width provided!");
        m_newWidth = 0.1;
    }

    m_oldWidths.reserve(m_targetLines.size());
    for (ConnectionLine* line : m_targetLines) {
        if (line) {
            m_oldWidths.append(line->getLineWidth());
        }
        else {
            m_oldWidths.append(0.0);
            qWarning("ChangeLineWidthCommand: Found null line pointer in list!");
        }
    }

    setText(QObject::tr("Change line width"));
}

void ChangeLineWidthCommand::undo()
{
    if (!m_drawingArea || m_targetLines.size() != m_oldWidths.size()) return;

    qDebug() << "Undo: Changing width for" << m_targetLines.size() << "lines back";
    for (int i = 0; i < m_targetLines.size(); ++i) {
        ConnectionLine* line = m_targetLines.at(i);
        if (line) {
            line->setLineWidth(m_oldWidths.at(i));
        }
    }

    m_drawingArea->update();
    m_drawingArea->lineFormatChanged();
}

void ChangeLineWidthCommand::redo()
{
    if (!m_drawingArea || m_targetLines.isEmpty()) return;

    qDebug() << "Redo: Changing width for" << m_targetLines.size() << "lines to" << m_newWidth;
    for (ConnectionLine* line : m_targetLines) {
        if (line) {
            line->setLineWidth(m_newWidth);
        }
    }

    m_drawingArea->update();
    m_drawingArea->lineFormatChanged();
}