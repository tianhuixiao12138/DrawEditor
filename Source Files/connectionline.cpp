#include "connectionline.h"
#include <QPainter>
#include <QPolygonF>
#include <cmath>
#include <QtMath>  
#include <QDebug>
#include <limits>
#include <QUuid>    
#include <algorithm>
#include "shape.h"

// --- ���岢��ʼ����̬��Ա ---
const qreal ConnectionLine::handleSize = 8.0;
const qreal ConnectionLine::selectionPadding = 2.5;
// baseWidth ���Ƴ�

// --- ���캯�� (��ȫ����) ---
ConnectionLine::ConnectionLine(Shape* startShape, int startPointIndex, Shape* endShape, int endPointIndex,
    QColor color, qreal lineWidth, Qt::PenStyle lineStyle, bool hasArrow) 
    : m_color(color), m_lineWidth(lineWidth), m_lineStyle(lineStyle), 
    m_selected(false),
    m_startShape(startShape), m_startPointIndex(startPointIndex), m_freeStartPoint(), // ��ʼ���ɵ���Ч
    m_endShape(endShape), m_endPointIndex(endPointIndex), m_freeEndPoint(),        // ��ʼ���ɵ���Ч
    m_hasArrowHead(hasArrow) // <<<--- ��ʼ�� m_hasArrowHead
{
    Q_ASSERT(m_startShape != nullptr && m_startPointIndex >= 0);
    Q_ASSERT(m_endShape != nullptr && m_endPointIndex >= 0);
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces); // <--- ��ʼ�� ID
    updatePen(); // <--- ���»���
}

// --- ���캯�� (��ȫ����) ---
ConnectionLine::ConnectionLine(const QPointF& p1, const QPointF& p2,
    QColor color, qreal lineWidth, Qt::PenStyle lineStyle, bool hasArrow) // <--- ����²���
    : m_color(color), m_lineWidth(lineWidth), m_lineStyle(lineStyle), // <--- ��ʼ���³�Ա
    m_selected(false),
    m_startShape(nullptr), m_startPointIndex(-1), m_freeStartPoint(p1),
    m_endShape(nullptr), m_endPointIndex(-1), m_freeEndPoint(p2),
    m_hasArrowHead(hasArrow) // <<<--- ��ʼ�� m_hasArrowHead
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces); // <--- ��ʼ�� ID
    updatePen(); // <--- ���»���
}

// --- Ĭ�Ϲ��캯�� (���ڼ���) ---
ConnectionLine::ConnectionLine()
    : m_color(Qt::darkGray), m_lineWidth(1.5), m_lineStyle(Qt::SolidLine), // <--- ����Ĭ��ֵ
    m_selected(false),
    m_startShape(nullptr), m_startPointIndex(-1), m_freeStartPoint(),
    m_endShape(nullptr), m_endPointIndex(-1), m_freeEndPoint()
{
    updatePen(); // <--- ���»���
}

// --- ���»��� ---
void ConnectionLine::updatePen() {
    m_pen.setColor(m_color);
    m_pen.setWidthF(m_lineWidth);       // <--- ʹ�ó�Ա�����߿�
    m_pen.setStyle(m_lineStyle);      // <--- ʹ�ó�Ա��������
    m_pen.setJoinStyle(Qt::RoundJoin);
    m_pen.setCapStyle(Qt::RoundCap);
}
// --- ���� Getter/Setter ʵ�� ---
bool ConnectionLine::hasArrowHead() const {
    return m_hasArrowHead;
}

void ConnectionLine::setHasArrowHead(bool hasArrow) {
    if (m_hasArrowHead != hasArrow) {
        m_hasArrowHead = hasArrow;
        
    }
}


// --- ��ȡʵ�ʶ˵����� (����) ---
QPointF ConnectionLine::getStartPointPos() const {
    if (isStartAttached()) {
        if (m_startPointIndex >= 0 && m_startShape && m_startPointIndex < m_startShape->getConnectionPointCount()) {
            return m_startShape->getConnectionPoint(m_startPointIndex);
        }
        else {
            qWarning() << "ConnectionLine: Invalid start point index or shape. Using center.";
            return m_startShape ? m_startShape->getRect().center() : m_freeStartPoint; // ���ShapeҲ��Ч���������ɵ�
        }
    }
    else {
        return m_freeStartPoint;
    }
}

QPointF ConnectionLine::getEndPointPos() const {
    if (isEndAttached()) {
        if (m_endPointIndex >= 0 && m_endShape && m_endPointIndex < m_endShape->getConnectionPointCount()) {
            return m_endShape->getConnectionPoint(m_endPointIndex);
        }
        else {
            qWarning() << "ConnectionLine: Invalid end point index or shape. Using center.";
            return m_endShape ? m_endShape->getRect().center() : m_freeEndPoint; // ���ShapeҲ��Ч���������ɵ�
        }
    }
    else {
        return m_freeEndPoint;
    }
}

// --- �������ɶ˵����� (����) ---
void ConnectionLine::setFreeStartPoint(const QPointF& p) {
    if (!isStartAttached()) {
        m_freeStartPoint = p;
    }
    else {
        qWarning() << "ConnectionLine::setFreeStartPoint called on an attached start point.";
    }
}

void ConnectionLine::setFreeEndPoint(const QPointF& p) {
    if (!isEndAttached()) {
        m_freeEndPoint = p;
    }
    else {
        qWarning() << "ConnectionLine::setFreeEndPoint called on an attached end point.";
    }
}

// --- ����/�Ͽ��˵� (����) ---
void ConnectionLine::attachStart(Shape* shape, int pointIndex) {
    if (!shape || pointIndex < 0 || pointIndex >= shape->getConnectionPointCount()) {
        qWarning() << "ConnectionLine::attachStart - Invalid shape or point index.";
        return;
    }
    m_startShape = shape;
    m_startPointIndex = pointIndex;
}

void ConnectionLine::attachEnd(Shape* shape, int pointIndex) {
    if (!shape || pointIndex < 0 || pointIndex >= shape->getConnectionPointCount()) {
        qWarning() << "ConnectionLine::attachEnd - Invalid shape or point index.";
        return;
    }
    m_endShape = shape;
    m_endPointIndex = pointIndex;
}

void ConnectionLine::detachStart() {
    if (isStartAttached()) {
        m_freeStartPoint = getStartPointPos();
        m_startShape = nullptr;
        m_startPointIndex = -1;
    }
}

void ConnectionLine::detachEnd() {
    if (isEndAttached()) {
        m_freeEndPoint = getEndPointPos();
        m_endShape = nullptr;
        m_endPointIndex = -1;
    }
}

void ConnectionLine::detachShape(const Shape* shape) {
    if (!shape) return;
    if (m_startShape == shape) {
        detachStart();
    }
    if (m_endShape == shape) {
        detachEnd();
    }
}

// --- ���� (�޸�ѡ��ʱ���߿����) ---
//void ConnectionLine::draw(QPainter& painter) const {
//    // ... (��ͷ����Ч�Լ�鲻��) ...
//    if (isStartAttached() && !m_startShape) return;
//    if (isEndAttached() && !m_endShape) return;
//
//    QPointF startPoint = getStartPointPos();
//    QPointF endPoint = getEndPointPos();
//
//    if (QLineF(startPoint, endPoint).length() < 0.5) {
//        return;
//    }
//
//    painter.save();
//
//    // ����ѡ�и�������
//    if (m_selected) {
//        QPen selectionPen;
//        selectionPen.setColor(QColor(0, 100, 255, 120)); // ��͸����ɫ
//        // <--- �޸ģ������߿���ڵ�ǰ�߿� ---
//        selectionPen.setWidthF(m_lineWidth + selectionPadding * 2.0);
//        selectionPen.setStyle(m_lineStyle); // <--- ������Ҳʹ�õ�ǰ����
//        selectionPen.setCapStyle(Qt::RoundCap);
//        selectionPen.setJoinStyle(Qt::RoundJoin);
//        painter.setPen(selectionPen);
//        painter.drawLine(startPoint, endPoint); // ���Ʊ�����
//    }
//
//    // ���������߱���
//    painter.setPen(m_pen);              // ʹ�û���Ļ��� (�Ѱ�����ɫ����ȡ���ʽ)
//    painter.setBrush(m_pen.color());    // ��ͷ������ߵ���ɫ
//    painter.drawLine(startPoint, endPoint);
//
//    // ���Ƽ�ͷ
//    // *** �޸ģ����� m_hasArrowHead �����Ƿ���Ƽ�ͷ ***
//    if (m_hasArrowHead) {
//        drawArrowHead(painter, startPoint, endPoint);
//    }
//    // *** �����޸� ***
//
//    // ���ѡ�У�Ϊ *����* �˵���ƿ����ֱ�
//    if (m_selected) {
//        if (!isStartAttached()) {
//            drawHandle(painter, startPoint);
//        }
//        if (!isEndAttached()) {
//            drawHandle(painter, endPoint);
//        }
//    }
//
//    painter.restore();
//}

void ConnectionLine::draw(QPainter& painter) const {
    // ... (��Ч�Լ��) ...
    if (isStartAttached() && !m_startShape) return;
    if (isEndAttached() && !m_endShape) return;

    QPointF startPoint = getStartPointPos();
    QPointF endPoint = getEndPointPos();
    QLineF mainLine(startPoint, endPoint);
    qreal mainLineLength = mainLine.length();

    if (mainLineLength < 0.5) {
        return;
    }

    painter.save();

    // --- 1. �����ͷ�ߴ� ---
    qreal actualArrowLength = 0;
    const qreal baseArrowAngleRad = M_PI / 7.0; 

    if (m_hasArrowHead) {
        const qreal baseArrowLength = 12.0;
        const qreal standardLineWidth = 1.5;
        qreal safeLineWidth = qMax(0.5, m_lineWidth);
        qreal scaleFactor = safeLineWidth / standardLineWidth;
        scaleFactor = qMax(0.6, scaleFactor);
        actualArrowLength = baseArrowLength * scaleFactor;
        const qreal minArrowLength = 6.0;
        const qreal maxArrowLength = 25.0;
        actualArrowLength = qBound(minArrowLength, actualArrowLength, maxArrowLength);
    }

    // --- 2. �������̺���߶��յ� lineEndPoint ---
    QPointF lineEndPoint = endPoint; // Ĭ����ԭʼ�յ�
    qreal pullbackDistance = 0.0;

    if (m_hasArrowHead && actualArrowLength > 0) {
        pullbackDistance = actualArrowLength * std::cos(baseArrowAngleRad);

      
        if (mainLineLength > pullbackDistance + 0.1) { // ��һ�������
            QLineF tempLine = mainLine;
            tempLine.setLength(mainLineLength - pullbackDistance);
            lineEndPoint = tempLine.p2();
        }
        else {

            QLineF tempLine = mainLine;
            tempLine.setLength(mainLineLength * 0.1);
            lineEndPoint = tempLine.p2();
            qDebug() << "Line too short for full pullback, drawing shortened line.";
            
        }
    }

    // --- 3. ����ѡ�и������� ---
    if (m_selected) {
        QPen selectionPen;
        selectionPen.setColor(QColor(0, 100, 255, 120));
        selectionPen.setWidthF(m_lineWidth + selectionPadding * 2.0);
        selectionPen.setStyle(m_lineStyle);
        selectionPen.setCapStyle(Qt::RoundCap);
        selectionPen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(selectionPen);
       
        painter.drawLine(startPoint, lineEndPoint);
    }

    // --- 4. ���������߱���  ---
    QPen linePen = m_pen; 
    linePen.setCapStyle(Qt::FlatCap); 
    painter.setPen(linePen);
    painter.drawLine(startPoint, lineEndPoint);

    // --- 5. ���Ƽ�ͷ  ---
    if (m_hasArrowHead) {
        drawArrowHead(painter, startPoint, endPoint);
    }

    // --- 6. ���ƿ����ֱ�  ---
    if (m_selected) {
        if (!isStartAttached()) {
            drawHandle(painter, startPoint);
        }
        if (!isEndAttached()) {
            drawHandle(painter, endPoint); // �ֱ���ԭʼ�յ�
        }
    }

    painter.restore();
}

// --- ���ƿ����ֱ�  ---
void ConnectionLine::drawHandle(QPainter& painter, const QPointF& pos) const {
    painter.save();
    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    qreal hs = handleSize;
    painter.drawRect(QRectF(pos.x() - hs / 2.0, pos.y() - hs / 2.0, hs, hs));
    painter.restore();
}

// --- ���Ƽ�ͷ  ---
void ConnectionLine::drawArrowHead(QPainter& painter, const QPointF& p1, const QPointF& p2) const {
    const qreal baseArrowLength = 12.0;
    const qreal baseArrowAngleRad = M_PI / 7.0;
    const qreal standardLineWidth = 1.5;

    qreal safeLineWidth = qMax(0.5, m_lineWidth);
    qreal scaleFactor = safeLineWidth / standardLineWidth;
    scaleFactor = qMax(0.6, scaleFactor);

    qreal actualArrowLength = baseArrowLength * scaleFactor;

    const qreal minArrowLength = 6.0;
    const qreal maxArrowLength = 25.0;
    actualArrowLength = qBound(minArrowLength, actualArrowLength, maxArrowLength);

    QLineF line(p1, p2);
    if (line.length() < actualArrowLength * 0.3) {
        return;
    }

    double angle = std::atan2(p2.y() - p1.y(), p2.x() - p1.x());

    QPointF arrowP1 = p2 - QPointF(actualArrowLength * std::cos(angle + baseArrowAngleRad),
        actualArrowLength * std::sin(angle + baseArrowAngleRad));
    QPointF arrowP2 = p2 - QPointF(actualArrowLength * std::cos(angle - baseArrowAngleRad),
        actualArrowLength * std::sin(angle - baseArrowAngleRad));

    QPolygonF arrowHead;
    arrowHead << p2 << arrowP1 << arrowP2;

    QPen originalPen = painter.pen();
    QBrush originalBrush = painter.brush();

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_pen.color());
    painter.drawPolygon(arrowHead);

    painter.setBrush(originalBrush);
    painter.setPen(originalPen);
}

// --- ѡ�м��  ---
bool ConnectionLine::contains(const QPointF& point, qreal tolerance) const {

    if (isStartAttached() && !m_startShape) return false;
    if (isEndAttached() && !m_endShape) return false;

    QPointF p1 = getStartPointPos();
    QPointF p2 = getEndPointPos();
    QLineF line(p1, p2);
    qreal lineLenSq = line.length() * line.length();

    if (lineLenSq < std::numeric_limits<qreal>::epsilon()) {
        QPointF diff = point - p1;
        return (diff.x() * diff.x() + diff.y() * diff.y()) <= tolerance * tolerance;
    }

    qreal t = QPointF::dotProduct(point - p1, p2 - p1) / lineLenSq;
    QPointF closestPointOnLineSegment;
    if (t < 0.0) closestPointOnLineSegment = p1;
    else if (t > 1.0) closestPointOnLineSegment = p2;
    else closestPointOnLineSegment = p1 + t * (p2 - p1);

    QPointF diff = point - closestPointOnLineSegment;
    qreal distSq = diff.x() * diff.x() + diff.y() * diff.y();

    
    qreal totalTolerance = tolerance + (m_lineWidth + selectionPadding) / 2.0;
    return distSq <= totalTolerance * totalTolerance;
}

// --- �ƶ� ---
void ConnectionLine::move(const QPointF& offset) {
    if (!isStartAttached()) {
        m_freeStartPoint += offset;
    }
    if (!isEndAttached()) {
        m_freeEndPoint += offset;
    }
}

// --- ������ɫ  ---
void ConnectionLine::setColor(const QColor& color) {
    if (m_color != color) {
        m_color = color;
        updatePen();
    }
}

// --- �����߿� ---
void ConnectionLine::setLineWidth(qreal width) {
    qreal w = qMax(0.5, width); // ��С�߿���Ϊ 0.5
    if (!qFuzzyCompare(m_lineWidth, w)) { // ���⸡�����Ƚ�����
        m_lineWidth = w;
        updatePen(); // ���»���
    }
}

// --- �������� ---
void ConnectionLine::setLineStyle(Qt::PenStyle style) {
    
    if (m_lineStyle != style) {
        m_lineStyle = style;
        updatePen(); // ���»���
    }
}


// --- ����ѡ��״̬  ---
void ConnectionLine::setSelected(bool selected) {
    if (m_selected != selected) {
        m_selected = selected;
    }
}

// --- ��ȡ��Χ�� ---
QRectF ConnectionLine::getBoundingRect() const {
    if (isStartAttached() && !m_startShape) return QRectF();
    if (isEndAttached() && !m_endShape) return QRectF();

    QPointF p1 = getStartPointPos();
    QPointF p2 = getEndPointPos();

    QRectF rect(p1, p2);
    rect = rect.normalized();

    qreal padding = (m_lineWidth + selectionPadding) / 2.0 + 1.0;
    return rect.adjusted(-padding, -padding, padding, padding);
}

// --- ����Ϊ JSON (����߿������) ---
QJsonObject ConnectionLine::toJson() const {
    QJsonObject obj;
    obj["id"] = m_id; 
    obj["color"] = m_color.name(QColor::HexRgb);
    obj["lineWidth"] = m_lineWidth; 
    obj["hasArrow"] = m_hasArrowHead; 

    // <--- �������� ---
    obj["lineStyle"] = static_cast<int>(m_lineStyle); 
    // ���������Ϣ 
    if (isStartAttached()) {
        obj["startShapeId"] = m_startShape->getId();
        obj["startIndex"] = m_startPointIndex;
    }
    else {
        obj["startPosX"] = m_freeStartPoint.x();
        obj["startPosY"] = m_freeStartPoint.y();
    }

    // �����յ���Ϣ 
    if (isEndAttached()) {
        obj["endShapeId"] = m_endShape->getId();
        obj["endIndex"] = m_endPointIndex;
    }
    else {
        obj["endPosX"] = m_freeEndPoint.x();
        obj["endPosY"] = m_freeEndPoint.y();
    }

    return obj;
}

// --- �� JSON ���� (����߿������) ---
ConnectionLine* ConnectionLine::fromJson(const QJsonObject& obj, const QMap<QString, Shape*>& shapeIdMap) {
    ConnectionLine* line = new ConnectionLine(); // ����Ĭ�϶���

     // <--- ���ػ����� ID ---
    line->m_id = obj.value("id").toString(QUuid::createUuid().toString(QUuid::WithoutBraces));
    if (line->m_id.isEmpty()) { // �ٴμ�飬��ֹ���ַ���
        line->m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        qWarning() << "fromJson: Line loaded with empty ID, assigned new one:" << line->m_id;
    }
    // <--- ���� ID ���� ---
    

    // ������ɫ 
    if (obj.contains("color") && obj["color"].isString()) {
        QColor color(obj["color"].toString());
        if (color.isValid()) {
            line->setColor(color); 
        }
    }

    // <---�����߿� ---
    if (obj.contains("lineWidth") && obj["lineWidth"].isDouble()) {
        line->setLineWidth(obj["lineWidth"].toDouble(1.5)); // �ṩĬ��ֵ
    }

    // <---�������� ---
    if (obj.contains("lineStyle") && obj["lineStyle"].isDouble()) { // JSON ����ͨ���� double
        int styleInt = obj["lineStyle"].toInt(static_cast<int>(Qt::SolidLine)); // Ĭ�� SolidLine
        // ������ת���� Qt::PenStyle
        line->setLineStyle(static_cast<Qt::PenStyle>(styleInt));
    }
    // <<<--- ���ؼ�ͷ״̬ ---
    
    line->m_hasArrowHead = obj.value("hasArrow").toBool(true);


    // ������� 
    if (obj.contains("startShapeId") && obj["startShapeId"].isString() && obj.contains("startIndex") && obj["startIndex"].isDouble()) {
       
        QString startId = obj["startShapeId"].toString();
        int startIndex = obj["startIndex"].toInt(-1);
        Shape* startShape = shapeIdMap.value(startId, nullptr);
        if (startShape && startIndex != -1) {
            if (startIndex >= 0 && startIndex < startShape->getConnectionPointCount()) {
                line->attachStart(startShape, startIndex);
            }
            else {
                qWarning() << "fromJson: Invalid startIndex" << startIndex << "for shape" << startId;
                
                if (obj.contains("startPosX") && obj.contains("startPosY")) {
                    line->setFreeStartPoint(QPointF(obj["startPosX"].toDouble(), obj["startPosY"].toDouble()));
                }
                else {
                    line->setFreeStartPoint(startShape->getRect().center()); 
                }
            }
        }
        else {
            qWarning() << "fromJson: Start shape with ID" << startId << "not found or index invalid.";
            if (obj.contains("startPosX") && obj.contains("startPosY")) {
                line->setFreeStartPoint(QPointF(obj["startPosX"].toDouble(), obj["startPosY"].toDouble()));
            }
            else {
                qWarning() << "fromJson: Missing free start point coordinates for detached start.";
                delete line; return nullptr;
            }
        }
    }
    else if (obj.contains("startPosX") && obj.contains("startPosY")) {
        line->setFreeStartPoint(QPointF(obj["startPosX"].toDouble(), obj["startPosY"].toDouble()));
    }
    else {
        qWarning() << "fromJson: Invalid or missing start point data.";
        delete line; return nullptr;
    }

    // �����յ� 
    if (obj.contains("endShapeId") && obj["endShapeId"].isString() && obj.contains("endIndex") && obj["endIndex"].isDouble()) {
       
        QString endId = obj["endShapeId"].toString();
        int endIndex = obj["endIndex"].toInt(-1);
        Shape* endShape = shapeIdMap.value(endId, nullptr);
        if (endShape && endIndex != -1) {
            if (endIndex >= 0 && endIndex < endShape->getConnectionPointCount()) {
                line->attachEnd(endShape, endIndex);
            }
            else {
                qWarning() << "fromJson: Invalid endIndex" << endIndex << "for shape" << endId;
                if (obj.contains("endPosX") && obj.contains("endPosY")) {
                    line->setFreeEndPoint(QPointF(obj["endPosX"].toDouble(), obj["endPosY"].toDouble()));
                }
                else {
                    line->setFreeEndPoint(endShape->getRect().center());
                }
            }
        }
        else {
            qWarning() << "fromJson: End shape with ID" << endId << "not found or index invalid.";
            if (obj.contains("endPosX") && obj.contains("endPosY")) {
                line->setFreeEndPoint(QPointF(obj["endPosX"].toDouble(), obj["endPosY"].toDouble()));
            }
            else {
                qWarning() << "fromJson: Missing free end point coordinates for detached end.";

            }
        }
    }
    else if (obj.contains("endPosX") && obj.contains("endPosY")) {
        line->setFreeEndPoint(QPointF(obj["endPosX"].toDouble(), obj["endPosY"].toDouble()));
    }
    else {
        qWarning() << "fromJson: Invalid or missing end point data.";

    }

    // ���ռ�� 
    if (!line->isStartAttached() && line->m_freeStartPoint.isNull() &&
        !line->isEndAttached() && line->m_freeEndPoint.isNull())
    {
        qWarning() << "fromJson: Line loaded with no valid start or end point.";
        delete line;
        return nullptr;
    }

    line->updatePen();

    return line;
}