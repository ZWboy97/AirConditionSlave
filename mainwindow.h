#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "control.h"
#include "QString"
#include "QtConcurrent/QtConcurrent"
#include <QCloseEvent>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /*********getter and setter***********/
public: signals:
    void signal_connect(QString,int);


private slots:
    void on_pushButton_land_clicked();

    void on_pushButton_temp_up_clicked();

    void on_pushButton_temp_down_clicked();

    void on_pushButton_speed_up_clicked();

    void on_pushButton_speed_down_clicked();

    void on_pushButton_on_off_clicked();

    void timeoutslot();

    void on_action_load_triggered();

    void on_action_landout_triggered();

    void on_action_quit_triggered();

    void on_action_network_setting_triggered();

    void on_pushButton_connect_clicked();

    void on_action_control_ban_triggered();

    void on_pushButton_help_back_clicked();

    void on_action_help_triggered();

    void switch_window(int index);      //切换主界面显示
    void display_infomation_window(QString message);    //展示弹窗信息
    void display_result_in_status_bar(QString result_message);
    void switch_on_off_button(bool state);

    void on_pushButton_model_switch_clicked();


private:
    Ui::MainWindow *ui;
    QString user_account;   //当前从机用户账户
    QString user_password;  //当前用户密码
    Control control;
    QTimer *timer;
    QPixmap pixmap;
    int count;

    QThread thread;

};

#endif // MAINWINDOW_H
