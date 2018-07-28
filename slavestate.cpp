#include "slavestate.h"
#include <QDebug>

SlaveState::SlaveState()
{
    LandState = false;
    UserName = "";
    UserPassword = "";
    IdeaTemperature = 26;
    WindSpeed = 3;
    WorkTime = QTime(0,0,0);
    NetWorkState = 0;
    OnOFF = false;
    WorkModel = 1;
    CurrTemperature = 20;
    Power = 0;
    Cost = 0;
}

void SlaveState::user_land_in(QString name,QString password){
    UserName = name;
    UserPassword = password;
    set_land_state(true);
    set_on_off(false);
}

void SlaveState::user_land_out(){
    UserName = "";
    UserPassword = "";
    set_land_state(false);
    set_on_off(false);
}

void SlaveState::current_temperature_add(){
    if(CurrTemperature < 45){
        CurrTemperature++;
    }
}

void SlaveState::current_temperature_sub(){
    if(CurrTemperature > -10){
        CurrTemperature--;
    }
}


void SlaveState::work_time_add(){
    WorkTime = WorkTime.addSecs(1);
}


void SlaveState::set_land_state(bool state){
    LandState = state;
}

bool SlaveState::get_land_state(){
    return LandState;
}

void SlaveState::set_user_name(QString name){
    UserName = name;
}

QString SlaveState::get_user_name(){
    return UserName;
}

void SlaveState::set_user_password(QString password){
    UserPassword = password;
}

QString SlaveState::get_user_password(){
    return UserPassword;
}

void SlaveState::set_idea_temperature(int temp){
    IdeaTemperature = temp;
}

void SlaveState::set_wind_speed(int speed){
    WindSpeed = speed;
}

int SlaveState::get_idea_temperature(){
    return IdeaTemperature;
}

int SlaveState::get_wind_speed(){
    return WindSpeed;
}

void SlaveState::idea_temperature_up(){
    if(IdeaTemperature < 30){
        IdeaTemperature++;
    }
}

void SlaveState::idea_temperature_down(){
    if(IdeaTemperature > 16){
        IdeaTemperature--;
    }
}

void SlaveState::wind_speed_up(){
    if(WindSpeed<3)
        WindSpeed++;
}

void SlaveState::wind_speed_down(){
    if(WindSpeed>1){
        WindSpeed--;
    }
}

QString SlaveState::get_work_time(){
    return QString(WorkTime.toString("hh:mm:ss"));
}

void SlaveState::set_work_time(QTime worktime){
    WorkTime = worktime;
}

void SlaveState::set_net_work_state(int state){
    NetWorkState = state;
}

int SlaveState::get_net_work_state(){
    return NetWorkState;
}

void SlaveState::set_work_model(int model){
    WorkModel = model;
}

int SlaveState::get_work_model(){
    return WorkModel;
}

void SlaveState::set_on_off(bool state){
    OnOFF = state;
}

bool SlaveState::get_on_off(){
    return OnOFF;
}

void SlaveState::set_curr_temperature(int temp){
    CurrTemperature = temp;
}

int SlaveState::get_curr_temperature(){
    return CurrTemperature;
}

void SlaveState::set_power(double power){
    Power = power;
}

void SlaveState::set_cost(double cost){
    Cost = cost;
}

double SlaveState::get_power(){
    return Power;
}

double SlaveState::get_cost(){
    return Cost;
}

QString SlaveState::get_ip(){
    return IP;
}

QString SlaveState::get_port(){
    return Port;
}

void SlaveState::set_ip(QString ip){
    IP = ip;
}
void SlaveState::set_port(QString port){
    Port = port;
}

int SlaveState::get_room_number(){
    bool ok = true;
    int dec=UserName.toInt(&ok,10); //dec=255 ; ok=true    //QString 转 int
    return dec;
}
