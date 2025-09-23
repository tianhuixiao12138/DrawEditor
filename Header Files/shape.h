#include <QRect>
#include <QColor>
#include <QPainter>
#include <QPoint>
#include <QPolygon>
#include <QVector>
#include <cmath>
#include <QDebug>
#include <QUuid>
#include <QJsonObject>
#include <QFont>        
#include <QTextOption> 

// --- 连接点命名空间 ---
namespace ConnectionPoint {
    const int Top = 0;
    const int Right = 1;
    const int Bottom = 2;
    const int Left = 3;
    const int Count = 4;
}

// --- 图形基类 ---
class Shape {
protected:
    QRect m_rect;            
    QColor m_borderColor;    
    QColor m_fillColor;      
    qreal m_borderWidth;     
    bool m_isFilled;         
    Qt::PenStyle m_borderStyle; 
    QString m_id;            

public:
    // --- 构造函数 (更新) ---
    Shape() : m_borderColor(Qt::black), m_fillColor(Qt::white), m_borderWidth(1.5), m_isFilled(false), m_borderStyle(Qt::SolidLine) { // 默认不填充，白色填充色，1.5边框宽
        m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    // 主要构造函数，允许设置所有基本属性
    Shape(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,
        Qt::PenStyle borderStyle = Qt::SolidLine) // <--- 添加 borderStyle 参数
        : m_rect(r.normalized()), m_borderColor(borderColor), m_fillColor(fillColor),
        m_borderWidth(borderWidth), m_isFilled(isFilled), m_borderStyle(borderStyle) // <--- 初始化 borderStyle
    {
        m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    virtual ~Shape() {} // 虚析构函数

    // --- 纯虚函数 (必须由派生类实现) ---
    virtual void draw(QPainter& painter) const = 0; // 绘制逻辑
    virtual bool isSelected(const QPoint& point) const = 0; // 选中点测试逻辑

    // --- Getters (更新) ---
    QRect getRect() const { return m_rect; }
    QColor getBorderColor() const { return m_borderColor; } // 重命名
    
    Qt::PenStyle getBorderStyle() const { return m_borderStyle; }// <--- 新增 Getter
    QColor getFillColor() const { return m_fillColor; }     // <--- 新增
    qreal getBorderWidth() const { return m_borderWidth; }   // <--- 新增
    bool isFilled() const { return m_isFilled; }            // <--- 新增
    QString getId() const { return m_id; }

    // --- Setters (更新) ---
    virtual void setRect(const QRect& newRect) { m_rect = newRect.normalized(); }
    void setBorderColor(const QColor& newColor) { m_borderColor = newColor; } // 重命名
    void setFillColor(const QColor& newColor) { m_fillColor = newColor; }     // <--- 新增
    void setBorderWidth(qreal newWidth) { m_borderWidth = qMax(0.1, newWidth); } // <--- 新增 (加最小宽度限制)
    void setBorderStyle(Qt::PenStyle style) {m_borderStyle = style;}  // <--- 新增 Setter
    void setFilled(bool filled) { m_isFilled = filled; }                      // <--- 新增
    void setId(const QString& newId) { m_id = newId; }

    // --- 公共操作 ---
    virtual void move(const QPoint& offset) {
        m_rect.translate(offset);
    }

    // --- 连接点相关方法 (基本不变，但使用 m_rect) ---
    virtual QPoint getConnectionPoint(int index) const {
        switch (index) {
        case ConnectionPoint::Top:    return QPoint(m_rect.center().x(), m_rect.top());
        case ConnectionPoint::Right:  return QPoint(m_rect.right(), m_rect.center().y());
        case ConnectionPoint::Bottom: return QPoint(m_rect.center().x(), m_rect.bottom());
        case ConnectionPoint::Left:   return QPoint(m_rect.left(), m_rect.center().y());
        default:                      qWarning() << "Shape::getConnectionPoint - 无效索引:" << index; return m_rect.center();
        }
    }

    virtual int getConnectionPointCount() const {
        return ConnectionPoint::Count;
    }

    // findClosestConnectionPoint (不变，内部已使用 getConnectionPoint)
    virtual bool findClosestConnectionPoint(const QPoint& globalPos, int& closestIndex, double& minDistanceSq, double maxDistanceThreshold = 10.0) const {
        closestIndex = -1;
        minDistanceSq = maxDistanceThreshold * maxDistanceThreshold + 1.0;
        bool found = false;
        for (int i = 0; i < getConnectionPointCount(); ++i) {
            QPoint cp = getConnectionPoint(i);
            double dx = globalPos.x() - cp.x();
            double dy = globalPos.y() - cp.y();
            double distSq = dx * dx + dy * dy;
            if (distSq < minDistanceSq && distSq <= maxDistanceThreshold * maxDistanceThreshold) {
                minDistanceSq = distSq;
                closestIndex = i;
                found = true;
            }
        }
        return found;
    }

    // drawConnectionPoints (不变，内部已使用 getConnectionPoint)
    virtual void drawConnectionPoints(QPainter& painter) const {
        painter.save();
        painter.setBrush(QColor(0, 200, 0, 180));
        painter.setPen(QPen(Qt::darkGreen, 1));
        const int radius = 4; // 调小一点半径
        for (int i = 0; i < getConnectionPointCount(); ++i) {
            painter.drawEllipse(getConnectionPoint(i), radius, radius);
        }
        painter.restore();
    }

    // --- 序列化 (更新，添加新属性) ---
    virtual QJsonObject toJson() const {
        QJsonObject obj;
        // 类型由外部添加或在派生类 toJson 中添加
        obj["id"] = m_id;
        obj["x"] = m_rect.x();
        obj["y"] = m_rect.y();
        obj["width"] = m_rect.width();
        obj["height"] = m_rect.height();
        obj["borderColor"] = m_borderColor.name(QColor::HexRgb); // 保存边框颜色 (重命名key)
        obj["borderStyle"] = static_cast<int>(m_borderStyle); // <--- 保存 borderStyle 为整数
        obj["borderWidth"] = m_borderWidth;                      // <--- 新增
        obj["isFilled"] = m_isFilled;                            // <--- 新增
        obj["fillColor"] = m_fillColor.name(QColor::HexRgb);     // <--- 新增
        return obj;
    }

    // --- （反序列化辅助方法 ---
    // 通常在 DiagramEditor::onOpen 中处理，但可以在这里提供一个辅助函数
    virtual void loadFromJson(const QJsonObject& obj) {
        // 基类加载通用属性
        m_id = obj.value("id").toString(QUuid::createUuid().toString(QUuid::WithoutBraces)); 
        int x = obj.value("x").toInt(0);
        int y = obj.value("y").toInt(0);
        int w = obj.value("width").toInt(10);
        int h = obj.value("height").toInt(10);
        m_rect = QRect(x, y, w, h).normalized();

        QColor loadedBorderColor(obj.value("borderColor").toString("black"));
        m_borderColor = loadedBorderColor.isValid() ? loadedBorderColor : Qt::black;

        m_borderWidth = obj.value("borderWidth").toDouble(1.5);
        if (m_borderWidth < 0.1) m_borderWidth = 0.1; // 检查最小值
        m_borderStyle = static_cast<Qt::PenStyle>(obj.value("borderStyle").toInt(Qt::SolidLine));

        m_isFilled = obj.value("isFilled").toBool(false);

        QColor loadedFillColor(obj.value("fillColor").toString("white"));
        m_fillColor = loadedFillColor.isValid() ? loadedFillColor : Qt::white;
    }
};

// --- 派生类：矩形 (Rectangle) ---
class Rectangle : public Shape {
public:
    // 构造函数调用基类新构造函数
    Rectangle(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,
        Qt::PenStyle borderStyle = Qt::SolidLine)
        : Shape(r, borderColor, borderWidth, isFilled, fillColor) {}

    // draw 实现更新，使用新属性
    void draw(QPainter& painter) const override {
        painter.save();
        // 设置画笔 (边框)
        QPen pen(m_borderColor);
       // pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // 无边框
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- 应用边框样式
        }


        painter.setPen(pen);

        // 设置画刷 (填充)
        if (m_isFilled) {
            painter.setBrush(m_fillColor);
        }
        else {
            painter.setBrush(Qt::NoBrush);
        }

        // 绘制矩形
        painter.drawRect(m_rect);
        painter.restore();
    }

    // isSelected (不变)
    bool isSelected(const QPoint& point) const override {
        // 稍微增大选中范围，考虑线宽
        qreal expand = qMax(3.0, m_borderWidth / 2.0 + 1.0);
        return m_rect.adjusted(-expand, -expand, expand, expand).contains(point);
    }
};

// --- 派生类：圆形 (Circle / Ellipse) ---
class Circle : public Shape {
public:
    // 构造函数调用基类新构造函数
    Circle(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,Qt::PenStyle borderStyle = Qt::SolidLine)
        : Shape(r, borderColor, borderWidth, isFilled, fillColor) {}

    // draw 实现更新，使用新属性
    void draw(QPainter& painter) const override {
        painter.save();
        QPen pen(m_borderColor);
        //pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // 无边框
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- 应用边框样式
        }


        painter.setPen(pen);
        
        if (m_isFilled) {
            painter.setBrush(m_fillColor);
        }
        else {
            painter.setBrush(Qt::NoBrush);
        }

        painter.drawEllipse(m_rect); // 绘制椭圆
        painter.restore();
    }

    // isSelected (不变)
    bool isSelected(const QPoint& point) const override {
        QPoint center = m_rect.center();
        double a = m_rect.width() / 2.0;
        double b = m_rect.height() / 2.0;

        if (a < 1e-6 || b < 1e-6) {
            return m_rect.contains(point);
        }

        double dx = point.x() - center.x();
        double dy = point.y() - center.y();
        // 增加一点容差，考虑线宽
        double borderFactor = 1.0 + qMax(0.05, m_borderWidth / qMin(a, b));
        return (dx * dx / (a * a) + dy * dy / (b * b)) <= borderFactor * borderFactor;
    }
};

// --- 派生类：三角形 (Triangle) ---
class Triangle : public Shape {
public:
    // 构造函数调用基类新构造函数
    Triangle(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,
        Qt::PenStyle borderStyle = Qt::SolidLine)
        : Shape(r, borderColor, borderWidth, isFilled, fillColor) {}

    // getPolygon (不变, 使用 m_rect)
    QPolygon getPolygon() const {
        QPolygon trianglePoly;
        trianglePoly << QPoint(m_rect.center().x(), m_rect.top())
            << m_rect.bottomLeft()
            << m_rect.bottomRight();
        return trianglePoly;
    }

    // draw 实现更新
    void draw(QPainter& painter) const override {
        painter.save();
        QPen pen(m_borderColor);
        //pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // 无边框
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- 应用边框样式
        }


        painter.setPen(pen);
       

        if (m_isFilled) {
            painter.setBrush(m_fillColor);
        }
        else {
            painter.setBrush(Qt::NoBrush);
        }

        painter.drawPolygon(getPolygon()); // 绘制多边形
        painter.restore();
    }

    // isSelected (不变)
    bool isSelected(const QPoint& point) const override {
        // 可以稍微扩大 Polygon 用于判断，或者使用到边界的距离
        return getPolygon().containsPoint(point, Qt::OddEvenFill);
        // 更精确的带边框判断会复杂
    }

    // getConnectionPoint (不变, 使用 getPolygon)
    QPoint getConnectionPoint(int index) const override {
        QPolygon poly = getPolygon();
        if (poly.size() < 3) return m_rect.center();
        switch (index) {
        case ConnectionPoint::Top:    return poly.at(0);
        case ConnectionPoint::Right:  return QPoint((poly.at(0).x() + poly.at(2).x()) / 2, (poly.at(0).y() + poly.at(2).y()) / 2);
        case ConnectionPoint::Bottom: return QPoint((poly.at(1).x() + poly.at(2).x()) / 2, poly.at(1).y());
        case ConnectionPoint::Left:   return QPoint((poly.at(0).x() + poly.at(1).x()) / 2, (poly.at(0).y() + poly.at(1).y()) / 2);
        default: return m_rect.center();
        }
    }
};

// --- 派生类：菱形 (Diamond) ---
class Diamond : public Shape {
public:
    // 构造函数调用基类新构造函数
    Diamond(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,
        Qt::PenStyle borderStyle = Qt::SolidLine)
        : Shape(r, borderColor, borderWidth, isFilled, fillColor) {}

    // getPolygon (不变, 使用 m_rect)
    QPolygon getPolygon() const {
        QPolygon diamondPoly;
        diamondPoly << QPoint(m_rect.center().x(), m_rect.top())
            << QPoint(m_rect.right(), m_rect.center().y())
            << QPoint(m_rect.center().x(), m_rect.bottom())
            << QPoint(m_rect.left(), m_rect.center().y());
        return diamondPoly;
    }

    // draw 实现更新
    void draw(QPainter& painter) const override {
        painter.save();
        QPen pen(m_borderColor);
        //pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // 无边框
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- 应用边框样式
        }


        painter.setPen(pen);

        if (m_isFilled) {
            painter.setBrush(m_fillColor);
        }
        else {
            painter.setBrush(Qt::NoBrush);
        }

        painter.drawPolygon(getPolygon()); // 绘制菱形多边形
        painter.restore();
    }

    // isSelected (不变)
    bool isSelected(const QPoint& point) const override {
        return getPolygon().containsPoint(point, Qt::OddEvenFill);
    }

    // getConnectionPoint (使用基类默认或重写为顶点)
    // 这里我们重写为顶点
    QPoint getConnectionPoint(int index) const override {
        QPolygon poly = getPolygon();
        if (poly.size() < 4) return m_rect.center();
        switch (index) {
        case ConnectionPoint::Top:    return poly.at(0); // 上顶点
        case ConnectionPoint::Right:  return poly.at(1); // 右顶点
        case ConnectionPoint::Bottom: return poly.at(2); // 下顶点
        case ConnectionPoint::Left:   return poly.at(3); // 左顶点
        default: return m_rect.center();
        }
    }
};

// --- 派生类：圆角矩形 (RoundedRectangle) ---
class RoundedRectangle : public Shape {
private:
    int cornerRadiusX;
    int cornerRadiusY;

public:
    // 构造函数更新
    RoundedRectangle(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,
        Qt::PenStyle borderStyle = Qt::SolidLine, // <--- 添加样式
        int radius = 15) // 圆角半径参数保留
        : Shape(r, borderColor, borderWidth, isFilled, fillColor)
    {
        setCornerRadius(radius); // 初始化圆角
    }

    // setCornerRadius (不变, 使用 m_rect)
    void setCornerRadius(int radius) {
        cornerRadiusX = qMin(radius, m_rect.width() / 2);
        cornerRadiusY = qMin(radius, m_rect.height() / 2);
        if (cornerRadiusX < 0) cornerRadiusX = 0;
        if (cornerRadiusY < 0) cornerRadiusY = 0;
    }
    int getCornerRadius() const { return qMax(cornerRadiusX, cornerRadiusY); } // Getter for radius

    // 重写 setRect 更新圆角限制 (不变)
    void setRect(const QRect& newRect) override {
        Shape::setRect(newRect);
        setCornerRadius(qMax(cornerRadiusX, cornerRadiusY));
    }



    // draw 实现更新
    void draw(QPainter& painter) const override {
        painter.save();
        QPen pen(m_borderColor);
        //pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // 无边框
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- 应用边框样式
        }
        painter.setPen(pen);


        if (m_isFilled) {
            painter.setBrush(m_fillColor);
        }
        else {
            painter.setBrush(Qt::NoBrush);
        }

        painter.drawRoundedRect(m_rect, cornerRadiusX, cornerRadiusY);
        painter.restore();
    }

    // isSelected (不变)
    bool isSelected(const QPoint& point) const override {
        qreal expand = qMax(3.0, m_borderWidth / 2.0 + 1.0);
        return m_rect.adjusted(-expand, -expand, expand, expand).contains(point);
        // 更精确的判断会复杂
    }

    // --- 序列化/反序列化 (可选增强，保存 radius) ---
    QJsonObject toJson() const override {
        QJsonObject obj = Shape::toJson(); // 调用基类方法
        obj["type"] = "rounded_rectangle"; // 添加类型
        obj["cornerRadius"] = qMax(cornerRadiusX, cornerRadiusY); // 保存圆角半径
        return obj;
    }
    void loadFromJson(const QJsonObject& obj) override {
        Shape::loadFromJson(obj); // 加载基类属性
        setCornerRadius(obj.value("cornerRadius").toInt(15)); // 加载圆角半径
    }
};

// --- 派生类：平行四边形 (Parallelogram) ---
class Parallelogram : public Shape {
private:
    int xOffset;

public:
    // 构造函数更新
    Parallelogram(const QRect& r,
        const QColor& borderColor = Qt::black,
        qreal borderWidth = 1.5,
        bool isFilled = false,
        const QColor& fillColor = Qt::white,
        Qt::PenStyle borderStyle = Qt::SolidLine, // <--- 添加样式
        int offset = 20) // 偏移量参数保留
        : Shape(r, borderColor, borderWidth, isFilled, fillColor)
    {
        setXOffset(offset);
    }

    // setXOffset (不变, 使用 m_rect)
    void setXOffset(int offset) {
        xOffset = qBound(-m_rect.width() / 2 + 1, offset, m_rect.width() / 2 - 1);
    }
    int getXOffset() const { return xOffset; } // Getter for offset

    // 重写 setRect 更新偏移量限制 (不变)
    void setRect(const QRect& newRect) override {
        Shape::setRect(newRect);
        setXOffset(xOffset);
    }

    // getPolygon (不变, 使用 m_rect 和 xOffset)
    QPolygon getPolygon() const {
        QPolygon parallelogramPoly;
        parallelogramPoly << QPoint(m_rect.left() + xOffset, m_rect.top())
            << QPoint(m_rect.right() + xOffset, m_rect.top()) // 修正：右上角也应偏移
            << QPoint(m_rect.right() - xOffset, m_rect.bottom())
            << QPoint(m_rect.left() - xOffset, m_rect.bottom()); // 修正：左下角也应偏移
        // 修正：上面的getPolygon实现更像梯形，平行四边形应该是对边平行
        // 重新实现平行四边形的顶点计算
        parallelogramPoly.clear();
        int effectiveOffset = xOffset; // 可以基于 m_rect.height() 来计算？ 暂用 xOffset
        parallelogramPoly << m_rect.topLeft() + QPoint(effectiveOffset, 0) // 左上角向右偏
            << m_rect.topRight() // 右上角不变 (或者也偏移？取决于定义)
            << m_rect.bottomRight() - QPoint(effectiveOffset, 0) // 右下角向左偏
            << m_rect.bottomLeft(); // 左下角不变
// 另一种定义：顶部向右偏，底部向右偏
// parallelogramPoly.clear();
// parallelogramPoly << m_rect.topLeft() + QPoint(xOffset, 0)
//                   << m_rect.topRight() + QPoint(xOffset, 0)
//                   << m_rect.bottomRight()
//                   << m_rect.bottomLeft();

        return parallelogramPoly;
    }

    // draw 实现更新
    void draw(QPainter& painter) const override {
        painter.save();
        QPen pen(m_borderColor);
        //pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // 无边框
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- 应用边框样式
        }
        painter.setPen(pen);

        if (m_isFilled) {
            painter.setBrush(m_fillColor);
        }
        else {
            painter.setBrush(Qt::NoBrush);
        }

        painter.drawPolygon(getPolygon()); // 绘制平行四边形
        painter.restore();
    }

    // isSelected (不变)
    bool isSelected(const QPoint& point) const override {
        return getPolygon().containsPoint(point, Qt::OddEvenFill);
    }

    // getConnectionPoint (使用修正后的 getPolygon)
    QPoint getConnectionPoint(int index) const override {
        QPolygon poly = getPolygon();
        if (poly.size() < 4) return m_rect.center();
        switch (index) {
        case ConnectionPoint::Top:    return QPoint((poly.at(0).x() + poly.at(1).x()) / 2, poly.at(0).y()); // 上边中点
        case ConnectionPoint::Right:  return QPoint((poly.at(1).x() + poly.at(2).x()) / 2, (poly.at(1).y() + poly.at(2).y()) / 2); // 右边中点
        case ConnectionPoint::Bottom: return QPoint((poly.at(2).x() + poly.at(3).x()) / 2, poly.at(2).y()); // 下边中点
        case ConnectionPoint::Left:   return QPoint((poly.at(3).x() + poly.at(0).x()) / 2, (poly.at(3).y() + poly.at(0).y()) / 2); // 左边中点
        default: return m_rect.center();
        }
    }
    // --- 序列化/反序列化 (可选增强，保存 xOffset) ---
    QJsonObject toJson() const override {
        QJsonObject obj = Shape::toJson();
        obj["type"] = "parallelogram";
        obj["xOffset"] = xOffset; // 保存偏移量
        return obj;
    }
    void loadFromJson(const QJsonObject& obj) override {
        Shape::loadFromJson(obj);
        setXOffset(obj.value("xOffset").toInt(20)); // 加载偏移量
    }
};


// --- 派生类：文本框 (TextBox) ---
class TextBox : public Shape {
private:
    QString m_text;
    QFont   m_font;
    QColor  m_textColor;
    QTextOption m_textOption; // 包含对齐、换行
public:
    // 构造函数更新，包含基类属性
    TextBox(const QRect& r,
        const QColor& borderColor = Qt::transparent, // 默认边框色
        qreal borderWidth = 0.0,                 // 默认边框宽度
        bool isFilled = false,                   // 默认不填充
        const QColor& fillColor = Qt::white,    // 默认填充色
        Qt::PenStyle borderStyle = Qt::SolidLine,
        const QString& text = QObject::tr("Double-click to edit"),
        const QFont& font = QFont("Arial", 10), // 默认字体
        const QColor& textColor = Qt::black,     // 默认文本颜色
        Qt::Alignment alignment = Qt::AlignCenter) // 默认对齐
        : Shape(r, borderColor, borderWidth, isFilled, fillColor), // 调用基类构造
        m_text(text), m_font(font), m_textColor(textColor)
    {
        m_textOption.setAlignment(alignment);
        m_textOption.setWrapMode(QTextOption::WordWrap); // 默认自动换行
    }

    // --- 文本相关 Getters/Setters (添加/确认存在) ---
    void setText(const QString& text) { m_text = text; }
    QString getText() const { return m_text; }
    void setFont(const QFont& font) { m_font = font; }
    QFont getFont() const { return m_font; }
    void setTextColor(const QColor& color) { m_textColor = color; }
    QColor getTextColor() const { return m_textColor; }
    void setTextAlignment(Qt::Alignment alignment) { m_textOption.setAlignment(alignment); }
    Qt::Alignment getTextAlignment() const { return m_textOption.alignment(); }

    // --- 重写 draw (使用新属性绘制边框和背景) ---
    void draw(QPainter& painter) const override {
        painter.save();
        // 1. 绘制边框和背景 (使用基类属性)
        QPen pen(m_borderColor);
        //pen.setWidthF(m_borderWidth);
        if (qFuzzyCompare(m_borderWidth, 0)) {
            pen.setStyle(Qt::NoPen); // 无边框
        }
        else {
            pen.setWidthF(m_borderWidth);
            pen.setStyle(m_borderStyle); // <--- 应用边框样式
        }
        painter.setPen(pen);
        if (m_isFilled) {
            painter.setBrush(m_fillColor); // 使用填充色
        }
        else {
            painter.setBrush(Qt::NoBrush);  // 透明背景
        }
        // 绘制矩形作为背景和边框
        painter.drawRect(m_rect);
        // 2. 绘制文本 (使用文本属性)
        painter.setPen(m_textColor);   // 设置文本颜色
        painter.setFont(m_font);       // 设置字体
        // 在矩形内部绘制文本，留出边距（边距可以考虑线宽）
        int margin = qMax(2, int(m_borderWidth / 2.0) + 1);
        QRect textRect = m_rect.adjusted(margin, margin, -margin, -margin);
        if (textRect.isValid()) {
            painter.drawText(textRect, m_text, m_textOption);
        }
        painter.restore();
    }

    // isSelected (不变)
    bool isSelected(const QPoint& point) const override {
        qreal expand = qMax(2.0, m_borderWidth / 2.0); // 基础选中范围
        return m_rect.adjusted(-expand, -expand, expand, expand).contains(point);
    }

    // --- 序列化 (调用基类 toJson 并添加文本属性) ---
    QJsonObject toJson() const override {
        QJsonObject obj = Shape::toJson(); // 获取基类属性(ID, rect, border, fill)
        obj["type"] = "textbox";           // 类型标识
        obj["text"] = m_text;
        obj["textColor"] = m_textColor.name(QColor::HexRgb);
        obj["fontFamily"] = m_font.family();
        obj["fontSize"] = m_font.pointSize();
        obj["fontBold"] = m_font.bold();
        obj["fontItalic"] = m_font.italic();
        obj["textAlignment"] = static_cast<int>(m_textOption.alignment()); // 保存对齐方式
        // 可以添加更多字体属性...
        return obj;
    }

    // --- 反序列化 (调用基类 loadFromJson 并加载文本属性) ---
    void loadFromJson(const QJsonObject& obj) override {
        Shape::loadFromJson(obj); // 加载基类属性
        m_text = obj.value("text").toString(QObject::tr("Text"));
        QColor loadedTextColor(obj.value("textColor").toString("black"));
        m_textColor = loadedTextColor.isValid() ? loadedTextColor : Qt::black;
        // 加载字体
        QString family = obj.value("fontFamily").toString("Arial");
        int size = obj.value("fontSize").toInt(10);
        bool bold = obj.value("fontBold").toBool(false);
        bool italic = obj.value("fontItalic").toBool(false);
        m_font = QFont(family, size, bold ? QFont::Bold : QFont::Normal, italic);
        // 加载对齐
        int alignInt = obj.value("textAlignment").toInt(static_cast<int>(Qt::AlignCenter));
        m_textOption.setAlignment(static_cast<Qt::Alignment>(alignInt));
    }
};