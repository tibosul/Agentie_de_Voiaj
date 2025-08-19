#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QEnterEvent>
#include <memory>

struct Offer_Data
{
    int id = 0;
    QString name;
    QString destination;
    QString description;
    double price_per_person = 0.0;
    int duration_days = 0;
    QString departure_date;
    QString return_date;
    int available_seats = 0;
    QString image_path;
    QString included_services;
};

class Offer_Card : public QWidget
{
    Q_OBJECT

public:
    explicit Offer_Card(QWidget* parent = nullptr);
    explicit Offer_Card(const Offer_Data& offer, QWidget* parent = nullptr);
    ~Offer_Card();

    void set_offer(const Offer_Data& offer);
    const Offer_Data& get_offer() const;

    void set_card_height(int height);
    void set_compact_mode(bool compact);

signals:
    void clicked();
    void book_now_clicked(int offer_id);
    void view_details_clicked(int offer_id);
    void favorite_toggled(int offer_id, bool is_favorite);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void on_book_now_clicked();
    void on_view_details_clicked();
    void on_favorite_clicked();

private:
    void setup_ui();
    void setup_animations();
    void update_content();
    void update_price_display();
    void update_availability_display();

    void animate_hover_enter();
    void animate_hover_leave();

    QHBoxLayout* m_main_layout;
    QWidget* m_image_container;
    QLabel* m_image_label;
    QWidget* m_content_widget;
    QVBoxLayout* m_content_layout;
    QLabel* m_name_label;
    QLabel* m_destination_label;
    QLabel* m_description_label;
    QLabel* m_duration_label;
    QLabel* m_dates_label;
    QLabel* m_services_label;
    QWidget* m_price_widget;
    QLabel* m_price_label;
    QLabel* m_availability_label;
    QVBoxLayout* m_actions_layout;
    QPushButton* m_book_button;
    QPushButton* m_details_button;
    QPushButton* m_favorite_button;

    Offer_Data m_offer;

    std::unique_ptr<QGraphicsDropShadowEffect> m_shadow_effect;
    std::unique_ptr<QPropertyAnimation> m_hover_animation;

    int m_card_height;
    bool m_compact_mode;
    bool m_is_favorite;
    bool m_is_hovered;
};