#ifndef PRODUCT_H
#define PRODUCT_H

#include <QObject>
#include <QString>

class Product : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(uint value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(uint count READ count WRITE setCount NOTIFY countChanged)
    
public:
    Product(QObject *parent = 0) : QObject(parent) {}
    Product(const Product &);
    QString name() const;
    QString icon() const;
    uint value() const;
    uint count() const;
    
public Q_SLOTS:
    void setName(const QString &name);
    void setIcon(const QString &icon);
    void setValue(uint value);
    void setCount(int count);
    void decreaseCount();
    
signals:
    void iconChanged();
    void nameChanged();
    void valueChanged();
    void countChanged();
    
private:
    QString m_icon;
    QString m_name {"none"};
    uint m_value {0};
    int m_count {0};
};

#endif // PRODUCT_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
