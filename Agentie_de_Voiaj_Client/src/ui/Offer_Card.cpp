#include "ui/Offer_Card.h"
#include "config.h"
#include "models/Offer_Data.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QFont>

Offer_Card::Offer_Card(QWidget* parent)
    : QWidget(parent)
    , m_main_layout(nullptr)
    , m_image_container(nullptr)
    , m_image_label(nullptr)
    , m_content_widget(nullptr)
    , m_content_layout(nullptr)
    , m_name_label(nullptr)
    , m_destination_label(nullptr)
    , m_description_label(nullptr)
    , m_duration_label(nullptr)
    , m_dates_label(nullptr)
    , m_services_label(nullptr)
    , m_price_widget(nullptr)
    , m_price_label(nullptr)
    , m_availability_label(nullptr)
    , m_actions_layout(nullptr)
    , m_book_button(nullptr)
    , m_details_button(nullptr)
    , m_favorite_button(nullptr)
    , m_shadow_effect(std::make_unique<QGraphicsDropShadowEffect>())
    , m_card_height(180)
    , m_compact_mode(false)
    , m_is_favorite(false)
    , m_is_hovered(false)
{
    setup_ui();
    setup_animations();
}

Offer_Card::Offer_Card(const Offer_Data& offer, QWidget* parent)
    : Offer_Card(parent)
{
    set_offer(offer);
}

Offer_Card::~Offer_Card() = default;

void Offer_Card::set_offer(const Offer_Data& offer)
{
    m_offer = offer;
    update_content();
}

const Offer_Data& Offer_Card::get_offer() const
{
    return m_offer;
}

void Offer_Card::set_card_height(int height)
{
    m_card_height = height;
    setFixedHeight(height);
}

void Offer_Card::set_compact_mode(bool compact)
{
    m_compact_mode = compact;
    if (compact)
    {
        set_card_height(120);
    }
    else
    {
        set_card_height(180);
    }
}

void Offer_Card::setup_ui()
{
    setFixedHeight(m_card_height);
    setObjectName("offerCard");
    
    // Set up shadow effect
    m_shadow_effect->setBlurRadius(8);
    m_shadow_effect->setColor(QColor(0, 0, 0, 60));
    m_shadow_effect->setOffset(2, 2);
    setGraphicsEffect(m_shadow_effect.get());
    
    // Main horizontal layout
    m_main_layout = new QHBoxLayout(this);
    m_main_layout->setContentsMargins(15, 15, 15, 15);
    m_main_layout->setSpacing(15);
    
    // Image container
    m_image_container = new QWidget();
    m_image_container->setFixedSize(140, m_card_height - 30);
    m_image_container->setStyleSheet(
        "QWidget { "
        "background-color: #f8f9fa; "
        "border-radius: 8px; "
        "border: 1px solid #e9ecef; "
        "}"
    );
    
    QVBoxLayout* imageLayout = new QVBoxLayout(m_image_container);
    imageLayout->setContentsMargins(0, 0, 0, 0);
    
    m_image_label = new QLabel();
    m_image_label->setAlignment(Qt::AlignCenter);
    m_image_label->setStyleSheet(
        "QLabel { "
        "background-color: transparent; "
        "border: none; "
        "font-size: 42px; "
        "color: #6c757d; "
        "}"
    );
    m_image_label->setText(QString::fromUtf8("🎫"));
    imageLayout->addWidget(m_image_label);
    
    // Favorite button overlay
    m_favorite_button = new QPushButton(QString::fromUtf8("🤍"));
    m_favorite_button->setParent(m_image_container);
    m_favorite_button->setFixedSize(30, 30);
    m_favorite_button->move(105, 5);
    m_favorite_button->setStyleSheet(
        "QPushButton { "
        "background-color: rgba(255, 255, 255, 0.9); "
        "border: 1px solid rgba(0, 0, 0, 0.1); "
        "border-radius: 15px; "
        "font-size: 14px; "
        "} "
        "QPushButton:hover { "
        "background-color: white; "
        "border-color: #4a90e2; "
        "}"
    );
    
    m_main_layout->addWidget(m_image_container);
    
    // Content widget
    m_content_widget = new QWidget();
    m_content_layout = new QVBoxLayout(m_content_widget);
    m_content_layout->setContentsMargins(0, 0, 0, 0);
    m_content_layout->setSpacing(8);
    
    // Offer name
    m_name_label = new QLabel();
    QFont nameFont;
    nameFont.setPointSize(16);
    nameFont.setBold(true);
    m_name_label->setFont(nameFont);
    m_name_label->setStyleSheet("color: #2c3e50;");
    m_content_layout->addWidget(m_name_label);
    
    // Destination
    m_destination_label = new QLabel();
    m_destination_label->setStyleSheet(
        "color: #7f8c8d; font-weight: bold; font-size: 14px;"
    );
    m_content_layout->addWidget(m_destination_label);
    
    // Description
    m_description_label = new QLabel();
    m_description_label->setWordWrap(true);
    m_description_label->setStyleSheet(
        "color: #5a6c7d; font-size: 13px; line-height: 1.3;"
    );
    m_description_label->setMaximumHeight(40);
    m_content_layout->addWidget(m_description_label);
    
    // Duration and dates
    QHBoxLayout* durationLayout = new QHBoxLayout();
    
    m_duration_label = new QLabel();
    m_duration_label->setStyleSheet(
        "color: #4a90e2; font-weight: bold; font-size: 12px;"
    );
    durationLayout->addWidget(m_duration_label);
    
    m_dates_label = new QLabel();
    m_dates_label->setStyleSheet(
        "color: #6c757d; font-size: 12px;"
    );
    durationLayout->addWidget(m_dates_label);
    
    durationLayout->addStretch();
    m_content_layout->addLayout(durationLayout);
    
    // Services (compact)
    m_services_label = new QLabel();
    m_services_label->setStyleSheet(
        "color: #28a745; font-size: 11px; font-style: italic;"
    );
    m_services_label->setWordWrap(true);
    m_services_label->setMaximumHeight(25);
    m_content_layout->addWidget(m_services_label);
    
    m_content_layout->addStretch();
    m_main_layout->addWidget(m_content_widget, 1);
    
    // Price widget
    m_price_widget = new QWidget();
    m_price_widget->setFixedWidth(120);
    QVBoxLayout* priceLayout = new QVBoxLayout(m_price_widget);
    priceLayout->setAlignment(Qt::AlignCenter);
    priceLayout->setSpacing(5);
    
    m_price_label = new QLabel();
    QFont priceFont;
    priceFont.setPointSize(18);
    priceFont.setBold(true);
    m_price_label->setFont(priceFont);
    m_price_label->setAlignment(Qt::AlignCenter);
    m_price_label->setStyleSheet("color: #e74c3c;");
    priceLayout->addWidget(m_price_label);
    
    QLabel* perPersonLabel = new QLabel("per persoană");
    perPersonLabel->setAlignment(Qt::AlignCenter);
    perPersonLabel->setStyleSheet(
        "color: #6c757d; font-size: 11px;"
    );
    priceLayout->addWidget(perPersonLabel);
    
    m_availability_label = new QLabel();
    m_availability_label->setAlignment(Qt::AlignCenter);
    m_availability_label->setStyleSheet(
        "color: #28a745; font-size: 11px; font-weight: bold;"
    );
    priceLayout->addWidget(m_availability_label);
    
    priceLayout->addStretch();
    m_main_layout->addWidget(m_price_widget);
    
    // Actions widget
    QWidget* actionsWidget = new QWidget();
    actionsWidget->setFixedWidth(100);
    m_actions_layout = new QVBoxLayout(actionsWidget);
    m_actions_layout->setSpacing(8);
    
    m_book_button = new QPushButton("Rezervă");
    m_book_button->setStyleSheet(
        "QPushButton { "
        "background-color: #28a745; "
        "color: white; "
        "border: none; "
        "border-radius: 6px; "
        "padding: 8px; "
        "font-weight: bold; "
        "font-size: 13px; "
        "} "
        "QPushButton:hover { "
        "background-color: #218838; "
        "} "
        "QPushButton:pressed { "
        "background-color: #1e7e34; "
        "}"
    );
    m_actions_layout->addWidget(m_book_button);
    
    m_details_button = new QPushButton("Detalii");
    m_details_button->setStyleSheet(
        "QPushButton { "
        "background-color: #6c757d; "
        "color: white; "
        "border: none; "
        "border-radius: 6px; "
        "padding: 8px; "
        "font-size: 12px; "
        "} "
        "QPushButton:hover { "
        "background-color: #5a6268; "
        "}"
    );
    m_actions_layout->addWidget(m_details_button);
    
    m_actions_layout->addStretch();
    m_main_layout->addWidget(actionsWidget);
    
    // Apply card styling
    setStyleSheet(
        "#offerCard { "
        "background-color: white; "
        "border: 1px solid #e9ecef; "
        "border-radius: 10px; "
        "} "
        "#offerCard:hover { "
        "border-color: #4a90e2; "
        "}"
    );
    
    // Connect signals
    connect(m_book_button, &QPushButton::clicked, 
            this, &Offer_Card::on_book_now_clicked);
    connect(m_details_button, &QPushButton::clicked, 
            this, &Offer_Card::on_view_details_clicked);
    connect(m_favorite_button, &QPushButton::clicked, 
            this, &Offer_Card::on_favorite_clicked);
}

void Offer_Card::setup_animations()
{
    // Hover animation
    m_hover_animation = std::make_unique<QPropertyAnimation>(this, "geometry");
    m_hover_animation->setDuration(200);
    m_hover_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void Offer_Card::update_content()
{
    if (m_name_label) 
    {
        m_name_label->setText(m_offer.name);
    }
    
    if (m_destination_label)
    {
        m_destination_label->setText(QString::fromUtf8("📍 ") + m_offer.destination);
    }
    
    if (m_description_label)
    {
        QString description = m_offer.description;
        if (description.length() > 100)
        {
            description = description.left(97) + "...";
        }
        m_description_label->setText(description);
    }
    
    if (m_duration_label)
    {
        m_duration_label->setText(QString::fromUtf8("⏱️ %1 zile").arg(m_offer.duration_days));
    }
    
    if (m_dates_label)
    {
        m_dates_label->setText(QString("%1 → %2")
                             .arg(m_offer.departure_date)
                             .arg(m_offer.return_date));
    }
    
    if (m_services_label)
    {
        QString services = m_offer.included_services;
        if (services.length() > 60)
        {
            services = services.left(57) + "...";
        }
        m_services_label->setText(QString::fromUtf8("✓ ") + services);
    }
    
    update_price_display();
    update_availability_display();
}

void Offer_Card::update_price_display()
{
    if (m_price_label)
    {
        m_price_label->setText(QString("€%1").arg(m_offer.price_per_person, 0, 'f', 0));
    }
}

void Offer_Card::update_availability_display()
{
    if (m_availability_label)
    {
        if (m_offer.available_seats > 10)
        {
            m_availability_label->setText(QString::fromUtf8("✓ Disponibil"));
            m_availability_label->setStyleSheet("color: #28a745; font-size: 11px; font-weight: bold;");
        } 
        else if (m_offer.available_seats > 0)
        {
            m_availability_label->setText(QString::fromUtf8("⚠️ %1 locuri").arg(m_offer.available_seats));
            m_availability_label->setStyleSheet("color: #ffc107; font-size: 11px; font-weight: bold;");
        }
        else
        {
            m_availability_label->setText(QString::fromUtf8("❌ Sold Out"));
            m_availability_label->setStyleSheet("color: #dc3545; font-size: 11px; font-weight: bold;");
            m_book_button->setEnabled(false);
            m_book_button->setText("Complet");
        }
    }
}

void Offer_Card::enterEvent(QEnterEvent* event)
{
    QWidget::enterEvent(event);
    m_is_hovered = true;
    
    animate_hover_enter();
    
    setCursor(Qt::PointingHandCursor);
}

void Offer_Card::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    m_is_hovered = false;
    
    animate_hover_leave();
    
    setCursor(Qt::ArrowCursor);
}

void Offer_Card::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    
    if (event->button() == Qt::LeftButton) 
    {
        emit clicked();
    }
}

void Offer_Card::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    
    QWidget::paintEvent(event);
}

void Offer_Card::on_book_now_clicked()
{
    emit book_now_clicked(m_offer.id);
}

void Offer_Card::on_view_details_clicked()
{
    emit view_details_clicked(m_offer.id);
}

void Offer_Card::on_favorite_clicked()
{
    m_is_favorite = !m_is_favorite;
    m_favorite_button->setText(m_is_favorite ? QString::fromUtf8("❤️") : QString::fromUtf8("🤍"));
    emit favorite_toggled(m_offer.id, m_is_favorite);
}

void Offer_Card::animate_hover_enter()
{
    if (!m_hover_animation)
    {
        return;
    }
    
    m_shadow_effect->setBlurRadius(15);
    m_shadow_effect->setOffset(3, 3);
}

void Offer_Card::animate_hover_leave()
{
    if (!m_hover_animation)
    {
        return;
    }
    
    m_shadow_effect->setBlurRadius(8);
    m_shadow_effect->setOffset(2, 2);
}