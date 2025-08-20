#include "ui/Main_Window.h"
#include "ui/Login_Window.h"
#include "network/Api_Client.h"
#include "models/User_Model.h"
#include "models/Destination_Model.h"
#include "models/Offer_Model.h"
#include "models/Reservation_Model.h"
#include "config.h"

#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QGroupBox>
#include <QScrollArea>

Main_Window::Main_Window(QWidget *parent)
    : QMainWindow(parent)
    , m_central_widget(nullptr)
    , m_tab_widget(nullptr)
    , m_destinations_tab(nullptr)
    , m_offers_tab(nullptr)
    , m_reservations_tab(nullptr)
    , m_profile_tab(nullptr)
    , m_menu_bar(nullptr)
    , m_file_menu(nullptr)
    , m_view_menu(nullptr)
    , m_help_menu(nullptr)
    , m_login_action(nullptr)
    , m_logout_action(nullptr)
    , m_exit_action(nullptr)
    , m_settings_action(nullptr)
    , m_toggle_theme_action(nullptr)
    , m_about_action(nullptr)
    , m_test_connection_action(nullptr)
    , m_status_bar(nullptr)
    , m_connection_status_label(nullptr)
    , m_user_status_label(nullptr)
    , m_progress_bar(nullptr)
    , m_header_widget(nullptr)
    , m_title_label(nullptr)
    , m_user_info_label(nullptr)
    , m_user_menu_button(nullptr)
    , m_theme_toggle_button(nullptr)
    , m_user_model(std::make_unique<User_Model>(this))
    , m_destination_model(std::make_unique<Destination_Model>(this))
    , m_offer_model(std::make_unique<Offer_Model>(this))
    , m_reservation_model(std::make_unique<Reservation_Model>(this))
    , m_style_manager(std::make_unique<Style_Manager>())
    , m_offers_container(nullptr)
    , m_offers_container_layout(nullptr)
    , m_offers_loading_label(nullptr)
    , m_offers_no_offers_label(nullptr)
    , m_reservations_auth_widget(nullptr)
    , m_reservations_scroll_area(nullptr)
    , m_reservations_container(nullptr)
    , m_reservations_container_layout(nullptr)
    , m_reservations_loading_label(nullptr)
    , m_reservations_no_reservations_label(nullptr)
    , m_profile_auth_widget(nullptr)
    , m_profile_form_widget(nullptr)
    , m_profile_username_edit(nullptr)
    , m_profile_email_edit(nullptr)
    , m_profile_first_name_edit(nullptr)
    , m_profile_last_name_edit(nullptr)
    , m_profile_phone_edit(nullptr)
    , m_profile_edit_button(nullptr)
    , m_profile_save_button(nullptr)
    , m_profile_cancel_button(nullptr)
    , m_is_authenticated(false)
    , m_current_theme("light")
{
    setup_ui();
    setup_menu_bar();
    setup_status_bar();
    setup_connections();
    
    // Initialize API client connection
    Api_Client::instance().initialize_connection();
    
    // Initialize with login prompt
    show_login_prompt();
    
    // Load destinations
    m_destination_model->refresh_destinations();
    
    setWindowTitle("Agentie de Voiaj");
    setMinimumSize(800, 600);
    resize(1200, 800);
}

Main_Window::~Main_Window() = default;

void Main_Window::setup_ui()
{
    // Central widget
    m_central_widget = new QWidget();
    setCentralWidget(m_central_widget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(m_central_widget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header
    m_header_widget = new QWidget();
    m_header_widget->setFixedHeight(80);
    m_header_widget->setObjectName("headerWidget");
    m_header_widget->setStyleSheet(
        "#headerWidget { "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #4a90e2, stop:1 #357abd); "
        "border-bottom: 2px solid #2968a3; "
        "}"
    );
    
    QHBoxLayout* headerLayout = new QHBoxLayout(m_header_widget);
    headerLayout->setContentsMargins(20, 10, 20, 10);
    
    // Title
    m_title_label = new QLabel("Agentie de Voiaj");
    m_title_label->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: white;"
    );
    headerLayout->addWidget(m_title_label);
    
    headerLayout->addStretch();
    
    // User info
    m_user_info_label = new QLabel("Neconectat");
    m_user_info_label->setStyleSheet("color: white; font-size: 14px;");
    headerLayout->addWidget(m_user_info_label);
    
    // Theme toggle button
    m_theme_toggle_button = new QPushButton("🌙");
    m_theme_toggle_button->setFixedSize(40, 40);
    m_theme_toggle_button->setStyleSheet(
        "QPushButton { "
        "background-color: rgba(255,255,255,0.2); "
        "border: 2px solid rgba(255,255,255,0.3); "
        "border-radius: 20px; "
        "font-size: 16px; "
        "} "
        "QPushButton:hover { "
        "background-color: rgba(255,255,255,0.3); "
        "}"
    );
    headerLayout->addWidget(m_theme_toggle_button);
    
    // User menu button
    m_user_menu_button = new QPushButton("👤");
    m_user_menu_button->setFixedSize(40, 40);
    m_user_menu_button->setStyleSheet(
        "QPushButton { "
        "background-color: rgba(255,255,255,0.2); "
        "border: 2px solid rgba(255,255,255,0.3); "
        "border-radius: 20px; "
        "font-size: 16px; "
        "} "
        "QPushButton:hover { "
        "background-color: rgba(255,255,255,0.3); "
        "}"
    );
    headerLayout->addWidget(m_user_menu_button);
    
    mainLayout->addWidget(m_header_widget);
    
    // Tab widget
    setup_tabs();
    mainLayout->addWidget(m_tab_widget);
}

void Main_Window::setup_tabs()
{
    m_tab_widget = new QTabWidget();
    m_tab_widget->setTabPosition(QTabWidget::North);
    m_tab_widget->setMovable(false);
    
    create_destinations_tab();
    create_offers_tab();
    create_reservations_tab();
    create_profile_tab();
    
    // Set icons (emoji for now, can be replaced with actual icons)
    m_tab_widget->setTabText(0, "🏖️ Destinații");
    m_tab_widget->setTabText(1, "🎫 Oferte");
    m_tab_widget->setTabText(2, "📋 Rezervările Mele");
    m_tab_widget->setTabText(3, "👤 Profilul Meu");
    
    connect(m_tab_widget, &QTabWidget::currentChanged, 
            this, &Main_Window::on_tab_changed);
}

void Main_Window::create_destinations_tab()
{
    m_destinations_tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_destinations_tab);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);
    
    // Search section
    QHBoxLayout* searchLayout = new QHBoxLayout();
    
    QLabel* searchLabel = new QLabel("Căutare destinații:");
    searchLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    searchLayout->addWidget(searchLabel);
    
    QLineEdit* searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Introduceți numele destinației sau țării...");
    searchEdit->setMinimumHeight(35);
    searchLayout->addWidget(searchEdit);
    
    QPushButton* searchButton = new QPushButton("🔍 Caută");
    searchButton->setMinimumHeight(35);
    searchLayout->addWidget(searchButton);
    
    layout->addLayout(searchLayout);
    
    // Destinations grid (placeholder)
    QScrollArea* scrollArea = new QScrollArea();
    QWidget* scrollContent = new QWidget();
    QGridLayout* gridLayout = new QGridLayout(scrollContent);
    
    // Add some placeholder destination cards
    for (int i = 0; i < 6; ++i) {
        QWidget* card = new QWidget();
        card->setFixedSize(300, 200);
        card->setStyleSheet(
            "QWidget { "
            "background-color: white; "
            "border: 1px solid #e0e0e0; "
            "border-radius: 8px; "
            "} "
            "QWidget:hover { "
            "border-color: #4a90e2; "
            "}"
        );
        
        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        
        QLabel* imageLabel = new QLabel("📷");
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setStyleSheet("font-size: 48px; background-color: #f8f9fa; border-radius: 4px;");
        imageLabel->setFixedHeight(120);
        cardLayout->addWidget(imageLabel);
        
        QLabel* nameLabel = new QLabel(QString("Destinația %1").arg(i + 1));
        nameLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
        nameLabel->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(nameLabel);
        
        QLabel* countryLabel = new QLabel("Țara Exemplu");
        countryLabel->setAlignment(Qt::AlignCenter);
        countryLabel->setStyleSheet("color: #666;");
        cardLayout->addWidget(countryLabel);
        
        QPushButton* viewButton = new QPushButton("Vezi Oferte");
        cardLayout->addWidget(viewButton);
        
        gridLayout->addWidget(card, i / 3, i % 3);
    }
    
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);
    layout->addWidget(scrollArea);
    
    m_tab_widget->addTab(m_destinations_tab, "Destinații");
}

void Main_Window::create_offers_tab()
{
    m_offers_tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_offers_tab);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);
    
    // Header section
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    QLabel* titleLabel = new QLabel("Oferte de Călătorie");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 24px; color: #2c3e50;");
    headerLayout->addWidget(titleLabel);
    
    QPushButton* refreshButton = new QPushButton("🔄 Actualizează");
    refreshButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3498db; "
        "color: white; "
        "border: none; "
        "padding: 8px 16px; "
        "border-radius: 4px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "background-color: #2980b9; "
        "}"
    );
    connect(refreshButton, &QPushButton::clicked, [this]() {
        if (m_offer_model)
            m_offer_model->refresh_offers();
    });
    headerLayout->addWidget(refreshButton);
    
    headerLayout->addStretch();
    layout->addLayout(headerLayout);
    
    // Search and filters section
    QHBoxLayout* searchLayout = new QHBoxLayout();
    
    QLabel* searchLabel = new QLabel("Caută:");
    searchLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    searchLayout->addWidget(searchLabel);
    
    QLineEdit* searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Introduceți destinația...");
    searchEdit->setMinimumWidth(200);
    searchEdit->setStyleSheet(
        "QLineEdit { "
        "padding: 8px; "
        "border: 2px solid #bdc3c7; "
        "border-radius: 4px; "
        "font-size: 14px; "
        "} "
        "QLineEdit:focus { "
        "border-color: #3498db; "
        "}"
    );
    searchLayout->addWidget(searchEdit);
    
    QLabel* priceLabel = new QLabel("Preț:");
    priceLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-left: 20px;");
    searchLayout->addWidget(priceLabel);
    
    QLineEdit* priceMinEdit = new QLineEdit();
    priceMinEdit->setPlaceholderText("Min");
    priceMinEdit->setMaximumWidth(80);
    priceMinEdit->setStyleSheet(
        "QLineEdit { "
        "padding: 6px; "
        "border: 1px solid #bdc3c7; "
        "border-radius: 4px; "
        "}"
    );
    searchLayout->addWidget(priceMinEdit);
    
    QLabel* toLabel = new QLabel("-");
    toLabel->setStyleSheet("margin: 0 5px;");
    searchLayout->addWidget(toLabel);
    
    QLineEdit* priceMaxEdit = new QLineEdit();
    priceMaxEdit->setPlaceholderText("Max");
    priceMaxEdit->setMaximumWidth(80);
    priceMaxEdit->setStyleSheet(
        "QLineEdit { "
        "padding: 6px; "
        "border: 1px solid #bdc3c7; "
        "border-radius: 4px; "
        "}"
    );
    searchLayout->addWidget(priceMaxEdit);
    
    QPushButton* searchButton = new QPushButton("🔍 Caută");
    searchButton->setStyleSheet(
        "QPushButton { "
        "background-color: #27ae60; "
        "color: white; "
        "border: none; "
        "padding: 8px 16px; "
        "border-radius: 4px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "background-color: #229954; "
        "}"
    );
    connect(searchButton, &QPushButton::clicked, [this, searchEdit, priceMinEdit, priceMaxEdit]() {
        if (m_offer_model) {
            QString destination = searchEdit->text();
            double minPrice = priceMinEdit->text().toDouble();
            double maxPrice = priceMaxEdit->text().toDouble();
            m_offer_model->search_offers(destination, minPrice, maxPrice);
        }
    });
    searchLayout->addWidget(searchButton);
    
    searchLayout->addStretch();
    layout->addLayout(searchLayout);
    
    // Offers list
    QScrollArea* scrollArea = new QScrollArea();
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* offersLayout = new QVBoxLayout(scrollContent);
    
    // Loading indicator
    QLabel* loadingLabel = new QLabel("Se încarcă ofertele...");
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setStyleSheet("font-size: 16px; color: #7f8c8d; padding: 40px;");
    offersLayout->addWidget(loadingLabel);
    
    // No offers message
    QLabel* noOffersLabel = new QLabel("Nu s-au găsit oferte. Încercați să modificați criteriile de căutare.");
    noOffersLabel->setAlignment(Qt::AlignCenter);
    noOffersLabel->setStyleSheet("font-size: 16px; color: #7f8c8d; padding: 40px;");
    noOffersLabel->hide();
    offersLayout->addWidget(noOffersLabel);
    
    // Offers container
    QWidget* offersContainer = new QWidget();
    QVBoxLayout* offersContainerLayout = new QVBoxLayout(offersContainer);
    offersContainerLayout->setSpacing(15);
    offersLayout->addWidget(offersContainer);
    
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);
    layout->addWidget(scrollArea);
    
    // Store references for later use
    m_offers_container = offersContainer;
    m_offers_container_layout = offersContainerLayout;
    m_offers_loading_label = loadingLabel;
    m_offers_no_offers_label = noOffersLabel;
    
    m_tab_widget->addTab(m_offers_tab, "Oferte");
}

void Main_Window::create_reservations_tab()
{
    m_reservations_tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_reservations_tab);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);
    
    // Header section
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    QLabel* titleLabel = new QLabel("Rezervările Mele");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 24px; color: #2c3e50;");
    headerLayout->addWidget(titleLabel);
    
    QPushButton* refreshButton = new QPushButton("🔄 Actualizează");
    refreshButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3498db; "
        "color: white; "
        "border: none; "
        "padding: 8px 16px; "
        "border-radius: 4px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "background-color: #2980b9; "
        "}"
    );
    connect(refreshButton, &QPushButton::clicked, [this]() {
        if (m_reservation_model)
            m_reservation_model->refresh_reservations();
    });
    headerLayout->addWidget(refreshButton);
    
    headerLayout->addStretch();
    layout->addLayout(headerLayout);
    
    // Authentication required message
    QWidget* authRequiredWidget = new QWidget();
    QVBoxLayout* authLayout = new QVBoxLayout(authRequiredWidget);
    authLayout->setAlignment(Qt::AlignCenter);
    
    QLabel* authLabel = new QLabel("Pentru a vedea rezervările, vă rugăm să vă conectați.");
    authLabel->setAlignment(Qt::AlignCenter);
    authLabel->setStyleSheet("font-size: 16px; color: #7f8c8d;");
    authLayout->addWidget(authLabel);
    
    QPushButton* loginButton = new QPushButton("Conectează-te");
    loginButton->setMaximumWidth(200);
    loginButton->setStyleSheet(
        "QPushButton { "
        "background-color: #27ae60; "
        "color: white; "
        "border: none; "
        "padding: 10px 20px; "
        "border-radius: 4px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "background-color: #229954; "
        "}"
    );
    connect(loginButton, &QPushButton::clicked, this, &Main_Window::on_login_action);
    authLayout->addWidget(loginButton);
    
    layout->addWidget(authRequiredWidget);
    
    // Reservations list (hidden initially)
    QScrollArea* scrollArea = new QScrollArea();
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* reservationsLayout = new QVBoxLayout(scrollContent);
    
    // Loading indicator
    QLabel* loadingLabel = new QLabel("Se încarcă rezervările...");
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setStyleSheet("font-size: 16px; color: #7f8c8d; padding: 40px;");
    reservationsLayout->addWidget(loadingLabel);
    
    // No reservations message
    QLabel* noReservationsLabel = new QLabel("Nu aveți rezervări active. Explorați ofertele pentru a face o rezervare!");
    noReservationsLabel->setAlignment(Qt::AlignCenter);
    noReservationsLabel->setStyleSheet("font-size: 16px; color: #7f8c8d; padding: 40px;");
    noReservationsLabel->hide();
    reservationsLayout->addWidget(noReservationsLabel);
    
    // Reservations container
    QWidget* reservationsContainer = new QWidget();
    QVBoxLayout* reservationsContainerLayout = new QVBoxLayout(reservationsContainer);
    reservationsContainerLayout->setSpacing(15);
    reservationsLayout->addWidget(reservationsContainer);
    
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);
    scrollArea->hide(); // Hidden until user is authenticated
    layout->addWidget(scrollArea);
    
    // Store references for later use
    m_reservations_auth_widget = authRequiredWidget;
    m_reservations_scroll_area = scrollArea;
    m_reservations_container = reservationsContainer;
    m_reservations_container_layout = reservationsContainerLayout;
    m_reservations_loading_label = loadingLabel;
    m_reservations_no_reservations_label = noReservationsLabel;
    
    m_tab_widget->addTab(m_reservations_tab, "Rezervări");
}

void Main_Window::create_profile_tab()
{
    m_profile_tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_profile_tab);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);
    
    // Header section
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    QLabel* titleLabel = new QLabel("Profilul Meu");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 24px; color: #2c3e50;");
    headerLayout->addWidget(titleLabel);
    
    QPushButton* refreshButton = new QPushButton("🔄 Actualizează");
    refreshButton->setStyleSheet(
        "QPushButton { "
        "background-color: #3498db; "
        "color: white; "
        "border: none; "
        "padding: 8px 16px; "
        "border-radius: 4px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "background-color: #2980b9; "
        "}"
    );
    connect(refreshButton, &QPushButton::clicked, [this]() {
        if (m_user_model)
            m_user_model->refresh_user_info();
    });
    headerLayout->addWidget(refreshButton);
    
    headerLayout->addStretch();
    layout->addLayout(headerLayout);
    
    // Authentication required message
    QWidget* authRequiredWidget = new QWidget();
    QVBoxLayout* authLayout = new QVBoxLayout(authRequiredWidget);
    authLayout->setAlignment(Qt::AlignCenter);
    
    QLabel* authLabel = new QLabel("Pentru a vedea profilul, vă rugăm să vă conectați.");
    authLabel->setAlignment(Qt::AlignCenter);
    authLabel->setStyleSheet("font-size: 16px; color: #7f8c8d;");
    authLayout->addWidget(authLabel);
    
    QPushButton* loginButton = new QPushButton("Conectează-te");
    loginButton->setMaximumWidth(200);
    loginButton->setStyleSheet(
        "QPushButton { "
        "background-color: #27ae60; "
        "color: white; "
        "border: none; "
        "padding: 10px 20px; "
        "border-radius: 4px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "background-color: #229954; "
        "}"
    );
    connect(loginButton, &QPushButton::clicked, this, &Main_Window::on_login_action);
    authLayout->addWidget(loginButton);
    
    layout->addWidget(authRequiredWidget);
    
    // Profile form (hidden initially)
    QWidget* profileFormWidget = new QWidget();
    QVBoxLayout* profileFormLayout = new QVBoxLayout(profileFormWidget);
    
    QGroupBox* profileGroup = new QGroupBox("Informații Personale");
    profileGroup->setStyleSheet(
        "QGroupBox { "
        "font-weight: bold; "
        "font-size: 16px; "
        "color: #2c3e50; "
        "border: 2px solid #bdc3c7; "
        "border-radius: 8px; "
        "margin-top: 10px; "
        "padding-top: 10px; "
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 10px; "
        "padding: 0 5px 0 5px; "
        "}"
    );
    
    QGridLayout* profileLayout = new QGridLayout(profileGroup);
    profileLayout->setSpacing(15);
    profileLayout->setColumnStretch(1, 1);
    
    // Username field
    QLabel* usernameLabel = new QLabel("Nume utilizator:");
    usernameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    profileLayout->addWidget(usernameLabel, 0, 0);
    
    QLineEdit* usernameEdit = new QLineEdit();
    usernameEdit->setStyleSheet(
        "QLineEdit { "
        "padding: 8px; "
        "border: 2px solid #bdc3c7; "
        "border-radius: 4px; "
        "font-size: 14px; "
        "background-color: #ecf0f1; "
        "} "
        "QLineEdit:enabled { "
        "background-color: white; "
        "border-color: #3498db; "
        "}"
    );
    usernameEdit->setEnabled(false);
    profileLayout->addWidget(usernameEdit, 0, 1);
    
    // Email field
    QLabel* emailLabel = new QLabel("Email:");
    emailLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    profileLayout->addWidget(emailLabel, 1, 0);
    
    QLineEdit* emailEdit = new QLineEdit();
    emailEdit->setStyleSheet(
        "QLineEdit { "
        "padding: 8px; "
        "border: 2px solid #bdc3c7; "
        "border-radius: 4px; "
        "font-size: 14px; "
        "background-color: #ecf0f1; "
        "} "
        "QLineEdit:enabled { "
        "background-color: white; "
        "border-color: #3498db; "
        "}"
    );
    emailEdit->setEnabled(false);
    profileLayout->addWidget(emailEdit, 1, 1);
    
    // First name field
    QLabel* firstNameLabel = new QLabel("Prenume:");
    firstNameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    profileLayout->addWidget(firstNameLabel, 2, 0);
    
    QLineEdit* firstNameEdit = new QLineEdit();
    firstNameEdit->setStyleSheet(
        "QLineEdit { "
        "padding: 8px; "
        "border: 2px solid #bdc3c7; "
        "border-radius: 4px; "
        "font-size: 14px; "
        "background-color: #ecf0f1; "
        "} "
        "QLineEdit:enabled { "
        "background-color: white; "
        "border-color: #3498db; "
        "}"
    );
    firstNameEdit->setEnabled(false);
    profileLayout->addWidget(firstNameEdit, 2, 1);
    
    // Last name field
    QLabel* lastNameLabel = new QLabel("Nume de familie:");
    lastNameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    profileLayout->addWidget(lastNameLabel, 3, 0);
    
    QLineEdit* lastNameEdit = new QLineEdit();
    lastNameEdit->setStyleSheet(
        "QLineEdit { "
        "padding: 8px; "
        "border: 2px solid #bdc3c7; "
        "border-radius: 4px; "
        "font-size: 14px; "
        "background-color: #ecf0f1; "
        "} "
        "QLineEdit:enabled { "
        "background-color: white; "
        "border-color: #3498db; "
        "}"
    );
    lastNameEdit->setEnabled(false);
    profileLayout->addWidget(lastNameEdit, 3, 1);
    
    // Phone field
    QLabel* phoneLabel = new QLabel("Telefon:");
    phoneLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    profileLayout->addWidget(phoneLabel, 4, 0);
    
    QLineEdit* phoneEdit = new QLineEdit();
    phoneEdit->setStyleSheet(
        "QLineEdit { "
        "padding: 8px; "
        "border: 2px solid #bdc3c7; "
        "border-radius: 4px; "
        "font-size: 14px; "
        "background-color: #ecf0f1; "
        "} "
        "QLineEdit:enabled { "
        "background-color: white; "
        "border-color: #3498db; "
        "}"
    );
    phoneEdit->setEnabled(false);
    profileLayout->addWidget(phoneEdit, 4, 1);
    
    // Action buttons
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    
    QPushButton* editButton = new QPushButton("✏️ Editează Profil");
    editButton->setStyleSheet(
        "QPushButton { "
        "background-color: #f39c12; "
        "color: white; "
        "border: none; "
        "padding: 10px 20px; "
        "border-radius: 4px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "background-color: #e67e22; "
        "}"
    );
    editButton->setEnabled(false);
    buttonsLayout->addWidget(editButton);
    
    QPushButton* saveButton = new QPushButton("💾 Salvează");
    saveButton->setStyleSheet(
        "QPushButton { "
        "background-color: #27ae60; "
        "color: white; "
        "border: none; "
        "padding: 10px 20px; "
        "border-radius: 4px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "background-color: #229954; "
        "}"
    );
    saveButton->setEnabled(false);
    saveButton->hide();
    buttonsLayout->addWidget(saveButton);
    
    QPushButton* cancelButton = new QPushButton("❌ Anulează");
    cancelButton->setStyleSheet(
        "QPushButton { "
        "background-color: #e74c3c; "
        "color: white; "
        "border: none; "
        "padding: 10px 20px; "
        "border-radius: 4px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "background-color: #c0392b; "
        "}"
    );
    cancelButton->setEnabled(false);
    cancelButton->hide();
    buttonsLayout->addWidget(cancelButton);
    
    buttonsLayout->addStretch();
    profileLayout->addLayout(buttonsLayout, 5, 0, 1, 2);
    
    profileFormLayout->addWidget(profileGroup);
    profileFormLayout->addStretch();
    
    profileFormWidget->hide(); // Hidden until user is authenticated
    layout->addWidget(profileFormWidget);
    
    // Store references for later use
    m_profile_auth_widget = authRequiredWidget;
    m_profile_form_widget = profileFormWidget;
    m_profile_username_edit = usernameEdit;
    m_profile_email_edit = emailEdit;
    m_profile_first_name_edit = firstNameEdit;
    m_profile_last_name_edit = lastNameEdit;
    m_profile_phone_edit = phoneEdit;
    m_profile_edit_button = editButton;
    m_profile_save_button = saveButton;
    m_profile_cancel_button = cancelButton;
    
    m_tab_widget->addTab(m_profile_tab, "Profil");
}

void Main_Window::setup_menu_bar()
{
    m_menu_bar = menuBar();
    
    // File Menu
    m_file_menu = m_menu_bar->addMenu("&Fișier");
    
    m_login_action = new QAction("&Conectare...", this);
    m_login_action->setShortcut(QKeySequence("Ctrl+L"));
    m_file_menu->addAction(m_login_action);
    
    m_logout_action = new QAction("&Deconectare", this);
    m_logout_action->setEnabled(false);
    m_file_menu->addAction(m_logout_action);
    
    m_file_menu->addSeparator();
    
    m_exit_action = new QAction("&Ieșire", this);
    m_exit_action->setShortcut(QKeySequence::Quit);
    m_file_menu->addAction(m_exit_action);
    
    // View Menu
    m_view_menu = m_menu_bar->addMenu("&Vizualizare");
    
    m_toggle_theme_action = new QAction("Comută &Tema", this);
    m_toggle_theme_action->setShortcut(QKeySequence("Ctrl+T"));
    m_view_menu->addAction(m_toggle_theme_action);
    
    m_settings_action = new QAction("&Setări...", this);
    m_view_menu->addAction(m_settings_action);
    
    m_view_menu->addSeparator();
    
    m_test_connection_action = new QAction("Test &Conexiune", this);
    m_test_connection_action->setShortcut(QKeySequence("Ctrl+Shift+T"));
    m_view_menu->addAction(m_test_connection_action);
    
    // Help Menu
    m_help_menu = m_menu_bar->addMenu("&Ajutor");
    
    m_about_action = new QAction("&Despre...", this);
    m_help_menu->addAction(m_about_action);
}

void Main_Window::setup_status_bar()
{
    m_status_bar = statusBar();
    
    // Connection status
    m_connection_status_label = new QLabel("🔴 Deconectat");
    m_status_bar->addWidget(m_connection_status_label);
    
    m_status_bar->addPermanentWidget(new QLabel("|"));
    
    // User status
    m_user_status_label = new QLabel("Neautentificat");
    m_status_bar->addPermanentWidget(m_user_status_label);
    
    // Progress bar
    m_progress_bar = new QProgressBar();
    m_progress_bar->setVisible(false);
    m_progress_bar->setMaximumWidth(200);
    m_status_bar->addPermanentWidget(m_progress_bar);
}

void Main_Window::setup_connections()
{
    // Menu actions
    connect(m_login_action, &QAction::triggered, this, &Main_Window::on_login_action);
    connect(m_logout_action, &QAction::triggered, this, &Main_Window::on_logout_action);
    connect(m_exit_action, &QAction::triggered, this, &Main_Window::on_exit_action);
    connect(m_about_action, &QAction::triggered, this, &Main_Window::on_about_action);
    connect(m_settings_action, &QAction::triggered, this, &Main_Window::on_settings_action);
    connect(m_toggle_theme_action, &QAction::triggered, this, &Main_Window::on_toggle_theme_action);
    connect(m_test_connection_action, &QAction::triggered, this, &Main_Window::on_test_connection_action);
    
    // Header buttons
    connect(m_user_menu_button, &QPushButton::clicked, this, &Main_Window::on_login_action);
    connect(m_theme_toggle_button, &QPushButton::clicked, this, &Main_Window::on_toggle_theme_action);
    
    // User model connections
    connect(m_user_model.get(), &User_Model::authentication_status_changed,
            this, &Main_Window::on_authentication_status_changed);
    connect(m_user_model.get(), &User_Model::login_success,
            this, &Main_Window::on_user_logged_in);
    connect(m_user_model.get(), &User_Model::logged_out,
            this, &Main_Window::on_user_logged_out);
    connect(m_user_model.get(), &User_Model::user_info_updated,
            this, &Main_Window::on_user_info_updated);
    
    // Destination model connections
    connect(m_destination_model.get(), &Destination_Model::destinations_loaded,
            this, &Main_Window::on_destinations_loaded);
    
    // Offer model connections
    connect(m_offer_model.get(), &Offer_Model::offers_loaded,
            this, &Main_Window::on_offers_loaded);
    connect(m_offer_model.get(), &Offer_Model::error_occurred,
            [this](const QString& error) {
                QMessageBox::warning(this, "Eroare Oferte", error);
            });
    
    // Reservation model connections
    connect(m_reservation_model.get(), &Reservation_Model::reservations_loaded,
            this, &Main_Window::on_reservations_loaded);
    connect(m_reservation_model.get(), &Reservation_Model::error_occurred,
            [this](const QString& error) {
                QMessageBox::warning(this, "Eroare Rezervări", error);
            });
    
    // API Client connection status monitoring
    connect(&Api_Client::instance(), &Api_Client::connection_status_changed,
            [this](bool connected) {
                if (m_connection_status_label) {
                    m_connection_status_label->setText(connected ? "Conectat" : "Deconectat");
                }
                qDebug() << "Connection status changed:" << (connected ? "Connected" : "Disconnected");
            });
    
    connect(&Api_Client::instance(), &Api_Client::network_error,
            [this](const QString& error) {
                if (m_connection_status_label) {
                    m_connection_status_label->setText("Eroare conexiune");
                }
                qDebug() << "Network error:" << error;
                QMessageBox::warning(this, "Eroare de rețea", 
                    QString("Eroare de conexiune la server: %1").arg(error));
            });
}

void Main_Window::on_login_action()
{
    if (m_is_authenticated) {
        return;
    }
    
    if (Login_Window::show_login_dialog(this)) {
        // Login successful - UI will be updated via signals
    }
}

void Main_Window::on_logout_action()
{
    if (!m_is_authenticated) {
        return;
    }
    
    m_user_model->logout();
}

void Main_Window::on_exit_action()
{
    close();
}

void Main_Window::on_about_action()
{
    QMessageBox::about(this, "Despre Agentie de Voiaj",
                      QString("Agentie de Voiaj v1.0\n\n"
                              "Aplicație client pentru agenția de voiaj.\n"
                              "Dezvoltat cu Qt și C++."));
}

void Main_Window::on_settings_action()
{
    QMessageBox::information(this, "Setări", "Fereastra de setări va fi implementată în curând.");
}

void Main_Window::on_toggle_theme_action()
{
    // Toggle between light and dark theme
    m_current_theme = (m_current_theme == "light") ? "dark" : "light";
    m_theme_toggle_button->setText((m_current_theme == "light") ? "🌙" : "☀️");
    
    // Apply theme using Style_Manager
    if (m_style_manager) {
        m_style_manager->apply_theme(m_current_theme);
        qDebug() << "Applied theme:" << m_current_theme;
    } else {
        qWarning() << "Style_Manager not available";
    }
}

void Main_Window::on_test_connection_action()
{
    qDebug() << "Manual connection test requested";
    
    Api_Client& client = Api_Client::instance();
    
    if (client.is_connected()) {
        // Test with keepalive message
        client.test_connection();
        QMessageBox::information(this, "Test Conexiune", 
            "Conexiunea este activă. Mesaj de test trimis la server.");
    } else {
        // Try to reconnect
        client.initialize_connection();
        QMessageBox::information(this, "Test Conexiune", 
            "Conexiunea nu este activă. Se încearcă reconectarea...");
    }
}

void Main_Window::on_tab_changed(int index)
{
    Q_UNUSED(index)
    animate_tab_change();
}

void Main_Window::on_user_logged_in()
{
    show_welcome_message();
    update_ui_for_authentication_state();
}

void Main_Window::on_user_logged_out()
{
    show_login_prompt();
    update_ui_for_authentication_state();
}

void Main_Window::on_authentication_status_changed(bool is_authenticated)
{
    m_is_authenticated = is_authenticated;
    update_ui_for_authentication_state();
}

void Main_Window::on_destinations_loaded()
{
    m_status_bar->showMessage("Destinații încărcate cu succes", 3000);
}

void Main_Window::on_offers_loaded()
{
    m_status_bar->showMessage("Oferte încărcate cu succes", 3000);
    
    // Hide loading indicator
    if (m_offers_loading_label) {
        m_offers_loading_label->hide();
    }
    
    // Show/hide no offers message
    if (m_offers_no_offers_label) {
        bool hasOffers = m_offer_model && m_offer_model->get_offer_count() > 0;
        m_offers_no_offers_label->setVisible(!hasOffers);
    }
    
    // Refresh offers display
    refresh_offers_display();
}

void Main_Window::on_reservations_loaded()
{
    m_status_bar->showMessage("Rezervări încărcate cu succes", 3000);
    
    // Hide loading indicator
    if (m_reservations_loading_label) {
        m_reservations_loading_label->hide();
    }
    
    // Show/hide no reservations message
    if (m_reservations_no_reservations_label) {
        bool hasReservations = m_reservation_model && m_reservation_model->get_reservation_count() > 0;
        m_reservations_no_reservations_label->setVisible(!hasReservations);
    }
    
    // Refresh reservations display
    refresh_reservations_display();
}

void Main_Window::on_user_info_updated()
{
    if (m_is_authenticated) {
        m_profile_username_edit->setText(m_user_model->get_username());
        m_profile_email_edit->setText(m_user_model->get_email());
        m_profile_first_name_edit->setText(m_user_model->get_first_name());
        m_profile_last_name_edit->setText(m_user_model->get_last_name());
        m_profile_phone_edit->setText(m_user_model->get_phone());
        
        m_profile_edit_button->setEnabled(true);
        m_profile_save_button->show();
        m_profile_cancel_button->hide();
    } else {
        m_profile_username_edit->setText("Neconectat");
        m_profile_email_edit->setText("");
        m_profile_first_name_edit->setText("");
        m_profile_last_name_edit->setText("");
        m_profile_phone_edit->setText("");
        
        m_profile_edit_button->setEnabled(false);
        m_profile_save_button->hide();
        m_profile_cancel_button->hide();
    }
}

void Main_Window::update_ui_for_authentication_state()
{
    // Update menu actions
    m_login_action->setEnabled(!m_is_authenticated);
    m_logout_action->setEnabled(m_is_authenticated);
    
    // Update user info
    if (m_is_authenticated) {
        QString displayName = m_user_model->get_display_name();
        m_user_info_label->setText("Bună ziua, " + displayName);
        m_user_status_label->setText("Conectat: " + displayName);
        m_connection_status_label->setText("🟢 Conectat");
    } else {
        m_user_info_label->setText("Neconectat");
        m_user_status_label->setText("Neautentificat");
        m_connection_status_label->setText("🔴 Deconectat");
    }
    
    // Update tabs availability
    m_tab_widget->setTabEnabled(2, m_is_authenticated); // Reservations
    m_tab_widget->setTabEnabled(3, m_is_authenticated); // Profile
    
    // Update profile form visibility
    if (m_profile_auth_widget && m_profile_form_widget) {
        m_profile_auth_widget->setVisible(!m_is_authenticated);
        m_profile_form_widget->setVisible(m_is_authenticated);
    }
    
    // Update reservations visibility
    if (m_reservations_auth_widget && m_reservations_scroll_area) {
        m_reservations_auth_widget->setVisible(!m_is_authenticated);
        m_reservations_scroll_area->setVisible(m_is_authenticated);
        
        // Load reservations if authenticated
        if (m_is_authenticated && m_reservation_model) {
            m_reservation_model->refresh_reservations();
        }
    }
    
    // Load offers if not already loaded
    if (m_offer_model && m_offer_model->get_offer_count() == 0) {
        m_offer_model->refresh_offers();
    }
}

void Main_Window::show_welcome_message()
{
    QString name = m_user_model->get_display_name();
    m_status_bar->showMessage(QString("Bună ziua, %1! Conectare reușită.").arg(name), 5000);
}

void Main_Window::show_login_prompt()
{
    m_status_bar->showMessage("Pentru a accesa toate funcționalitățile, vă rugăm să vă conectați.", 5000);
}

void Main_Window::animate_tab_change()
{
    // Simple fade animation (can be enhanced)
    // Animation implementation would go here
}

void Main_Window::show_loading_indicator(bool show)
{
    m_progress_bar->setVisible(show);
    if (show) {
        m_progress_bar->setRange(0, 0); // Indeterminate progress
    }
}

void Main_Window::refresh_offers_display()
{
    if (!m_offers_container_layout || !m_offer_model) {
        return;
    }
    
    // Clear existing offers
    QLayoutItem* item;
    while ((item = m_offers_container_layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // Add new offers
    const auto& offers = m_offer_model->get_offers();
    for (const auto& offer : offers) {
        QWidget* offerCard = create_offer_card(offer);
        m_offers_container_layout->addWidget(offerCard);
    }
}

void Main_Window::refresh_reservations_display()
{
    if (!m_reservations_container_layout || !m_reservation_model) {
        return;
    }
    
    // Clear existing reservations
    QLayoutItem* item;
    while ((item = m_reservations_container_layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // Add new reservations
    const auto& reservations = m_reservation_model->get_reservations();
    for (const auto& reservation : reservations) {
        QWidget* reservationCard = create_reservation_card(reservation);
        m_reservations_container_layout->addWidget(reservationCard);
    }
}

QWidget* Main_Window::create_offer_card(const Offer_Model::Offer& offer)
{
    QWidget* offerCard = new QWidget();
    offerCard->setFixedHeight(150);
    offerCard->setStyleSheet(
        "QWidget { "
        "background-color: white; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 8px; "
        "margin: 5px; "
        "} "
        "QWidget:hover { "
        "border-color: #4a90e2; "
        "box-shadow: 0 2px 8px rgba(0,0,0,0.1); "
        "}"
    );
    
    QHBoxLayout* offerLayout = new QHBoxLayout(offerCard);
    
    // Image placeholder
    QLabel* imageLabel = new QLabel("🏖️");
    imageLabel->setFixedSize(120, 120);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("background-color: #f8f9fa; border-radius: 4px; font-size: 32px;");
    offerLayout->addWidget(imageLabel);
    
    // Offer details
    QVBoxLayout* detailsLayout = new QVBoxLayout();
    
    QLabel* titleLabel = new QLabel(offer.name);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 18px;");
    detailsLayout->addWidget(titleLabel);
    
    QLabel* descLabel = new QLabel(offer.description);
    descLabel->setWordWrap(true);
    detailsLayout->addWidget(descLabel);
    
    QLabel* priceLabel = new QLabel(QString("€%1/persoană").arg(offer.price_per_person));
    priceLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #e74c3c;");
    detailsLayout->addWidget(priceLabel);
    
    detailsLayout->addStretch();
    offerLayout->addLayout(detailsLayout);
    
    // Actions
    QVBoxLayout* actionsLayout = new QVBoxLayout();
    actionsLayout->addStretch();
    
    QPushButton* bookButton = new QPushButton("Rezervă Acum");
    bookButton->setStyleSheet("background-color: #27ae60; color: white; font-weight: bold;");
    connect(bookButton, &QPushButton::clicked, [this, offer_id = offer.id, offer_name = offer.name]() {
        // TODO: Open booking dialog
        QMessageBox::information(this, "Rezervare", 
            QString("Rezervare pentru oferta: %1").arg(offer_name));
    });
    actionsLayout->addWidget(bookButton);
    
    QPushButton* detailsButton = new QPushButton("Detalii");
    actionsLayout->addWidget(detailsButton);
    
    offerLayout->addLayout(actionsLayout);
    
    return offerCard;
}

QWidget* Main_Window::create_reservation_card(const Reservation_Model::Reservation& reservation)
{
    QWidget* reservationCard = new QWidget();
    reservationCard->setFixedHeight(120);
    reservationCard->setStyleSheet(
        "QWidget { "
        "background-color: white; "
        "border: 1px solid #e0e0e0; "
        "border-radius: 8px; "
        "margin: 5px; "
        "} "
        "QWidget:hover { "
        "border-color: #4a90e2; "
        "box-shadow: 0 2px 8px rgba(0,0,0,0.1); "
        "}"
    );
    
    QHBoxLayout* reservationLayout = new QHBoxLayout(reservationCard);
    
    // Reservation details
    QVBoxLayout* detailsLayout = new QVBoxLayout();
    
    QLabel* titleLabel = new QLabel(reservation.offer_name);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    detailsLayout->addWidget(titleLabel);
    
    QLabel* destinationLabel = new QLabel(QString("Destinație: %1").arg(reservation.destination));
    detailsLayout->addWidget(destinationLabel);
    
    QLabel* statusLabel = new QLabel(QString("Status: %1").arg(reservation.status));
    statusLabel->setStyleSheet("color: #7f8c8d;");
    detailsLayout->addWidget(statusLabel);
    
    detailsLayout->addStretch();
    reservationLayout->addLayout(detailsLayout);
    
    // Actions
    QVBoxLayout* actionsLayout = new QVBoxLayout();
    actionsLayout->addStretch();
    
    if (reservation.status == "Confirmed") {
        QPushButton* cancelButton = new QPushButton("Anulează");
        cancelButton->setStyleSheet("background-color: #e74c3c; color: white; font-weight: bold;");
        connect(cancelButton, &QPushButton::clicked, [this, reservation_id = reservation.id]() {
            m_reservation_model->cancel_reservation(reservation_id);
        });
        actionsLayout->addWidget(cancelButton);
    }
    
    reservationLayout->addLayout(actionsLayout);
    
    return reservationCard;
}

QWidget* Main_Window::create_offer_card(const int offer_index)
{
    if (!m_offer_model || offer_index < 0 || offer_index >= m_offer_model->get_offer_count()) {
        return nullptr;
    }
    
    Offer_Model::Offer offer = m_offer_model->get_offer(offer_index);
    return create_offer_card(offer);
}

QWidget* Main_Window::create_reservation_card(const int reservation_index)
{
    if (!m_reservation_model || reservation_index < 0 || reservation_index >= m_reservation_model->get_reservation_count()) {
        return nullptr;
    }
    
    Reservation_Model::Reservation reservation = m_reservation_model->get_reservation(reservation_index);
    return create_reservation_card(reservation);
}

