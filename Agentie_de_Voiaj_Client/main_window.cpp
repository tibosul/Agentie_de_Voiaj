#include "main_window.h"
#include "login_dialog.h"
#include "config.h"
#include "utils.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>
#include <QSplitter>
#include <QApplication>
#include <QTimer>
#include <QFrame>
#include <QCloseEvent>

Main_Window::Main_Window(QWidget *parent)
    : QMainWindow(parent), network_manager(nullptr), login_dialog(nullptr)
{
    // Initialize network manager
    network_manager = new Network_Manager(this);
    
    setup_ui();
    setup_menu_bar();
    setup_tool_bar();
    setup_status_bar();
    setup_connections();
    
    // Initial UI state
    update_ui_for_user_state();
    set_connection_status(Config::StatusMessages::DISCONNECTED, false);
    
    // Show login dialog immediately on startup (don't show main window yet)
    QTimer::singleShot(100, this, &Main_Window::show_login_dialog);
}

Main_Window::~Main_Window()
{
    if (network_manager)
    {
        network_manager->disconnect_from_server();
    }
}

void Main_Window::setup_ui()
{
    setWindowTitle("Agentie de Voiaj - Client");
    setMinimumSize(1000, 700);
    resize(1200, 800);
    
    // Central widget with tabs
    main_tabs = new QTabWidget(this);
    setCentralWidget(main_tabs);
    
    setup_offers_tab();
    setup_search_tab();
    setup_reservations_tab();
    setup_profile_tab();
    
    // Set tab styles
    main_tabs->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #C0C0C0; } "
        "QTabBar::tab { background: #E0E0E0; padding: 8px 16px; margin-right: 2px; } "
        "QTabBar::tab:selected { background: #4CAF50; color: white; } "
        "QTabBar::tab:hover { background: #F0F0F0; }"
    );
}

void Main_Window::setup_offers_tab()
{
    offers_tab = new QWidget();
    auto* layout = new QVBoxLayout(offers_tab);
    
    // Header
    auto* header_layout = new QHBoxLayout();
    offers_info_label = new QLabel("Oferte disponibile");
    offers_info_label->setStyleSheet("font-size: 16px; font-weight: bold; margin: 10px;");
    header_layout->addWidget(offers_info_label);
    header_layout->addStretch();
    
    refresh_offers_button = new QPushButton("Actualizeaza");
    refresh_offers_button->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    header_layout->addWidget(refresh_offers_button);
    
    layout->addLayout(header_layout);
    
    // Offers table
    offers_table = new QTableWidget();
    offers_table->setColumnCount(8);
    QStringList headers = {"ID", "Nume", "Destinatie", "Pret/Persoana", "Durata", "Plecare", "Locuri", "Status"};
    offers_table->setHorizontalHeaderLabels(headers);
    offers_table->horizontalHeader()->setStretchLastSection(true);
    offers_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    offers_table->setAlternatingRowColors(true);
    offers_table->setSortingEnabled(true);
    offers_table->horizontalHeader()->setSortIndicatorShown(true);
    layout->addWidget(offers_table);
    
    // Action buttons
    auto* button_layout = new QHBoxLayout();
    button_layout->addStretch();
    book_selected_offer_button = new QPushButton("Rezerva Oferta Selectata");
    book_selected_offer_button->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 10px; border: none; border-radius: 5px; font-weight: bold; }");
    book_selected_offer_button->setEnabled(false);
    button_layout->addWidget(book_selected_offer_button);
    
    layout->addLayout(button_layout);
    
    main_tabs->addTab(offers_tab, "Oferte");
    
    // Connect table selection
    connect(offers_table, &QTableWidget::itemSelectionChanged, [this]() {
        book_selected_offer_button->setEnabled(offers_table->currentRow() >= 0);
    });
}

void Main_Window::setup_search_tab()
{
    search_tab = new QWidget();
    auto* layout = new QVBoxLayout(search_tab);
    
    // Search form
    auto* search_group = new QGroupBox("Criterii de cautare");
    auto* search_form = new QGridLayout(search_group);
    
    search_form->addWidget(new QLabel("Destinatie:"), 0, 0);
    destination_combo = new QComboBox();
    destination_combo->setEditable(true);
    destination_combo->addItem("Toate destinatiile", "");
    search_form->addWidget(destination_combo, 0, 1);
    
    search_form->addWidget(new QLabel("Pret minim:"), 0, 2);
    min_price_edit = new QLineEdit();
    min_price_edit->setPlaceholderText("0");
    search_form->addWidget(min_price_edit, 0, 3);
    
    search_form->addWidget(new QLabel("Pret maxim:"), 1, 0);
    max_price_edit = new QLineEdit();
    max_price_edit->setPlaceholderText("Fara limita");
    search_form->addWidget(max_price_edit, 1, 1);
    
    search_form->addWidget(new QLabel("Data plecare:"), 1, 2);
    start_date_edit = new QDateEdit();
    start_date_edit->setDate(QDate::currentDate());
    start_date_edit->setCalendarPopup(true);
    search_form->addWidget(start_date_edit, 1, 3);
    
    // Search buttons
    auto* search_buttons = new QHBoxLayout();
    search_button = new QPushButton("Cauta");
    search_button->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px 16px; border: none; border-radius: 4px; }");
    clear_search_button = new QPushButton("Reseteaza");
    search_buttons->addWidget(search_button);
    search_buttons->addWidget(clear_search_button);
    search_buttons->addStretch();
    
    search_form->addLayout(search_buttons, 2, 0, 1, 4);
    
    layout->addWidget(search_group);
    
    // Search status
    search_status_label = new QLabel();
    layout->addWidget(search_status_label);
    
    // Results table
    search_results_table = new QTableWidget();
    search_results_table->setColumnCount(8);
    QStringList headers = {"ID", "Nume", "Destinatie", "Pret/Persoana", "Durata", "Plecare", "Locuri", "Status"};
    search_results_table->setHorizontalHeaderLabels(headers);
    search_results_table->horizontalHeader()->setStretchLastSection(true);
    search_results_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    search_results_table->setAlternatingRowColors(true);
    search_results_table->setSortingEnabled(true);
    search_results_table->horizontalHeader()->setSortIndicatorShown(true);
    layout->addWidget(search_results_table);
    
    main_tabs->addTab(search_tab, "Cautare");
}

void Main_Window::setup_reservations_tab()
{
    reservations_tab = new QWidget();
    auto* layout = new QVBoxLayout(reservations_tab);
    
    // Header
    auto* header_layout = new QHBoxLayout();
    reservations_info_label = new QLabel("Rezervarile mele");
    reservations_info_label->setStyleSheet("font-size: 16px; font-weight: bold; margin: 10px;");
    header_layout->addWidget(reservations_info_label);
    header_layout->addStretch();
    
    refresh_reservations_button = new QPushButton("Actualizeaza");
    refresh_reservations_button->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    header_layout->addWidget(refresh_reservations_button);
    
    layout->addLayout(header_layout);
    
    // Reservations table
    reservations_table = new QTableWidget();
    reservations_table->setColumnCount(6);
    QStringList headers = {"ID", "Oferta", "Persoane", "Pret Total", "Data Rezervare", "Status"};
    reservations_table->setHorizontalHeaderLabels(headers);
    reservations_table->horizontalHeader()->setStretchLastSection(true);
    reservations_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    reservations_table->setAlternatingRowColors(true);
    reservations_table->setSortingEnabled(true);
    reservations_table->horizontalHeader()->setSortIndicatorShown(true);
    layout->addWidget(reservations_table);
    
    // Action buttons
    auto* button_layout = new QHBoxLayout();
    button_layout->addStretch();
    cancel_selected_reservation_button = new QPushButton("Anuleaza Rezervarea");
    cancel_selected_reservation_button->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 10px; border: none; border-radius: 5px; font-weight: bold; }");
    cancel_selected_reservation_button->setEnabled(false);
    button_layout->addWidget(cancel_selected_reservation_button);
    
    layout->addLayout(button_layout);
    
    main_tabs->addTab(reservations_tab, "Rezervarile Mele");
    
    // Connect table selection
    connect(reservations_table, &QTableWidget::itemSelectionChanged, [this]() {
        cancel_selected_reservation_button->setEnabled(reservations_table->currentRow() >= 0);
    });
}

void Main_Window::setup_profile_tab()
{
    profile_tab = new QWidget();
    auto* layout = new QVBoxLayout(profile_tab);
    layout->addStretch();
    
    // Profile form
    auto* profile_group = new QGroupBox("Informatiile mele");
    profile_group->setMaximumWidth(500);
    profile_group->setAlignment(Qt::AlignCenter);
    auto* profile_form = new QGridLayout(profile_group);
    
    profile_form->addWidget(new QLabel("Utilizator:"), 0, 0);
    profile_username_edit = new QLineEdit();
    profile_username_edit->setReadOnly(true);
    profile_form->addWidget(profile_username_edit, 0, 1);
    
    profile_form->addWidget(new QLabel("Email:"), 1, 0);
    profile_email_edit = new QLineEdit();
    profile_form->addWidget(profile_email_edit, 1, 1);
    
    profile_form->addWidget(new QLabel("Prenume:"), 2, 0);
    profile_first_name_edit = new QLineEdit();
    profile_form->addWidget(profile_first_name_edit, 2, 1);
    
    profile_form->addWidget(new QLabel("Nume:"), 3, 0);
    profile_last_name_edit = new QLineEdit();
    profile_form->addWidget(profile_last_name_edit, 3, 1);
    
    profile_form->addWidget(new QLabel("Telefon:"), 4, 0);
    profile_phone_edit = new QLineEdit();
    profile_form->addWidget(profile_phone_edit, 4, 1);
    
    // Buttons
    auto* profile_buttons = new QHBoxLayout();
    update_profile_button = new QPushButton("Actualizeaza Profil");
    update_profile_button->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px 16px; border: none; border-radius: 4px; }");
    logout_button = new QPushButton("Deconectare");
    logout_button->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 8px 16px; border: none; border-radius: 4px; }");
    
    profile_buttons->addWidget(update_profile_button);
    profile_buttons->addWidget(logout_button);
    
    profile_form->addLayout(profile_buttons, 5, 0, 1, 2);
    
    auto* center_layout = new QHBoxLayout();
    center_layout->addStretch();
    center_layout->addWidget(profile_group);
    center_layout->addStretch();
    
    layout->addLayout(center_layout);
    layout->addStretch();
    
    main_tabs->addTab(profile_tab, "Profilul Meu");
}

void Main_Window::setup_menu_bar()
{
    auto* file_menu = menuBar()->addMenu("&Fisier");
    
    connect_action = new QAction("&Conecteaza la server", this);
    connect_action->setShortcut(QKeySequence::New);
    file_menu->addAction(connect_action);
    
    disconnect_action = new QAction("&Deconecteaza de la server", this);
    disconnect_action->setEnabled(false);
    file_menu->addAction(disconnect_action);
    
    file_menu->addSeparator();
    
    login_action = new QAction("&Autentificare", this);
    login_action->setShortcut(QKeySequence("Ctrl+L"));
    file_menu->addAction(login_action);
    
    logout_action = new QAction("&Deconectare utilizator", this);
    logout_action->setEnabled(false);
    file_menu->addAction(logout_action);
    
    file_menu->addSeparator();
    
    auto* exit_action = new QAction("&Iesire", this);
    exit_action->setShortcut(QKeySequence::Quit);
    connect(exit_action, &QAction::triggered, this, &QWidget::close);
    file_menu->addAction(exit_action);
    
    auto* view_menu = menuBar()->addMenu("&Vizualizare");
    refresh_action = new QAction("&Actualizeaza", this);
    refresh_action->setShortcut(QKeySequence::Refresh);
    view_menu->addAction(refresh_action);
    
    auto* help_menu = menuBar()->addMenu("&Ajutor");
    about_action = new QAction("&Despre", this);
    connect(about_action, &QAction::triggered, [this]() {
        QMessageBox::about(this, "Despre", 
            "Agentie de Voiaj - Client v1.0\n\n"
            "Aplicatie pentru rezervarea calatoriilor\n"
            "Dezvoltat cu Qt Framework");
    });
    help_menu->addAction(about_action);
}

void Main_Window::setup_tool_bar()
{
    auto* toolbar = addToolBar("Principal");
    toolbar->addAction(connect_action);
    toolbar->addAction(disconnect_action);
    toolbar->addSeparator();
    toolbar->addAction(login_action);
    toolbar->addAction(logout_action);
    toolbar->addSeparator();
    toolbar->addAction(refresh_action);
}

void Main_Window::setup_status_bar()
{
    connection_status_label = new QLabel("Deconectat");
    connection_status_label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBar()->addWidget(connection_status_label);
    
    user_status_label = new QLabel("Neautentificat");
    user_status_label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBar()->addWidget(user_status_label);
    
    operation_progress = new QProgressBar();
    operation_progress->setVisible(false);
    operation_progress->setMaximumWidth(200);
    statusBar()->addPermanentWidget(operation_progress);
}

void Main_Window::setup_connections()
{
    // Menu actions
    connect(connect_action, &QAction::triggered, [this]() {
        network_manager->connect_to_server();
    });
    connect(disconnect_action, &QAction::triggered, [this]() {
        network_manager->disconnect_from_server();
    });
    connect(login_action, &QAction::triggered, this, &Main_Window::show_login_dialog);
    connect(logout_action, &QAction::triggered, [this]() {
        network_manager->logout_user();
    });
    connect(refresh_action, &QAction::triggered, this, &Main_Window::on_refresh_data);
    
    // Button connections
    connect(refresh_offers_button, &QPushButton::clicked, [this]() {
        network_manager->get_offers();
    });
    connect(book_selected_offer_button, &QPushButton::clicked, this, &Main_Window::on_book_offer);
    
    connect(search_button, &QPushButton::clicked, this, &Main_Window::on_search_offers);
    connect(clear_search_button, &QPushButton::clicked, [this]() {
        destination_combo->setCurrentIndex(0);
        min_price_edit->clear();
        max_price_edit->clear();
        start_date_edit->setDate(QDate::currentDate());
        search_results_table->setRowCount(0);
        search_status_label->clear();
    });
    
    connect(refresh_reservations_button, &QPushButton::clicked, [this]() {
        network_manager->get_user_reservations();
    });
    connect(cancel_selected_reservation_button, &QPushButton::clicked, this, &Main_Window::on_cancel_reservation);
    
    connect(update_profile_button, &QPushButton::clicked, this, &Main_Window::on_update_profile);
    connect(logout_button, &QPushButton::clicked, [this]() {
        network_manager->logout_user();
    });
    
    // Network manager connections
    connect(network_manager, &Network_Manager::connected_to_server, 
            this, &Main_Window::on_connected_to_server);
    connect(network_manager, &Network_Manager::disconnected_from_server, 
            this, &Main_Window::on_disconnected_from_server);
    connect(network_manager, &Network_Manager::connection_error, 
            this, &Main_Window::on_connection_error);
    connect(network_manager, &Network_Manager::operation_completed, 
            this, &Main_Window::on_operation_completed);
    
    connect(network_manager, &Network_Manager::destinations_received, 
            this, &Main_Window::on_destinations_received);
    connect(network_manager, &Network_Manager::offers_received, 
            this, &Main_Window::on_offers_received);
    connect(network_manager, &Network_Manager::reservations_received, 
            this, &Main_Window::on_reservations_received);
    connect(network_manager, &Network_Manager::user_info_received, 
            this, &Main_Window::on_user_info_received);
    
    connect(network_manager, &Network_Manager::user_logged_out,
            this, &Main_Window::on_user_logged_out);
}

void Main_Window::show_login_dialog()
{
    if (!login_dialog)
    {
        login_dialog = new Login_Dialog(network_manager, this);
        connect(login_dialog, &Login_Dialog::login_successful, 
                this, &Main_Window::on_login_successful);
    }
    
    login_dialog->exec();
}

void Main_Window::on_login_successful()
{
    // Show the main window only after successful login
    show();
    raise();
    activateWindow();
    
    update_ui_for_user_state();
    
    // Load initial data
    show_status_message(Config::StatusMessages::LOADING_DATA);
    network_manager->get_destinations();
    network_manager->get_offers();
    network_manager->get_user_info();
    
    show_status_message(Config::SuccessMessages::AUTHENTICATION_SUCCESSFUL);
}

void Main_Window::on_user_logged_out()
{
    update_ui_for_user_state();
    
    // Clear data
    offers_table->setRowCount(0);
    search_results_table->setRowCount(0);
    reservations_table->setRowCount(0);
    destination_combo->clear();
    destination_combo->addItem("Toate destinatiile", "");
    
    // Clear profile
    profile_username_edit->clear();
    profile_email_edit->clear();
    profile_first_name_edit->clear();
    profile_last_name_edit->clear();
    profile_phone_edit->clear();
    
    show_status_message(Config::SuccessMessages::LOGOUT_SUCCESSFUL);
    
    // Hide main window and show login dialog again
    hide();
    QTimer::singleShot(500, this, &Main_Window::show_login_dialog);
}

void Main_Window::on_destinations_received(const QList<Destination_Info>& destinations)
{
    current_destinations = destinations;
    populate_destinations_combo();
}

void Main_Window::on_offers_received(const QList<Offer_Info>& offers)
{
    current_offers = offers;
    populate_offers_table(offers);
    offers_info_label->setText(QString("Oferte disponibile (%1)").arg(offers.size()));
}

void Main_Window::on_reservations_received(const QList<Reservation_Info>& reservations)
{
    current_reservations = reservations;
    populate_reservations_table(reservations);
    reservations_info_label->setText(QString("Rezervarile mele (%1)").arg(reservations.size()));
}

void Main_Window::on_user_info_received(const User_Info& user)
{
    update_profile_fields(user);
}

void Main_Window::on_search_offers()
{
    QString destination = destination_combo->currentData().toString();
    double min_price = min_price_edit->text().isEmpty() ? 0.0 : min_price_edit->text().toDouble();
    double max_price = max_price_edit->text().isEmpty() ? 0.0 : max_price_edit->text().toDouble();
    QString start_date = start_date_edit->date().toString("yyyy-MM-dd");
    
    // Validate price range if provided
    if (min_price > 0 && !Utils::Validation::is_valid_price(min_price))
    {
        QMessageBox::warning(this, "Eroare validare", 
            QString("Prețul minim trebuie să fie între %1 și %2 RON")
            .arg(Config::Business::MIN_PRICE)
            .arg(Config::Business::MAX_PRICE));
        return;
    }
    
    if (max_price > 0 && !Utils::Validation::is_valid_price(max_price))
    {
        QMessageBox::warning(this, "Eroare validare", 
            QString("Prețul maxim trebuie să fie între %1 și %2 RON")
            .arg(Config::Business::MIN_PRICE)
            .arg(Config::Business::MAX_PRICE));
        return;
    }
    
    if (min_price > 0 && max_price > 0 && min_price >= max_price)
    {
        QMessageBox::warning(this, "Eroare validare", "Prețul minim trebuie să fie mai mic decât prețul maxim");
        return;
    }
    
    search_status_label->setText(Config::StatusMessages::PROCESSING);
    search_results_table->setRowCount(0);
    show_status_message(Config::StatusMessages::PROCESSING);
    
    // Connect to search results
    connect(network_manager, &Network_Manager::offers_received, this, [this](const QList<Offer_Info>& offers) {
        populate_offers_table(offers, search_results_table);
        search_status_label->setText(QString("Găsit %1 rezultate").arg(offers.size()));
        show_status_message(Config::SuccessMessages::DATA_LOADED);
    }, Qt::SingleShotConnection);
    
    network_manager->search_offers(destination, min_price, max_price, start_date, "");
}

void Main_Window::on_book_offer()
{
    int row = offers_table->currentRow();
    if (row < 0 || row >= current_offers.size())
    {
        QMessageBox::warning(this, "Atenție", "Selectați o ofertă pentru a rezerva.");
        return;
    }
    
    bool ok;
    int person_count = QInputDialog::getInt(this, "Rezervare", 
        "Numărul de persoane:", 1, 1, 10, 1, &ok);
    
    if (ok)
    {
        // Validate person count
        if (!Utils::Validation::is_valid_person_count(person_count))
        {
            QMessageBox::warning(this, "Eroare validare", 
                QString("Numărul de persoane trebuie să fie între 1 și %1")
                .arg(Config::Business::MAX_PERSONS_PER_RESERVATION));
            return;
        }
        
        show_status_message(Config::StatusMessages::BOOKING);
        const Offer_Info& offer = current_offers[row];
        network_manager->book_offer(offer.id, person_count);
        
        connect(network_manager, &Network_Manager::booking_successful, this, [this](int reservation_id) {
            QMessageBox::information(this, "Succes", 
                Config::SuccessMessages::BOOKING_SUCCESSFUL + QString("! ID rezervare: %1").arg(reservation_id));
            show_status_message(Config::SuccessMessages::BOOKING_SUCCESSFUL);
            // Refresh reservations
            network_manager->get_user_reservations();
        }, Qt::SingleShotConnection);
        
        connect(network_manager, &Network_Manager::booking_failed, this, [this](const QString& error) {
            QMessageBox::warning(this, "Eroare", Config::ErrorMessages::BOOKING_FAILED + ": " + error);
            show_status_message(Config::ErrorMessages::BOOKING_FAILED);
        }, Qt::SingleShotConnection);
    }
}

void Main_Window::on_cancel_reservation()
{
    int row = reservations_table->currentRow();
    if (row < 0 || row >= current_reservations.size())
    {
        QMessageBox::warning(this, "Atenție", "Selectați o rezervare pentru a anula.");
        return;
    }
    
    int ret = QMessageBox::question(this, "Confirmare", 
        "Sigur doriți să anulați această rezervare?", 
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes)
    {
        show_status_message(Config::StatusMessages::CANCELLING);
        const Reservation_Info& reservation = current_reservations[row];
        network_manager->cancel_reservation(reservation.id);
        
        connect(network_manager, &Network_Manager::cancellation_successful, this, [this]() {
            QMessageBox::information(this, "Succes", Config::SuccessMessages::CANCELLATION_SUCCESSFUL);
            show_status_message(Config::SuccessMessages::CANCELLATION_SUCCESSFUL);
            // Refresh reservations
            network_manager->get_user_reservations();
        }, Qt::SingleShotConnection);
        
        connect(network_manager, &Network_Manager::cancellation_failed, this, [this](const QString& error) {
            QMessageBox::warning(this, "Eroare", Config::ErrorMessages::CANCELLATION_FAILED + ": " + error);
            show_status_message(Config::ErrorMessages::CANCELLATION_FAILED);
        }, Qt::SingleShotConnection);
    }
}

void Main_Window::on_refresh_data()
{
    if (network_manager->is_user_logged_in())
    {
        show_status_message(Config::StatusMessages::LOADING_DATA);
        network_manager->get_destinations();
        network_manager->get_offers();
        network_manager->get_user_reservations();
        
        // Connect to success signals to show completion message
        connect(network_manager, &Network_Manager::offers_received, this, [this]() {
            show_status_message(Config::SuccessMessages::DATA_LOADED);
        }, Qt::SingleShotConnection);
    }
}

void Main_Window::on_update_profile()
{
    // Validate input fields before sending
    QString email = profile_email_edit->text().trimmed();
    QString first_name = profile_first_name_edit->text().trimmed();
    QString last_name = profile_last_name_edit->text().trimmed();
    QString phone = profile_phone_edit->text().trimmed();
    
    // Validate email if provided
    if (!email.isEmpty() && !Utils::Validation::is_valid_email(email))
    {
        QMessageBox::warning(this, "Eroare validare", Utils::Validation::get_validation_error("email", email));
        return;
    }
    
    // Validate first name if provided
    if (!first_name.isEmpty() && !Utils::Validation::is_valid_name(first_name))
    {
        QMessageBox::warning(this, "Eroare validare", Utils::Validation::get_validation_error("name", first_name));
        return;
    }
    
    // Validate last name if provided
    if (!last_name.isEmpty() && !Utils::Validation::is_valid_name(last_name))
    {
        QMessageBox::warning(this, "Eroare validare", Utils::Validation::get_validation_error("name", last_name));
        return;
    }
    
    // Validate phone if provided
    if (!phone.isEmpty() && !Utils::Validation::is_valid_phone(phone))
    {
        QMessageBox::warning(this, "Eroare validare", Utils::Validation::get_validation_error("phone", phone));
        return;
    }
    
    show_status_message(Config::StatusMessages::UPDATING);
    
    network_manager->update_user_info(email, first_name, last_name, phone);
    
    connect(network_manager, &Network_Manager::user_info_updated, this, [this]() {
        QMessageBox::information(this, "Succes", Config::SuccessMessages::UPDATE_SUCCESSFUL);
        show_status_message(Config::SuccessMessages::UPDATE_SUCCESSFUL);
    }, Qt::SingleShotConnection);
    
    connect(network_manager, &Network_Manager::user_info_update_failed, this, [this](const QString& error) {
        QMessageBox::warning(this, "Eroare", Config::ErrorMessages::UPDATE_FAILED + ": " + error);
        show_status_message(Config::ErrorMessages::UPDATE_FAILED);
    }, Qt::SingleShotConnection);
}

// Tab click handlers
void Main_Window::on_offers_tab_clicked()
{
    // Tab functionality is handled by QTabWidget automatically
}

void Main_Window::on_search_tab_clicked()
{
    // Tab functionality is handled by QTabWidget automatically
}

void Main_Window::on_reservations_tab_clicked()
{
    // Tab functionality is handled by QTabWidget automatically
}

void Main_Window::on_profile_tab_clicked()
{
    // Tab functionality is handled by QTabWidget automatically
}

void Main_Window::on_connected_to_server()
{
    set_connection_status(Config::StatusMessages::CONNECTED, true);
    show_status_message(Config::SuccessMessages::CONNECTION_SUCCESSFUL);
}

void Main_Window::on_disconnected_from_server()
{
    set_connection_status(Config::StatusMessages::DISCONNECTED, false);
    show_status_message(Config::SuccessMessages::LOGOUT_SUCCESSFUL);
}

void Main_Window::on_connection_error(const QString& error)
{
    set_connection_status("Eroare conexiune", false);
    show_status_message(Config::ErrorMessages::CONNECTION_FAILED + ": " + error);
}

void Main_Window::on_operation_completed(const QString& operation, bool success, const QString& message)
{
    Q_UNUSED(operation)
    if (!success && !message.isEmpty())
    {
        show_status_message(Config::ErrorMessages::SERVER_ERROR + ": " + message);
    }
}

void Main_Window::update_ui_for_user_state()
{
    bool is_logged_in = network_manager->is_user_logged_in();
    bool is_connected = network_manager->get_connection_status() == Network_Manager::Connection_Status::Connected;
    
    // Update actions
    disconnect_action->setEnabled(is_connected);
    connect_action->setEnabled(!is_connected);
    login_action->setEnabled(is_connected && !is_logged_in);
    logout_action->setEnabled(is_logged_in);
    refresh_action->setEnabled(is_logged_in);
    
    // Update tabs
    main_tabs->setTabEnabled(2, is_logged_in); // Reservations tab
    main_tabs->setTabEnabled(3, is_logged_in); // Profile tab
    
    // Update status
    if (is_logged_in)
    {
        const User_Info& user = network_manager->get_current_user();
        user_status_label->setText(Config::StatusMessages::AUTHENTICATED + ": " + user.username);
    }
    else
    {
        user_status_label->setText(Config::StatusMessages::NOT_AUTHENTICATED);
    }
}

void Main_Window::populate_destinations_combo()
{
    destination_combo->clear();
    destination_combo->addItem("Toate destinatiile", "");
    
    for (const auto& dest : current_destinations)
    {
        destination_combo->addItem(dest.name, dest.name);
    }
}

void Main_Window::populate_offers_table(const QList<Offer_Info>& offers)
{
    populate_offers_table(offers, offers_table);
}

void Main_Window::populate_offers_table(const QList<Offer_Info>& offers, QTableWidget* table)
{
    table->setRowCount(offers.size());
    
    for (int i = 0; i < offers.size(); ++i)
    {
        const Offer_Info& offer = offers[i];
        
        table->setItem(i, 0, new QTableWidgetItem(QString::number(offer.id)));
        table->setItem(i, 1, new QTableWidgetItem(offer.name));
        table->setItem(i, 2, new QTableWidgetItem(offer.destination_name));
        table->setItem(i, 3, new QTableWidgetItem(QString::number(offer.price_per_person, 'f', 2) + " RON"));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(offer.duration_days) + " zile"));
        table->setItem(i, 5, new QTableWidgetItem(offer.departure_date));
        table->setItem(i, 6, new QTableWidgetItem(QString::number(offer.available_seats)));
        table->setItem(i, 7, new QTableWidgetItem(offer.status));
    }
    
    table->resizeColumnsToContents();
}

void Main_Window::populate_reservations_table(const QList<Reservation_Info>& reservations)
{
    reservations_table->setRowCount(reservations.size());
    
    for (int i = 0; i < reservations.size(); ++i)
    {
        const Reservation_Info& reservation = reservations[i];
        
        reservations_table->setItem(i, 0, new QTableWidgetItem(QString::number(reservation.id)));
        reservations_table->setItem(i, 1, new QTableWidgetItem(reservation.offer_name));
        reservations_table->setItem(i, 2, new QTableWidgetItem(QString::number(reservation.number_of_persons)));
        reservations_table->setItem(i, 3, new QTableWidgetItem(QString::number(reservation.total_price, 'f', 2) + " RON"));
        reservations_table->setItem(i, 4, new QTableWidgetItem(reservation.reservation_date));
        reservations_table->setItem(i, 5, new QTableWidgetItem(reservation.status));
    }
    
    reservations_table->resizeColumnsToContents();
}

void Main_Window::update_profile_fields(const User_Info& user)
{
    profile_username_edit->setText(user.username);
    profile_email_edit->setText(user.email);
    profile_first_name_edit->setText(user.first_name);
    profile_last_name_edit->setText(user.last_name);
    profile_phone_edit->setText(user.phone_number);
}

void Main_Window::show_status_message(const QString& message, int timeout)
{
    statusBar()->showMessage(message, timeout);
}

void Main_Window::set_connection_status(const QString& status, bool is_connected)
{
    connection_status_label->setText(status);
    connection_status_label->setStyleSheet(is_connected ? "color: green;" : "color: red;");
    
    update_ui_for_user_state();
}

void Main_Window::closeEvent(QCloseEvent* event)
{
    // Gracefully disconnect from server if connected
    if (network_manager && network_manager->get_connection_status() == Network_Manager::Connection_Status::Connected)
    {
        network_manager->disconnect_from_server();
    }
    
    event->accept();
}

