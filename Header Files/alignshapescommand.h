#ifndef ALIGNSHAPESCOMMAND_H
#define ALIGNSHAPESCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QMap>
#include <QRectF>
#include <Qt> // 包含 Qt::Alignment

// 前向声明
class DrawingArea;
class Shape;

class AlignShapesCommand : public QUndoCommand
{
public:
    // 构造函数：传入绘图区、要对齐的图形列表和对齐方式
    AlignShapesCommand(DrawingArea* area, const QList<Shape*>& shapes, Qt::Alignment alignment, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    DrawingArea* m_drawingArea;
    Qt::Alignment m_alignment; // 对齐方式 
    QList<Shape*> m_targetShapes; // 只存储实际移动的图形
    QMap<Shape*, QRectF> m_oldRects; // 存储移动前的位置 <Shape*, OldRect>

    // 辅助函数（可选，在 redo 中计算新位置）
    void alignShapes();
};

#endif // ALIGNSHAPESCOMMAND_H