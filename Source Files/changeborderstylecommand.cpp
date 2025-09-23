// changeborderstylecommand.cpp
#include "changeborderstylecommand.h"
#include "diagrameditor.h"// ���� DrawingArea ����
#include "shape.h"       // ���� Shape ����
#include <QDebug>        // ���ڵ������

ChangeBorderStyleCommand::ChangeBorderStyleCommand(DrawingArea* area,
    const QList<Shape*>& shapes,
    Qt::PenStyle newStyle,
    QUndoCommand* parent)
    : QUndoCommand(parent),
    m_drawingArea(area),
    m_targetShapes(shapes), // ֱ�Ӹ����б�
    m_newStyle(newStyle)
{
    // �ڹ��캯���м�¼ÿ�� Shape �ľ���ʽ
    for (Shape* shape : m_targetShapes) {
        if (shape) {
            m_oldStyles[shape] = shape->getBorderStyle();
        }
    }
    // ��������������ı�
    setText(QObject::tr("Change Border Style"));
}

void ChangeBorderStyleCommand::undo()
{
    if (!m_drawingArea) return;

    qDebug() << "Undo: Changing border style back for" << m_oldStyles.count() << "shapes";
    for (auto it = m_oldStyles.constBegin(); it != m_oldStyles.constEnd(); ++it) {
        Shape* shape = it.key();
        Qt::PenStyle oldStyle = it.value();
        if (shape) {
            shape->setBorderStyle(oldStyle);
        }
    }

    // ֪ͨ DrawingArea ���½�����������
    m_drawingArea->shapeFormatChanged(); // �����źŸ��� FormatPanel
    m_drawingArea->update();             // �����ػ�
}

void ChangeBorderStyleCommand::redo()
{
    if (!m_drawingArea) return;

    qDebug() << "Redo: Changing border style to" << static_cast<int>(m_newStyle) << "for" << m_targetShapes.count() << "shapes";
    for (Shape* shape : m_targetShapes) {
        if (shape) {
            shape->setBorderStyle(m_newStyle);
        }
    }

    // ֪ͨ DrawingArea ���½�����������
    m_drawingArea->shapeFormatChanged(); // �����źŸ��� FormatPanel
    m_drawingArea->update();             // �����ػ�
}