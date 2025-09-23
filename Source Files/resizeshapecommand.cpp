#include "resizeshapecommand.h"
#include "diagrameditor.h"  
#include "shape.h"       
#include <QDebug>
#include <QRectF> 

ResizeShapeCommand::ResizeShapeCommand(DrawingArea* area, Shape* shape, const QRectF& newRect, QUndoCommand* parent)
    : QUndoCommand(parent),
    m_drawingArea(area),
    m_targetShape(shape),
    m_newRect(newRect) // 存储传入的新浮点矩形
{
    if (!m_drawingArea || !m_targetShape) {
        qWarning() << QObject::tr("ResizeShapeCommand: Invalid area or shape provided!");
        setText(QObject::tr("Invalid resizing"));
        return;
    }

   
    m_oldRect = QRectF(m_targetShape->getRect()); 

    // 设置命令的描述文本
    setText(QObject::tr("Adjust the size of the graphic"));
}

void ResizeShapeCommand::undo()
{
    if (!m_drawingArea || !m_targetShape) return;

    qDebug() << QObject::tr("Undo: Resizing shape") << m_targetShape->getId() << QObject::tr("back to") << m_oldRect;
   
    m_targetShape->setRect(m_oldRect.toAlignedRect());

    // 更新绘图区并通知属性面板
    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged(); // 通知属性面板更新显示
    
}

void ResizeShapeCommand::redo()
{
    if (!m_drawingArea || !m_targetShape) return;

    qDebug() << QObject::tr("Redo: Resizing shape") << m_targetShape->getId() << QObject::tr("to") << m_newRect;
    
    m_targetShape->setRect(m_newRect.toAlignedRect()); // <--- 应用新矩形

    // 更新绘图区并通知属性面板
    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged(); // 通知属性面板更新显示
    // m_drawingArea->lineFormatChanged();
}

