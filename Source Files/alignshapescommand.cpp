#include "alignshapescommand.h"
#include "diagrameditor.h" 
#include "shape.h"       
#include <QDebug>
#include <limits> 
#include <algorithm> 

AlignShapesCommand::AlignShapesCommand(DrawingArea* area, const QList<Shape*>& shapes, Qt::Alignment alignment, QUndoCommand* parent)
    : QUndoCommand(parent),
    m_drawingArea(area),
    m_alignment(alignment)
    // m_targetShapes 在构造函数中填充
{
    if (!m_drawingArea || shapes.size() < 2) { // 对齐至少需要两个图形
        qWarning("AlignShapesCommand: Invalid area or insufficient shapes!");
        setText(QObject::tr("Invalid Alignment"));
        setObsolete(true); // 标记为无效命令
        return;
    }

    // --- 计算对齐目标值并记录需要移动的图形及其旧位置 ---
    qreal targetY = 0.0; // 垂直对齐的目标 Y 值
    qreal targetX = 0.0; // 水平对齐的目标 X 值 (如果实现水平对齐)
    bool verticalAlign = (alignment & Qt::AlignTop) || (alignment & Qt::AlignVCenter) || (alignment & Qt::AlignBottom);
    bool horizontalAlign = (alignment & Qt::AlignLeft) || (alignment & Qt::AlignHCenter) || (alignment & Qt::AlignRight);


    // 找到基准位置（例如最顶端的、最底端的、最左边、最右边、平均中心）
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxY = std::numeric_limits<qreal>::lowest();
    qreal totalCenterY = 0;
    qreal minX = std::numeric_limits<qreal>::max();
    qreal maxX = std::numeric_limits<qreal>::lowest();
    qreal totalCenterX = 0;

    for (const Shape* shape : shapes) {
        if (!shape) continue;
        QRectF rect = QRectF(shape->getRect());
        minY = std::min(minY, rect.top());
        maxY = std::max(maxY, rect.bottom());
        totalCenterY += rect.center().y();
        minX = std::min(minX, rect.left());
        maxX = std::max(maxX, rect.right());
        totalCenterX += rect.center().x();
    }

    // 根据对齐方式确定目标 Y 值
    if (alignment & Qt::AlignTop) {
        targetY = minY;
        setText(QObject::tr("Align to Top"));
    }
    else if (alignment & Qt::AlignBottom) {
        // 目标 Y 应该是使得图形底部与 maxY 对齐
        // targetY 的计算需要在 redo 中根据每个图形的高度确定
        setText(QObject::tr("Align to Bottom"));
    }
    else if (alignment & Qt::AlignVCenter) {
        targetY = totalCenterY / shapes.size(); // 平均垂直中心
        // targetY 的计算也需要在 redo 中根据每个图形的高度确定
        setText(QObject::tr("Align Vertically Centered"));
    }
    // (类似地实现水平对齐 targetX 的计算)
    else {
        qWarning("AlignShapesCommand: Unsupported alignment type!");
        setText(QObject::tr("Invalid Alignment"));
        setObsolete(true);
        return;
    }


    // --- 记录需要移动的图形及其旧位置 ---
    // 理论上所有参与对齐的图形都需要移动，除非它已经在目标位置
    // （为了简化撤销，我们记录所有参与图形的旧位置）
    for (Shape* shape : shapes) {
        if (shape) {
            m_targetShapes.append(shape); // 添加到目标列表
            m_oldRects[shape] = QRectF(shape->getRect()); // 记录旧位置
        }
    }
}

void AlignShapesCommand::undo()
{
    if (!m_drawingArea || m_targetShapes.isEmpty()) return;

    qDebug() << "Undo: Aligning shapes back to original positions";
    for (Shape* shape : m_targetShapes) {
        if (shape && m_oldRects.contains(shape)) {
            shape->setRect(m_oldRects[shape].toAlignedRect()); // 恢复旧位置
        }
    }

    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged(); // 更新属性面板
}

void AlignShapesCommand::redo()
{
    if (!m_drawingArea || m_targetShapes.isEmpty()) return;

    qDebug() << "Redo: Aligning" << m_targetShapes.size() << "shapes with alignment:" << m_alignment;

    // --- 重新计算基准位置 (代码不变) ---
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxY = std::numeric_limits<qreal>::lowest();
    qreal totalCenterY = 0;
    int validShapeCount = 0;
    // (水平对齐变量)

    for (Shape* shape : m_targetShapes) {
        if (!shape) continue;
        if (!m_oldRects.contains(shape)) continue;
        QRectF rect = m_oldRects.value(shape);

        minY = std::min(minY, rect.top());
        maxY = std::max(maxY, rect.bottom());
        totalCenterY += rect.center().y();
        // (水平基准计算)
        validShapeCount++;
    }

    if (validShapeCount < 2) {
        qWarning("AlignShapesCommand::redo: Not enough valid shapes to align.");
        return;
    }

    // --- 根据对齐方式移动每个图形 ---
    for (Shape* shape : m_targetShapes) {
        if (!shape || !m_oldRects.contains(shape)) continue;
        QRectF currentRect = m_oldRects.value(shape);
        QRectF newRect = currentRect; // 从旧矩形开始

        // --- 计算新位置 (代码不变) ---
        if (m_alignment & Qt::AlignTop) {
            newRect.moveTop(minY);
        }
        else if (m_alignment & Qt::AlignBottom) {
            newRect.moveBottom(maxY);
        }
        else if (m_alignment & Qt::AlignVCenter) {
            qreal averageCenterY = totalCenterY / validShapeCount;
            newRect.moveCenter(QPointF(newRect.center().x(), averageCenterY));
        }


        // --- 应用新位置，使用修正后的比较逻辑 ---
        bool positionChanged = !qFuzzyCompare(newRect.x(), currentRect.x()) ||
            !qFuzzyCompare(newRect.y(), currentRect.y());
        bool sizeChanged = !qFuzzyCompare(newRect.width(), currentRect.width()) ||
            !qFuzzyCompare(newRect.height(), currentRect.height());

        if (positionChanged || sizeChanged) // 如果位置或大小任意一个改变了
        {
            shape->setRect(newRect.toAlignedRect());
        }
    }

    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged();
}