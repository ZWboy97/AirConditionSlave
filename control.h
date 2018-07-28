#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>
#include "QString"
#include "slavestate.h"
#include "QLabel"
#include "QLCDNumber"
#include "QPixmap"
#include "QTimer"
#include "qfile.h"
#include "qpushbutton.h"
#include "net.h"
#include "QtConcurrent/QtConcurrent"

class Control: public QObject
{
    Q_OBJECT
public:
    Control();
    Control(SlaveState slavestate);

    /*************operation*****************/
    void operation_user_land(QString name,QString password);    //验证用户登录
    void operation_user_out();
    int idea_temperature_up();
    int idea_temperature_down();
    int idea_wind_speed_up();
    int idea_wind_speed_down();
    void work_time_change(QTime time);
    void net_work_change(int net_state);
    void curr_temperature_change(int curr_temp);
    void power_change(double power);
    void cost_change(double cost);
    void work_model_change(int work_model);
    void on_off_change(bool state);
    void on_off_switch(bool state);
    bool connect_server(QString ip, QString port);



    /***************界面信息更新*********************/
    void update_screen();
    void update_work_model();
    void update_network_state();
    void update_wind_speed();
    void update_on_off();
    void update_work_time();
    void update_current_temperature();

    /********getter and setter************/
    void setmodel(SlaveState model);
    void set_timer(QTimer *timer);
    SlaveState get_model();
    bool is_master_ready();

    void set_lcd_number_total(QLCDNumber * lcd_total);
    void set_lcd_number_power(QLCDNumber * lcd_power);
    void set_lcd_number_curr_temp(QLCDNumber * lcd_curr_temp);
    void set_lcd_number_idea_temp(QLCDNumber * lcd_idea_temp);
    void set_label_running_state(QLabel * label_running_state);
    void set_label_wind_speed(QLabel * label_wind_speed);
    void set_label_work_state(QLabel * label_work_state);
    void set_label_net_state(QLabel * label_net_state);
    void set_label_work_time(QLabel * label_work_time);
    void set_label_room_name(QLabel * label_room_name);
    void set_push_button_land(QPushButton * button);
    void set_push_button_connect(QPushButton * button);

    /****定时******/
    void handleTimeout();  //超时处理函数
    void timerEvent(QTimerEvent *event);

    /*******其他处理*********/
    void save_to_disk();
    void load_from_disk();
    void get_message_from_server();
    void environment_simulate();

    /**********通信********/
    void ask_wind_supply();
    void stop_wind_supply();
    void ask_login();
    void ask_logout();
    void send_state();

    void send_temp_to_host();

public slots:
    void handle_message(QJsonObject json);
    void tcp_disconnect();
    void connect_tcp();


public: signals:
    void switch_window_signal(int);
    void send_infomation_window(QString);
    void send_result_to_status_bar(QString);
    void signal_switch_on_off_button(bool);



private:
    SlaveState Model;
    Net * myNetTool;

    QLabel * LabelRoomName;
    QLabel * LabelWorkTime;
    QLabel * LabelNetState;
    QLabel * LabelWorkState;
    QLabel * LabelWindSpeed;
    QLabel * LabelRunningState;
    QLCDNumber * LCDNumberIdeaTemp;
    QLCDNumber * LCDNumberCurrTemp;
    QLCDNumber * LCDNumberPower;
    QLCDNumber * LCDNumberTotal;
    QPushButton * PushButtonLand;
    QPushButton * PushButtonConnect;
    QTimer *timer;

    int m_nTimerID = this->startTimer(1000); //一秒一个定时
    int timer_count = 1;
    int temperature_timer = 1;
    int push_button_timer = 1;
    int request_count = 0;
    int temperature_update_frequency = 5;   //温度

    bool isSimulation = false;   //是否进行温度模拟
    bool runable = false;    //用户开关机状态，true标识用户允许送风，false标识用户阻止送风
    bool masterState = true;    //记录主机是否处于受理状态

    bool ask_signal_sended = false;
    bool stop_signal_sended = false;


};


#endif // CONTROL_H
