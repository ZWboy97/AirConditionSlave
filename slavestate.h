#ifndef SLAVESTATE_H
#define SLAVESTATE_H

#include"QString"
#include "QDate"
//标识当前从机的所有状态
class SlaveState
{
public:
    SlaveState();

    /***********operation****************/
    void user_land_in(QString name,QString password);
    void user_land_out();
    void idea_temperature_up();
    void idea_temperature_down();
    void wind_speed_up();
    void wind_speed_down();
    void work_time_add();

    /*********getter and setter***********/
    void set_land_state(bool state);
    bool get_land_state();
    void set_user_name(QString name);
    QString get_user_name();
    void set_user_password(QString password);
    QString get_user_password();
    void set_idea_temperature(int temp);
    void set_wind_speed(int speed);
    int get_idea_temperature();
    int get_wind_speed();
    QString get_work_time();
    void set_work_time(QTime worktime);
    void set_net_work_state(int state);
    int get_net_work_state();
    void set_work_model(int model);
    int get_work_model();
    void set_on_off(bool state);
    bool get_on_off();
    void set_curr_temperature(int temp);
    int get_curr_temperature();
    void set_power(double power);
    void set_cost(double cost);
    double get_power();
    double get_cost();
    QString get_ip();
    QString get_port();
    void set_ip(QString ip);
    void set_port(QString port);
    void current_temperature_add();
    void current_temperature_sub();
    int get_room_number();

private:
    bool LandState;        //标识用户是否登录
    bool ConnectState;     //标识从机是否连接成功
    QString UserName;      //当前用户名
    QString UserPassword;  //用户密码
    int IdeaTemperature;   //目标温度
    int WindSpeed;         //风速等级，1，2，3等级
    QTime WorkTime;        //工作时长
    int NetWorkState;      //网络状况 0失败，1成功，2正在连接中
    int WorkModel;         //工作模式,0为制冷模式，1为制热模式
    bool OnOFF;            //送风，待机
    int CurrTemperature;   //当前温度，
    double Power;          //电力使用量
    double Cost;          //总价格
    QString IP;
    QString Port;


};

#endif // SLAVESTATE_H
