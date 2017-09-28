#include "product.h"

QString Product::name() const
{
    return m_name;
}

void Product::setName(const QString &name)
{
    m_name = name;
    emit nameChanged();
}

QString Product::icon() const
{
    return m_icon;
}

void Product::setIcon(const QString &icon)
{
    m_icon = icon;
    emit iconChanged();
}

uint Product::value() const
{
    return m_value;
}

void Product::setValue(uint value)
{
    m_value = value;
    emit valueChanged();
}

uint Product::count() const
{
    return m_count;
}

void Product::setCount(int count)
{
    m_count = count;
    emit countChanged();
}

void Product::decreaseCount()
{
    m_count--;
    emit countChanged();
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
