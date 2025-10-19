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
    // m_targetShapes �ڹ��캯�������
{
    if (!m_drawingArea || shapes.size() < 2) { // ����������Ҫ����ͼ��
        qWarning("AlignShapesCommand: Invalid area or insufficient shapes!");
        setText(QObject::tr("Invalid Alignment"));
        setObsolete(true); // ���Ϊ��Ч����
        return;
    }

    // --- �������Ŀ��ֵ����¼��Ҫ�ƶ���ͼ�μ����λ�� ---
    qreal targetY = 0.0; // ��ֱ�����Ŀ�� Y ֵ
    qreal targetX = 0.0; // ˮƽ�����Ŀ�� X ֵ (���ʵ��ˮƽ����)
    bool verticalAlign = (alignment & Qt::AlignTop) || (alignment & Qt::AlignVCenter) || (alignment & Qt::AlignBottom);
    bool horizontalAlign = (alignment & Qt::AlignLeft) || (alignment & Qt::AlignHCenter) || (alignment & Qt::AlignRight);


    // �ҵ���׼λ�ã�������˵ġ���׶˵ġ�����ߡ����ұߡ�ƽ�����ģ�
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

    // ���ݶ��뷽ʽȷ��Ŀ�� Y ֵ
    if (alignment & Qt::AlignTop) {
        targetY = minY;
        setText(QObject::tr("Align to Top"));
    }
    else if (alignment & Qt::AlignBottom) {
        // Ŀ�� Y Ӧ����ʹ��ͼ�εײ��� maxY ����
        // targetY �ļ�����Ҫ�� redo �и���ÿ��ͼ�εĸ߶�ȷ��
        setText(QObject::tr("Align to Bottom"));
    }
    else if (alignment & Qt::AlignVCenter) {
        targetY = totalCenterY / shapes.size(); // ƽ����ֱ����
        // targetY �ļ���Ҳ��Ҫ�� redo �и���ÿ��ͼ�εĸ߶�ȷ��
        setText(QObject::tr("Align Vertically Centered"));
    }
    // (���Ƶ�ʵ��ˮƽ���� targetX �ļ���)
    else {
        qWarning("AlignShapesCommand: Unsupported alignment type!");
        setText(QObject::tr("Invalid Alignment"));
        setObsolete(true);
        return;
    }


    // --- ��¼��Ҫ�ƶ���ͼ�μ����λ�� ---
    // ���������в�������ͼ�ζ���Ҫ�ƶ����������Ѿ���Ŀ��λ��
    // ��Ϊ�˼򻯳��������Ǽ�¼���в���ͼ�εľ�λ�ã�
    for (Shape* shape : shapes) {
        if (shape) {
            m_targetShapes.append(shape); // ��ӵ�Ŀ���б�
            m_oldRects[shape] = QRectF(shape->getRect()); // ��¼��λ��
        }
    }
}

void AlignShapesCommand::undo()
{
    if (!m_drawingArea || m_targetShapes.isEmpty()) return;

    qDebug() << "Undo: Aligning shapes back to original positions";
    for (Shape* shape : m_targetShapes) {
        if (shape && m_oldRects.contains(shape)) {
            shape->setRect(m_oldRects[shape].toAlignedRect()); // �ָ���λ��
        }
    }

    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged(); // �����������
}

void AlignShapesCommand::redo()
{
    if (!m_drawingArea || m_targetShapes.isEmpty()) return;

    qDebug() << "Redo: Aligning" << m_targetShapes.size() << "shapes with alignment:" << m_alignment;

    // --- ���¼����׼λ�� (���벻��) ---
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxY = std::numeric_limits<qreal>::lowest();
    qreal totalCenterY = 0;
    int validShapeCount = 0;
    // (ˮƽ�������)

    for (Shape* shape : m_targetShapes) {
        if (!shape) continue;
        if (!m_oldRects.contains(shape)) continue;
        QRectF rect = m_oldRects.value(shape);

        minY = std::min(minY, rect.top());
        maxY = std::max(maxY, rect.bottom());
        totalCenterY += rect.center().y();
        // (ˮƽ��׼����)
        validShapeCount++;
    }

    if (validShapeCount < 2) {
        qWarning("AlignShapesCommand::redo: Not enough valid shapes to align.");
        return;
    }

    // --- ���ݶ��뷽ʽ�ƶ�ÿ��ͼ�� ---
    for (Shape* shape : m_targetShapes) {
        if (!shape || !m_oldRects.contains(shape)) continue;
        QRectF currentRect = m_oldRects.value(shape);
        QRectF newRect = currentRect; // �Ӿɾ��ο�ʼ

        // --- ������λ�� (���벻��) ---
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


        // --- Ӧ����λ�ã�ʹ��������ıȽ��߼� ---
        bool positionChanged = !qFuzzyCompare(newRect.x(), currentRect.x()) ||
            !qFuzzyCompare(newRect.y(), currentRect.y());
        bool sizeChanged = !qFuzzyCompare(newRect.width(), currentRect.width()) ||
            !qFuzzyCompare(newRect.height(), currentRect.height());

        if (positionChanged || sizeChanged) // ���λ�û��С����һ���ı���
        {
            shape->setRect(newRect.toAlignedRect());
        }
    }

    m_drawingArea->update();
    m_drawingArea->shapeFormatChanged();
}