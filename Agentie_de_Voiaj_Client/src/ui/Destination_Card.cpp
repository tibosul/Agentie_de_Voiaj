#include "ui/Destination_Card.h"
#include "config.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QDir>

Destination_Card::Destination_Card(QWidget* parent)
    : QWidget(parent)
    , m_main_layout(nullptr)
    , m_image_container(nullptr)
    , m_image_label(nullptr)
    , m_name_label(nullptr)
    , m_country_label(nullptr)
    , m_description_label(nullptr)
    , m_view_offers_button(nullptr)
    , m_favorite_button(nullptr)
    , m_shadow_effect(std::make_unique<QGraphicsDropShadowEffect>())
    , m_card_size(300, 350)
    , m_image_size(280, 120)
    , m_animation_enabled(true)
    , m_is_favorite(false)
    , m_is_hovered(false)
{
    setup_ui();
    setup_animations();
}

Destination_Card::Destination_Card(const Destination_Model::Destination& destination, QWidget* parent)
    : Destination_Card(parent)
{
    set_destination(destination);
}

Destination_Card::~Destination_Card() = default;

void Destination_Card::set_destination(const Destination_Model::Destination& destination)
{
    m_destination = destination;
    update_content();
    load_destination_image();
}

const Destination_Model::Destination& Destination_Card::get_destination() const
{
    return m_destination;
}

void Destination_Card::set_card_size(const QSize& size)
{
    m_card_size = size;
    setFixedSize(size);
}

void Destination_Card::set_image_size(const QSize& size)
{
    m_image_size = size;
    if (m_image_label)
    {
        m_image_label->setFixedSize(size);
    }
}

void Destination_Card::set_animation_enabled(bool enabled)
{
    m_animation_enabled = enabled;
}

void Destination_Card::setup_ui()
{
    setFixedSize(m_card_size);
    setObjectName("destinationCard");
    
    // Set up shadow effect
    m_shadow_effect->setBlurRadius(10);
    m_shadow_effect->setColor(QColor(0, 0, 0, 80));
    m_shadow_effect->setOffset(2, 2);
    setGraphicsEffect(m_shadow_effect.get());
    
    // Main layout
    m_main_layout = new QVBoxLayout(this);
    m_main_layout->setContentsMargins(15, 15, 15, 15);
    m_main_layout->setSpacing(10);
    
    // Image container
    m_image_container = new QWidget();
    m_image_container->setFixedSize(m_image_size);
    m_image_container->setStyleSheet(
        "QWidget { "
        "background-color: #f8f9fa; "
        "border-radius: 8px; "
        "border: 2px solid #e9ecef; "
        "}"
    );
    
    QHBoxLayout* imageLayout = new QHBoxLayout(m_image_container);
    imageLayout->setContentsMargins(0, 0, 0, 0);
    
    // Image label
    m_image_label = new QLabel();
    m_image_label->setAlignment(Qt::AlignCenter);
    m_image_label->setFixedSize(m_image_size);
    m_image_label->setStyleSheet(
        "QLabel { "
        "background-color: transparent; "
        "border: none; "
        "font-size: 48px; "
        "color: #6c757d; "
        "}"
    );
    m_image_label->setText(QString::fromUtf8("🏖️"));
    imageLayout->addWidget(m_image_label);
    
    // Favorite button overlay
    m_favorite_button = new QPushButton(QString::fromUtf8("🤍"));
    m_favorite_button->setParent(m_image_container);
    m_favorite_button->setFixedSize(35, 35);
    m_favorite_button->move(m_image_size.width() - 40, 5);
    m_favorite_button->setStyleSheet(
        "QPushButton { "
        "background-color: rgba(255, 255, 255, 0.9); "
        "border: 2px solid rgba(0, 0, 0, 0.1); "
        "border-radius: 17px; "
        "font-size: 16px; "
        "} "
        "QPushButton:hover { "
        "background-color: rgba(255, 255, 255, 1.0); "
        "border-color: #4a90e2; "
        "}"
    );
    
    m_main_layout->addWidget(m_image_container);
    
    // Destination name
    m_name_label = new QLabel();
    m_name_label->setAlignment(Qt::AlignCenter);
    m_name_label->setStyleSheet(
        "QLabel { "
        "font-weight: bold; "
        "font-size: 18px; "
        "color: #2c3e50; "
        "margin: 5px 0px; "
        "}"
    );
    m_name_label->setWordWrap(true);
    m_main_layout->addWidget(m_name_label);
    
    // Country
    m_country_label = new QLabel();
    m_country_label->setAlignment(Qt::AlignCenter);
    m_country_label->setStyleSheet(
        "QLabel { "
        "font-size: 14px; "
        "color: #7f8c8d; "
        "margin-bottom: 10px; "
        "}"
    );
    m_main_layout->addWidget(m_country_label);
    
    // Description
    m_description_label = new QLabel();
    m_description_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_description_label->setWordWrap(true);
    m_description_label->setStyleSheet(
        "QLabel { "
        "font-size: 12px; "
        "color: #5a6c7d; "
        "line-height: 1.4; "
        "}"
    );
    m_description_label->setMaximumHeight(60);
    m_main_layout->addWidget(m_description_label);
    
    m_main_layout->addStretch();
    
    // View offers button
    m_view_offers_button = new QPushButton("Vezi Oferte");
    m_view_offers_button->setStyleSheet(
        "QPushButton { "
        "background-color: #4a90e2; "
        "color: white; "
        "border: none; "
        "border-radius: 6px; "
        "padding: 10px; "
        "font-weight: bold; "
        "font-size: 14px; "
        "} "
        "QPushButton:hover { "
        "background-color: #357abd; "
        "} "
        "QPushButton:pressed { "
        "background-color: #2968a3; "
        "}"
    );
    m_main_layout->addWidget(m_view_offers_button);
    
    // Apply card styling
    setStyleSheet(
        "#destinationCard { "
        "background-color: white; "
        "border: 1px solid #e9ecef; "
        "border-radius: 12px; "
        "} "
        "#destinationCard:hover { "
        "border-color: #4a90e2; "
        "}"
    );
    
    // Connect signals
    connect(m_view_offers_button, &QPushButton::clicked, 
            this, &Destination_Card::on_view_offers_clicked);
    connect(m_favorite_button, &QPushButton::clicked, 
            this, &Destination_Card::on_favourite_clicked);
}

void Destination_Card::setup_animations()
{
    if (!m_animation_enabled)
    {
        return;
    }
    
    // Hover animation
    m_hover_animation = std::make_unique<QPropertyAnimation>(this, "geometry");
    m_hover_animation->setDuration(200);
    m_hover_animation->setEasingCurve(QEasingCurve::OutCubic);
    
    // Press animation
    m_press_animation = std::make_unique<QPropertyAnimation>(this, "geometry");
    m_press_animation->setDuration(150);
    m_press_animation->setEasingCurve(QEasingCurve::OutQuart);
}

void Destination_Card::update_content()
{
    if (m_name_label)
    {
        m_name_label->setText(m_destination.name);
    }
    
    if (m_country_label)
    {
        m_country_label->setText(QString::fromUtf8("📍 ") + m_destination.country);
    }
    
    if (m_description_label)
    {
        QString description = m_destination.description;
        if (description.length() > 120)
        {
            description = description.left(117) + "...";
        }
        m_description_label->setText(description);
    }
}

void Destination_Card::load_destination_image()
{
    if (!m_image_label)
    {
        return;
    }
    
    // Try to load image from path
    QString imagePath = m_destination.image_path;
    if (!imagePath.isEmpty() && QDir().exists(imagePath))
    {
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull())
        {
            pixmap = pixmap.scaled(m_image_size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            m_image_label->setPixmap(pixmap);
            m_image_label->setStyleSheet("border: none;");
            return;
        }
    }
    
    // Use emoji based on destination name or country
    QString emoji = QString::fromUtf8("🏖️"); // Default
    QString name = m_destination.name.toLower();
    QString country = m_destination.country.toLower();
    
    if (name.contains("paris") || country.contains("france"))
    {
        emoji = QString::fromUtf8("🗼");
    } else if (name.contains("rome") || country.contains("italy"))
    {
        emoji = QString::fromUtf8("🏛️");
    } else if (name.contains("barcelona") || country.contains("spain"))
    {
        emoji = QString::fromUtf8("🏖️");
    } else if (name.contains("london") || country.contains("united kingdom"))
    {
        emoji = QString::fromUtf8("🏰");
    } else if (name.contains("amsterdam") || country.contains("netherlands"))
    {
        emoji = QString::fromUtf8("🌷");
    } else if (country.contains("germany"))
    {
        emoji = QString::fromUtf8("🏰");
    } else if (country.contains("greece"))
    {
        emoji = QString::fromUtf8("🏛️");
    }
    
    m_image_label->setText(emoji);
}

void Destination_Card::enterEvent(QEnterEvent* event)
{
    QWidget::enterEvent(event);
    m_is_hovered = true;
    
    if (m_animation_enabled)
    {
        animate_hover_enter();
    }
    
    setCursor(Qt::PointingHandCursor);
}

void Destination_Card::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    m_is_hovered = false;
    
    if (m_animation_enabled)
    {
        animate_hover_leave();
    }
    
    setCursor(Qt::ArrowCursor);
}

void Destination_Card::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    
    if (event->button() == Qt::LeftButton)
    {
        if (m_animation_enabled) 
        {
            animate_press();
        }
        emit clicked();
        
        // Check if click was on image
        QRect imageRect = m_image_container->geometry();
        if (imageRect.contains(event->pos()))
        {
            emit image_clicked(m_destination.id);
        }
    }
}

void Destination_Card::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    
    QWidget::paintEvent(event);
}

void Destination_Card::on_view_offers_clicked()
{
    emit view_offers_clicked(m_destination.id);
}

void Destination_Card::on_favourite_clicked()
{
    m_is_favorite = !m_is_favorite;
    m_favorite_button->setText(m_is_favorite ? QString::fromUtf8("❤️") : QString::fromUtf8("🤍"));
    emit favourite_toggled(m_destination.id, m_is_favorite);
}

void Destination_Card::animate_hover_enter()
{
    if (!m_hover_animation)
    {
        return;
    }
    
    m_shadow_effect->setBlurRadius(20);
    m_shadow_effect->setOffset(4, 4);
    
    QRect currentGeometry = geometry();
    QRect targetGeometry = currentGeometry.adjusted(-2, -2, 2, 2);
    
    m_hover_animation->setStartValue(currentGeometry);
    m_hover_animation->setEndValue(targetGeometry);
    m_hover_animation->start();
}

void Destination_Card::animate_hover_leave()
{
    if (!m_hover_animation)
    {
        return;
    }
    
    m_shadow_effect->setBlurRadius(10);
    m_shadow_effect->setOffset(2, 2);
    
    QRect currentGeometry = geometry();
    QRect targetGeometry = currentGeometry.adjusted(2, 2, -2, -2);
    
    m_hover_animation->setStartValue(currentGeometry);
    m_hover_animation->setEndValue(targetGeometry);
    m_hover_animation->start();
}

void Destination_Card::animate_press()
{
    if (!m_press_animation)
    {
        return;
    }
    
    QRect currentGeometry = geometry();
    QRect pressedGeometry = currentGeometry.adjusted(1, 1, -1, -1);
    
    m_press_animation->setStartValue(currentGeometry);
    m_press_animation->setEndValue(pressedGeometry);
    connect(m_press_animation.get(), &QPropertyAnimation::finished, [this, currentGeometry]() {
        m_press_animation->setStartValue(geometry());
        m_press_animation->setEndValue(currentGeometry);
        m_press_animation->start();
    });
    
    m_press_animation->start();
}