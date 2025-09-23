// changeborderstylecommand.cpp
#include "changeborderstylecommand.h"
#include "diagrameditor.h"// 包含 DrawingArea 定义
#include "shape.h"       // 包含 Shape 定义
#include <QDebug>        // 用于调试输出

ChangeBorderStyleCommand::ChangeBorderStyleCommand(DrawingArea* area,
    const QList<Shape*>& shapes,
    Qt::PenStyle newStyle,
    QUndoCommand* parent)
    : QUndoCommand(parent),
    m_drawingArea(area),
    m_targetShapes(shapes), // 直接复制列表
    m_newStyle(newStyle)
{
    // 在构造函数中记录每个 Shape 的旧样式
    for (Shape* shape : m_targetShapes) {
        if (shape) {
            m_oldStyles[shape] = shape->getBorderStyle();
        }
    }
    // 设置命令的描述文本
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

    // 通知 DrawingArea 更新界面和属性面板
    m_drawingArea->shapeFormatChanged(); // 发射信号更新 FormatPanel
    m_drawingArea->update();             // 请求重绘
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

    // 通知 DrawingArea 更新界面和属性面板
    m_drawingArea->shapeFormatChanged(); // 发射信号更新 FormatPanel
    m_drawingArea->update();             // 请求重绘
}