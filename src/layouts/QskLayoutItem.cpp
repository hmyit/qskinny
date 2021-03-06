/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 * This file may be used under the terms of the QSkinny License, Version 1.0
 *****************************************************************************/

#include "QskLayoutItem.h"
#include "QskControl.h"
#include "QskLayoutConstraint.h"
#include "QskQuick.h"

QskLayoutItem::QskLayoutItem( QQuickItem* item, int row, int column, int rowSpan, int columnSpan )
    : Inherited( row, column, qMax( rowSpan, 1 ), qMax( columnSpan, 1 ), Qt::Alignment() )
    , m_item( item )
    , m_isGeometryDirty( false )
    , m_isStretchable( false )
    , m_retainSizeWhenHidden( false )
    , m_unlimitedRowSpan( rowSpan <= 0 )
    , m_unlimitedColumnSpan( columnSpan <= 0 )
    , m_updateMode( UpdateWhenVisible )
{
}

QskLayoutItem::QskLayoutItem( const QSizeF& size, int stretch, int row, int column )
    : Inherited( row, column, 1, 1, Qt::Alignment() )
    , m_item( nullptr )
    , m_spacingHint( size )
    , m_isGeometryDirty( false )
    , m_isStretchable( stretch > 0 )
    , m_retainSizeWhenHidden( false )
    , m_unlimitedRowSpan( false )
    , m_unlimitedColumnSpan( false )
    , m_updateMode( UpdateWhenVisible )
{
}

QskLayoutItem::~QskLayoutItem()
{
}

QskLayoutItem::UpdateMode QskLayoutItem::updateMode() const
{
    return m_updateMode;
}

void QskLayoutItem::setUpdateMode( UpdateMode mode )
{
    m_updateMode = mode;
}

bool QskLayoutItem::retainSizeWhenHidden() const
{
    return m_retainSizeWhenHidden;
}

void QskLayoutItem::setRetainSizeWhenHidden( bool on )
{
    m_retainSizeWhenHidden = on;
}

QSizeF QskLayoutItem::spacingHint() const
{
    return m_spacingHint;
}

void QskLayoutItem::setSpacingHint( const QSizeF& hint )
{
    m_spacingHint = hint;
}

QSizeF QskLayoutItem::sizeHint(
    Qt::SizeHint whichHint, const QSizeF& constraint ) const
{
    if ( m_item == nullptr )
    {
        if ( whichHint < Qt::MinimumSize || whichHint > Qt::MaximumSize )
            return QSizeF( 0, 0 );

        // a spacer item
        if ( whichHint == Qt::MaximumSize )
        {
            if ( m_isStretchable )
                return QSizeF( QskLayoutConstraint::unlimited, QskLayoutConstraint::unlimited );

            if ( m_spacingHint.width() < 0 )
                return QSizeF( QskLayoutConstraint::unlimited, m_spacingHint.height() );
            else
                return QSizeF( m_spacingHint.width(), QskLayoutConstraint::unlimited );
        }
        else
        {
            if ( m_spacingHint.width() < 0 )
                return QSizeF( 0, m_spacingHint.height() );
            else
                return QSizeF( m_spacingHint.width(), 0 );
        }
    }
    else
    {
        return QskLayoutConstraint::sizeHint( m_item, whichHint, constraint );
    }
}

QLayoutPolicy::Policy QskLayoutItem::sizePolicy( Qt::Orientation orientation ) const
{
    auto policy = QskLayoutConstraint::sizePolicy( m_item ).policy( orientation );

#if 0
    if ( ( policy == QskSizePolicy::Preferred ) && m_item )
    {
        // QskSizePolicy::Preferred without having a preferred size is the default
        // setting of QskControl - taken from what QWidget does - but this combination
        // doesn't make much sense. Usually every derived control is supposed
        // to set specific values, but in case it has been forgotten we better
        // ignore the preferred size then.

        const QSizeF hint = QskLayoutConstraint::effectiveConstraint( m_item, Qt::PreferredSize );

        const qreal value = ( orientation == Qt::Horizontal ) ? hint.width() : hint.height();
        if ( value <= 0 )
            policy = QskSizePolicy::Ignored;
    }
#endif

    return static_cast< QLayoutPolicy::Policy >( policy );
}

void QskLayoutItem::setGeometry( const QRectF& rect )
{
    if ( m_item == nullptr )
        return;

    if ( m_updateMode == UpdateNone )
    {
        if ( !m_isGeometryDirty )
            m_isGeometryDirty = ( rect != qskItemGeometry( m_item ) );

        return;
    }

    if ( m_updateMode == UpdateWhenVisible )
    {
        if ( !m_item->isVisible() )
            return;
    }

    m_isGeometryDirty = false;
    qskSetItemGeometry( m_item, rect );
}

bool QskLayoutItem::hasDynamicConstraint() const
{
    if ( m_item )
    {
        using namespace QskLayoutConstraint;
        return constraintType( m_item ) != Unconstrained;
    }

    return false;
}

Qt::Orientation QskLayoutItem::dynamicConstraintOrientation() const
{
    Qt::Orientation orientation = Qt::Vertical;

    if ( auto control = qskControlCast( m_item ) )
    {
        const auto policy = control->sizePolicy().horizontalPolicy();

        return ( policy == QskSizePolicy::Constrained )
            ? Qt::Horizontal : Qt::Vertical;
    }

    return orientation;
}

bool QskLayoutItem::isIgnored() const
{
    if ( m_item && !qskIsVisibleToParent( m_item ) )
        return !m_retainSizeWhenHidden;

    return false;
}

QLayoutPolicy::ControlTypes QskLayoutItem::controlTypes( LayoutSide side ) const
{
    return Inherited::controlTypes( side );
}
