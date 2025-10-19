#include "changelinestylecommand.h"
#include "diagrameditor.h"      
#include "connectionline.h" 
#include <QDebug>

ChangeLineStyleCommand::ChangeLineStyleCommand(DrawingArea* area, const QList<ConnectionLine*>& lines, Qt::PenStyle newStyle, QUndoCommand* parent)
    : QUndoCommand(parent),
    m_drawingArea(area),
    m_targetLines(lines),
    m_newStyle(newStyle)
{
    if (!m_drawingArea || m_targetLines.isEmpty()) {
        qWarning("ChangeLineStyleCommand: Invalid area or empty line list!");
        setText(QObject::tr("Invalid line style change"));
        return;
    }

    m_oldStyles.reserve(m_targetLines.size());
    for (ConnectionLine* line : m_targetLines) {
        if (line) {
            m_oldStyles.append(line->getLineStyle());
        }
        else {
            m_oldStyles.append(Qt::SolidLine);
            qWarning("ChangeLineStyleCommand: Found null line pointer in list!");
        }
    }

    setText(QObject::tr("Change line style"));
}

void ChangeLineStyleCommand::undo()
{
    if (!m_drawingArea || m_targetLines.size() != m_oldStyles.size()) return;

    qDebug() << "Undo: Changing line style for" << m_targetLines.size() << "lines back";
    for (int i = 0; i < m_targetLines.size(); ++i) {
        ConnectionLine* line = m_targetLines.at(i);
        if (line) {
            line->setLineStyle(m_oldStyles.at(i));
        }
    }

    m_drawingArea->update();
    m_drawingArea->lineFormatChanged();
}

void ChangeLineStyleCommand::redo()
{
    if (!m_drawingArea || m_targetLines.isEmpty()) return;

    qDebug() << "Redo: Changing line style for" << m_targetLines.size() << "lines to" << m_newStyle;
    for (ConnectionLine* line : m_targetLines) {
        if (line) {
            line->setLineStyle(m_newStyle);
        }
    }

    m_drawingArea->update();
    m_drawingArea->lineFormatChanged();
}