#ifndef ALIGNSHAPESCOMMAND_H
#define ALIGNSHAPESCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QMap>
#include <QRectF>
#include <Qt> // ���� Qt::Alignment

// ǰ������
class DrawingArea;
class Shape;

class AlignShapesCommand : public QUndoCommand
{
public:
    // ���캯���������ͼ����Ҫ�����ͼ���б�Ͷ��뷽ʽ
    AlignShapesCommand(DrawingArea* area, const QList<Shape*>& shapes, Qt::Alignment alignment, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    DrawingArea* m_drawingArea;
    Qt::Alignment m_alignment; // ���뷽ʽ 
    QList<Shape*> m_targetShapes; // ֻ�洢ʵ���ƶ���ͼ��
    QMap<Shape*, QRectF> m_oldRects; // �洢�ƶ�ǰ��λ�� <Shape*, OldRect>

    // ������������ѡ���� redo �м�����λ�ã�
    void alignShapes();
};

#endif // ALIGNSHAPESCOMMAND_H