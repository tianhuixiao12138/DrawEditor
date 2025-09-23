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

// --- 定义并初始化静态成员 ---
const qreal ConnectionLine::handleSize = 8.0;
const qreal ConnectionLine::selectionPadding = 2.5;
// baseWidth 被移除

// --- 构造函数 (完全连接) ---
ConnectionLine::ConnectionLine(Shape* startShape, int startPointIndex, Shape* endShape, int endPointIndex,
    QColor color, qreal lineWidth, Qt::PenStyle lineStyle, bool hasArrow) 
    : m_color(color), m_lineWidth(lineWidth), m_lineStyle(lineStyle), 
    m_selected(false),
    m_startShape(startShape), m_startPointIndex(startPointIndex), m_freeStartPoint(), // 初始自由点无效
    m_endShape(endShape), m_endPointIndex(endPointIndex), m_freeEndPoint(),        // 初始自由点无效
    m_hasArrowHead(hasArrow) // <<<--- 初始化 m_hasArrowHead
{
    Q_ASSERT(m_startShape != nullptr && m_startPointIndex >= 0);
    Q_ASSERT(m_endShape != nullptr && m_endPointIndex >= 0);
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces); // <--- 初始化 ID
    updatePen(); // <--- 更新画笔
}

// --- 构造函数 (完全自由) ---
ConnectionLine::ConnectionLine(const QPointF& p1, const QPointF& p2,
    QColor color, qreal lineWidth, Qt::PenStyle lineStyle, bool hasArrow) // <--- 添加新参数
    : m_color(color), m_lineWidth(lineWidth), m_lineStyle(lineStyle), // <--- 初始化新成员
    m_selected(false),
    m_startShape(nullptr), m_startPointIndex(-1), m_freeStartPoint(p1),
    m_endShape(nullptr), m_endPointIndex(-1), m_freeEndPoint(p2),
    m_hasArrowHead(hasArrow) // <<<--- 初始化 m_hasArrowHead
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces); // <--- 初始化 ID
    updatePen(); // <--- 更新画笔
}

// --- 默认构造函数 (用于加载) ---
ConnectionLine::ConnectionLine()
    : m_color(Qt::darkGray), m_lineWidth(1.5), m_lineStyle(Qt::SolidLine), // <--- 设置默认值
    m_selected(false),
    m_startShape(nullptr), m_startPointIndex(-1), m_freeStartPoint(),
    m_endShape(nullptr), m_endPointIndex(-1), m_freeEndPoint()
{
    updatePen(); // <--- 更新画笔
}

// --- 更新画笔 ---
void ConnectionLine::updatePen() {
    m_pen.setColor(m_color);
    m_pen.setWidthF(m_lineWidth);       // <--- 使用成员变量线宽
    m_pen.setStyle(m_lineStyle);      // <--- 使用成员变量线型
    m_pen.setJoinStyle(Qt::RoundJoin);
    m_pen.setCapStyle(Qt::RoundCap);
}
// --- 新增 Getter/Setter 实现 ---
bool ConnectionLine::hasArrowHead() const {
    return m_hasArrowHead;
}

void ConnectionLine::setHasArrowHead(bool hasArrow) {
    if (m_hasArrowHead != hasArrow) {
        m_hasArrowHead = hasArrow;
        
    }
}


// --- 获取实际端点坐标 (不变) ---
QPointF ConnectionLine::getStartPointPos() const {
    if (isStartAttached()) {
        if (m_startPointIndex >= 0 && m_startShape && m_startPointIndex < m_startShape->getConnectionPointCount()) {
            return m_startShape->getConnectionPoint(m_startPointIndex);
        }
        else {
            qWarning() << "ConnectionLine: Invalid start point index or shape. Using center.";
            return m_startShape ? m_startShape->getRect().center() : m_freeStartPoint; // 如果Shape也无效，返回自由点
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
            return m_endShape ? m_endShape->getRect().center() : m_freeEndPoint; // 如果Shape也无效，返回自由点
        }
    }
    else {
        return m_freeEndPoint;
    }
}

// --- 设置自由端点坐标 (不变) ---
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

// --- 连接/断开端点 (不变) ---
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

// --- 绘制 (修改选中时的线宽计算) ---
//void ConnectionLine::draw(QPainter& painter) const {
//    // ... (开头的有效性检查不变) ...
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
//    // 绘制选中高亮背景
//    if (m_selected) {
//        QPen selectionPen;
//        selectionPen.setColor(QColor(0, 100, 255, 120)); // 半透明蓝色
//        // <--- 修改：背景线宽基于当前线宽 ---
//        selectionPen.setWidthF(m_lineWidth + selectionPadding * 2.0);
//        selectionPen.setStyle(m_lineStyle); // <--- 背景线也使用当前线型
//        selectionPen.setCapStyle(Qt::RoundCap);
//        selectionPen.setJoinStyle(Qt::RoundJoin);
//        painter.setPen(selectionPen);
//        painter.drawLine(startPoint, endPoint); // 绘制背景线
//    }
//
//    // 绘制连接线本身
//    painter.setPen(m_pen);              // 使用缓存的画笔 (已包含颜色、宽度、样式)
//    painter.setBrush(m_pen.color());    // 箭头填充用线的颜色
//    painter.drawLine(startPoint, endPoint);
//
//    // 绘制箭头
//    // *** 修改：根据 m_hasArrowHead 决定是否绘制箭头 ***
//    if (m_hasArrowHead) {
//        drawArrowHead(painter, startPoint, endPoint);
//    }
//    // *** 结束修改 ***
//
//    // 如果选中，为 *自由* 端点绘制控制手柄
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
    // ... (有效性检查) ...
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

    // --- 1. 计算箭头尺寸 ---
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

    // --- 2. 计算缩短后的线段终点 lineEndPoint ---
    QPointF lineEndPoint = endPoint; // 默认是原始终点
    qreal pullbackDistance = 0.0;

    if (m_hasArrowHead && actualArrowLength > 0) {
        pullbackDistance = actualArrowLength * std::cos(baseArrowAngleRad);

      
        if (mainLineLength > pullbackDistance + 0.1) { // 留一点点余量
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

    // --- 3. 绘制选中高亮背景 ---
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

    // --- 4. 绘制连接线本身  ---
    QPen linePen = m_pen; 
    linePen.setCapStyle(Qt::FlatCap); 
    painter.setPen(linePen);
    painter.drawLine(startPoint, lineEndPoint);

    // --- 5. 绘制箭头  ---
    if (m_hasArrowHead) {
        drawArrowHead(painter, startPoint, endPoint);
    }

    // --- 6. 绘制控制手柄  ---
    if (m_selected) {
        if (!isStartAttached()) {
            drawHandle(painter, startPoint);
        }
        if (!isEndAttached()) {
            drawHandle(painter, endPoint); // 手柄在原始终点
        }
    }

    painter.restore();
}

// --- 绘制控制手柄  ---
void ConnectionLine::drawHandle(QPainter& painter, const QPointF& pos) const {
    painter.save();
    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    qreal hs = handleSize;
    painter.drawRect(QRectF(pos.x() - hs / 2.0, pos.y() - hs / 2.0, hs, hs));
    painter.restore();
}

// --- 绘制箭头  ---
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

// --- 选中检测  ---
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

// --- 移动 ---
void ConnectionLine::move(const QPointF& offset) {
    if (!isStartAttached()) {
        m_freeStartPoint += offset;
    }
    if (!isEndAttached()) {
        m_freeEndPoint += offset;
    }
}

// --- 设置颜色  ---
void ConnectionLine::setColor(const QColor& color) {
    if (m_color != color) {
        m_color = color;
        updatePen();
    }
}

// --- 设置线宽 ---
void ConnectionLine::setLineWidth(qreal width) {
    qreal w = qMax(0.5, width); // 最小线宽设为 0.5
    if (!qFuzzyCompare(m_lineWidth, w)) { // 避免浮点数比较问题
        m_lineWidth = w;
        updatePen(); // 更新画笔
    }
}

// --- 设置线型 ---
void ConnectionLine::setLineStyle(Qt::PenStyle style) {
    
    if (m_lineStyle != style) {
        m_lineStyle = style;
        updatePen(); // 更新画笔
    }
}


// --- 设置选中状态  ---
void ConnectionLine::setSelected(bool selected) {
    if (m_selected != selected) {
        m_selected = selected;
    }
}

// --- 获取包围盒 ---
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

// --- 保存为 JSON (添加线宽和线型) ---
QJsonObject ConnectionLine::toJson() const {
    QJsonObject obj;
    obj["id"] = m_id; 
    obj["color"] = m_color.name(QColor::HexRgb);
    obj["lineWidth"] = m_lineWidth; 
    obj["hasArrow"] = m_hasArrowHead; 

    // <--- 保存线型 ---
    obj["lineStyle"] = static_cast<int>(m_lineStyle); 
    // 保存起点信息 
    if (isStartAttached()) {
        obj["startShapeId"] = m_startShape->getId();
        obj["startIndex"] = m_startPointIndex;
    }
    else {
        obj["startPosX"] = m_freeStartPoint.x();
        obj["startPosY"] = m_freeStartPoint.y();
    }

    // 保存终点信息 
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

// --- 从 JSON 加载 (添加线宽和线型) ---
ConnectionLine* ConnectionLine::fromJson(const QJsonObject& obj, const QMap<QString, Shape*>& shapeIdMap) {
    ConnectionLine* line = new ConnectionLine(); // 创建默认对象

     // <--- 加载或生成 ID ---
    line->m_id = obj.value("id").toString(QUuid::createUuid().toString(QUuid::WithoutBraces));
    if (line->m_id.isEmpty()) { // 再次检查，防止空字符串
        line->m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        qWarning() << "fromJson: Line loaded with empty ID, assigned new one:" << line->m_id;
    }
    // <--- 结束 ID 处理 ---
    

    // 加载颜色 
    if (obj.contains("color") && obj["color"].isString()) {
        QColor color(obj["color"].toString());
        if (color.isValid()) {
            line->setColor(color); 
        }
    }

    // <---加载线宽 ---
    if (obj.contains("lineWidth") && obj["lineWidth"].isDouble()) {
        line->setLineWidth(obj["lineWidth"].toDouble(1.5)); // 提供默认值
    }

    // <---加载线型 ---
    if (obj.contains("lineStyle") && obj["lineStyle"].isDouble()) { // JSON 数字通常是 double
        int styleInt = obj["lineStyle"].toInt(static_cast<int>(Qt::SolidLine)); // 默认 SolidLine
        // 将整数转换回 Qt::PenStyle
        line->setLineStyle(static_cast<Qt::PenStyle>(styleInt));
    }
    // <<<--- 加载箭头状态 ---
    
    line->m_hasArrowHead = obj.value("hasArrow").toBool(true);


    // 加载起点 
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

    // 加载终点 
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

    // 最终检查 
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