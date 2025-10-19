#include "pastecommand.h"
#include "diagrameditor.h" // Ϊ�˷��� DrawingArea
#include "shape.h"
#include "ConnectionLine.h"
#include <QDebug>
#include <QApplication>

PasteCommand::PasteCommand(DrawingArea* drawingArea,
    const QList<Shape*>& shapesToPaste,
    const QList<ConnectionLine*>& linesToPaste,
    QUndoCommand* parent)
    : QUndoCommand(parent),
    m_drawingArea(drawingArea),
    m_pastedShapes(shapesToPaste),   // ��ʼʱ�������Ȩ
    m_pastedLines(linesToPaste),     // ��ʼʱ�������Ȩ
    m_itemsAddedToArea(false)        // ��δ��ӵ���ͼ��
{
    // ��������������ı�������ʾ�ڡ��༭���˵��ĳ���/��������
    setText(QObject::tr("Paste %1 shapes, %2 lines")
        .arg(m_pastedShapes.count())
        .arg(m_pastedLines.count()));
}

PasteCommand::~PasteCommand()
{
    // ��Ҫ��������Ŀ��δ�ɹ���ӵ���ͼ��ʱ��ɾ������
    // ���� redo ��δ�����ã��� undo �������ˣ���
    if (!m_itemsAddedToArea) {
        qDebug() << QObject::tr("PasteCommand destructor is deleting")
            << m_pastedShapes.count()
            << QObject::tr("shapes and")
            << m_pastedLines.count()
            << QObject::tr("lines.");
        qDeleteAll(m_pastedShapes);
        qDeleteAll(m_pastedLines);
    }
    else {
        qDebug() << QObject::tr("PasteCommand destructor: Items are owned by DrawingArea.");
    }
}

// ����ճ������
void PasteCommand::undo()
{
    if (!m_drawingArea || !m_itemsAddedToArea) {
        qWarning(QObject::tr("PasteCommand::undo() is called in an invalid state.")
            .toUtf8().constData());
        return;
    }
    qDebug() << QObject::tr("Undoing paste...");

    // 1. �����ǰѡ��
    m_drawingArea->selectedShapes.clear();
    m_drawingArea->selectedLines.clear();

    // 2. �� DrawingArea ���б����Ƴ���Ŀ
    for (Shape* shape : m_pastedShapes) {
        m_drawingArea->detachConnectionsAttachedTo(shape);
        m_drawingArea->shapes.removeOne(shape);
    }
    for (ConnectionLine* line : m_pastedLines) {
        m_drawingArea->allLines.removeOne(line);
    }

    // 3. ���� UI ״̬
    m_drawingArea->updateSelectionStates();
    m_drawingArea->showControlPoints = false;
    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged();
    m_drawingArea->lineFormatChanged();

    // 4. �����ĿΪ���Ƴ�������������������ɾ�����ǣ�
    m_itemsAddedToArea = false;
    qDebug() << QObject::tr("Paste undo completed.");
}

// ����ճ�����������״�ִ��ճ����
void PasteCommand::redo()
{
    if (!m_drawingArea || m_itemsAddedToArea) {
        qWarning(QObject::tr("PasteCommand::redo() is called in an invalid state.")
            .toUtf8().constData());
        return;
    }
    qDebug() << QObject::tr("Redoing paste...");

    // 1. �����ǰѡ���Է���һ��
    m_drawingArea->selectedShapes.clear();
    m_drawingArea->selectedLines.clear();

    // 2. ����Ŀ��ӵ� DrawingArea ���б���
    for (Shape* shape : m_pastedShapes) {
        if (!m_drawingArea->shapes.contains(shape)) {
            m_drawingArea->shapes.append(shape);
        }
        m_drawingArea->expandCanvasIfNeeded(shape->getRect());
    }
    for (ConnectionLine* line : m_pastedLines) {
        if (!m_drawingArea->allLines.contains(line)) {
            m_drawingArea->allLines.append(line);
        }
        m_drawingArea->expandCanvasIfNeeded(line->getBoundingRect().toAlignedRect());
    }

    // 3. ѡ����ճ������Ŀ
    m_drawingArea->selectedShapes = m_pastedShapes;
    m_drawingArea->selectedLines = m_pastedLines;

    // 4. ���� UI ״̬
    m_drawingArea->updateSelectionStates();
    m_drawingArea->showControlPoints = true;
    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged();
    m_drawingArea->lineFormatChanged();

    // 5. �����ĿΪ����ӣ�����Ȩת�Ƹ� DrawingArea��
    m_itemsAddedToArea = true;
    qDebug() << QObject::tr("Paste redo completed.");
}